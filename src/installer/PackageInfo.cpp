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

#include "base/JUtil.h"
#include "PackageInfo.h"

const std::string DEFAULT_VERSION = "1.0.0";

PackageInfo::PackageInfo(std::string packagePath)
    : m_packagePath(packagePath),
      m_loaded(false)
{
    m_loaded = load();
}

bool PackageInfo::isLoaded() const
{
    return m_loaded;
}

bool PackageInfo::getServices(std::vector<std::string> &serviceLists)
{
    if (m_info.isNull())
        return false;

    pbnjson::JValue services = m_info["services"];
    if (!services.isArray())
        return false;

    size_t arraySize = services.arraySize();
    for (size_t i = 0; i < arraySize; ++i)
        serviceLists.push_back(services[i].asString());

    return true;
}

std::string PackageInfo::getId() const
{
    return m_info["id"].asString();
}

std::string PackageInfo::getVersion() const
{
    return m_info.hasKey("version") ? m_info["version"].asString() : DEFAULT_VERSION;
}

bool PackageInfo::load()
{
    std::string path = m_packagePath + "/packageinfo.json";
    m_info = JUtil::parseFile(path, std::string(""));

    if (m_info.isNull())
        return false;

    return true;
}
