// Copyright (c) 2017-2019 LG Electronics, Inc.
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

#ifndef MANIFEST_H
#define MANIFEST_H

#include <pbnjson.hpp>
#include <string>

/*! Manifest class is responsible for parsing manifests.d.json
 * and contain parsed data
 */
class Manifest {
public:
    /*! Constructor
     * Load manifest from path
     */
    Manifest(const std::string &path);

    //! Whether manifest is loaded
    bool isLoaded() const;

    //! get all paths
    bool getPaths(std::vector<std::string> &paths);

private:
    //! Load manifest.json from m_path
    bool load(const std::string &path);

private:
    pbnjson::JValue m_info;
    std::vector<std::string> m_paths;
};

#endif
