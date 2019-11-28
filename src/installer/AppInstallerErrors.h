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

#ifndef APPINSTALLERERRORS_H
#define APPINSTALLERERRORS_H

//! error value from AppInstaller
enum {
    APP_INSTALL_ERR_NONE            = 0,
    APP_INSTALL_ERR_GENERAL         = -1,
    APP_INSTALL_ERR_BADPARAM        = -2,
    APP_INSTALL_ERR_DISKFULL        = -3,
    APP_INSTALL_ERR_DOWNLOAD        = -4,
    APP_INSTALL_ERR_INSTALL         = -5,
    APP_REMOVE_ERR_GENERAL          = -6,
    APP_REMOVE_ERR_REMOVE           = -7,
    APP_INSTALL_ERR_DEVMOD          = -8,
    APP_REMOVE_ERR_PARENTLOCK       = -9,
    APP_INSTALL_ERR_NOTSUITABLE     = -10, // Unavailable USB
    APP_INSTALL_ERR_INTERNALONLY    = -11, // Not permit to install USB
    APP_INSTALL_ERR_RESTORE         = -12, // Failed to restore task
    APP_INSTALL_ERR_UPDATESTORAGE   = -13, // Update storage should be same
    APP_INSTALL_ERR_TARGETNOTEXIST  = -14, // Target does not exist
    APP_INSTALL_ERR_PRIVILEGED      = -15, // Cannot install privileged app on dev mode
    APP_INSTALL_ERR_UNAUTHORIZED    = -16, // Need login
    APP_INSTALL_ERR_DUPLICATED      = -17, // Install request is duplicated
    APP_INSTALL_ERR_TARGETISBUSY    = -18, // The USB is busy
    APP_REMOVE_ERR_PRIVILEGED       = -19,
};

#endif
