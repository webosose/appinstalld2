// Copyright (c) 2013-2018 LG Electronics, Inc.
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

#include <boost/algorithm/string/replace.hpp>

#include "AppInstallerUtilityErrors.h"
#include "base/JUtil.h"
#include "base/Utils.h"
#include "base/LSUtils.h"
#include "base/Logging.h"
#include "CallChainEventHandler.h"
#include "settings/Settings.h"
#include "PackageInfo.h"
#include "ServiceInfo.h"

using namespace std::placeholders;

namespace CallChainEventHandler
{
    const int SETTINGSERVICE_GET_VALUE_NUM = 3;

    AppRunning::AppRunning(const char *serviceName, std::string id)
        : LSCallItem(serviceName, "luna://com.webos.applicationManager/running", "{}"),
          m_id(id)
    {
    }

    bool AppRunning::onReceiveCall(pbnjson::JValue message)
    {
        bool returnValue = message["returnValue"].asBool();
        if (!returnValue) {
            setError("Get running list failed");
            return false;
        }

        pbnjson::JValue runningApps = message["running"];
        if (!runningApps.isArray()) {
            setError("Invalid running list format");
            return false;
        }

        pbnjson::JValue app;
        int arraySize = runningApps.arraySize();
        for(int i = 0 ; i < arraySize ; ++i) {
            app = runningApps[i];
            if (app["id"].asString() == m_id)
                return true;
        }

        return false;
    }

    AppClose::AppClose(const char *serviceName, std::string id)
        : LSCallItem(serviceName, "luna://com.webos.applicationManager/closeByAppId", "")
    {
        pbnjson::JValue payload = pbnjson::Object();
        payload.put("id", id);
        setPayload(JUtil::toSimpleString(payload).c_str());
    }

    bool AppClose::onReceiveCall(pbnjson::JValue message)
    {
        bool returnValue = message["returnValue"].asBool();
        if (!returnValue) {
            setError("Close app failed");
            return false;
        }

        return true;
    }

    AppInfo::AppInfo(const char *serviceName, std::string id)
        : LSCallItem(serviceName, "luna://com.webos.applicationManager/getAppInfo", "")
    {
        pbnjson::JValue payload = pbnjson::Object();
        payload.put("id", id);
        setPayload(JUtil::toSimpleString(payload).c_str());
    }

    bool AppInfo::onReceiveCall(pbnjson::JValue message)
    {
        bool returnValue = message["returnValue"].asBool();
        if (returnValue) {
            pbnjson::JValue chainData = getChainData();
            chainData.put("appInfo", message["appInfo"]);
            setChainData(chainData);
            return true;
        }

        return false;
    }

    AppRemovable::AppRemovable()
    {
    }

    bool AppRemovable::Call()
    {
        pbnjson::JValue chainData = getChainData();
        pbnjson::JValue appInfo = chainData["appInfo"];
        if (appInfo.isNull() || !appInfo["removable"].asBool()) {
            onError("The app is not removable");
            return false;
        }

        onFinished(true, "");
        return true;
    }

    AppLock::AppLock(const char *serviceName, std::string id)
        : LSCallItem(serviceName, "luna://com.webos.applicationManager/lockApp", "")
    {
        pbnjson::JValue payload = pbnjson::Object();
        payload.put("id", id);
        payload.put("lock", true);
        setPayload(JUtil::toSimpleString(payload).c_str());
    }

    bool AppLock::onReceiveCall(pbnjson::JValue message)
    {
        bool returnValue = message["returnValue"].asBool();
        if (!returnValue) {
            setError("Lock app failed");
            return false;
        }

        pbnjson::JValue chainData = getChainData();
        chainData.put("id", message["id"].asString());
        setChainData(chainData);

        return true;
    }

    SvcClose::SvcClose()
        : m_numResponse(0),
          m_numServices(0)
    {
    }

    bool SvcClose::Call()
    {
        pbnjson::JValue chainData = getChainData();
        if (chainData.isNull()) {
            Utils::async([=] { onFinished(true, ""); });
            return true;
        }

        pbnjson::JValue appInfo = chainData["appInfo"];
        if (appInfo.isNull()) {
            Utils::async([=] { onFinished(true, ""); });
            return true;
        }

        std::string appPath = appInfo["folderPath"].asString();
        std::string appId = appInfo["id"].asString();
        std::string appDir = Settings::instance().getApplicationPath();

        std::string packagePath = boost::replace_all_copy(appPath, appDir, Settings::instance().getPackagePath());
        LOG_DEBUG("[SVC_CLOSE] package path : %s", packagePath.c_str());

        PackageInfo packageInfo(packagePath);

        std::vector<std::string> serviceLists;
        packageInfo.getServices(serviceLists);

        m_numServices = static_cast<int>(serviceLists.size());
        for(auto iter = serviceLists.begin(); iter != serviceLists.end(); ++iter) {
            std::string servicePath = boost::replace_all_copy(appPath, appDir + "/" + appId, std::string("/usr/palm/services/") + (*iter));
            LOG_DEBUG("[SVC_CLOSE] service path : %s", servicePath.c_str());

            ServiceInfo serviceInfo(servicePath);
            if (serviceInfo.getType() != "native") {
                std::string errorText;
                std::string uri = "luna://" + serviceInfo.getId() + "/quit";
                LSCaller caller = LSUtils::acquireCaller("com.webos.appInstallService");
                LOG_DEBUG("[NODEJS_SVC_CLOSE] uri : %s", uri.c_str());
                if (!caller.CallOneReply(uri.c_str(), "{}", cbQuit, this, NULL, errorText)) {
                    Utils::async([=] { onFinished(false, errorText); });
                    break;
                }
            } else {
                std::string serviceExec = serviceInfo.getExec(true);
                std::string closeCmd = std::string("killall ") + serviceExec;
                LOG_DEBUG("[NATIVE_SVC_CLOSE] closeCmd : %s", closeCmd.c_str());
                ::system(closeCmd.c_str());

                ++m_numResponse;
            }
        }

        if (m_numServices == m_numResponse) {
            Utils::async([=] {
                onFinished(true, "");
            });
        }

        return true;
    }

    bool SvcClose::cbQuit(LSHandle *lshandle, LSMessage *msg, void *user_data)
    {
        SvcClose *item = static_cast<SvcClose*>(user_data);
        if (!item)
            return false;

        pbnjson::JValue json = JUtil::parse(LSMessageGetPayload(msg), std::string(""));
        bool returnValue = json["returnValue"].asBool();

        if (!returnValue) {
            std::string errorText = json["errorText"].asString();
            if (errorText.empty())
                errorText = "Failed to quit nodejs service";

            Utils::async([=] { item->onFinished(false, errorText); });

            return true;
        }

        ++(item->m_numResponse);
        if (item->m_numResponse == item->m_numServices) {
            Utils::async([=] { item->onFinished(true, ""); });
        }

        return true;
    }

    RemoveDb::RemoveDb(const char* serviceName, pbnjson::JValue owners)
        : LSCallItem(serviceName, "luna://com.webos.service.db/removeAppData", "")
    {
        pbnjson::JValue json = pbnjson::Object();
        json.put("owners", owners);
        setPayload(JUtil::toSimpleString(json).c_str());
    }

    bool RemoveDb::onReceiveCall(pbnjson::JValue message)
    {
        return true;
    }

    UpdateManifest::UpdateManifest(const char *serviceName, bool isAdd, std::string path, std::string prefix, std::string id)
        : LSCallItem(serviceName, "", "{}")
    {
        if (isAdd)
            setUri("luna://com.webos.service.bus/addOneManifest");
        else
            setUri("luna://com.webos.service.bus/removeOneManifest");

        pbnjson::JValue payload = pbnjson::Object();
        payload.put("path", path + "/" + id + ".json" );
        if (prefix.empty())
            payload.put("prefix","/");
        else
            payload.put("prefix", prefix + "/");

        setPayload(JUtil::toSimpleString(payload).c_str());
    }

    bool UpdateManifest::onReceiveCall(pbnjson::JValue message)
    {
        bool returnValue = message["returnValue"].asBool();
        if (!returnValue) {
            setError("update manifest file failed");
            return false;
        }
        return returnValue;
    }

    RemoveIpk::RemoveIpk(std::string id, bool verify, std::string externalPath)
        : m_id(id),
          m_verify(verify),
          m_externalPath(externalPath)
    {
    }

    bool RemoveIpk::Call()
    {
        AppInstallerUtility::Result result =
            m_installerUtility.remove(m_id,
                                      m_verify,
                                      m_externalPath,
                                      std::bind(&RemoveIpk::cbRemoveIpkProgress, this, _1),
                                      std::bind(&RemoveIpk::cbRemoveIpkComplete, this, _1));

        switch(result)
        {
            case AppInstallerUtility::FAIL:
                onError("unable to call ApplicationInstallerUtility");
                return false;
            case AppInstallerUtility::LOCKED: {
                pbnjson::JValue chainData = getChainData();
                chainData.put("locked", true);
                setChainData(chainData);
                onError("Opkg is locked");
                return false;
            }
            default:
                break;
        }

        return true;
    }

    void RemoveIpk::cbRemoveIpkProgress(const char *str)
    {
    }

    void RemoveIpk::cbRemoveIpkComplete(int status)
    {
        LOG_DEBUG("Remove Complete with %d", status);

        if (!WIFEXITED(status) || (WEXITSTATUS(status) != 0)) {
            switch (WEXITSTATUS(status))
            {
                case AI_ERR_INSTALL_TARGETNOTFOUND:
                    break;
                case AI_ERR_REMOVE_FAILEDIPKGREMOVE:
                default:
                    LOG_WARNING(MSGID_IPK_REMOVE_INFO, 2,
                                PMLOGKS(FUNCTION,__PRETTY_FUNCTION__),
                                PMLOGKFV(STATUS,"%d",status), "");

                    pbnjson::JValue chainData = getChainData();
                    int failed = 0;
                    if (chainData.hasKey("failed"))
                        failed = chainData["failed"].asNumber<int>();
                    ++failed;
                    chainData.put("failed", failed);
                    setChainData(chainData);
                    break;
            }
        } else {
            pbnjson::JValue chainData = getChainData();
            int removed = 0;
            if (chainData.hasKey("removed"))
                removed = chainData["removed"].asNumber<int>();
            ++removed;
            chainData.put("removed", removed);
            setChainData(chainData);
        }

        sync();
        onFinished(true, std::string(""));
    }
}
