/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
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
    static AMap<AString, _<AField<T>>> getFields() { AUI_ASSERT(0); return {};}
    static AString getSqlTable() { AUI_ASSERT(0); return {};}
};

#define A_META(name) template<> struct AModelMeta< name >: AModelMetaBase< name >
#define A_FIELDS static AMap<AString, _<AField<Model>>> getFields()
#define A_FIELD(name) { #name, AField<Model>::make(&Model:: name ) },
#define A_SQL_TABLE(name) static AString getSqlTable() { return name;}

