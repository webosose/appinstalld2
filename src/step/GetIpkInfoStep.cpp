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

#include "GetIpkInfoStep.h"
#include "base/JUtil.h"
#include "installer/Task.h"

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

    auto itemAppInfo = std::make_shared<CallChainEventHandler::AppInfo>(
        "com.webos.appInstallService",
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
