// Copyright (c) 2016-2019 LG Electronics, Inc.
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

#include "base/Logging.h"
#include "settings/Settings.h"
#include "Jailer.h"

bool Jailer::remove(std::string appId, std::function<void(bool)> onRemove)
{
    gchar* argv[16] = { 0 }; ///WARNING! look out below if number of params goes > size of this array (keep them in sync)
    GError* gerr = NULL;
    GSpawnFlags flags = (GSpawnFlags) (G_SPAWN_SEARCH_PATH | G_SPAWN_DO_NOT_REAP_CHILD);

    GPid childPid;
    gboolean result;
    int index = 0;

    argv[index++] = (gchar *) Settings::instance().getJailerPath().c_str();
    argv[index++] = (gchar *) "-D";
    argv[index++] = (gchar *) "-i";
    argv[index++] = (gchar *) appId.c_str();
    argv[index] = NULL;

    result = g_spawn_async(NULL,
                           argv,
                           NULL,
                           flags,
                           NULL,
                           NULL,
                           &childPid,
                           &gerr);

    if (result) {
        g_child_watch_add(childPid, cbRemoveComplete, this);
        m_funcComplete = onRemove;

        return true;
    }

    if (gerr) {
        LOG_ERROR(MSGID_APPREMOVE_FAIL, 2,
                  PMLOGKS(REASON,"Failed to execute jailer command"),
                  PMLOGKS(LOGKEY_ERRTEXT,gerr->message),
                  " ");

        g_error_free(gerr);
    }

    return false;
}

void Jailer::cbRemoveComplete(GPid pid, gint status, gpointer user_data)
{
    LOG_DEBUG("child pid %d removed jailer directories with status %d", pid, status);

    Jailer *jailer = reinterpret_cast<Jailer*>(user_data);
    if (!jailer)
        return;

    if (!WIFEXITED(status) || (WEXITSTATUS(status) != 0)) {
        jailer->m_funcComplete(false);
        return;
    }

    jailer->m_funcComplete(true);
}
