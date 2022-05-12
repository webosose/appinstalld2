// Copyright (c) 2013-2021  LG Electronics, Inc.
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

#ifndef SERVICEINSTALLERUTILITY_H
#define SERVICEINSTALLERUTILITY_H

#include <functional>
#include <pbnjson.hpp>
#include <string>
#include <vector>

class AppInfo;
class ServiceInfo;
class PackageInfo;

//! utility class for service installation
class ServiceInstallerUtility {
public:
    typedef struct {
        bool verified;               // indicates belows paths are verified or not
        std::string root;            // unified root path for an app on USB.
        std::string legacyRoles;           // legacy role file path. should indicates above pub/prv
        std::string legacyServices;        // legacy service file path. should indicates above pub/prv
        std::string roled;           // unified role path
        std::string serviced;        // unified service path
        std::string permissiond;     // unified permission path
        std::string api_permissiond; // unified api permission path
        std::string groupd;    // unified group path
        std::string manifestsd;      // unified manifest path
    } PathInfo;

    typedef std::vector<PathInfo> PathInfos;

    ServiceInstallerUtility() {}
    virtual ~ServiceInstallerUtility() {}

    //! install service files
    static bool install(std::string appId, std::string installBasePath, const PathInfo &pathInfo, std::function<void(bool, std::string)> onComplete, bool isUpdate);

    //! remove service files from paths
    static bool remove(std::string appId, const PathInfos &pathInfos, std::function<void(bool, std::string)> onComplete);

private:
    static bool onUpdateManifest(pbnjson::JValue result, void *user_data, std::function<void(bool, std::string)> onComplete);
    static bool onRemoveManifest(pbnjson::JValue result, void *user_data, const std::string &appId, const PathInfos &pathInfos,
                                 std::function<void(bool, std::string)> onComplete);

    //! remove service files from path
    static bool removeOne(const std::string &appId, const PathInfo &pathInfo);

    //! generate security files for service
    static bool generateFilesForService(const PathInfo &pathInfo, const ServiceInfo &servicesInfo, const AppInfo &appInfo, const PackageInfo &packageInfo);

    /*! generate public & private role files, the pathes would be..
     * public for /var/palm/ls2/roles/pub
     * private for /var/palm/ls2/roles/prv
     */
    static bool generateRoleFile(std::string path, bool isPublic, const ServiceInfo &servicesInfo);

    /*! generate public & private service files, the pathes would be..
     * public for /var/palm/ls2/services/pub
     * private for /var/palm/ls2/services/prv
     */
    static bool generateServiceFile(std::string path, const ServiceInfo &servicesInfo, const PackageInfo &packageInfo);

    /*! generate role file fileName for appId in passed path
     *  according to passed template templatePath
     */
    static bool generateUnifiedAppRoleFile(const std::string &path, const std::string &templatePath, const std::string &fileName, const std::string &id,
                                           const std::string &exec);

    //! generate role file for web application in passed path
    static bool generateRoleFileForWebApp(const std::string &path, const std::string &appId);

    //! generate role file for native application in passed path
    static bool generateRoleFileForNativeApp(const std::string& path, const std::string& appId, const std::string& exec);

    //! generate role file for service in passed path
    static bool generateRoleFileForService(const std::string &path, const ServiceInfo &servicesInfo, const AppInfo &appInfo);

    //! generate permission file for "id" in passed path
    static bool generateUnifiedAppPermissionsFile(const std::string &path, bool verified, const std::string &fileName, const PackageInfo &packageInfo, const std::string &id,
                                                  const std::string &allowedMask, const std::vector<std::string> &requiredServices = { }, const std::string appType = "");

    //! generate permission file for web application in passed path
    static bool generatePermissionFileForWebApp(const std::string &path, bool verified, const PackageInfo &packageInfo, const std::vector<std::string> &requiredServices, const std::string appType);

    //! generate permission file for web application in passed path
    static bool generatePermissionFileForNativeApp(const std::string &path, bool verified, const PackageInfo &packageInfo, const std::vector<std::string> &requiredServices);

    //! generate permission file for service in passed path
    static bool generatePermissionFileForService(const std::string &path, bool verified, const ServiceInfo &servicesInfo, const PackageInfo &packageInfo);

    //! generate provided permissions file for service in passed path
    static bool generateAPIPermissionsFileForService(const PathInfo &pathInfo, const ServiceInfo &servicesInfo, const AppInfo &appInfo);

    static bool generateAPIPermissionsFileForServiceNewSchema(const std::string &path, bool verified, const ServiceInfo &servicesInfo, const AppInfo &appInfo);
    static bool generateAPIPermissionsFileForServiceOldSchema(const std::string &path, bool verified, const ServiceInfo &servicesInfo, const AppInfo &appInfo);

    static bool generateGroupFileForService(const PathInfo &pathInfo, const ServiceInfo &servicesInfo, const AppInfo &appInfo);
    static bool generateGroupFileForServiceNewSchema(const std::string &path, bool verified, const ServiceInfo &servicesInfo, const AppInfo &appInfo);
    static bool generateGroupFileForServiceOldSchema(const std::string &path, bool verified, const ServiceInfo &servicesInfo, const AppInfo &appInfo);


    //! generate manifest file
    static bool generateManifestFile(const PathInfo &pathInfo, const std::string& installBasePath, PackageInfo &packageInfo, const AppInfo &appInfo);

    //! get luna unified ls configuration directory path for manifest file
    static std::string adjustLunaDirForManifest(const std::string &rootPath, const std::string &relativePath);
};


#endif
