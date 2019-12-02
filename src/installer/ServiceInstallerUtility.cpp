// Copyright (c) 2013-2018 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#include <boost/algorithm/string/predicate.hpp>
#include <iostream>

#include "AppInfo.h"
#include "base/CallChain.h"
#include "base/JUtil.h"
#include "base/Logging.h"
#include "base/Utils.h"
#include "base/System.h"
#include "installer/CallChainEventHandler.h"
#include "PackageInfo.h"
#include "ServiceInfo.h"
#include "ServiceInstallerUtility.h"
#include "settings/Settings.h"
#include "Manifest.h"

using namespace std::placeholders;

static void roleGenerate(std::string templatePath,
                         std::string destinationPath,
                         std::string id,
                         std::string executablePath)
{
    std::string line;
    size_t n;
    static const std::string ID_TAG = "XXXIDXXX";
    static const std::string PATH_TAG = "XXXEXEPATHXXX";

    std::ifstream myfile(templatePath.c_str());
    std::ofstream newfile(destinationPath.c_str());

    if (!myfile.is_open() || !newfile.is_open()) {
        return;
    }

    while (!myfile.eof()) {
        std::getline(myfile, line);
        while ((n = line.find(ID_TAG)) != std::string::npos)
            line.replace(n, ID_TAG.length(), id);
        while ((n = line.find(PATH_TAG)) != std::string::npos)
            line.replace(n, PATH_TAG.length(), executablePath);
        newfile << line << std::endl;
    }
    newfile.close();
    myfile.close();
}

bool ServiceInstallerUtility::install(std::string appId,
                                      std::string installBasePath,
                                      const PathInfo &pathInfo,
                                      std::function<void(bool, std::string)> onComplete,
                                      bool isUpdate)
{
    std::string applicationPath = installBasePath + Settings::instance().getApplicationInstallPath() + "/" + appId;
    std::string packagePath = installBasePath + Settings::instance().getPackageinstallPath() + "/" + appId;

    AppInfo appInfo(applicationPath);
    if (!appInfo.isLoaded()) {
        Utils::async([=] {onComplete(false, "Cannot find appinfo.json");});
        return false;
    }

    std::vector<std::string> serviceLists;

    PackageInfo packageInfo(packagePath);
    if (packageInfo.isLoaded())
        packageInfo.getServices(serviceLists);

    // generate Manifest file
    if (!generateManifestFile(pathInfo, installBasePath, packageInfo, appInfo)) {
        Utils::async([=] {onComplete(false, "Failed to generate Manifest file");});
        return false;
    }

    // generate service files
    for (auto iter = serviceLists.begin(); iter != serviceLists.end(); ++iter) {
        std::string servicePath = installBasePath + Settings::instance().getServiceinstallPath() + "/" + (*iter);
        ServiceInfo serviceInfo(servicePath);
        serviceInfo.applyRootPath(pathInfo.root);

        if (serviceInfo.getType() == "native") {
            if (!pathInfo.verified)
                serviceInfo.applyJailer(ServiceInfo::JAILER_DEV);
        }

        if (!boost::starts_with(serviceInfo.getId(), appId + std::string("."))) {
            LOG_WARNING(MSGID_WRONG_SERVICEID, 2,
                        PMLOGKS("SERVICE_ID", serviceInfo.getId().c_str()),
                        PMLOGKS("APP_ID", appId.c_str()),
                        "Service id should start with app id");
            Utils::async([=] {onComplete(false, "Service id should start with app id");});
            return false;
        }

        if (!generateFilesForService(pathInfo, serviceInfo, appInfo)) {
            Utils::async([=] {onComplete(false, "Failed to generate service files");});
            return false;
        }
    }

    // native app has default role file
    if (appInfo.isNative()) {
        if (!generateRoleFileForNativeApp(pathInfo.roled, appInfo.getId(), appInfo.getMain(true)) ||
            !generatePermissionFileForNativeApp(pathInfo.permissiond, pathInfo.verified, appInfo, serviceLists)) {
            Utils::async([=] {onComplete(false, "Failed to generate role and permission file for Native application");});
            return false;
        }
    } else if (appInfo.isWeb() || appInfo.isQml()) {
        // Web/Qml applications should have role file too
        if (!generateRoleFileForWebApp(pathInfo.roled, appInfo.getId()) ||
            !generatePermissionFileForWebApp(pathInfo.permissiond, pathInfo.verified, appInfo, serviceLists)) {
            Utils::async([=] {onComplete(false, "Failed to generate role and permission file for Web application");});
            return false;
        }
    }

    // tell the hub there's a new service in town
    CallChain& callchain = CallChain::acquire(std::bind(&ServiceInstallerUtility::onUpdateManifest,
                                                        this, _1, _2, onComplete));

    if (isUpdate) {
        auto removeItemAppInfo =
            std::make_shared<CallChainEventHandler::UpdateManifest>("com.webos.appInstallService",
                                                                    false,
                                                                    adjustLunaDirForManifest(pathInfo.root, pathInfo.manifestsd),
                                                                    pathInfo.root,
                                                                    appId);
        callchain.add(removeItemAppInfo);
    }

    auto addItemAppInfo = std::make_shared<CallChainEventHandler::UpdateManifest>("com.webos.appInstallService",
                                                                                  true,
                                                                                  adjustLunaDirForManifest(pathInfo.root, pathInfo.manifestsd),
                                                                                  pathInfo.root,
                                                                                  appId);
    callchain.add(addItemAppInfo);

    if (!callchain.run())
        return false;

    return true;
}

bool ServiceInstallerUtility::onUpdateManifest(pbnjson::JValue result,
                                               void *user_data,
                                               std::function<void(bool, std::string)> onComplete)
{
    bool returnValue = result["returnValue"].asBool();

    if (!returnValue) {
        std::string errorText = result["errorText"].asString();
        onComplete(false, errorText);
        return false;
    }
    onComplete(true, "success");

    return true;
}

bool ServiceInstallerUtility::onRemoveManifest(pbnjson::JValue result,
                                               void *user_data,
                                               const std::string &appId,
                                               const PathInfos &pathInfos,
                                               std::function<void(bool, std::string)> onComplete)
{
    for (const auto &pathInfo : pathInfos)
        removeOne(appId, pathInfo);

    // always true
    onComplete(true, "success");

    return true;
}

bool ServiceInstallerUtility::remove(std::string appId,
                                     const PathInfos &pathInfos,
                                     std::function<void(bool, std::string)> onComplete)
{
    CallChain& callchain = CallChain::acquire(std::bind(&ServiceInstallerUtility::onRemoveManifest,
                                                        this, _1, _2, appId, pathInfos, onComplete));

    for (auto pathInfo : pathInfos) {
        auto itemAppInfo =
            std::make_shared<CallChainEventHandler::UpdateManifest>("com.webos.appInstallService", false,
                                                                    adjustLunaDirForManifest(pathInfo.root, pathInfo.manifestsd),
                                                                    pathInfo.root,
                                                                    appId);
        callchain.add(itemAppInfo);
    }

    // tell the hub there's a new service in town
    if (!callchain.run())
        return false;

    return true;
}

bool ServiceInstallerUtility::removeOne(const std::string &appId, const PathInfo &pathInfo)
{
    std::string manifestPath = pathInfo.manifestsd + "/" + appId + ".json";

    LOG_DEBUG("[ServiceInstallerUtility] remove luna manifest file : %s", manifestPath.c_str());

    Manifest manifest(manifestPath);
    if (!manifest.isLoaded())
        return false;

    std::vector<std::string> paths;
    manifest.getPaths(paths);

    // Remove files
    std::string path;
    for (auto it = paths.begin(); it != paths.end(); ++it) {
        if (!((*it).empty()) && (*it)[0] == '/')
            path = *it;
        else
            path = pathInfo.root + "/" + *it;
        LOG_DEBUG("[ServiceInstallerUtility] remove luna service file : %s", path.c_str());

        Utils::remove_file(path);
    }

    // Remove manifest file
    Utils::remove_file(manifestPath);

    return true;
}

bool ServiceInstallerUtility::generateFilesForService(const PathInfo &pathInfo,
                                                      const ServiceInfo &servicesInfo,
                                                      const AppInfo &appInfo)
{
    return (generateRoleFileForService(pathInfo.roled, servicesInfo, appInfo) &&
            generatePermissionFileForService(pathInfo.permissiond, pathInfo.verified, servicesInfo, appInfo) &&
            generateAPIPermissionsFileForService(pathInfo.api_permissiond, pathInfo.verified, servicesInfo, appInfo) &&
            generateServiceFile(pathInfo.serviced, servicesInfo, appInfo));
}

bool ServiceInstallerUtility::generateRoleFile(std::string path, bool isPublic, const ServiceInfo &servicesInfo)
{
    if (!Utils::make_dir(path))
        return false;

    std::string templatePath;

    if (servicesInfo.getType() == "ndk")
        templatePath = Settings::instance().getRoleTemplatePathNDK();
    else
        return false; // invalid type!! Use generateUnifiedAppRoleFile instead

    if (isPublic)
        templatePath += ".pub";
    else
        templatePath += ".prv";

    std::string filename = path + "/" + servicesInfo.getId() + ".json";

    // remove exist file
    Utils::remove_file(filename);

    // generate new one
    LOG_DEBUG("[ServiceInstallerUtility] generateRoleFile : filename - %s", filename.c_str());
    roleGenerate(templatePath, filename, servicesInfo.getId(), servicesInfo.getExec(true));

    return true;
}

bool ServiceInstallerUtility::generateUnifiedAppRoleFile(const std::string &path,
                                                         const std::string &templatePath,
                                                         const std::string &fileName,
                                                         const std::string &id,
                                                         const std::string &exec)
{
    if (!Utils::make_dir(path))
        return false;

    // Prepare role file for "id" with
    //  "appId": id
    //  "allowedNames": id + mask according to template
    std::string fullName = path + std::string("/") + fileName;

    // remove exist file
    Utils::remove_file(fullName);

    // generate new one
    LOG_DEBUG("[ServiceInstallerUtility] generateUnifiedAppRoleFile : filename - %s", fullName.c_str());
    roleGenerate(templatePath, fullName, id, exec);

    return true;
}

bool ServiceInstallerUtility::generateRoleFileForWebApp(const std::string &path,
                                                        const std::string &appId)
{
    return generateUnifiedAppRoleFile(path,
                                      Settings::instance().getRoleTemplatePathWebApp(),
                                      Settings::instance().getLunaUnifiedAppJsonFileName(appId),
                                      appId,
                                      "");
}

bool ServiceInstallerUtility::generateRoleFileForNativeApp(const std::string& path,
                                                           const std::string& appId,
                                                           const std::string& exec)
{
    return generateUnifiedAppRoleFile(path,
                                      Settings::instance().getRoleTemplatePathNativeService(),
                                      Settings::instance().getLunaUnifiedAppJsonFileName(appId),
                                      appId,
                                      exec);
}

bool ServiceInstallerUtility::generateRoleFileForService(const std::string &path,
                                                         const ServiceInfo &servicesInfo,
                                                         const AppInfo &appInfo)
{
    std::string templatePath;
    if (servicesInfo.getType() == "native")
        templatePath = Settings::instance().getRoleTemplatePathNativeService();
    else
        templatePath = Settings::instance().getRoleTemplatePathJSService();

    return generateUnifiedAppRoleFile(path,
                                      templatePath,
                                      Settings::instance().getLunaUnifiedServiceJsonFileName(servicesInfo.getId()),
                                      servicesInfo.getId(),
                                      servicesInfo.getExec(true));
}

bool ServiceInstallerUtility::generateUnifiedAppPermissionsFile(const std::string &path,
                                                                bool verified,
                                                                const std::string &fileName,
                                                                const AppInfo &appInfo,
                                                                const std::string &id,
                                                                const std::string &allowedMask,
                                                                const std::vector<std::string> &requiredServices)
{
    if (!Utils::make_dir(path))
        return false;

    // Prepare permissions file fileName for service name "id" with "allowedMask"
    std::ofstream ofs { path + ("/" + fileName) };
    if (!ofs)
        return false;

    using namespace pbnjson;

    JValue groups = Array();

    JValue requires = appInfo.getRequiredPermissions();
    static const JSchemaFragment requires_schema { R"(
        {
            "type": "array",
            "items": {"type": "string"}
        }
    )" };
    if (JValidator { }.isValid(requires, requires_schema, nullptr)) {
        groups = requires;
    } else {
        LOG_WARNING(MSGID_WRONG_SERVICEID, 1,
                    PMLOGKS("APP_ID", id.c_str()),
                    "#/requiredPermissions is missing in appinfo.json, set as public");
        groups << "public";
    }

    for (const auto &service : requiredServices)
        groups << Settings::instance().getGroupNameForService(service);

    // Finally, dump the permissions object into the file.
    JValue perm = Object() << JValue::KeyValue(id + allowedMask, groups);
    ofs << JUtil::toSimpleString(perm) << std::endl;
    ofs.close();

    return true;
}

bool ServiceInstallerUtility::generatePermissionFileForWebApp(const std::string &path,
                                                              bool verified,
                                                              const AppInfo &appInfo,
                                                              const std::vector<std::string> &requiredServices)
{
    return generateUnifiedAppPermissionsFile(path,
                                             verified,
                                             Settings::instance().getLunaUnifiedAppJsonFileName(appInfo.getId()),
                                             appInfo,
                                             appInfo.getId(),
                                             "-*",
                                             requiredServices);
}

bool ServiceInstallerUtility::generatePermissionFileForNativeApp(const std::string &path,
                                                                 bool verified,
                                                                 const AppInfo &appInfo,
                                                                 const std::vector<std::string> &requiredServices)
{
    return generateUnifiedAppPermissionsFile(path,
                                             verified,
                                             Settings::instance().getLunaUnifiedAppJsonFileName(appInfo.getId()),
                                             appInfo,
                                             appInfo.getId(),
                                             "",
                                             requiredServices);
}

bool ServiceInstallerUtility::generatePermissionFileForService(const std::string &path,
                                                               bool verified,
                                                               const ServiceInfo &servicesInfo,
                                                               const AppInfo &appInfo)
{
    return generateUnifiedAppPermissionsFile(path,
                                             verified,
                                             Settings::instance().getLunaUnifiedServiceJsonFileName(servicesInfo.getId()),
                                             appInfo,
                                             servicesInfo.getId(),
                                             "*");
}

bool ServiceInstallerUtility::generateAPIPermissionsFileForService(const std::string &path,
                                                                   bool verified,
                                                                   const ServiceInfo &servicesInfo,
                                                                   const AppInfo &appInfo)
{
    if (!Utils::make_dir(path))
        return false;

    // Prepare API permissions file for service
    std::ofstream ofs { path + ("/" + Settings::instance().getLunaUnifiedAPIJsonFileName(servicesInfo.getId())) };
    if (!ofs)
        return false;

    using namespace pbnjson;

    // Array of provided methods masks - include all methods for target service
    JValue methods = Array();
    methods << (servicesInfo.getId() + "/*");

    // Dump the API permissions object into the file
    JValue perms = Object() << JValue::KeyValue(Settings::instance().getGroupNameForService(servicesInfo.getId()), methods);
    if (!verified)
        perms = perms << JValue::KeyValue("ares.webos.cli", methods);
    ofs << JUtil::toSimpleString(perms) << std::endl;
    ofs.close();

    return true;
}

bool ServiceInstallerUtility::generateServiceFile(std::string path,
                                                  const ServiceInfo &servicesInfo,
                                                  const AppInfo &appInfo)
{
    if (!Utils::make_dir(path))
        return false;

    std::string filename = path + "/" + servicesInfo.getId() + ".service";

    std::ofstream outputFile(filename.c_str());
    if (!outputFile.is_open())
        return false;

    LOG_DEBUG("Generated service file: service - %s ", servicesInfo.getId().c_str());

    std::string exec;
    if (servicesInfo.getType() == "native") {
        if (Settings::instance().isJailMode()) {
            exec = Settings::instance().getJailerPath();
            exec += " -t " + servicesInfo.getJailerType();
            exec += " -i " + appInfo.getId();
            exec += " -p " + servicesInfo.getPath(false); //relative path
            exec += " " + servicesInfo.getExec(true); //full path by adding relative path
        } else {
            exec = servicesInfo.getExec(true);
        }
    } else {
        exec = Settings::instance().getJsservicePath() + " -n " + servicesInfo.getPath(false);
    }

    outputFile << "[D-BUS Service]" << std::endl;
    outputFile << "Name=" << servicesInfo.getId() << std::endl;
    outputFile << "Exec=" << exec << std::endl;
    outputFile << "Type=dynamic" << std::endl;
    outputFile.close();

    return true;
}

bool ServiceInstallerUtility::generateManifestFile(const PathInfo &pathInfo,
                                                   const std::string& installBasePath,
                                                   PackageInfo &packageInfo,
                                                   const AppInfo &appInfo)
{
    pbnjson::JValue manifestObj = pbnjson::Object();
    pbnjson::JValue rolePrvFileArray = pbnjson::Array();
    pbnjson::JValue rolePubFileArray = pbnjson::Array();
    pbnjson::JValue roledFileArray = pbnjson::Array();
    pbnjson::JValue serviceFileArray = pbnjson::Array();
    pbnjson::JValue permissiondFileArray = pbnjson::Array();
    pbnjson::JValue api_permissiondFileArray = pbnjson::Array();
    std::string fileDir = "";
    std::string filePath = "";

    std::vector<std::string> serviceLists;
    if (packageInfo.isLoaded())
        packageInfo.getServices(serviceLists);

    for (auto iter = serviceLists.begin(); iter != serviceLists.end(); ++iter) {
        std::string servicePath = installBasePath + Settings::instance().getServiceinstallPath() + "/" + (*iter);
        ServiceInfo serviceInfo(servicePath);

        //generateRoleFileForService
        filePath = adjustLunaDirForManifest(pathInfo.root, pathInfo.roled);
        fileDir = filePath + "/" + Settings::instance().getLunaUnifiedServiceJsonFileName(serviceInfo.getId());
        roledFileArray.append(fileDir);

        //generatePermissionFileForService
        filePath = adjustLunaDirForManifest(pathInfo.root, pathInfo.permissiond);
        fileDir = filePath + "/" + Settings::instance().getLunaUnifiedServiceJsonFileName(serviceInfo.getId());
        permissiondFileArray.append(fileDir);

        //generateAPIPermissionsFileForService
        filePath = adjustLunaDirForManifest(pathInfo.root, pathInfo.api_permissiond);
        fileDir = filePath + "/" + Settings::instance().getLunaUnifiedAPIJsonFileName(serviceInfo.getId());
        api_permissiondFileArray.append(fileDir);

        //service files for service
        filePath = adjustLunaDirForManifest(pathInfo.root, pathInfo.serviced);
        fileDir = filePath + "/" + serviceInfo.getId() + ".service";
        serviceFileArray.append(fileDir);
    }

    //generateRoleFileForApp
    filePath = adjustLunaDirForManifest(pathInfo.root, pathInfo.roled);
    fileDir = filePath + "/" + Settings::instance().getLunaUnifiedAppJsonFileName(appInfo.getId());
    roledFileArray.append(fileDir);

    //generatePermissionFileForApp
    filePath = adjustLunaDirForManifest(pathInfo.root, pathInfo.permissiond);
    fileDir = filePath + "/" + Settings::instance().getLunaUnifiedAppJsonFileName(appInfo.getId());
    permissiondFileArray.append(fileDir);

    //TODO : There is no case only service in package.
    //set id & version from appinfo.json or packageinfo.json
    std::string id = serviceLists.empty() ? appInfo.getId() : packageInfo.getId();
    manifestObj.put("id", id);
    manifestObj.put("version", serviceLists.empty() ? appInfo.getVersion() : packageInfo.getVersion());
    //Don't put empty configuration value
    if (0 < roledFileArray.arraySize())
        manifestObj.put("roleFiles", roledFileArray);
    if (0 < rolePubFileArray.arraySize())
        manifestObj.put("roleFilesPub", rolePubFileArray);
    if (0 < rolePrvFileArray.arraySize())
        manifestObj.put("roleFilesPrv", rolePrvFileArray);
    if (0 < serviceFileArray.arraySize())
        manifestObj.put("serviceFiles", serviceFileArray);
    if (0 < api_permissiondFileArray.arraySize())
        manifestObj.put("apiPermissionFiles", api_permissiondFileArray);
    if (0 < permissiondFileArray.arraySize())
        manifestObj.put("clientPermissionFiles", permissiondFileArray);

    if (!Utils::make_dir(pathInfo.manifestsd))
        return false;

    std::string manifestfullPath = pathInfo.manifestsd + "/" + id + ".json";
    LOG_DEBUG("[ServiceInstallerUtility] generateManifestFile : manifestfullPath - %s", manifestfullPath.c_str());

    std::ofstream outputFile(manifestfullPath); //file name should be the same as "id" in manifest file
    if (!outputFile.is_open())
        return false;

    outputFile << JUtil::toSimpleString(manifestObj) << std::endl;
    outputFile.close();

    return true;
}

std::string ServiceInstallerUtility::adjustLunaDirForManifest(const std::string &rootPath,
                                                              const std::string &relativePath)
{
    if (rootPath.empty()) {
        return relativePath;
    } else {
        std::size_t pos = relativePath.find(rootPath);
        if (pos == std::string::npos) {
            return relativePath;
        }
        std::string resultPath = relativePath;
        resultPath.erase(0, rootPath.length() + 1);
        return resultPath;
    }
}
