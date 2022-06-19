/*
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

#pragma once

#include <cassert>
#include <AUI/Common/AMap.h>
#include <AUI/Common/AString.h>
#include <AUI/Reflect/AField.h>

template<class T>
struct AModelMetaBase {
    using Model = T;
};

/**
 * @brief Defines model metadata (list of fields, name of appropriate sql table, etc...)
 * \tparam T Model
 * \code
 * // user model
 * struct User: ASqlModel<User> {
 *   id_t id;
 *   AString username;
 *   AString password;
 * };
 *
 * // metadata
 * A_META(User) {
 *   // appropriate sql table
 *   A_SQL_TABLE("users")
 *
 *   // appropriate fields
 *   A_FIELDS {
 *     return {
 *       A_FIELD(id)
 *       A_FIELD(username)
 *       A_FIELD(password)
 *     };
 *   }
 * };
 * \endcode
 */
template<class T>
struct AModelMeta: AModelMetaBase<T> {
    static AMap<AString, _<AField<T>>> getFields() { assert(0); return {};}
    static AString getSqlTable() { assert(0); return {};}
};

#define A_META(name) template<> struct AModelMeta< name >: AModelMetaBase< name >
#define A_FIELDS static AMap<AString, _<AField<Model>>> getFields()
#define A_FIELD(name) {#name, AField<Model>::make(&Model:: name )},
#define A_SQL_TABLE(name) static AString getSqlTable() { return name;}

