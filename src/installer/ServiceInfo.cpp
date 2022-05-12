// Copyright (c)  2013-2021 LG Electronics, Inc.
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

#include "ServiceInfo.h"
#include "base/JUtil.h"
#include "base/Logging.h"

ServiceInfo ServiceInfo::generate(std::string id, std::string type, std::string path, std::string exec)
{
    ServiceInfo info;
    info.m_info = pbnjson::Object();
    info.m_info.put("id", id);
    info.m_info.put("engine", type);
    info.m_info.put("executable", exec);
    info.m_servicePath = path;
    return info;
}

ServiceInfo::ServiceInfo(std::string servicePath)
    : m_servicePath(servicePath)
{
    load();
}

ServiceInfo::ServiceInfo()
{
}

void ServiceInfo::applyJailer(JailerType type)
{
    switch (type) {
        case JAILER_DEV:
            m_jailerType = "native_devmode";
            break;
        default:
            m_jailerType = "native";
            break;
    }
}

void ServiceInfo::applyRootPath(std::string rootPath)
{
    m_rootPath = rootPath;
}

std::string ServiceInfo::getId() const
{
    return m_info["id"].asString();
}

std::string ServiceInfo::getType() const
{
    return m_info["engine"].asString();
}

std::string ServiceInfo::getExec(bool fullPath) const
{
    std::string exec = m_info["executable"].asString();
    if (!fullPath) {
        return exec;
    }
    if (m_rootPath.empty()) {
        return m_servicePath + "/" + exec;
    }

    std::size_t pos = m_servicePath.find(m_rootPath);
    if (pos == std::string::npos) {
        return m_servicePath + "/" + exec;
    }

    std::string relativePrefixPath = m_servicePath;
    relativePrefixPath.erase(0, m_rootPath.length() + 1);
    return relativePrefixPath + "/" + exec;
}

std::string ServiceInfo::getPath(bool absolute) const
{
    if (absolute || m_rootPath.empty()) {
        return m_servicePath;
    }
    std::size_t pos = m_servicePath.find(m_rootPath);
    if (pos == std::string::npos) {
        return m_servicePath;
    }
    std::string relativePath = m_servicePath;
    relativePath.erase(0, m_rootPath.length() + 1);
    return relativePath;
}

std::string ServiceInfo::getJailerType() const
{
    return m_jailerType;
}

bool ServiceInfo::load()
{
    std::string path = m_servicePath + "/services.json";
    LOG_DEBUG("[ServiceInfo::load]  path: %s ",path.c_str());
    m_info = JUtil::parseFile(path, std::string(""));

    if (m_info.isNull())
        return false;

    // apply default jailer
    if (getType() == "native")
        applyJailer(JAILER_NATIVE);

    return true;
}

pbnjson::JValue ServiceInfo::getServiceList() const
{
    return m_info["services"];
}

bool ServiceInfo::hasSchemaVersion() const
{

    return m_info.hasKey("schemaVersion");
}


bool ServiceInfo::isValidSchema()
{
   std::string path = m_servicePath + "/services.json";
    LOG_DEBUG("[ServiceInfo::isValidSchema]  path: %s ",path.c_str());
   if (hasSchemaVersion() )
    {
        std::string servicesJsonSchema = "servicesJson-" + m_info["schemaVersion"].asString();
	LOG_DEBUG("[ServiceInfo::isValidSchema]  start schema  validation against %s  schema \n" ,servicesJsonSchema.c_str());
        m_info = JUtil::parseFile(path, servicesJsonSchema);
        if (m_info.isNull())
        {
	    LOG_DEBUG("[ServiceInfo::isValidSchema]  schema  validation against %s   schema is failed  \n",servicesJsonSchema.c_str());
            return false;
        }
	LOG_DEBUG("[ServiceInfo::isValidSchema]  schema  validation against %s  schema is success",servicesJsonSchema.c_str());
    }
    else
    {
        LOG_DEBUG("[ServiceInfo::isValidSchema]  start schema  validation against old  schema \n");
        std::string servicesJsonSchema = "servicesJson-old";
        m_info = JUtil::parseFile(path, servicesJsonSchema);
        if (m_info.isNull())
        {
            LOG_DEBUG("[ServiceInfo::isValidSchema]  schema  validation against old schema is failed  \n");
            return false;
        }
        LOG_DEBUG("[ServiceInfo::isValidSchema]  schema  validation against old  schema is success");
    }

    return true;
}

pbnjson::JValue ServiceInfo::getRequiredPermissions() const
{
    return hasSchemaVersion() ? m_info["services"][0]["requiredPermissions"] : pbnjson::JValue();
}
