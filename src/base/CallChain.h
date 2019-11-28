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

#ifndef CALLCHAIN_H
#define CALLCHAIN_H

#include <boost/signals2.hpp>
#include <deque>
#include <luna-service2/lunaservice.h>
#include <memory>
#include <pbnjson.hpp>
#include <tuple>
#include <vector>

#include "Utils.h"

class CallChain;

//! This class is base class for CallChain class's item
class CallItem {
friend class CallChain;
public:
    typedef enum {
        OPTION_NONSTOP = 0x01,
    } OPTION;

    //! Constructor
    CallItem();

    virtual ~CallItem();

    //! Execute this item
    virtual bool Call() = 0;

    //! Set option
    void setOption(uint32_t option);

    //! Get option
    uint32_t getOption() const;

    //! It's called when item is finished
    boost::signals2::signal<void (bool, std::string)> onFinished;
    //! It's called when item has error
    boost::signals2::signal<void (std::string)> onError;

protected:
    //! Set error text
    void setError(std::string errorText);
    //! Get error text
    std::string getError() const;

    /*! Set chain data
     * It helps data delivering to next item
     */
    void setChainData(pbnjson::JValue value);
    /*! Get chain data
     * It's delivered from previous call item
     */
    pbnjson::JValue getChainData() const;

private:
    std::string m_errorText;
    pbnjson::JValue m_chainData;
    uint32_t m_option;
};

//! This class for call item using function
template <typename R, typename... T>
class FunctionCallItem : public CallItem {
public:
    //! Constructor
    FunctionCallItem(std::function<R (T...)> func)
        : m_func(func)
    {
    }

    //! Execute this item
    virtual bool Call()
    {
        if (!onBeforeCall()) {
            onError("Cancelled");
            return false;
        }

        R funcResult = Utils::apply_tuple(m_func, m_params);

        bool result = onResult(funcResult);
        Utils::async([=] { onFinished(result, getError()); });

        return true;
    }

protected:
    //! It's called when just before function call
    virtual bool onBeforeCall()
    {
        return true;
    }

    //! It's called when just after function call
    virtual bool onResult(R result)
    {
        return true;
    }

    //! get param value holder
    template <std::size_t I>
    typename std::tuple_element<I, std::tuple<T...> >::type& param()
    {
        return std::get<I>(m_params);
    }

private:
    std::function<R (T...)> m_func;
    std::tuple<T...> m_params;
};

//! This class for call item using luna-service call
class LSCallItem : public CallItem {
public:
    //! Constructor
    LSCallItem(const char *serviceName, const char *uri, const char *payload);

    //! Execute this item
    virtual bool Call();

protected:
    //! It's called when just before luna-service call
    virtual bool onBeforeCall();
    //! It's called when receive luna-service response
    virtual bool onReceiveCall(pbnjson::JValue message);

    //! Set service name
    void setServiceName(const char *serviceName);
    //! Set uri
    void setUri(const char *uri);
    //! Set payload
    void setPayload(const char *payload);

private:
    //! handler for luna-service response
    static bool handler(LSHandle *lshandle, LSMessage *message, void *user_data);

private:
    std::string m_serviceName;
    std::string m_uri;
    std::string m_payload;
};

//! This class helps call the items in consecutive order
class CallChain {
    typedef std::shared_ptr<CallItem> CallItemPtr;
    typedef std::function<void (pbnjson::JValue, void*)> CallCompleteHandler;

    //! This strcuture for checking call condition
    struct CallCondition {
        CallCondition(CallItemPtr _condition_call, bool _expected_result, CallItemPtr _target_call)
            : condition_call(_condition_call),
              expected_result(_expected_result),
              target_call(_target_call)
        {}

        CallItemPtr condition_call;
        bool expected_result;
        CallItemPtr target_call;
    };

    typedef std::shared_ptr<CallCondition> CallConditionPtr;

public:
    /*! Acquire new call chain
     * should run chain after acquire one
     */
    static CallChain& acquire(CallCompleteHandler handler = nullptr, void *user_data = NULL);

    //! Add call item
    CallChain& add(CallItemPtr call, bool push_front = false);
    //! Add call item if condition call returns expected_result
    CallChain& add_if(CallItemPtr condition_call, bool expected_result, CallItemPtr target_call);

    //! Run chain
    bool run(pbnjson::JValue chainData = pbnjson::Object());

private:
    //! Constructor
    CallChain(CallCompleteHandler handler, void *user_data);
    //! Destructor
    ~CallChain();

    //! Proceed next item
    bool proceed(pbnjson::JValue chainData);
    //! Finish chain
    void finish(pbnjson::JValue chainData);

    //! This is called when one item is finished
    void onCallFinished(bool result, std::string errorText);
    //! This is called when one item has error
    void onCallError(std::string errorText);

private:
    std::deque<CallItemPtr> m_calls;
    std::vector<CallConditionPtr> m_conditions;

    CallCompleteHandler m_handler;
    void *m_user_data;
};

#endif
