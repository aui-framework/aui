// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

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
 * @tparam T Model
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

