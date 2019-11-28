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

#ifndef CALLCHAIN_EVENT_HANDLER_H
#define CALLCHAIN_EVENT_HANDLER_H

#include "base/CallChain.h"
#include "installer/AppInstallerUtility.h"

namespace CallChainEventHandler
{
    class AppRunning : public LSCallItem {
    public:
        AppRunning(const char *serviceName, std::string id);

    protected:
        virtual bool onReceiveCall(pbnjson::JValue message);

    private:
        std::string m_id;
    };

    class AppClose : public LSCallItem {
    public:
        AppClose(const char *serviceName, std::string id);

    protected:
        virtual bool onReceiveCall(pbnjson::JValue message);
    };

    class AppInfo : public LSCallItem {
    public:
        AppInfo(const char *serviceName, std::string id);

    protected:
        virtual bool onReceiveCall(pbnjson::JValue message);
    };

    class AppRemovable : public CallItem {
    public:
        AppRemovable();
        virtual bool Call();
    };

    class AppLock : public LSCallItem {
    public:
        AppLock(const char *serviceName, std::string id);

    protected:
        virtual bool onReceiveCall(pbnjson::JValue message);
    };

    class SvcClose : public CallItem {
    public:
        SvcClose();

        virtual bool Call();

    protected:
        static bool cbQuit(LSHandle *lshandle, LSMessage *msg, void *user_data);

    private:
        int m_numResponse;
        int m_numServices;
    };

    class RemoveDb : public LSCallItem {
    public:
        RemoveDb(const char* serviceName, pbnjson::JValue owners);

    protected:
        virtual bool onReceiveCall(pbnjson::JValue message);
    };

    class UpdateManifest : public LSCallItem {
    public:
        UpdateManifest(const char *serviceName, bool add, std::string path, std::string prefix, std::string id);

    protected:
        virtual bool onReceiveCall(pbnjson::JValue message);
    };

    class RemoveIpk : public CallItem {
    public:
        RemoveIpk(std::string id, bool verify, std::string externalPath);
        virtual bool Call();

    private:
        //! callback progress function for removeIpk
        void cbRemoveIpkProgress(const char *str);

        //! callback complete function for removeIpk
        void cbRemoveIpkComplete(int status);

    private:
        std::string m_id;
        bool m_verify;
        std::string m_externalPath;

        AppInstallerUtility m_installerUtility;
    };

}

#endif
