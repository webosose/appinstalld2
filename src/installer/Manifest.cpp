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

#include "base/JUtil.h"
#include "Manifest.h"

Manifest::Manifest(const std::string& path)
{
    load(path);
}

bool Manifest::isLoaded() const
{
    return (m_info.isNull() ? false : true);
}

bool Manifest::load(const std::string& path)
{
    m_info = JUtil::parseFile(path, std::string(""));
    if (m_info.isNull())
        return false;

    const char *keys[] = {"roleFiles",
                          "roleFilesPub",
                          "roleFilesPrv",
                          "serviceFiles",
                          "clientPermissionFiles",
                          "apiPermissionFiles"};

    for (const char *key : keys) {
        if (m_info.hasKey(key)) {
            for (const auto &v : m_info[key].items())
                m_paths.push_back(v.asString());
        }
    }

    return true;
}

bool Manifest::getPaths(std::vector<std::string> &paths)
{
    if (!isLoaded())
        return false;

    paths = m_paths;
    return true;
}
