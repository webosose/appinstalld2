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

#include "InstallSmackStep.h"

#include "base/Logging.h"
#include "installer/AppInfo.h"
#include "installer/ServiceInfo.h"
#include "installer/PackageInfo.h"
#include "installer/InstallHistory.h"
#include "installer/Task.h"
#include "settings/Settings.h"
#include "settings/Smack.h"

InstallSmackStep::InstallSmackStep()
{
}

InstallSmackStep::~InstallSmackStep()
{
    LOG_DEBUG("InstallSmackStep::~InstallSmackStep() called\n");
}

bool InstallSmackStep::proceed(Task *task) {
    LOG_DEBUG("InstallSmackStep::proceed() called\n");

    m_parentTask = task;
    std::string packageId = m_parentTask->getPackageId();
    std::string applicationPath = m_parentTask->getInstallBasePath() + Settings::instance().getApplicationInstallPath() + "/" + packageId;
    std::string packagePath = m_parentTask->getInstallBasePath() + Settings::instance().getPackageinstallPath() + "/" + packageId;
    std::string label;
    std::vector<std::string> serviceLists;
    AppInfo appInfo(applicationPath);
    PackageInfo packageInfo(packagePath);

    GSpawnFlags flags = (GSpawnFlags)(G_SPAWN_SEARCH_PATH);
    gchar * argv[9] = {0};
    GError * gerr = NULL;
    gint exit_status = 0;
    gboolean resultStatus;
    int index = 0;

    /* Skip SMACK labeling if app is stub or smack off */
    if (appInfo.isStub()) {
        m_parentTask->setStep(InstallSmackComplete);
        m_parentTask->proceed();
        return true;
    }

    label = SMACK_EXEC_PREFFIX + packageId;

    argv[index++] = (gchar *)CHSMACK_EXEC;
    argv[index++] = (gchar *)"-r";
    argv[index++] = (gchar *)"-t";
    argv[index++] = (gchar *)"-a";
    argv[index++] = (gchar *)label.c_str();
    if (appInfo.isNative()) {
        argv[index++] = (gchar *)"-e";
        argv[index++] = (gchar *)label.c_str();
    }
    argv[index++] = (gchar *)applicationPath.c_str();
    argv[index++] = NULL;

    LOG_DEBUG("Executing %s %s %s %s %s %s", argv[0], argv[1], argv[2], argv[3], argv[4], argv[5]);
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
    LOG_DEBUG("chsmack result status is %d, exit status is %d", (int)resultStatus, (int)WEXITSTATUS(exit_status));

    if (gerr) {
        LOG_ERROR(MSGID_INSTALL_SMACK_FAIL, 2,
                  PMLOGKS(REASON, "Failed to execute chsmack"),
                  PMLOGKS(LOGKEY_ERRTEXT,gerr->message),
                  " ");
        g_error_free(gerr);
    }

    if (!resultStatus || exit_status) {
        LOG_DEBUG("Non-zero exit code from chsmack.");
        m_parentTask->setError(ErrorInstall, APP_INSTALL_ERR_SMACK , "unable to execute chsmack command");
        m_parentTask->proceed();
        return false;
    }

    std::string rulesFilePath = SMACK_RULES_DIR + packageId;
    if (!g_file_test(rulesFilePath.c_str(), G_FILE_TEST_IS_REGULAR)) {
        g_mkdir_with_parents(SMACK_RULES_DIR, 0755);
        index = 0;
        argv[index++] = (gchar *)SMACK_RULES_GEN_EXEC;
        if (appInfo.isWeb()) argv[index++] = (gchar *)"-bw";
        else if (appInfo.isQml()) argv[index++] = (gchar *)"-bq";
        else if (appInfo.isNative()) argv[index++] = (gchar *)"-bn";
        argv[index++] = (gchar *)packageId.c_str();
        argv[index++] = (gchar *)"-o";
        argv[index++] = (gchar *)rulesFilePath.c_str();
        argv[index++] = NULL;
        LOG_DEBUG("Executing %s %s %s %s %s", argv[0], argv[1], argv[2], argv[3], argv[4]);
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
        LOG_DEBUG("smack_rules_gen result status is %d, exit status is %d", (int)resultStatus, (int)WEXITSTATUS(exit_status));

        if (gerr) {
            LOG_ERROR(MSGID_INSTALL_SMACK_FAIL, 2,
                      PMLOGKS(REASON, "Failed to execute smack_rule_gen"),
                      PMLOGKS(LOGKEY_ERRTEXT,gerr->message),
                      " ");
            g_error_free(gerr);
        }

        if (!resultStatus || exit_status) {
            LOG_DEBUG("Non-zero exit code from smack rules generator.");
            m_parentTask->setError(ErrorInstall, APP_INSTALL_ERR_SMACK , "unable to execute smack_rules_gen command");
            m_parentTask->proceed();
            return false;
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
    }


    if (packageInfo.isLoaded())
        packageInfo.getServices(serviceLists);

    for (auto iter = serviceLists.begin(); iter != serviceLists.end(); ++iter) {
        std::string servicePath = m_parentTask->getInstallBasePath() + Settings::instance().getServiceinstallPath() + "/" + (*iter);
        ServiceInfo serviceInfo(servicePath);
        std::string serviceId = serviceInfo.getId();

        if (serviceInfo.getType() != "native"){
            label = SMACK_SERVICE_PREFFIX + serviceId;

            index = 0;
            argv[index++] = (gchar *)CHSMACK_EXEC;
            argv[index++] = (gchar *)"-r";
            argv[index++] = (gchar *)"-t";
            argv[index++] = (gchar *)"-a";
            argv[index++] = (gchar *)label.c_str();
            argv[index++] = (gchar *)servicePath.c_str();
            argv[index++] = NULL;

            LOG_DEBUG("Executing %s %s %s %s %s %s", argv[0], argv[1], argv[2], argv[3], argv[4], argv[5]);
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
            LOG_DEBUG("chsmack result status is %d, exit status is %d", (int)resultStatus, (int)WEXITSTATUS(exit_status));

            if (gerr) {
                LOG_ERROR(MSGID_INSTALL_SMACK_FAIL, 2,
                        PMLOGKS(REASON, "Failed to execute chsmack"),
                        PMLOGKS(LOGKEY_ERRTEXT,gerr->message),
                        " ");
                g_error_free(gerr);
            }

            if (!resultStatus || exit_status) {
                LOG_DEBUG("Non-zero exit code from chsmack.");
                m_parentTask->setError(ErrorInstall, APP_INSTALL_ERR_SMACK , "unable to execute chsmack command");
                m_parentTask->proceed();
                return false;
            }

            std::string rulesFilePath = SMACK_RULES_DIR + serviceId;
            if (!g_file_test(rulesFilePath.c_str(), G_FILE_TEST_IS_REGULAR)) {
                g_mkdir_with_parents(SMACK_RULES_DIR, 0755);
                index = 0;
                argv[index++] = (gchar *)SMACK_RULES_GEN_EXEC;
                argv[index++] = (gchar *)"-bs";
                argv[index++] = (gchar *)serviceId.c_str();
                argv[index++] = (gchar *)"-o";
                argv[index++] = (gchar *)rulesFilePath.c_str();
                argv[index++] = NULL;
                LOG_DEBUG("Executing %s %s %s %s %s", argv[0], argv[1], argv[2], argv[3], argv[4]);
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
                LOG_DEBUG("smack_rules_gen result status is %d, exit status is %d", (int)resultStatus, (int)WEXITSTATUS(exit_status));

                if (gerr) {
                    LOG_ERROR(MSGID_INSTALL_SMACK_FAIL, 2,
                            PMLOGKS(REASON, "Failed to execute smack_rule_gen"),
                            PMLOGKS(LOGKEY_ERRTEXT,gerr->message),
                            " ");
                    g_error_free(gerr);
                }

                if (!resultStatus || exit_status) {
                    LOG_DEBUG("Non-zero exit code from smack rules generator.");
                    m_parentTask->setError(ErrorInstall, APP_INSTALL_ERR_SMACK , "unable to execute smack_rules_gen command");
                    m_parentTask->proceed();
                    return false;
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
            }
        }
    }
    m_parentTask->setStep(InstallSmackComplete);
    m_parentTask->proceed();
    return true;
}

