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

#include <string.h>
#include <unistd.h>

#include "AppImpl.h"
#include "AppInstallerUtility.h"
#include "base/Logging.h"
#include "base/System.h"
#include "base/Utils.h"
#include "settings/Settings.h"
#include "webospaths.h"

bool AppInstallerUtility::m_locked = false;

std::string getOpkgLockPath(std::string installBasePath)
{
    std::string opkgLockFilePath = installBasePath + Settings::instance().getOpkgLockFilePath();
    std::size_t pos = opkgLockFilePath.rfind("opkg");

    return opkgLockFilePath.substr(0, pos);
}

gboolean AppInstallerUtility::cbChildProgress(GIOChannel *channel, GIOCondition condition, gpointer user_data)
{
    GString *str = g_string_new("");
    GError *error = NULL;

    GIOStatus status = g_io_channel_read_line_string(channel, str, NULL, &error);
    if (status != G_IO_STATUS_NORMAL) {
        if (error) {
            LOG_WARNING(MSGID_INSTALL_IPK_IO_ERR, 1,
                        PMLOGKS(LOGKEY_ERRTEXT,error->message),
                        "Failed to read from child's stdout pipe");

            g_error_free(error);
        }

        g_string_free(str, TRUE);
        return true;
    }

    LOG_DEBUG("Got status message from child: %s\n", str->str);

    if (!str->str || (strncmp(str->str, "status:", 7) && strncmp(str->str, " * ", 3))) {
        g_string_free(str, TRUE);
        return true;
    }

    AppInstallerUtility *installer = reinterpret_cast<AppInstallerUtility*>(user_data);
    if (installer && installer->m_funcProgress)
        installer->m_funcProgress(str->str);

    g_string_free(str, TRUE);

    return true;
}

void AppInstallerUtility::cbChildComplete(GPid pid, gint status, gpointer user_data) {
    LOG_DEBUG("child pid %d done with status %d", pid, status);

    AppInstallerUtility *installer = reinterpret_cast<AppInstallerUtility*>(user_data);
    AppInstallerUtility::m_locked = false;

    if (installer)
        installer->m_funcComplete(status);
}

AppInstallerUtility::AppInstallerUtility()
    : m_childStdOutChannel(NULL),
      m_childStdOutSource(NULL),
      m_sourceId(0),
      m_pid(-1)
{
}

AppInstallerUtility::~AppInstallerUtility()
{
    clear();
}

AppInstallerUtility::Result AppInstallerUtility::install(std::string target,
                                                         unsigned int uncompressedSizeInKB,
                                                         bool verify,
                                                         bool allowDowngrade,
                                                         bool allowReInstall,
                                                         std::string installBasePath,
                                                         FuncProgress cbProgress,
                                                         FuncComplete cbComplete)
{
    clear();

    std::string opkgBasePath = installBasePath;
    if (installBasePath.empty()) {
        opkgBasePath = Settings::instance().getInstallPath(verify);
    }

    if (isLocked())
        return LOCKED;
    restore(opkgBasePath);

    gchar* argv[16] = {0}; ///WARNING! look out below if number of params goes > size of this array (keep them in sync)
    GError* gerr = NULL;
    GSpawnFlags flags = (GSpawnFlags)(G_SPAWN_SEARCH_PATH |
                                      G_SPAWN_STDERR_TO_DEV_NULL |
                                      G_SPAWN_DO_NOT_REAP_CHILD);
    GPid childPid;
    gint childStdoutFd;
    gboolean result;
    int index = 0;

    argv[index++] = (gchar *) WEBOS_INSTALL_SBINDIR "/setcpushares-task";
    argv[index++] = (gchar *) WEBOS_INSTALL_BINDIR "/ApplicationInstallerUtility";
    argv[index++] = (gchar *) "-c";
    argv[index++] = (gchar *) "install";
    argv[index++] = (gchar *) "-p";
    argv[index++] = (gchar *) target.c_str();
    argv[index++] = (gchar *) "-f";
    argv[index++] = (gchar *) Settings::instance().getOpkgConfPath().c_str();
    argv[index++] = (gchar *) "-u";
    argv[index++] = (gchar *) "0";//Utils::toString(uncompressedSizeInKB).c_str();

    // When the app will be installed in the external storage,
    // Send the base path for installing apps in it.
    if (!installBasePath.empty()) {
        argv[index++] = (gchar *) "-l";
        argv[index++] = (gchar *) installBasePath.c_str();
    } else {
        argv[index++] = (gchar*) "-t";
        if (verify)
            argv[index++] = (gchar*) "internal";
        else
            argv[index++] = (gchar*) "developer";
    }
    if (allowDowngrade)
        argv[index++] = (gchar*) "-d";
    if (allowReInstall)
        argv[index++] = (gchar*) "-r";
    argv[index] = NULL;

    std::string opkgLockPath = getOpkgLockPath(installBasePath);
    if (!Utils::make_dir(opkgLockPath.c_str(), true)) {
        LOG_ERROR(MSGID_APPINSTALL_FAIL, 2,
                  PMLOGKS(REASON, "Failed to create opkg lock directory"),
                  PMLOGKS(PATH, opkgLockPath.c_str()),
                  "");
        return FAIL;
    }

    result = g_spawn_async_with_pipes(NULL,
                                      argv,
                                      NULL,
                                      flags,
                                      NULL,
                                      NULL,
                                      &childPid,
                                      NULL,
                                      &childStdoutFd,
                                      NULL,
                                      &gerr);

    if (result) {
        m_childStdOutChannel = g_io_channel_unix_new(childStdoutFd);
        m_childStdOutSource = g_io_create_watch(m_childStdOutChannel, G_IO_IN);
        g_source_set_callback(m_childStdOutSource, (GSourceFunc)cbChildProgress, this, NULL);
        MainApp::instance().attach(m_childStdOutSource);

        guint sourceId = g_child_watch_add_full(G_PRIORITY_DEFAULT_IDLE, childPid, cbChildComplete, this, NULL);

        m_pid = childPid;
        m_sourceId = sourceId;
        m_funcProgress = cbProgress;
        m_funcComplete = cbComplete;
        m_locked = true;

        return SUCCESS;
    }

    if (gerr) {
        LOG_ERROR(MSGID_APPINSTALL_FAIL, 2,
                  PMLOGKS(REASON,"Failed to execute ApplicationInstallerUtility command"),
                  PMLOGKS(LOGKEY_ERRTEXT,gerr->message),
                  "");
        g_error_free(gerr);
    }

    return FAIL;
}

AppInstallerUtility::Result AppInstallerUtility::remove(std::string appId, bool verify, std::string installUSBPath, FuncProgress cbProgress, FuncComplete cbComplete)
{
    clear();

    if (isLocked())
        return LOCKED;
    restore(Settings::instance().getInstallPath(true));
    if (Settings::instance().isDevMode())
        restore(Settings::instance().getInstallPath(false));
    if (!installUSBPath.empty())
        restore(installUSBPath);

    gchar* argv[16] = {0}; ///WARNING! look out below if number of params goes > size of this array (keep them in sync)
    GError* gerr = NULL;
    GSpawnFlags flags = (GSpawnFlags)(G_SPAWN_SEARCH_PATH |
                                      G_SPAWN_STDERR_TO_DEV_NULL |
                                      G_SPAWN_DO_NOT_REAP_CHILD);
    GPid childPid;
    gint childStdoutFd;
    gboolean result;
    int index = 0;

    argv[index++] = (gchar *) WEBOS_INSTALL_SBINDIR "/setcpushares-task";
    argv[index++] = (gchar *) WEBOS_INSTALL_BINDIR "/ApplicationInstallerUtility";
    argv[index++] = (gchar *) "-c";
    argv[index++] = (gchar *) "remove";
    argv[index++] = (gchar *) "-p";
    argv[index++] = (gchar *) appId.c_str();
    argv[index++] = (gchar *) "-f";
    argv[index++] = (gchar *) Settings::instance().getOpkgConfPath().c_str();
    // Send USB path for find USB folders,
    // when the application is uninstalled.
    argv[index++] = (gchar *) "-l";
    argv[index++] = (gchar *) installUSBPath.c_str();
    if (!verify) {
        argv[index++] = (gchar*) "-t";
        argv[index++] = (gchar*) "developer";
    }
    argv[index] = NULL;

    result = g_spawn_async_with_pipes(NULL,
                                      argv,
                                      NULL,
                                      flags,
                                      NULL,
                                      NULL,
                                      &childPid,
                                      NULL,
                                      &childStdoutFd,
                                      NULL,
                                      &gerr);

    if (result) {
        m_childStdOutChannel = g_io_channel_unix_new(childStdoutFd);
        m_childStdOutSource = g_io_create_watch(m_childStdOutChannel, G_IO_IN);
        g_source_set_callback(m_childStdOutSource, (GSourceFunc)cbChildProgress, this, NULL);
        MainApp::instance().attach(m_childStdOutSource);

        guint sourceId = g_child_watch_add_full(G_PRIORITY_DEFAULT_IDLE, childPid, cbChildComplete, this, NULL);

        m_pid = childPid;
        m_sourceId = sourceId;
        m_funcProgress = cbProgress;
        m_funcComplete = cbComplete;
        m_locked = true;

        return SUCCESS;
    }

    if (gerr) {
        LOG_ERROR(MSGID_APPREMOVE_FAIL, 2,
                  PMLOGKS(REASON,"Failed to execute ApplicationInstallerUtility command"),
                  PMLOGKS(LOGKEY_ERRTEXT,gerr->message),
                  "");

        g_error_free(gerr);
    }

    return FAIL;
}

bool AppInstallerUtility::cancel()
{
    if (m_pid == -1)
        return false;

    LOG_DEBUG("[AppInstallerUtility] kill: %d", m_pid);
    int result = System::kill(m_pid);
    LOG_DEBUG("[AppInstallerUtility] killed: %d", result);

    m_funcProgress = nullptr;

    clear();

    return true;
}

void AppInstallerUtility::clear()
{
    if (m_childStdOutChannel) {
        g_io_channel_unref(m_childStdOutChannel);
        m_childStdOutChannel = NULL;
    }
    if (m_childStdOutSource) {
        g_source_destroy(m_childStdOutSource);
        g_source_unref(m_childStdOutSource);
        m_childStdOutSource = NULL;
    }

    m_sourceId = 0;
    m_pid = -1;
}

bool AppInstallerUtility::isLocked() const
{
    return m_locked;
}

void AppInstallerUtility::restore(std::string installBasePath)
{
    if (!installBasePath.empty())
        Utils::remove_file(installBasePath + Settings::instance().getOpkgLockFilePath());
}
