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

//
// Created by alex2772 on 2020-08-13.
//

#include "ASqlBuilder.h"
#include "ASqlDatabase.h"
#include <cassert>
#include <AUI/Autumn/Autumn.h>

ASqlBuilder::ASqlBuilder(const AString& tableName) : mTableName(tableName) {}

ASqlBuilder::Insert ASqlBuilder::insert(const AStringVector& columnNames) {
    return {*this, "INSERT INTO " + mTableName + "(" + columnNames.join(',') + ") VALUES "};
}

ASqlBuilder::Select ASqlBuilder::select(const AStringVector& columnNames) {
    return {*this, "SELECT " + (columnNames.empty() ? '*' : columnNames.join(',')) + " FROM " + mTableName};
}

ASqlBuilder::Update ASqlBuilder::update(const AMap<AString, AVariant>& data) {
    return {data, *this, "UPDATE " + mTableName + " SET "};
}

ASqlBuilder::Delete ASqlBuilder::remove() {
    return {*this, "DELETE FROM " + mTableName + " "};
}


ASqlBuilder::Select::Select(ASqlBuilder& builder, const AString& sql) : WhereStatement(builder, sql)
{

}

ASqlBuilder::Statement::Statement(ASqlBuilder& builder, const AString& sql):
        mBuilder(builder) {
    mSql.reserve(0x1000);
    mSql = sql;
}

ASqlBuilder::Insert::Insert(ASqlBuilder& builder, const AString& sql) : Statement(builder, sql) {}

ASqlBuilder::Insert& ASqlBuilder::Insert::row(const AVector<AVariant>& data) {
    mData.insertAll(data);
    if (mSql.last() == ')')
        mSql += ',';
    mSql += "(";
    for (unsigned i = 0; i < data.size(); ++i) {
        if (i)
            mSql += ',';
        mSql += '?';
    }
    mSql += ")";
    return *this;
}

ASqlBuilder::Insert& ASqlBuilder::Insert::rows(const AVector<AVector<AVariant>>& data) {
    for (const auto& r : data)
        row(r);
    return *this;
}

ASqlBuilder::Insert::~Insert() {
    if (!mInsertDone)
        doInsert();
}

int ASqlBuilder::Insert::doInsert() {
    AUI_ASSERT(!mData.empty());
    int id = Autumn::get<ASqlDatabase>()->execute(mSql, mData);
    mInsertDone = true;
    return id;
}

id_t ASqlBuilder::Insert::rowId() {
    return doInsert();
}

ASqlBuilder::WhereStatement::WhereStatement(ASqlBuilder& builder, const AString& sql) : Statement(builder, sql) {}

void ASqlBuilder::WhereStatement::whereImpl(const ASqlBuilder::WhereStatement::WhereExpr& w) {
    if (mWhereExpr.empty()) {
        mWhereExpr = "WHERE " + w.mExprString;
        mWhereParams = w.mWhereParams;
    } else {
        mWhereExpr += "AND " + w.mExprString;
        mWhereParams.insertAll(w.mWhereParams);
    }
}

ASqlBuilder::WhereStatement::WhereExpr::WhereExpr(const AString& exprString) : mExprString(exprString) {}


AVector<AVector<AVariant>> ASqlBuilder::Select::get() {
    AVector<AVector<AVariant>> r;
    r.reserve(0x100);
    mSql += " ";
    mSql += mWhereExpr;
    auto result = Autumn::get<ASqlDatabase>()->query(mSql, mWhereParams);
    for (auto& row : result) {
        AVector<AVariant> myRow;
        for (size_t i = 0; i < result->getColumns().size(); ++i) {
            myRow.push_back(row->getValue(i));
        }
        r << myRow;
    }
    return r;
}


ASqlBuilder::Update::Update(const AMap<AString, AVariant>& values, ASqlBuilder& builder, const AString& sql) :
    WhereStatement(builder, sql),
    mValues(values)
    {

}

ASqlBuilder::Update::~Update() {
    bool hasCommas = false;

    AVector<AVariant> setValues;

    for (auto& p : mValues) {
        if (hasCommas) {
            mSql += ',';
        } else {
            hasCommas = true;
        }
        mSql += p.first;
        mSql += "=?";
        setValues << p.second;
    }

    mSql += " ";
    mSql += mWhereExpr;
    setValues.insertAll(mWhereParams);

    Autumn::get<ASqlDatabase>()->execute(mSql, setValues);
}

ASqlBuilder::Delete::Delete(ASqlBuilder& builder, const AString& sql) : WhereStatement(builder, sql) {

}

ASqlBuilder::Delete::~Delete() {
    mSql += mWhereExpr;
    Autumn::get<ASqlDatabase>()->execute(mSql, mWhereParams);
}