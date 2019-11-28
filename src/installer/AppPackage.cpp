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

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <fstream>
#include <sstream>
#include <stdlib.h>

#include "AppPackage.h"
#include "base/Logging.h"
#include "base/Utils.h"

#define FILENAME_CONTROL "control.tar.gz"
#define FILENAME_DATA    "data.tar.gz"
#define FILENAME_DEBIAN  "debian-binary"

void AppPackage::cbExtractComplete(GPid pid, gint status, gpointer user_data)
{
    AppPackage *package = reinterpret_cast<AppPackage*>(user_data);
    if (!package)
        return;

    if (!WIFEXITED(status) || (WEXITSTATUS(status) != 0) || package->isCanceled()) {
        package->m_funcExtracted(false);
        return;
    }

    if (!package->extractOneItem())
        package->m_funcExtracted(false);
}

std::string AppPackage::Control::getPackage() const
{
    return m_package;
}

std::string AppPackage::Control::getVersion() const
{
    return m_version;
}

std::string AppPackage::Control::getArchitecture() const
{
    return m_architecture;
}

uint64_t AppPackage::Control::getInstalledSize() const
{
    return m_installedSize;
}

bool AppPackage::extract(std::string targetFile,
                         int targetItem,
                         std::string targetPath,
                         std::function<void (bool)> onExtract)
{
    gchar* argv[16] = {0};
    GError* gerr = NULL;
    GSpawnFlags flags = (GSpawnFlags)(G_SPAWN_SEARCH_PATH | G_SPAWN_DO_NOT_REAP_CHILD);

    GPid childPid;
    gboolean result;
    int index = 0;

    argv[index++] = (gchar *) "ar";
    argv[index++] = (gchar *) "x";
    argv[index++] = (gchar *) targetFile.c_str();
    if (targetItem & CONTROL)
        argv[index++] = (gchar *) FILENAME_CONTROL;
    if (targetItem & DATA)
        argv[index++] = (gchar *) FILENAME_DATA;
    if (targetItem & DEBIAN)
        argv[index++] = (gchar *) FILENAME_DEBIAN;
    argv[index] = NULL;

    if (targetItem & CONTROL)
        Utils::remove_file(targetPath + "/" + FILENAME_CONTROL);
    if (targetItem & DATA)
        Utils::remove_file(targetPath + "/" + FILENAME_DATA);
    if (targetItem & DEBIAN)
        Utils::remove_file(targetPath + "/" + FILENAME_DEBIAN);

    m_targetItemList.clear();
    m_canceled = false;

    result = g_spawn_async(targetPath.c_str(),
                           argv,
                           NULL,
                           flags,
                           NULL,
                           NULL,
                           &childPid,
                           &gerr);

    if (result) {
        g_child_watch_add(childPid, cbExtractComplete, this);

        m_targetFile = targetFile;
        m_targetPath = targetPath;
        m_funcExtracted = onExtract;
        if (targetItem & CONTROL)
            m_targetItemList.push_back(FILENAME_CONTROL);
        if (targetItem & DATA)
            m_targetItemList.push_back(FILENAME_DATA);

        return true;
    }

    if (gerr) {
        LOG_ERROR(MSGID_APPUNPACK_IPK_FAIL, 2,
                  PMLOGKS(FILENAME, targetFile.c_str()),
                  PMLOGKS(LOGKEY_ERRTEXT,gerr->message),
                  "Failed to unpack file");
        g_error_free(gerr);
    }

    return false;
}

bool AppPackage::extractOneItem()
{
    if (m_targetItemList.empty()) {
        m_funcExtracted(true);
        return true;
    }

    std::string targetFile = m_targetItemList.front();
    m_targetItemList.pop_front();

    gchar* argv[16] = {0};
    GError* gerr = NULL;
    GSpawnFlags flags = (GSpawnFlags)(G_SPAWN_SEARCH_PATH | G_SPAWN_DO_NOT_REAP_CHILD);

    GPid childPid;
    gboolean result;
    int index = 0;

    argv[index++] = (gchar *) "tar";
    argv[index++] = (gchar *) "xzf";
    argv[index++] = (gchar *) targetFile.c_str();
    argv[index] = NULL;

    result = g_spawn_async(m_targetPath.c_str(),
                           argv,
                           NULL,
                           flags,
                           NULL,
                           NULL,
                           &childPid,
                           &gerr);

    if (result) {
        g_child_watch_add(childPid, cbExtractComplete, this);
        return true;
    }

    if (gerr) {
        LOG_ERROR(MSGID_APPUNPACK_TAR_FAIL, 2,
                  PMLOGKS(FILENAME, targetFile.c_str()),
                  PMLOGKS(LOGKEY_ERRTEXT,gerr->message),
                  "Failed to unzip file");
        g_error_free(gerr);
    }

    return false;
}

bool AppPackage::parseControl(std::string controlFilePath, AppPackage::Control &control)
{
    std::ifstream file(controlFilePath.c_str());
    std::string line;
    std::string field, value;

    if (file.good()) {
        while(std::getline(file, line)) {
            std::istringstream lineStream(line);

            if (!std::getline(lineStream, field, ':'))
                continue;
            if (!std::getline(lineStream, value))
                continue;

            boost::trim(value);
            if (field == "Package")
                control.m_package = value;
            else if (field == "Version")
                control.m_version = value;
            else if (field == "Architecture")
                control.m_architecture = value;
            else if (field == "Installed-Size")
                control.m_installedSize = boost::lexical_cast<uint64_t>(value);
        }

        file.close();
        return true;
    }

    return false;
}

void AppPackage::saveInstalledSizeToControlFile(std::string controlFilePath, uint64_t unpackFileSize)
{
    std::ofstream file(controlFilePath.c_str(), std::ios::app);

    if (file.good())
        file << "Installed-Size: " << unpackFileSize << std::endl;

    file.close();
}

void AppPackage::cancel()
{
    m_canceled = true;
}

bool AppPackage::isCanceled() const
{
    return m_canceled;
}
