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

#ifndef APPINFO_H
#define APPINFO_H

#include <string>
#include <pbnjson.hpp>

/*! AppInfo class is responsible for parsing appinfo.json
 * and contain parsed data
 */
class AppInfo {
    enum LOCALIZE {
        LOCALIZE_SCRIPT = 0,
        LOCALIZE_REGION,
        LOCALIZE_LANGUAGE,
        NUM_LOCALIZE
    };

public:
    /*! Constructor
     * Load appinfo from appPath
     */
    AppInfo(std::string appPath);

    //! Load localization data
    bool loadLocalization();

    //! Whether app info is loaded
    bool isLoaded() const;

    //! Whether is web app type
    bool isWeb() const;
    //! Whether is qml app type
    bool isQml() const;
    //! Whether is native app type
    bool isNative() const;
    //! Whether is stub app type
    bool isStub() const;
    //! Whether is privilegedJail
    bool isPrivilegedJail() const;
    //! Get "type" field from appinfo.json
    std::string getType() const;
    /*! Get "main" field from appinfo.json
     * if fullPath is set, returns full path of "main" field
     */
    std::string getMain(bool fullPath) const;
    //! Get "id" field from appinfo.json
    std::string getId() const;
    //! Get "version" field from appinfo.json
    std::string getVersion() const;
    //! Get "title" field from appinfo.json
    std::string getTitle() const;
    //! Get app path
    std::string getPath() const;
    //! Get install config
    pbnjson::JValue getInstallConfig() const;
    //! Get "icon" field from appinfo.json
    std::string getIcon(bool fullPath) const;
    //! Get required ACG for permissions
    pbnjson::JValue getRequiredPermissions() const;

private:
    //! Load appinfo.json from m_appPath
    bool load();

private:
    std::string m_appPath;
    pbnjson::JValue m_info;
    pbnjson::JValue m_info_localize[NUM_LOCALIZE];
    bool m_loaded;
};

#endif
