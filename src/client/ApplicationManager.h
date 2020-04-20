// Copyright (c) 2020 LG Electronics, Inc.
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

#ifndef CLIENT_APPLICATIONMANAGER_H_
#define CLIENT_APPLICATIONMANAGER_H_

#include <iostream>
#include <pbnjson.hpp>
#include <luna-service2/lunaservice.hpp>

#include "client/AbsLunaClient.h"
#include "interface/IClassName.h"
#include "interface/ISingleton.h"

using namespace LS;
using namespace std;
using namespace pbnjson;

class ApplicationManager : public ISingleton<ApplicationManager>,
                           public AbsLunaClient {
friend ISingleton<ApplicationManager>;
public:
    virtual ~ApplicationManager();

    bool lockApp(const char* sessionId, const string& id, bool lock);

protected:
    // AbsLunaClient
    virtual void onInitialzed() override;
    virtual void onFinalized() override;
    virtual void onServerStatusChanged(bool isConnected) override;

private:
    static bool onLockApp(LSHandle* sh, LSMessage* reply, void* ctx);

    ApplicationManager();
};

#endif /* CLIENT_APPLICATIONMANAGER_H_ */
