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
#include "base/Logging.h"
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

    m_parentTask = task;
    std::string rules_file_path = SMACK_RULES_DIR + m_parentTask->getAppId();

    if (g_file_test(rules_file_path.c_str(), G_FILE_TEST_EXISTS)) {
        if (remove(rules_file_path.c_str())) {
            LOG_WARNING(MSGID_REMOVE_SMACK_FAIL, 3,
                PMLOGKS(APP_ID, m_parentTask->getAppId().c_str()),
                PMLOGKS(PATH, rules_file_path.c_str()),
                PMLOGKS(LOGKEY_ERRTEXT, "Cannot remove SMACK rules"), "");
        }
    }

    m_parentTask->setStep(RemoveSmackComplete);
    m_parentTask->proceed();
    return true;
}

