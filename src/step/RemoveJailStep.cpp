// Copyright (c) 2017-2018 LG Electronics, Inc.
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

#include "RemoveJailStep.h"

using namespace std::placeholders;

RemoveJailStep::RemoveJailStep()
{
}

RemoveJailStep::~RemoveJailStep()
{
    LOG_DEBUG("RemoveJailStep::~RemoveNeededStep called\n");
}

bool RemoveJailStep::proceed(Task *task)
{
    LOG_DEBUG("RemoveJailStep::proceed() called\n");

    m_parentTask = task;

    if(!m_jailer.remove(m_parentTask->getAppId(),std::bind(&RemoveJailStep::onRemoveJail, this, _1)))
    {
        LOG_WARNING(MSGID_JAILER_REMOVE_FAIL, 2,
            PMLOGKS(APP_ID, m_parentTask->getAppId().c_str()),
            PMLOGKS(LOGKEY_ERRTEXT, "unable to call Jailer"), " ");

        Utils::async([=] { onRemoveJail(false); });
    }

    m_parentTask->setStep(RemoveJailRequested);

    return true;
}

void RemoveJailStep::onRemoveJail(bool success)
{
    LOG_DEBUG("RemoveJailStep::onRemoveJail() called\n");

    if(!success)
    {
        LOG_WARNING(MSGID_JAILER_REMOVE_FAIL, 2,
            PMLOGKS(APP_ID, m_parentTask->getAppId().c_str()),
            PMLOGKS(LOGKEY_ERRTEXT, "failed to remove jailer directory"), " ");
    }

    m_parentTask->setStep(RemoveJailComplete);
    m_parentTask->proceed();
}
