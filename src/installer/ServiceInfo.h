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

#ifndef SERVICEINFO_H
#define SERVICEINFO_H

#include <pbnjson.hpp>
#include <string>

/*! ServiceInfo class is responsible for parsing services.json
 * and contain parsed data
 */
class ServiceInfo {
public:
    typedef enum {
        JAILER_NATIVE = 0, JAILER_DEV
    } JailerType;

    //! generate ServiceInfo object
    static ServiceInfo generate(std::string id, std::string type, std::string path, std::string exec);

    /*! Constructor
     * Load serviceinfo from servicePath
     */
    ServiceInfo(std::string servicePath);

    //! Apply jailer to native service
    void applyJailer(JailerType type);

    //! Apply root path
    void applyRootPath(std::string rootPath);

    //! Get "id" field from services.json
    std::string getId() const;
    //! Get "engine" field from services.json
    std::string getType() const;
    /*! Get "executable" field from services.json
     * if fullpath is set, returns full path of "executable" field
     */
    std::string getExec(bool fullPath = true) const;
    //! Get service path
    std::string getPath(bool absolute = true) const;
    //! Get Jailer type for native service
    std::string getJailerType() const;

private:
    //! Null Constructor
    ServiceInfo();

    //! Load serviceinfo.json from m_servicePath
    bool load();

private:
    std::string m_servicePath;
    std::string m_rootPath;
    std::string m_jailerType;
    pbnjson::JValue m_info;
};

#endif
