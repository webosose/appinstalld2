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

#ifndef LOCALES_H
#define LOCALES_H

#include <string>

//! This class is wrapper locale file
class Locales {
public:
    //! Constructor
    Locales();

    //! get ui locale
    std::string getUI() const;

    //! get language
    std::string language() const;
    //! get script
    std::string script() const;
    //! get region
    std::string region() const;

private:
    //! load locale file
    void load();

private:
    std::string m_ui;
    std::string m_language;
    std::string m_script;
    std::string m_region;
};

#endif
