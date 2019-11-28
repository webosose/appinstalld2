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

#ifndef UTILS_H
#define UTILS_H

#include <fstream>
#include <glib.h>
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>

template <size_t N>
struct Tuple {
    template <typename F, typename T, typename ... A>
    static inline auto apply(F && f, T && t, A &&... a)
        -> decltype(Tuple<N-1>::apply(std::forward<F>(f),
                                      std::forward<T>(t),
                                      std::get<N-1>(std::forward<T>(t)),
                                      std::forward<A>(a)...))
    {
        return Tuple<N-1>::apply(std::forward<F>(f),
                                 std::forward<T>(t),
                                 std::get<N-1>(std::forward<T>(t)),
                                 std::forward<A>(a)...);
    }
};

template <>
struct Tuple<0> {
    template <typename F, typename T, typename ... A>
    static inline auto apply(F && f, T && t, A &&... a)
        -> decltype(std::forward<F>(f)(std::forward<A>(a)...))
    {
        return std::forward<F>(f)(std::forward<A>(a)...);
    }
};

//! List of utilites for common
class Utils {
private:
    // abstract class for async call
    class IAsyncCall {
    public:
        virtual ~IAsyncCall() { }
        virtual void Call() = 0;
    };

    // implementaion for async call
    template <typename T>
    class AsyncCall : public IAsyncCall {
    public:
        AsyncCall(T _func) : func(_func) {}

        void Call() { func(); }
    private:
        T func;
    };

public:
    //! Read file contents
    static std::string read_file(const std::string &path);

    //! Make directory
    static bool make_dir(const std::string &path, bool withParent = true);

    //! Remove directory recursive
    static bool remove_dir(const std::string &path);

    //! Remove file
    static bool remove_file(const std::string &path);

    //! Get file size
    static long long file_size(const std::string &path);

    //! Make std::string for type T
    template <class T>
    static std::string toString(const T &arg)
    {
        std::ostringstream out;
        out << arg;
        return (out.str());
    }

    //! Call function asynchronously
    template <typename T>
    static bool async(T function, guint timeout = 0)
    {
        AsyncCall<T> *p = new AsyncCall<T>(function);
        g_timeout_add(timeout, cbAsync, (gpointer)p);
        return true;
    }

    //! unpack tuple and apply to function
    template <typename F, typename T>
    static inline auto apply_tuple(F && f, T && t)
        -> decltype(Tuple<std::tuple_size<typename std::decay<T>::type>::value>::apply(std::forward<F>(f), std::forward<T>(t)))
    {
        return Tuple<std::tuple_size<typename std::decay<T>::type>::value>::apply(std::forward<F>(f), std::forward<T>(t));
    }

private:
    //! It's called when get response async call
    static gboolean cbAsync(gpointer data);
};

#endif
