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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <glib.h>
#include <set>
#include <string>
#include <vector>

#include "base/Singleton.hpp"

class Settings: public Singleton<Settings> {
public:
    /*! parse opkg conf path from opkg.conf file */
    bool parseOpkgConfigure();

    /*! get install base path
     * if it's verified returns m_userinstallPath, or returns m_developerinstallPath
     */
    std::string getInstallPath(bool verified) const;

    /*! get alias install base path
     * if it's persistence returns m_aliasinstallPath, or returns m_aliasTempinstallPath
     */
    std::string getAliasInstallPath(bool persistence) const;

    /*! get install application path
     * if it's verified returns m_userinstallPath + m_applicationinstallPath,
     * or m_developerinstallPath + m_applicationinstallPath
     */
    std::string getInstallApplicationPath(bool verified) const;

    /*! get install package path
     * if it's verified returns m_userinstallPath + m_packageinstallPath,
     * or m_developerinstallPath + m_packageinstallPath
     */
    std::string getInstallPackagePath(bool verified) const;

    /*! get install service path
     * if it's verified returns m_userinstallPath + m_serviceinstallPath,
     * or m_developerinstallPath + m_serviceinstallPath
     */
    std::string getInstallServicePath(bool verified) const;

    /*! get luna file base path
     * if it's verified returns m_lunaFilesPath, or returns m_developerlunaFilesPath
     */
    std::string getLunaFilesPath(bool verified) const;

    /*! get luna role file path
     * if it's verified returns m_lunaFilesPath + /roles/ + {/pub or /prv}
     * or m_developerlunaFilesPath + /roles/ + {/pub or /prv}
     */
    std::string getLunaRoleFilesPath(bool verified, bool pub) const;

    /*! get luna service file path
     * if it's verified returns m_lunaFilesPath + /services/ + {/pub or /prv}
     * or m_developerlunaFilesPath + /services/ + {/pub or /prv}
     */
    std::string getLunaServiceFilesPath(bool verified, bool pub) const;
    std::string getAppInstallBase() const;
    std::string getDevAppsBasePath() const;
    std::string getApplicationInstallPath() const;
    std::string getSignageContentsPath() const;

    /*! get luna role file path (public/private-agnostic)
     * Versions of luna-service2 approximately newer than 3.11 don't feature
     * separate public and private hubs. New format of role files is also used.
     */
    const char *getLunaUnifiedRolesDir(bool verified);

    /*! get luna role client permissions path
     * Versions of luna-service2 approximately newer than 3.11 don't feature
     * separate public and private hubs. Permission file is used to request
     * access to specific access control groups like _public_ or _private_.
     */
    const char *getLunaUnifiedPermissionsDir(bool verified, bool full);

    /*! get luna unified services directory path
     * Versions of luna-service2 approximately newer than 3.11 don't feature
     * separate public and private hubs. Service file is used to specify
     * command line for launching dynamic service.
     */
    const char *getLunaUnifiedServicesDir(bool verified);

    /*! get luna unified API permissions directory path
     * Versions of luna-service2 approximately newer than 3.11 don't feature
     * separate public and private hubs. API permissions specify security groups which
     * service provides.
     */
    const char *getLunaUnifiedAPIPermissionsDir(bool verified);

    /*! get luna unified manifest directory path
     */
    const char *getLunaUnifiedManifestsDir(bool verified, bool full);

    /*! form file name appId.suffix.json
     */
    std::string getLunaUnifiedJsonFileName(const std::string &appId, const std::string &suffix);

    /*! form file name appId.app.json
     */
    std::string getLunaUnifiedAppJsonFileName(const std::string &appId);

    /*! form file name appId.service.json
     */
    std::string getLunaUnifiedServiceJsonFileName(const std::string &appId);

    /*! form file name appId.api.json
     */
    std::string getLunaUnifiedAPIJsonFileName(const std::string &appId);

    /*! form security group name for specific service
     */
    std::string getGroupNameForService(const std::string &serviceId);

    const std::string& getInstallerDataPath() const;
    const std::string& getApplicationPath() const;
    const std::string& getPackagePath() const;
    const std::string& getPackageinstallPath() const;
    const std::string& getServiceinstallPath() const;
    const std::string& getOpkgConfPath() const;
    const std::string& getOpkgInfoPath() const;
    const std::string& getOpkgLockFilePath() const;
    const std::string& getJsservicePath() const;
    const std::string& getJailerPath() const;
    const std::string& getRoleTemplatePathNDK() const;
    const std::string& getRoleTemplatePathWebApp() const;
    const std::string& getRoleTemplatePathJSService() const;
    const std::string& getRoleTemplatePathNativeService() const;
    const std::string& getConfPath() const;
    const std::string& getSchemaPath() const;
    bool isDevMode();
    bool isJailMode();
    const std::string& getLocalePath() const;

protected:
friend class Singleton<Settings> ;
    Settings();
    virtual ~Settings();

private:
    std::string m_installerDataPath;        //default : @WEBOS_INSTALL_WEBOS_LOCALSTATEDIR@/data/com.webos.appInstallService
    std::string m_userinstallPath;          //default : @WEBOS_INSTALL_CRYPTOFSDIR@
    std::string m_systeminstallPath;        //default : @WEBOS_INSTALL_CRYPTOFSDIR@  TODO: change this dir after system partition is created
    std::string m_developerinstallPath;     //default : /media/developer
    std::string m_aliasinstallPath;         //default : /media/alias
    std::string m_aliasTempinstallPath;     //default : /tmp/alias
    std::string m_applicationPath;          //default : /usr/palm/applications
    std::string m_packagePath;              //default : /usr/palm/packages
    std::string m_servicePath;              //default : /usr/palm/services
    std::string m_applicationinstallPath;   //default : /apps + m_applicationPath
    std::string m_packageinstallPath;       //default : /apps + m_packagePath
    std::string m_serviceinstallPath;       //default : /apps + m_servicePath
    std::string m_opkgConfPath;             //default : @WEBOS_INSTALL_WEBOS_SYSCONFDIR@/opkg.conf
    std::string m_lunaFilesPath;            //default : @WEBOS_INSTALL_WEBOS_LOCALSTATEDIR@/ls2
    std::string m_developerlunaFilesPath;   //default : @WEBOS_INSTALL_WEBOS_LOCALSTATEDIR@/ls2-dev
    std::string m_jsservicePath;            //default : @WEBOS_INSTALL_BINDIR@/run-js-service
    std::string m_jailerPath;               //default : @WEBOS_INSTALL_BINDIR@/usr/bin/jailer
    std::string m_roleTemplatePathNDK;      //default : @WEBOS_INSTALL_DATADIR@/rolegen/templates/NDK
    std::string m_roleTemplatePathWebApp;   //default : @WEBOS_INSTALL_DATADIR@/rolegen/templates/WebApp.json
    std::string m_roleTemplatePathJSService;   //default : @WEBOS_INSTALL_DATADIR@/rolegen/templates/JSService.json
    std::string m_roleTemplatePathNativeService;   //default : @WEBOS_INSTALL_DATADIR@/rolegen/templates/NativeService.json
    std::string m_confPath;                 //default : @WEBOS_INSTALL_WEBOS_SYSCONFDIR@/appinstalld-conf.json
    std::string m_schemaPath;               //default : @WEBOS_INSTALL_WEBOS_SYSCONFDIR@/schemas/appinstalld/
    // TODO : This is temp code for opkg works properly
    std::string m_devModePath;              // default : /var/luna/preferences/devmode_enabled
    bool m_isDevMode;                       // default : false
    bool m_isJailMode;                      // default : false
    std::string m_localePath;               // default : /var/luna/preferences/localeInfo

    // Signage media file path which need to link app path
    std::string m_signageContentsPath;      // default : /mnt/lg/appstore/scap/contents/

    bool m_supportUI;                       // default : true
    bool m_supportUpdateService;            // default : true
    int m_timeout;                          // default : 3 * 60 * 1000
    int m_minimumAppSize;                  // default : 100 * 1024

    std::string m_opkgInfoPath;             //default : /apps/var/lib/opkg/info
    std::string m_opkgStatusFilePath;       //default : /apps/var/lib/opkg/status
    std::string m_opkgLockFilePath;         //default : /apps/var/lock/opkg

};

#endif // Settings
