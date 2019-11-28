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

#ifndef APP_INSTALL_SERVICE_BASE_H
#define APP_INSTALL_SERVICE_BASE_H

#include "base/ServiceBase.h"
#include "installer/AppInstallerErrors.h"

class AppInstaller;

using namespace LS;
namespace pbnjson
{
    class JValue;
}

//! Service class for com.webos.appInstallService
class AppInstallService : public ServiceBase {
public:
    //! Constructor
    AppInstallService();

    //! Destructor
    ~AppInstallService();

    //! luna-service attach
    bool attach(GMainLoop* gml);

protected:
    //! LS callback for com.webos.appInstallService/install
    bool cb_install(LSMessage &message);

    //! LS callback for com.webos.appInstallService/remove
    bool cb_remove(LSMessage &message);

    //! LS callback for com.webos.appInstallService/status
    bool cb_status(LSMessage &message);

    //! LS callback for com.webos.appInstallService/dev/install
    bool cb_dev_install(LSMessage &message);

    //! LS callback for com.webos.appInstallService/dev/remove
    bool cb_dev_remove(LSMessage &message);

    //! on attached
    virtual void onAttached();

    //! on detached
    virtual void onDetached();

    //! get service name
    virtual const char* get_service_name() const { return "com.webos.appInstallService"; }

};

#endif
