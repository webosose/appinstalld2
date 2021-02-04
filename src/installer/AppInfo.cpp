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

#include "AppInfo.h"
#include "base/JUtil.h"
#include "base/Locales.h"

static const char *KEY_APPBASE = "_app_base";
const std::string DEFAULT_VERSION = "1.0.0";

AppInfo::AppInfo(std::string appPath)
    : m_appPath(appPath),
      m_loaded(false)
{
    m_loaded = load();
}

bool AppInfo::isLoaded() const
{
    return m_loaded;
}

bool AppInfo::isWeb() const
{
    return (getType() == "web");
}

bool AppInfo::isQml() const
{
    return (getType() == "qml");
}

bool AppInfo::isNative() const
{
    return (!isWeb() && !isQml() && !isStub());
}

bool AppInfo::isStub() const
{
    return (getType() == "stub");
}

bool AppInfo::isPrivilegedJail() const
{
    return m_info["privilegedJail"].asBool();
}

std::string AppInfo::getType() const
{
    return m_info["type"].asString();
}

std::string AppInfo::getMain(bool fullPath) const
{
    std::string strMain = m_info["main"].asString();
    return (fullPath) ? (m_appPath + "/" + strMain) : strMain;
}

std::string AppInfo::getId() const
{
    return m_info["id"].asString();
}

std::string AppInfo::getVersion() const
{
    return m_info.hasKey("version") ? m_info["version"].asString() : DEFAULT_VERSION;
}

std::string AppInfo::getTitle() const
{
    for(int i = 0; i < NUM_LOCALIZE; ++i)
    {
        if (!m_info_localize[i].isNull() &&
            m_info_localize[i].hasKey("title"))
            return m_info_localize[i]["title"].asString();
    }
    return m_info["title"].asString();
}

std::string AppInfo::getPath() const
{
    return m_appPath;
}

std::string AppInfo::getIcon(bool fullPath) const
{
    std::string strIcon;
    for(int i = 0 ; i < NUM_LOCALIZE; ++i)
    {
        if (!m_info_localize[i].isNull() &&
            m_info_localize[i].hasKey("icon"))
        {
            strIcon = m_info_localize[i]["icon"].asString();
            return (fullPath) ? (m_info_localize[i][KEY_APPBASE].asString() + "/" + strIcon) : strIcon;
        }
    }

    strIcon = m_info["icon"].asString();
    return (fullPath) ? (m_appPath + "/" + strIcon) : strIcon;
}

pbnjson::JValue AppInfo::getInstallConfig() const
{
    if (m_info.isNull())
        return pbnjson::Object();

    return m_info.hasKey("installConfig") ? m_info["installConfig"] : pbnjson::Object();
}

pbnjson::JValue AppInfo::getRequiredPermissions() const
{
    if (m_info.isNull())
        return pbnjson::JValue();

    return m_info["requiredPermissions"];
}

bool AppInfo::load()
{
    std::string path = m_appPath + "/appinfo.json";
    m_info = JUtil::parseFile(path, std::string(""));

    if (m_info.isNull())
        return false;

    return true;
}

bool AppInfo::loadLocalization()
{
    Locales locale;
    std::string language = locale.language();
    std::string script = locale.script();
    std::string region = locale.region();

    std::string languagePath = m_appPath + "/resources/" + language;
    std::string regionPath = m_appPath + "/resources/" + language + "/" + region;
    std::string scriptPath = m_appPath + "/resources/" + language + "/" + script + "/" + region;

    m_info_localize[LOCALIZE_LANGUAGE] = JUtil::parseFile(languagePath + "/appinfo.json", std::string(""));
    if (!m_info_localize[LOCALIZE_LANGUAGE].isNull())
        m_info_localize[LOCALIZE_LANGUAGE].put(KEY_APPBASE, languagePath);

    m_info_localize[LOCALIZE_REGION] = JUtil::parseFile(regionPath + "/appinfo.json", std::string(""));
    if (!m_info_localize[LOCALIZE_REGION].isNull())
        m_info_localize[LOCALIZE_REGION].put(KEY_APPBASE, regionPath);

    m_info_localize[LOCALIZE_SCRIPT] = JUtil::parseFile(scriptPath + "/appinfo.json", std::string(""));
    if (!m_info_localize[LOCALIZE_SCRIPT].isNull())
        m_info_localize[LOCALIZE_SCRIPT].put(KEY_APPBASE, scriptPath);

    return true;
}
