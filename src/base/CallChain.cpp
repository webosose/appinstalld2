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

#include <algorithm>

#include "CallChain.h"
#include "LSUtils.h"
#include "JUtil.h"
#include "Utils.h"

using namespace std::placeholders;

static pbnjson::JValue makeResult(bool returnValue, std::string errorText)
{
    pbnjson::JValue result = pbnjson::Object();
    result.put("returnValue", returnValue);
    if (!returnValue)
        result.put("errorText", errorText);
    return result;
}

CallItem::CallItem()
    : m_chainData(pbnjson::Object()),
      m_option(0)
{
}

CallItem::~CallItem()
{
}

void CallItem::setError(std::string errorText)
{
    m_errorText = errorText;
}

std::string CallItem::getError() const
{
    return m_errorText;
}

void CallItem::setOption(uint32_t option)
{
    m_option = option;
}

uint32_t CallItem::getOption() const
{
    return m_option;
}

void CallItem::setChainData(pbnjson::JValue chainData)
{
    m_chainData = chainData.duplicate();
}

pbnjson::JValue CallItem::getChainData() const
{
    return m_chainData.duplicate();
}

LSCallItem::LSCallItem(const char *serviceName, const char *uri, const char *payload)
    : m_serviceName(serviceName),
      m_uri(uri),
      m_payload(payload)
{
}

bool LSCallItem::Call()
{
    if (!onBeforeCall()) {
        onError("Cancelled");
        return false;
    }

    std::string errorText;
    LSCaller caller = LSUtils::acquireCaller(m_serviceName);
    if (!caller.CallOneReply(m_uri.c_str(), m_payload.c_str(), LSCallItem::handler, this, NULL, errorText)) {
        onError(errorText.c_str());
        return false;
    }

    return true;
}

bool LSCallItem::onBeforeCall()
{
    return true;
}

bool LSCallItem::onReceiveCall(pbnjson::JValue message)
{
    return true;
}

void LSCallItem::setServiceName(const char *serviceName)
{
    m_serviceName = serviceName;
}

void LSCallItem::setUri(const char *uri)
{
    m_uri = uri;
}

void LSCallItem::setPayload(const char *payload)
{
    m_payload = payload;
}

bool LSCallItem::handler(LSHandle *lshandle, LSMessage *message, void *user_data)
{
    LSCallItem *call = reinterpret_cast<LSCallItem*>(user_data);
    if (!call)
        return false;

    pbnjson::JValue json = JUtil::parse(LSMessageGetPayload(message), std::string(""));

    bool result = call->onReceiveCall(json);

    call->onFinished(result, call->getError());

    return result;
}

CallChain& CallChain::acquire(CallCompleteHandler handler, void *user_data)
{
    CallChain *chain = new CallChain(handler, user_data);
    return *chain;
}

CallChain::CallChain(CallCompleteHandler handler, void *user_data)
    : m_handler(handler),
      m_user_data(user_data)
{
}

CallChain::~CallChain()
{
}

CallChain& CallChain::add(CallItemPtr call, bool push_front)
{
    call->onFinished.disconnect_all_slots();
    call->onError.disconnect_all_slots();

    call->onFinished.connect(std::bind(&CallChain::onCallFinished, this, _1, _2));
    call->onError.connect(std::bind(&CallChain::onCallError, this, _1));

    if (push_front) m_calls.push_front(call);
    else m_calls.push_back(call);
    return *this;
}

CallChain& CallChain::add_if(CallItemPtr condition_call, bool expected_result, CallItemPtr target_call)
{
    m_conditions.push_back(std::make_shared<CallCondition>(condition_call, expected_result, target_call));
    return *this;
}

bool CallChain::run(pbnjson::JValue chainData)
{
    return proceed(chainData);
}

bool CallChain::proceed(pbnjson::JValue chainData)
{
    if (m_calls.empty()) {
        chainData.put("returnValue", true);
        finish(chainData);
        return true;
    }

    CallItemPtr call = m_calls.front();

    call->setChainData(chainData);
    if (!call->Call()) {
        return false;
    }

    return true;
}

void CallChain::finish(pbnjson::JValue chainData)
{
    if (m_handler)
        m_handler(chainData, m_user_data);

    Utils::async([=] { delete this; });
}

void CallChain::onCallError(std::string errorText)
{
    CallItemPtr call = m_calls.front();
    if (!call) {
        finish(makeResult(false, errorText));
    } else {
        pbnjson::JValue chainData = call->getChainData();
        chainData.put("returnValue", false);
        chainData.put("errorText", errorText);
        finish(chainData);
    }
}

void CallChain::onCallFinished(bool result, std::string errorText)
{
    CallItemPtr call = m_calls.front();
    if ((m_calls.size() == 0) || (!call)) {
        finish(makeResult(false, "Callchain broken"));
        return;
    }

    if (!errorText.empty()) {
        pbnjson::JValue chainData = call->getChainData();
        chainData.put("returnValue", false);
        chainData.put("errorText", errorText);
        finish(chainData);
        return;
    }
    m_calls.pop_front();

    bool processNext = result;

    std::vector<CallConditionPtr>::iterator it =
        std::find_if(m_conditions.begin(),
                     m_conditions.end(),
                     [=] (const CallConditionPtr p) -> bool {return p->condition_call == call;});

    if (it != m_conditions.end()) {
        if (result == (*it)->expected_result) {
            add((*it)->target_call, true);
            processNext = true;
        }

        m_conditions.erase(it);
    }

    if (!processNext &&
        ((call->getOption() & CallItem::OPTION_NONSTOP) == CallItem::OPTION_NONSTOP))
        processNext = true;

    if (processNext) {
        proceed(call->getChainData());
    } else {
        pbnjson::JValue chainData = call->getChainData();
        chainData.put("returnValue", false);
        chainData.put("errorText", errorText);
        finish(chainData);
    }
}
