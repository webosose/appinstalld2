// Copyright (c) 2020 LG Electronics, Inc.
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

#include "RemoveSmackStep.h"
#include "installer/ServiceInfo.h"
#include "installer/PackageInfo.h"
#include "installer/AppInfo.h"
#include "base/Logging.h"
#include "settings/Settings.h"
#include "installer/InstallHistory.h"
#include "installer/Task.h"
#include "settings/Smack.h"

RemoveSmackStep::RemoveSmackStep()
{
}

RemoveSmackStep::~RemoveSmackStep()
{
    LOG_DEBUG("RemoveSmackStep::~RemoveSmackStep() called\n");
}

bool RemoveSmackStep::proceed(Task *task)
{
    LOG_DEBUG("RemoveSmackStep::proceed() called\n");

    std::string prefix;
    std::string mountExec = "mount";
    std::vector<std::string> rulePaths;
    GSpawnFlags flags = (GSpawnFlags)(G_SPAWN_SEARCH_PATH);
    gchar * argv[9] = {0};
    GError * gerr = NULL;
    gint exit_status = 0;
    gboolean resultStatus;
    int index = 0;

    m_parentTask = task;

    if (g_file_test((SMACK_RULES_OVERLAY + m_parentTask->getAppId()).c_str(), G_FILE_TEST_EXISTS)) {
        prefix = SMACK_RULES_OVERLAY;
    } else {
        prefix = SMACK_RULES_DIR;
    }

    rulePaths.push_back(prefix + m_parentTask->getAppId());

    pbnjson::JValue param = task->getParam();
    bool verify = param["verify"].asBool();
    std::vector<std::string> serviceLists;
    std::string packagePath = Settings::instance().getInstallPath(verify) +
            Settings::instance().getPackageinstallPath() +
            std::string("/") + m_parentTask->getAppId();

    PackageInfo packageInfo(packagePath);
    if (packageInfo.isLoaded())
        packageInfo.getServices(serviceLists);

    for (auto iter = serviceLists.begin(); iter != serviceLists.end(); ++iter) {
        std::string servicePath = Settings::instance().getInstallServicePath(verify) + "/" + (*iter);
        ServiceInfo serviceInfo(servicePath);
        rulePaths.push_back(prefix + serviceInfo.getId());
    }

    // check dev services
    if (verify && serviceLists.empty()) {
        std::string packagePathDev = Settings::instance().getInstallPath(false) +
            Settings::instance().getPackageinstallPath() +
            std::string("/") + m_parentTask->getAppId();

        PackageInfo packageInfoDev(packagePathDev);
        if (packageInfoDev.isLoaded())
            packageInfoDev.getServices(serviceLists);

        for (auto iter = serviceLists.begin(); iter != serviceLists.end(); ++iter) {
            std::string servicePath = Settings::instance().getInstallServicePath(false) + "/" + (*iter);
            ServiceInfo serviceInfo(servicePath);
            rulePaths.push_back(prefix + serviceInfo.getId());
        }
    }

    for (auto iter = rulePaths.begin(); iter != rulePaths.end(); ++iter) {
        std::string& rulePath = (*iter);
        if (g_file_test(rulePath.c_str(), G_FILE_TEST_EXISTS)) {
            if (remove(rulePath.c_str())) {
                LOG_WARNING(MSGID_REMOVE_SMACK_FAIL, 3,
                    PMLOGKS(APP_ID, m_parentTask->getAppId().c_str()),
                    PMLOGKS(PATH, rulePath.c_str()),
                    PMLOGKS(LOGKEY_ERRTEXT, "Cannot remove SMACK rules"), "");
            }
        }
    }
    if (prefix == SMACK_RULES_OVERLAY) {
        index = 0;
        argv[index++] = (gchar *)mountExec.c_str();
        argv[index++] = (gchar *)"-o";
        argv[index++] = (gchar *)"remount";
        argv[index++] = (gchar *)SMACK_RULES_DIR;
        argv[index++] = NULL;
        LOG_DEBUG("Executing %s %s %s %s", argv[0], argv[1], argv[2], argv[3]);
        resultStatus = g_spawn_sync(NULL,
                                    argv,
                                    NULL,
                                    flags,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL,
                                    &exit_status,
                                    &gerr);
        LOG_DEBUG("%s result status is %d, exit status is %d", mountExec.c_str(), (int)resultStatus, (int)WEXITSTATUS(exit_status));

        if (gerr) {
            LOG_ERROR(MSGID_INSTALL_SMACK_FAIL, 2,
                PMLOGKS(REASON, "Failed to execute mount"),
                PMLOGKS(LOGKEY_ERRTEXT,gerr->message),
                "");
            g_error_free(gerr);
        }

        if (!resultStatus || exit_status) {
            LOG_DEBUG("Non-zero exit code from mount.");
            m_parentTask->setError(ErrorInstall, APP_INSTALL_ERR_SMACK, "unable to execute mount command");
            m_parentTask->proceed();
            return false;
        }
    }

    index = 0;
    argv[index++] = (gchar *)SMACKCTL_EXEC;
    argv[index++] = (gchar *)"apply";
    argv[index++] = NULL;
    LOG_DEBUG("Executing %s %s", argv[0], argv[1]);
    resultStatus = g_spawn_sync(NULL,
                                argv,
                                NULL,
                                flags,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                &exit_status,
                                &gerr);
    LOG_DEBUG("smackctl result status is %d, exit status is %d", (int)resultStatus, (int)WEXITSTATUS(exit_status));

    if (gerr) {
        LOG_ERROR(MSGID_INSTALL_SMACK_FAIL, 2,
            PMLOGKS(REASON, "Failed to execute smackctl"),
            PMLOGKS(LOGKEY_ERRTEXT,gerr->message),
            "");
        g_error_free(gerr);
    }

    if (!resultStatus || exit_status) {
        LOG_DEBUG("Non-zero exit code from smackctl.");
        m_parentTask->setError(ErrorInstall, APP_INSTALL_ERR_SMACK, "unable to execute smackctl command");
        m_parentTask->proceed();
        return false;
    }

    m_parentTask->setStep(RemoveSmackComplete);
    m_parentTask->proceed();
    return true;
}

