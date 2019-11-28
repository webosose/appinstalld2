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

#ifndef PACKAGEINFO_H
#define PACKAGEINFO_H

#include <pbnjson.hpp>
#include <string>
#include <vector>

/*! PackageInfo class is responsible for parsing packageinfo.json
 * and contain parsed data
 */
class PackageInfo {
public:
    /*! Constructor
     * Load packageinfo from packagePath
     */
    PackageInfo(std::string packagePath);

    //! Whether package info is loaded
    bool isLoaded() const;

    //! get service ids
    bool getServices(std::vector<std::string> &serviceLists);
    //! Get "id" field from packageinfo.json
    std::string getId() const;
    //! Get "version" field from packageinfo.json
    std::string getVersion() const;

private:
    //! Load packageinfo.json from m_packagePath
    bool load();

private:
    std::string m_packagePath;
    pbnjson::JValue m_info;
    bool m_loaded;
};

#endif
