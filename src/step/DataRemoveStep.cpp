// Copyright (c) 2017-2024 LG Electronics, Inc.
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

#include "DataRemoveStep.h"
#include "base/SessionList.h"
#include "installer/Task.h"

using namespace std::placeholders;

DataRemoveStep::DataRemoveStep()
{
}

DataRemoveStep::~DataRemoveStep()
{
    LOG_DEBUG("DataRemoveStep::~DataRemoveStep() called\n");
}

bool DataRemoveStep::proceed(Task *task)
{
    LOG_DEBUG("DataRemoveStep::proceed() called\n");

    m_parentTask = task;

    CallChain& callchain = CallChain::acquire(std::bind(&DataRemoveStep::onDataRemoved,
        this, _1, _2));


    pbnjson::JValue dbOwners = pbnjson::Array();
    dbOwners.append(m_parentTask->getAppId());
    std::vector<std::string> services = m_parentTask->getServices();

    for(auto it = services.begin(); services.end() != it; ++it)
    {
        dbOwners.append(*it);
    }

#if defined(ENABLE_SESSION)
    size_t size = SessionList::getInstance().size();
    for (size_t i = 0; i < size; ++i) {
        const std::string& sessionId = SessionList::getInstance().at(i);
        auto itemRemoveDb = std::make_shared<CallChainEventHandler::RemoveDb>(
            "com.webos.appInstallService",
            sessionId.c_str(),
            dbOwners);
        callchain.add(itemRemoveDb);
    }
#else
    auto itemRemoveDb = std::make_shared<CallChainEventHandler::RemoveDb>(
        "com.webos.appInstallService",
        nullptr,
        dbOwners);
    callchain.add(itemRemoveDb);
#endif

    m_parentTask->setStep(DataRemoveRequested);
    return callchain.run();
}

bool DataRemoveStep::onDataRemoved(pbnjson::JValue result, void *user_data)
{
    pbnjson::JValue param = m_parentTask->getParam();
    bool verify = param["verify"].asBool();

    std::vector<std::string> dirs;
    if (verify)
        dirs.push_back(Settings::instance().getInstallApplicationPath(true) + "/" + m_parentTask->getAppId());
    dirs.push_back(Settings::instance().getInstallApplicationPath(false) + "/" + m_parentTask->getAppId());

    std::vector<std::string> services = m_parentTask->getServices();
    for(auto it = services.begin(); services.end() != it; ++it)
    {
        if (verify)
            dirs.push_back(Settings::instance().getInstallServicePath(true) + "/" + (*it));
        dirs.push_back(Settings::instance().getInstallServicePath(false) + "/" + (*it));
    }

    for(auto it = dirs.begin(); it != dirs.end(); ++it)
    {
        if (0 == access((*it).c_str(), F_OK))
            Utils::remove_dir(*it);
    }

    m_parentTask->setStep(DataRemoveComplete);
    m_parentTask->proceed();

    return true;
}
