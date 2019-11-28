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

#ifndef APPINSTALLERUTILITY_H
#define APPINSTALLERUTILITY_H

#include <functional>
#include <glib.h>
#include <string>

//! This class for wrapping @WEBOS_INSTALL_BINDIR@/ApplicationInstallerUtility
class AppInstallerUtility {
    typedef std::function<void (const char*)> FuncProgress;
    typedef std::function<void (int)> FuncComplete;

public:
    typedef enum {
        SUCCESS = 0,
        FAIL,
        LOCKED
    } Result;

    //! Construcor
    AppInstallerUtility();

    //! Destructor
    ~AppInstallerUtility();

    //! request install to @WEBOS_INSTALL_BINDIR@/ApplicationInstallerUtility
    Result install(std::string target,
                   unsigned int uncompressedSizeInKB,
                   bool verify,
                   bool allowDowngrade,
                   bool allowReInstall,
                   std::string installBasePath,
                   FuncProgress cbProgress,
                   FuncComplete cbComplete);

    //! request remove to @WEBOS_INSTALL_BINDIR@/ApplicationInstallerUtility
    Result remove(std::string appId,
                  bool verify,
                  std::string installBasePath,
                  FuncProgress cbProgress,
                  FuncComplete cbComplete);

    //! cancel processing
    bool cancel();

    //! clear resource
    void clear();

protected:
    //! watch function for child progress
    static gboolean cbChildProgress(GIOChannel *channel, GIOCondition condition, gpointer user_data);

    //! watch function for child complete
    static void cbChildComplete(GPid pid, gint status, gpointer data);

    /*! checks it's locked or not
     * opkg can handle only one command at once
     */
    bool isLocked() const;

    /*! in case of lock file is remained
     * all of install commands will fail
     * remove lock file if it's not created by appinstalld
     */
    void restore(std::string installBasePath);

private:
    static bool m_locked;

    GIOChannel* m_childStdOutChannel;
    GSource* m_childStdOutSource;
    guint m_sourceId;
    GPid m_pid;

    FuncProgress m_funcProgress;
    FuncComplete m_funcComplete;
};

#endif
