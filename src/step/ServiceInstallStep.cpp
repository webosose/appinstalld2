// Copyright (c)  2017-2022  LG Electronics, Inc.
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

#include "ServiceInstallStep.h"
#include "installer/Task.h"

using namespace std::placeholders;

ServiceInstallStep::ServiceInstallStep()
{
}

ServiceInstallStep::~ServiceInstallStep()
{
    LOG_DEBUG("ServiceInstallStep::~ServiceInstallStep() called\n");
}

bool ServiceInstallStep::proceed(Task *task)
{
    LOG_DEBUG("ServiceInstallStep::proceed() called\n");

    m_parentTask = task;
    pbnjson::JValue param = task->getParam();
    pbnjson::JValue targetInfo = param["target"];
    bool verify = param["verify"].asBool();
    std::string packageId = task->getPackageId();

    if (targetInfo.isNull())
    {
        targetInfo = pbnjson::Object();
        targetInfo.put("deviceId", DeviceId::INTERNAL_STORAGE);
    }

    ServiceInstallerUtility::PathInfo pathInfo;
    bool isExtStorage = false;

    if (!DeviceId::isInternal(targetInfo["deviceId"].asString()))
        isExtStorage = true;

    const char *getLunaUnifiedRolesDir = Settings::instance().getLunaUnifiedRolesDir(verify);
    const char *getLunaUnifiedServicesDir = Settings::instance().getLunaUnifiedServicesDir(verify);
    const char *getLunaUnifiedPermissionsDirExternal = Settings::instance().getLunaUnifiedPermissionsDir(verify, false);
    const char *getLunaUnifiedPermissionsDirInternal = Settings::instance().getLunaUnifiedPermissionsDir(verify, true);
    const char *getLunaUnifiedAPIPermissionsDir = Settings::instance().getLunaUnifiedAPIPermissionsDir(verify);
    const char *getLunaUnifiedGroupsDir =  Settings::instance().getLunaUnifiedGroupsDir(verify);
    const char *getLunaUnifiedManifestsDirExternal = Settings::instance().getLunaUnifiedManifestsDir(verify, false);
    const char *getLunaUnifiedManifestsDirInternal = Settings::instance().getLunaUnifiedManifestsDir(verify, true);

    if(getLunaUnifiedRolesDir == NULL || getLunaUnifiedServicesDir == NULL || getLunaUnifiedPermissionsDirInternal == NULL || getLunaUnifiedPermissionsDirExternal == NULL || getLunaUnifiedAPIPermissionsDir == NULL || getLunaUnifiedGroupsDir == NULL || getLunaUnifiedManifestsDirInternal == NULL || getLunaUnifiedManifestsDirExternal ==NULL)
    {
        LOG_DEBUG("[ServiceInstallStep::proceed] failed  :");
        return false;
    }

    if(isExtStorage && verify)
    {
        std::string lunaFilesPath = task->getInstallBasePath();
        pathInfo.verified = verify;
        pathInfo.root = lunaFilesPath;
        pathInfo.legacyRoles = lunaFilesPath + std::string(getLunaUnifiedRolesDir);
        pathInfo.legacyServices = lunaFilesPath + std::string(getLunaUnifiedServicesDir); 
        pathInfo.roled = lunaFilesPath + std::string(getLunaUnifiedRolesDir);
        pathInfo.serviced = lunaFilesPath + std::string(getLunaUnifiedServicesDir);
        pathInfo.permissiond = lunaFilesPath + std::string(getLunaUnifiedPermissionsDirExternal);
        pathInfo.api_permissiond = lunaFilesPath + std::string(getLunaUnifiedAPIPermissionsDir);
        pathInfo.groupd =  lunaFilesPath + std::string(getLunaUnifiedGroupsDir);
        pathInfo.manifestsd = lunaFilesPath + std::string(getLunaUnifiedManifestsDirExternal);
    }
    else
    {
        std::string lunaFilesPath = Settings::instance().getLunaFilesPath(verify);

        pathInfo.verified = verify;
        pathInfo.legacyRoles = lunaFilesPath + std::string("/roles");
        pathInfo.legacyServices = lunaFilesPath + std::string("/services");
        pathInfo.roled = std::string(getLunaUnifiedRolesDir);
        pathInfo.serviced = std::string(getLunaUnifiedServicesDir);
        pathInfo.permissiond = std::string(getLunaUnifiedPermissionsDirInternal);
        pathInfo.api_permissiond = std::string(getLunaUnifiedAPIPermissionsDir);
        pathInfo.groupd =  std::string(getLunaUnifiedGroupsDir);
        pathInfo.manifestsd = std::string(getLunaUnifiedManifestsDirInternal);
    }

    LOG_DEBUG("[InstallTask] install paths : pathInfo - root : %s, verified :%d, roles : %s, services :%s, roled : %s, serviced:%s, permissiond :%s,api_permissiond :%s, manifestsd : %s",
              pathInfo.root.c_str(),
              (int)pathInfo.verified,
              pathInfo.legacyRoles.c_str(),
              pathInfo.legacyServices.c_str(),
              pathInfo.roled.c_str(),
              pathInfo.serviced.c_str(),
              pathInfo.permissiond.c_str(),
              pathInfo.api_permissiond.c_str(),
              pathInfo.manifestsd.c_str());

    if (!m_svcinstallerUtility.install(packageId, task->getInstallBasePath(), pathInfo,
        std::bind(&ServiceInstallStep::onInstallServiceComplete, this, _1, _2), task->isUpdate()))
    {
        task->setError(ErrorInstall, APP_INSTALL_ERR_GENERAL, "failed to install service");
        return true;
    }

    sync();

    task->setStep(ServiceInstallRequested);
    return true;

}

void ServiceInstallStep::onInstallServiceComplete(bool success, std::string errorText)
{
    LOG_DEBUG("ServiceInstallStep::onInstallServiceComplete() called\n");

    if (success)
    {
        m_parentTask->setStep(ServiceInstallComplete);
    }
    else
    {
        m_parentTask->setError(ErrorInstall, APP_INSTALL_ERR_INSTALL, errorText);
    }

    m_parentTask->proceed();
}

