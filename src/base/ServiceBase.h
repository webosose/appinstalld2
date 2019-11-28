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

#ifndef SERVICE_BASE_H
#define SERVICE_BASE_H

#include <glib.h>
#include <luna-service2/lunaservice.hpp>
#include <string>
#include <vector>

using namespace LS;

//! Base class for Service
class ServiceBase : public Handle {
public:
    //! Constructor
    ServiceBase(std::string serviceName);

    //! Destructor
    virtual ~ServiceBase();

    //! attach luna-service
    bool attach(GMainLoop* gml);

    //! detach luna-service
    void detach();

    //! get luna-handle
    Handle* getHandle() { return this; }

    //! get service name
    virtual const char* get_service_name() const = 0;

    //! on Attached
    virtual void onAttached() = 0;

    //! on detached
    virtual void onDetached() = 0;
};
#endif
