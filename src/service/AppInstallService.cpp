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

#include "AppInstallService.h"
#include "base/JUtil.h"
#include "base/Logging.h"
#include "base/LSUtils.h"
#include "base/Utils.h"
#include "installer/AppInstaller.h"
#include "settings/Settings.h"

using namespace std::placeholders;

AppInstallService::AppInstallService()
    : ServiceBase(get_service_name())
{
}

AppInstallService::~AppInstallService()
{
}

bool AppInstallService::attach(GMainLoop* gml)
{
    if (!ServiceBase::attach(gml))
        return false;

    AppInstaller::instance().initialize();

    return true;
}

void AppInstallService::onAttached()
{
    //register appinstall service methods
    LS_CREATE_CATEGORY_BEGIN(AppInstallService, base)
        LS_CATEGORY_MAPPED_METHOD(install, cb_install)
        LS_CATEGORY_MAPPED_METHOD(remove, cb_remove)
        LS_CATEGORY_MAPPED_METHOD(status, cb_status)
    LS_CREATE_CATEGORY_END

    registerCategory("/", LS_CATEGORY_TABLE_NAME(base), NULL, NULL);
    setCategoryData("/", this);

    if (Settings::instance().isDevMode()) {
        LS_CREATE_CATEGORY_BEGIN(AppInstallService, dev)
            LS_CATEGORY_MAPPED_METHOD(install, cb_dev_install)
            LS_CATEGORY_MAPPED_METHOD(remove, cb_dev_remove)
        LS_CREATE_CATEGORY_END

        registerCategory("/dev", LS_CATEGORY_TABLE_NAME(dev), NULL, NULL);
        setCategoryData("/dev", this);
    }
}

void AppInstallService::onDetached()
{
}

bool AppInstallService::cb_install(LSMessage &message)
{
    JUtil::Error error;
    Message request(&message);
    pbnjson::JValue json = JUtil::parse(request.getPayload(), "appInstallService.install", &error);

    if (json.isNull()) {
        return LSUtils::replyError(&request, APP_INSTALL_ERR_BADPARAM, error.detail());
    }

    std::string id = json["id"].asString();
    std::string ipkUrl = json["ipkUrl"].asString();

    if (id.empty())
        return LSUtils::replyError(&request, APP_INSTALL_ERR_BADPARAM, "id is empty");

    if (ipkUrl.empty())
        return LSUtils::replyError(&request, APP_INSTALL_ERR_BADPARAM, "ipkUrl is empty");

    pbnjson::JValue details = json;
    if (!details["subscribe"].isNull())
        details.remove("subscribe");
    details.put("client", LSUtils::getCallerId(&request));

    pbnjson::JValue appInfo = pbnjson::Object();
    appInfo.put("id", id);
    appInfo.put("details", details);

    int errorCode = 0;
    std::string errorText;

    if (!AppInstaller::instance().install(id, ipkUrl, appInfo, errorCode, errorText)) {
        LOG_ERROR(MSGID_APP_INSTALL_ERR, 3,
                  PMLOGKS(APP_ID, id.c_str()),
                  PMLOGKFV(LOGKEY_ERRCODE, "%d", errorCode),
                  PMLOGKS(LOGKEY_ERRTEXT, errorText.c_str()),
                  "Application install failed");

        return LSUtils::replyError(&request, errorCode, errorText);
    }

    LSError lserror;
    bool subscribed = false;
    if (request.isSubscription())
        subscribed = LSSubscriptionAdd(Handle::get(),
                                       (std::string("status_") + id).c_str(),
                                       &message,
                                       &lserror);

    pbnjson::JValue reply = pbnjson::Object();
    reply.put("returnValue", true);
    reply.put("subscribed", subscribed);

    try {
        request.respond(JUtil::toSimpleString(reply).c_str());
    } catch (const LS::Error &lserror) {
        LOG_ERROR(MSGID_LSCALL_ERR, 1,
                  PMLOGKS("[AppInstallService]-install", lserror.what()),
                  "");
        return false;
    }

    return true;
}

bool AppInstallService::cb_remove(LSMessage &message)
{
    JUtil::Error error;
    Message request(&message);
    pbnjson::JValue json = JUtil::parse(request.getPayload(), "appInstallService.remove", &error);

    if (json.isNull()) {
        return LSUtils::replyError(&request, APP_INSTALL_ERR_BADPARAM, error.detail());
    }

    std::string id = json["id"].asString();

    if (id.empty())
        return LSUtils::replyError(&request, APP_INSTALL_ERR_BADPARAM, "id is empty");

    pbnjson::JValue details = pbnjson::Object();
    if (!details["subscribe"].isNull())
        details.remove("subscribe");
    details.put("client", LSUtils::getCallerId(&request));

    pbnjson::JValue appInfo = pbnjson::Object();
    appInfo.put("id", id);
    appInfo.put("details", details);

    int errorCode = 0;
    std::string errorText;

    if (!AppInstaller::instance().remove(id, appInfo, errorCode, errorText)) {
        LOG_ERROR(MSGID_APP_REMOVE_ERR, 3,
                  PMLOGKS(APP_ID, id.c_str()),
                  PMLOGKFV(LOGKEY_ERRCODE, "%d", errorCode),
                  PMLOGKS(LOGKEY_ERRTEXT, errorText.c_str()),
                  "Application remove failed");
        return LSUtils::replyError(&request, errorCode, errorText);
    }

    LSError lserror;
    bool subscribed = false;
    if (request.isSubscription())
        subscribed = LSSubscriptionAdd(Handle::get(),
                                       (std::string("status_") + id).c_str(),
                                       &message,
                                       &lserror);

    pbnjson::JValue reply = pbnjson::Object();
    reply.put("returnValue", true);
    reply.put("subscribed", subscribed);

    try {
        request.respond(JUtil::toSimpleString(reply).c_str());
    } catch (const LS::Error &lserror) {
        LOG_ERROR(MSGID_LSCALL_ERR, 1, PMLOGKS("[AppInstallService]-remove", lserror.what()), "");
        return false;
    }

    return true;
}

bool AppInstallService::cb_status(LSMessage &message)
{
    JUtil::Error error;
    Message request(&message);
    pbnjson::JValue json = JUtil::parse(request.getPayload(), "appInstallService.status", &error);

    if (json.isNull()) {
        return LSUtils::replyError(&request, APP_INSTALL_ERR_BADPARAM, error.detail());
    }

    LSError lserror;
    bool subscribed = false;
    if (request.isSubscription())
        subscribed = LSSubscriptionAdd(Handle::get(), "status", &message, &lserror);

    pbnjson::JValue reply = pbnjson::Object();
    pbnjson::JValue status = pbnjson::Object();
    status.put("apps", AppInstaller::instance().toJValue());
    reply.put("status", status);
    reply.put("returnValue", true);
    reply.put("subscribed", subscribed);

    try {
        request.respond(JUtil::toSimpleString(reply).c_str());
    } catch (const LS::Error &lserror) {
        LOG_ERROR(MSGID_LSCALL_ERR, 1, PMLOGKS("[AppInstallService]-status", lserror.what()), "");
        return false;
    }

    return true;
}

bool AppInstallService::cb_dev_install(LSMessage &message)
{
    JUtil::Error error;
    Message request(&message);
    pbnjson::JValue json = JUtil::parse(request.getPayload(), "appInstallService.dev.install", &error);

    if (json.isNull()) {
        return LSUtils::replyError(&request, APP_INSTALL_ERR_BADPARAM, error.detail());
    }

    std::string id = json["id"].asString();
    std::string ipkUrl = json["ipkUrl"].asString();

    if (id.empty())
        return LSUtils::replyError(&request, APP_INSTALL_ERR_BADPARAM, "id is empty");

    if (ipkUrl.empty())
        return LSUtils::replyError(&request, APP_INSTALL_ERR_BADPARAM, "ipkUrl is empty");

    pbnjson::JValue details = json;
    if (!details["subscribe"].isNull())
        details.remove("subscribe");
    details.put("client", LSUtils::getCallerId(&request));

    pbnjson::JValue appInfo = pbnjson::Object();
    appInfo.put("id", id);
    appInfo.put("details", details);

    LOG_INFO(MSGID_APP_INSTALL_REQ, 1,
             PMLOGKS(APP_ID, id.c_str()),
             "Dev application install request received");

    int errorCode = 0;
    std::string errorText;

    if (!AppInstaller::instance().install(id, ipkUrl, appInfo, errorCode, errorText, false)) {
        LOG_ERROR(MSGID_APP_INSTALL_ERR, 3,
                  PMLOGKS(APP_ID, id.c_str()),
                  PMLOGKFV(LOGKEY_ERRCODE, "%d", errorCode),
                  PMLOGKS(LOGKEY_ERRTEXT, errorText.c_str()),
                  "Application dev/install failed");

        return LSUtils::replyError(&request, errorCode, errorText);
    }

    LSError lserror;
    bool subscribed = false;
    if (request.isSubscription())
        subscribed = LSSubscriptionAdd(Handle::get(),
                                       (std::string("status_") + id).c_str(),
                                       &message,
                                       &lserror);

    pbnjson::JValue reply = pbnjson::Object();
    reply.put("returnValue", true);
    reply.put("subscribed", subscribed);

    try {
        request.respond(JUtil::toSimpleString(reply).c_str());
    } catch (const LS::Error &lserror) {
        LOG_ERROR(MSGID_LSCALL_ERR, 1, PMLOGKS("[AppInstallService]-dev/install", lserror.what()), "");
        return false;
    }

    return true;
}

bool AppInstallService::cb_dev_remove(LSMessage &message)
{
    JUtil::Error error;
    Message request(&message);
    pbnjson::JValue json = JUtil::parse(request.getPayload(), "appInstallService.dev.remove", &error);

    if (json.isNull()) {
        return LSUtils::replyError(&request, APP_INSTALL_ERR_BADPARAM, error.detail());
    }

    std::string id = json["id"].asString();

    if (id.empty())
        return LSUtils::replyError(&request, APP_INSTALL_ERR_BADPARAM, "id is empty");

    pbnjson::JValue details = pbnjson::Object();
    if (!details["subscribe"].isNull())
        details.remove("subscribe");
    details.put("client", LSUtils::getCallerId(&request));

    pbnjson::JValue appInfo = pbnjson::Object();
    appInfo.put("id", id);
    appInfo.put("details", details);

    LOG_INFO(MSGID_APP_REMOVE_REQ, 1,
             PMLOGKS(APP_ID, id.c_str()),
             "Dev application remove request received");

    int errorCode = 0;
    std::string errorText;

    if (!AppInstaller::instance().remove(id, appInfo, errorCode, errorText, false)) {
        LOG_ERROR(MSGID_APP_REMOVE_ERR, 3,
                  PMLOGKS(APP_ID, id.c_str()),
                  PMLOGKFV(LOGKEY_ERRCODE, "%d", errorCode),
                  PMLOGKS(LOGKEY_ERRTEXT, errorText.c_str()),
                  "Application dev/remove failed");
        return LSUtils::replyError(&request, errorCode, errorText);
    }

    LSError lserror;
    bool subscribed = false;
    if (request.isSubscription())
        subscribed = LSSubscriptionAdd(Handle::get(),
                                       (std::string("status_") + id).c_str(),
                                       &message,
                                       &lserror);

    pbnjson::JValue reply = pbnjson::Object();
    reply.put("returnValue", true);
    reply.put("subscribed", subscribed);

    try {
        request.respond(JUtil::toSimpleString(reply).c_str());
    } catch (const LS::Error &lserror) {
        LOG_ERROR(MSGID_LSCALL_ERR, 1, PMLOGKS("[AppInstallService]-dev/remove", lserror.what()), "");
        return false;
    }

    return true;
}
