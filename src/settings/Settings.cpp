// Copyright (c) 2013-2019 LG Electronics, Inc.
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

#include <string>

#include "webospaths.h"
#include "Settings.h"
#include "base/Utils.h"
#include "base/Logging.h"
#include "base/JUtil.h"

Settings::Settings()
    : m_installerDataPath(WEBOS_INSTALL_WEBOS_LOCALSTATEDIR "/data/com.webos.appInstallService"),
      m_userinstallPath( WEBOS_INSTALL_CRYPTOFSDIR),
      m_systeminstallPath(WEBOS_INSTALL_CRYPTOFSDIR),
      m_developerinstallPath("/media/developer"),
      m_aliasinstallPath("/media/alias"),
      m_aliasTempinstallPath("/tmp/alias"),
      m_applicationPath("/usr/palm/applications"),
      m_packagePath("/usr/palm/packages"),
      m_servicePath("/usr/palm/services"),
      m_applicationinstallPath(std::string("/apps") + m_applicationPath),
      m_packageinstallPath(std::string("/apps") + m_packagePath),
      m_serviceinstallPath(std::string("/apps") + m_servicePath),
      m_opkgConfPath( WEBOS_INSTALL_WEBOS_SYSCONFDIR "/appinstalld/opkg.conf"),
      m_lunaFilesPath(WEBOS_INSTALL_WEBOS_LOCALSTATEDIR "/ls2"),
      m_developerlunaFilesPath(WEBOS_INSTALL_WEBOS_LOCALSTATEDIR "/ls2-dev"),
      m_jsservicePath(WEBOS_INSTALL_BINDIR "/run-js-service"),
      m_jailerPath( WEBOS_INSTALL_BINDIR "/jailer"),
      m_roleTemplatePathNDK(WEBOS_INSTALL_DATADIR "/rolegen/templates/NDK"),
      m_roleTemplatePathWebApp(WEBOS_INSTALL_DATADIR "/rolegen/templates/WebApp.json"),
      m_roleTemplatePathJSService(WEBOS_INSTALL_DATADIR "/rolegen/templates/JSService.json"),
      m_roleTemplatePathNativeService(WEBOS_INSTALL_DATADIR "/rolegen/templates/NativeService.json"),
      m_confPath(WEBOS_INSTALL_WEBOS_SYSCONFDIR "/appinstalld-conf.json"),
      m_schemaPath(WEBOS_INSTALL_WEBOS_SYSCONFDIR "/schemas/appinstalld/"),
      m_devModePath("/var/luna/preferences/devmode_enabled"),
      m_isDevMode(false),
      m_isJailMode(false),
      m_localePath("/var/luna/preferences/localeInfo"),
      m_signageContentsPath("/mnt/lg/appstore/scap/contents/"),
      m_supportUI(true),
      m_supportUpdateService(true),
      m_timeout(3 * 60 * 1000),
      m_minimumAppSize(100 * 1024)
{
    if (0 == access(m_devModePath.c_str(), F_OK))
        m_isDevMode = true;

    if (0 == access(m_jailerPath.c_str(), F_OK))
        m_isJailMode = true;

    if (!parseOpkgConfigure())
        LOG_WARNING(MSGID_SETTINGS_PARSE_FAIL, 0, "opkg conf parse fail");
}

Settings::~Settings()
{
}

bool Settings::parseOpkgConfigure()
{
    std::ifstream file(m_opkgConfPath.c_str());
    std::string line;
    std::string option, field, value;

    if (!file.good()) {
        return false;
    }

    while (std::getline(file, line)) {
        std::istringstream lineStream(line);

        if (!std::getline(lineStream, option, ' '))
            continue;
        if (!std::getline(lineStream, field, ' '))
            continue;
        if (!std::getline(lineStream, value))
            continue;

        if (field == "info_dir")
            m_opkgInfoPath = std::string("/apps") + value;
        else if (field == "status_file")
            m_opkgStatusFilePath = std::string("/apps") + value;
        else if (field == "lock_file")
            m_opkgLockFilePath = std::string("/apps") + value;
    }

    file.close();
    return true;
}

std::string Settings::getInstallPath(bool verified) const
{
    return (verified) ? m_userinstallPath : m_developerinstallPath;
}

std::string Settings::getAliasInstallPath(bool persistence) const
{
    return (persistence) ? m_aliasinstallPath : m_aliasTempinstallPath;
}

std::string Settings::getInstallApplicationPath(bool verified) const
{
    return getInstallPath(verified) + m_applicationinstallPath;
}

std::string Settings::getInstallPackagePath(bool verified) const
{
    return getInstallPath(verified) + m_packageinstallPath;
}

std::string Settings::getInstallServicePath(bool verified) const
{
    return getInstallPath(verified) + m_serviceinstallPath;
}

std::string Settings::getLunaFilesPath(bool verified) const
{
    return (verified) ? m_lunaFilesPath : m_developerlunaFilesPath;
}

std::string Settings::getLunaRoleFilesPath(bool verified, bool pub) const
{
    return getLunaFilesPath(verified) + "/roles" + ((pub) ? "/pub" : "/prv");
}

std::string Settings::getLunaServiceFilesPath(bool verified, bool pub) const
{
    return getLunaFilesPath(verified) + "/services" + ((pub) ? "/pub" : "/prv");
}

std::string Settings::getAppInstallBase() const
{
    return m_userinstallPath;
}

std::string Settings::getSignageContentsPath() const
{
    return m_signageContentsPath;
}

std::string Settings::getDevAppsBasePath() const
{
    return m_developerinstallPath;
}

std::string Settings::getApplicationInstallPath() const
{
    return m_applicationinstallPath;
}

const char *Settings::getLunaUnifiedRolesDir(bool verified)
{
    return verified ? WEBOS_INSTALL_SYSBUS_DYNROLESDIR : WEBOS_INSTALL_SYSBUS_DEVROLESDIR;
}

const char *Settings::getLunaUnifiedPermissionsDir(bool verified, bool full)
{
    if (full)
        return verified ? WEBOS_INSTALL_SYSBUS_DYNPERMISSIONSDIR : WEBOS_INSTALL_SYSBUS_DEVPERMISSIONSDIR;
    else
        return verified ? "/var/luna-service2/client-permissions.d" : "/var/luna-service2-dev/client-permissions.d";
}

const char *Settings::getLunaUnifiedServicesDir(bool verified)
{
    return verified ? WEBOS_INSTALL_SYSBUS_DYNSERVICESDIR : WEBOS_INSTALL_SYSBUS_DEVSERVICESDIR;
}

const char *Settings::getLunaUnifiedAPIPermissionsDir(bool verified)
{
    return verified ? WEBOS_INSTALL_SYSBUS_DYNAPIPERMISSIONSDIR : WEBOS_INSTALL_SYSBUS_DEVAPIPERMISSIONSDIR;
}

const char *Settings::getLunaUnifiedManifestsDir(bool verified, bool full)
{
    //TODO : Replace path definition from webospaths.h
    if (full)
        return verified ? WEBOS_INSTALL_SYSBUS_DYNDATADIR "/manifests.d" : WEBOS_INSTALL_SYSBUS_DEVDATADIR "/manifests.d";
    else
        return verified ? "/var/luna-service2/manifests.d" : "/var/luna-service2-dev/manifests.d";
}

std::string Settings::getLunaUnifiedJsonFileName(const std::string &appId, const std::string &suffix)
{
    return appId + "." + suffix + ".json";
}

std::string Settings::getLunaUnifiedAppJsonFileName(const std::string &appId)
{
    return getLunaUnifiedJsonFileName(appId, "app");
}

std::string Settings::getLunaUnifiedServiceJsonFileName(const std::string &appId)
{
    return getLunaUnifiedJsonFileName(appId, "service");
}

std::string Settings::getLunaUnifiedAPIJsonFileName(const std::string &appId)
{
    return getLunaUnifiedJsonFileName(appId, "api");
}

std::string Settings::getGroupNameForService(const std::string &serviceId)
{
    return serviceId + ".group";
}

const std::string& Settings::getInstallerDataPath() const
{
    return m_installerDataPath;
}

const std::string& Settings::getApplicationPath() const
{
    return m_applicationPath;
}

const std::string& Settings::getPackagePath() const
{
    return m_packagePath;
}

const std::string& Settings::getPackageinstallPath() const
{
    return m_packageinstallPath;
}

const std::string& Settings::getServiceinstallPath() const
{
    return m_serviceinstallPath;
}

const std::string& Settings::getOpkgConfPath() const
{
    return m_opkgConfPath;
}

const std::string& Settings::getOpkgInfoPath() const
{
    return m_opkgInfoPath;
}

const std::string& Settings::getOpkgLockFilePath() const
{
    return m_opkgLockFilePath;
}

const std::string& Settings::getJsservicePath() const
{
    return m_jsservicePath;
}

const std::string& Settings::getJailerPath() const
{
    return m_jailerPath;
}

const std::string& Settings::getRoleTemplatePathNDK() const
{
    return m_roleTemplatePathNDK;
}

const std::string& Settings::getRoleTemplatePathWebApp() const
{
    return m_roleTemplatePathWebApp;
}

const std::string& Settings::getRoleTemplatePathJSService() const
{
    return m_roleTemplatePathJSService;
}

const std::string& Settings::getRoleTemplatePathNativeService() const
{
    return m_roleTemplatePathNativeService;
}

const std::string& Settings::getConfPath() const
{
    return m_confPath;
}

const std::string& Settings::getSchemaPath() const
{
    return m_schemaPath;
}

bool Settings::isDevMode()
{
    return m_isDevMode;
}

bool Settings::isJailMode()
{
    return m_isJailMode;
}

const std::string& Settings::getLocalePath() const
{
    return m_localePath;
}

