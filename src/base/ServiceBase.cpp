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

#include "Logging.h"
#include "LSUtils.h"
#include "ServiceBase.h"

ServiceBase::ServiceBase(std::string serviceName)
    : LS::Handle(LS::registerService(serviceName.c_str()))
{
}

ServiceBase::~ServiceBase()
{
}

bool ServiceBase::attach(GMainLoop * gml)
{
    try {
        attachToLoop(gml);
    } catch (const LS::Error &lserror) {
        LOG_ERROR(MSGID_LSCALL_ERR, 1, PMLOGKS("[serviceBase]-attach", lserror.what()), "");
        return false;
    }

    LOG_DEBUG("%s attached\n",get_service_name());

    LSUtils::instance()._registerService(this);
    onAttached();

    return true;
}

void ServiceBase::detach()
{
    try {
        LS::Handle::detach();
    } catch (const LS::Error &lserror) {
        LOG_ERROR(MSGID_SRVC_DETACH_FAIL, 1, PMLOGKS("[serviceBase]-detach", lserror.what()), "");
        return;
    }

    LSUtils::instance()._unregisterService(this);

    onDetached();
}
