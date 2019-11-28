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

#include "App.h"

App::App()
    : m_mainLoop(NULL)
{
}

App::~App()
{
}

bool App::create()
{
    m_mainLoop = g_main_loop_new(NULL, FALSE);
    if (m_mainLoop == NULL)
        return false;

    return onCreate();
}

int App::run()
{
    if (!m_mainLoop)
        return 0;

    g_main_loop_run(m_mainLoop);
    return 0;
}

void App::quit()
{
    if (m_mainLoop) {
        g_main_loop_quit(m_mainLoop);
    }

    destroy();
}

void App::destroy()
{
    onDestroy();

    if (m_mainLoop) {
        g_main_loop_unref(m_mainLoop);
        m_mainLoop = NULL;
    }
}

GMainLoop *App::mainLoop()
{
    return m_mainLoop;
}

bool App::attach(GSource *gsource)
{
    if (gsource == NULL)
        return false;
    g_source_attach(gsource, g_main_loop_get_context(mainLoop()));
    return true;
}
