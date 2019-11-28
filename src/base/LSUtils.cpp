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

#include "JUtil.h"
#include "Logging.h"
#include "LSUtils.h"
#include "ServiceBase.h"
#include "Utils.h"

#include <pbnjson.hpp>

using namespace std::placeholders;

LSCaller::LSCaller(LSHandle *handle)
    : m_handle(handle)
{
}

bool LSCaller::Call(const char *uri,
                    const char *payload,
                    LSFilterFunc callback,
                    void *user_data,
                    LSMessageToken *ret_token,
                    std::string &errorText)
{
    if (m_handle == NULL) {
        errorText = "LSHandle is NULL";
        return false;
    }

    LS::Error lserror;
    if (!LSCall(m_handle, uri, payload, callback, user_data, ret_token, lserror)) {
        errorText = lserror.what();
        return false;
    }

    return true;
}

bool LSCaller::CallOneReply(const char *uri,
                            const char *payload,
                            LSFilterFunc callback,
                            void *user_data,
                            LSMessageToken *ret_token,
                            std::string &errorText,
                            int timeout)
{
    if (m_handle == NULL) {
        errorText = "LSHandle is NULL";
        return false;
    }

    LS::Error lserror;
    LSMessageToken token = 0;
    if (!LSCallOneReply(m_handle, uri, payload, callback, user_data, &token, lserror)) {
        errorText = lserror.what();
        return false;
    }

    if (timeout != 0) {
        LSCallSetTimeout(m_handle, token, timeout, NULL);
    }

    if (ret_token)
        *ret_token = token;

    return true;
}

bool LSCaller::CallCancel(LSMessageToken token, std::string &errorText)
{
    if (m_handle == NULL) {
        errorText = "LSHandle is NULL";
        return false;
    }

    LS::Error lserror;
    if (!LSCallCancel(m_handle, token, lserror)) {
        errorText = lserror.what();
        return false;
    }

    return true;
}

bool LSCaller::replySubscription(const char *key, const char *payload)
{
    if (!m_handle)
        return false;

    LS::Error lserror;
    if (!LSSubscriptionReply(m_handle, key, payload, lserror)) {
        lserror.log(getPmLogContext(), MSGID_LSCALL_ERR);
        return false;
    }

    return true;
}

LSCaller LSUtils::acquireCaller(std::string serviceName)
{
    return LSUtils::instance()._acquireCaller(serviceName);
}

std::string LSUtils::getCallerId(Message *LSRequest)
{
    const char *name = LSRequest->getApplicationID();
    if (!name)
        name = LSRequest->getSenderServiceName();
    if (!name)
        return std::string("");

    std::vector<std::string> strs;
    std::istringstream iss(name);
    std::copy(std::istream_iterator<std::string>(iss),
              std::istream_iterator<std::string>(),
              std::back_inserter(strs));

    if (strs.empty())
        return std::string("");

    return strs[0];
}

bool LSUtils::replyError(Message *LSRequest, int errorCode, std::string errorText)
{
    pbnjson::JValue reply = pbnjson::Object();
    if (reply.isNull())
        return false;

    reply.put("returnValue", false);
    reply.put("errorCode", errorCode);
    reply.put("errorText", errorText);
    reply.put("subscribed", false);

    LSRequest->respond(JUtil::toSimpleString(reply).c_str());

    return true;
}

bool LSUtils::_registerService(ServiceBase *service)
{
    m_mapService.insert( std::pair<std::string, ServiceBase*>(service->get_service_name(), service) );
    return true;
}

bool LSUtils::_unregisterService(ServiceBase *service)
{
    std::map<std::string, ServiceBase*>::iterator it = m_mapService.find(service->get_service_name());
    if (it != m_mapService.end()) {
        m_mapService.erase(it);
        return true;
    }
    return false;
}

LSCaller LSUtils::_acquireCaller(std::string serviceName)
{
    std::map<std::string, ServiceBase*>::iterator it = m_mapService.find(serviceName);
    if (it == m_mapService.end())
        return LSCaller(NULL);

    return LSCaller(it->second->get());
}

