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

#ifndef APP_H
#define APP_H

#include <glib.h>

//! Base class for Application
class App {
public:
    //! Constructor
    App();

    //! Destructor
    virtual ~App();

    //! Create App
    bool create();

    //! Run App
    int run();

    //! Quit App
    void quit();

    //! Attach
    bool attach(GSource *gsource);

protected:
    //! Destroy App
    void destroy();

    //! Get GMainLoop
    GMainLoop *mainLoop();

protected:
    //! It is called when App is created
    virtual bool onCreate() = 0;

    //! It is called when App is destroyed
    virtual bool onDestroy() = 0;

private:
    GMainLoop* m_mainLoop;
};

#endif
