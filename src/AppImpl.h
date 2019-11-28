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

#ifndef MAINAPP_H
#define MAINAPP_H

#include "base/App.h"
#include "base/Singleton.hpp"

class AppInstallService;

class AppImpl : public App {
public:
    //! Constructor
    AppImpl();

protected:
    //! This is called when process receive signal
    static void term_handler(int signal);

    //! This is called when App is created
    virtual bool onCreate();

    //! This is called when App is destroyed
    virtual bool onDestroy();

private:
    AppInstallService *m_installService;
};

typedef Singleton<AppImpl> MainApp;

#endif
