// Copyright (c) 2017-2020 LG Electronics, Inc.
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

#include "GetIpkInfoStep.h"

#include "base/JUtil.h"
#include "base/SessionList.h"
#include "installer/Task.h"
#include "settings/Settings.h"

using namespace std::placeholders;

GetIpkInfoStep::GetIpkInfoStep()
{
}

GetIpkInfoStep::~GetIpkInfoStep()
{
    LOG_DEBUG("GetIpkInfoStep::~GetIpkInfoStep called\n");
}

bool GetIpkInfoStep::proceed(Task *task)
{
    LOG_DEBUG("GetIpkInfoStep::proceed() called\n");
    m_parentTask = task;

    CallChain& callchain = CallChain::acquire(std::bind(&GetIpkInfoStep::onAppInfo,
        this, _1, _2));

    const char *sessionId = nullptr;

#if defined(WEBOS_TARGET_DISTRO_WEBOS_AUTO)
    if (SessionList::getInstance().empty()) {
        // TODO Cannot query getAppInfo
        task->setError(ErrorInstall, APP_INSTALL_ERR_GENERAL, "empty session list, cannot query getAppInfo");
        task->proceed();
        return false;
    } else {
        sessionId = SessionList::getInstance().at(0).c_str();
    }
#endif

    auto itemAppInfo = std::make_shared<CallChainEventHandler::AppInfo>(
        "com.webos.appInstallService",
        sessionId,
        task->getPackageId()
    );

    task->setStep(GetIpkInfoRequested);

    return callchain
        .add(itemAppInfo)
        .run();
}

void GetIpkInfoStep::onAppInfo(pbnjson::JValue result, void *user_data)
{
    LOG_DEBUG("GetIpkInfoStep::onAppInfo() called\n");

    m_parentTask->setOriginAppInfo(result["appInfo"]);
    m_parentTask->setStep(GetIpkInfoComplete);
    m_parentTask->proceed();
}
