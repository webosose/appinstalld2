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

#ifndef _LSUTILS_H_
#define _LSUTILS_H_

#include <algorithm>
#include <iterator>
#include <luna-service2/lunaservice.hpp>
#include <map>
#include <string>
#include <sstream>

#include "base/ServiceBase.h"
#include "Singleton.hpp"

namespace pbnjson
{
    class JValue;
}

using namespace LS;

#define LS_CATEGORY_TABLE_NAME(name) name##_table

#define LS_CREATE_CLASS_CATEGORY_BEGIN(cl, name) \
    constexpr static const LSMethod LS_CATEGORY_TABLE_NAME(name)[] = {

#define LS_CREATE_CATEGORY_BEGIN(cl, name) \
    typedef cl cl_t; \
    constexpr static const LSMethod LS_CATEGORY_TABLE_NAME(name)[] = {

#define LS_CATEGORY_MAPPED_METHOD(name, func) { #name, \
    &LS::Handle::methodWraper<cl_t, &cl_t::func>, \
    static_cast<LSMethodFlags>(0) },

#define LS_CATEGORY_CLASS_METHOD(cls, name) { #name, \
    &LS::Handle::methodWraper<cls, &cls::name>, \
    static_cast<LSMethodFlags>(0) },

#define LS_CREATE_CATEGORY_END \
{ nullptr, nullptr } \
};

class LSCaller {
public:
    //! Constructor
    LSCaller(LSHandle *handle);

    //! Call LSCall
    bool Call(const char *uri,
              const char *payload,
              LSFilterFunc callback,
              void *user_data,
              LSMessageToken *ret_token,
              std::string &errorText);

    //! Call LSCallOneReply
    bool CallOneReply(const char *uri,
                      const char *payload,
                      LSFilterFunc callback,
                      void *user_data,
                      LSMessageToken *ret_token,
                      std::string &errorText,
                      int timeout = 0);

    //! CallCancel
    bool CallCancel(LSMessageToken token, std::string &errorText);

    /*! Reply message to public and private service subscribers
     * Subscribers is selected by key
     */
    bool replySubscription(const char *key, const char *payload);

private:
    LSHandle *m_handle;
};

class ServiceBase;

//! List of utilites for luna-service
class LSUtils : public Singleton<LSUtils> {
public:
    //! Retrieve ServiceHandle for given service name
    static LSCaller acquireCaller(std::string serviceName);

    //! Parse caller from msg
    static std::string getCallerId(Message *LSRequest);

    //! Reply error
    static bool replyError(Message *LSRequest, int errorCode, std::string errorText);

private:
    friend class ServiceBase;

    //! Register service
    bool _registerService(ServiceBase *service);

    //! Unregister service
    bool _unregisterService(ServiceBase *service);

    //! Retrieve LSCaller for given service name
    LSCaller _acquireCaller(std::string serviceName);

private:
    std::map<std::string, ServiceBase*> m_mapService;
};

#endif
