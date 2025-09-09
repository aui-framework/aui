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

#include "AModelMeta.h"
#include <AUI/Data/ASqlBuilder.h>

#include <utility>

/**
 * @brief Defines a model that can be stored in an SQL database. Implements queries for this type to the database
 *        (insert, update, select, delete)
 * @tparam Model ORM model.
 * @details
 * Model should implement AModelMeta (see AUI/Data/AModelMeta.h)
 */
template<typename Model>
struct ASqlModel {
    /**
     * @brief Thrown when a single row is expected to be received, but the database did not return any rows.
     */
    class NoSuchRowException: public AException {};

    /**
     * @brief Thrown when one row is expected to be received, but the database returned more than one row.
     */
    class TooManyRowsException: public AException {};


    typedef AModelMeta<Model> Meta;
    id_t id = 0;

    /**
     * @brief Saves this model in DB.
     *        If id = 0 then a new row will be created in the table, and the id of the created row will be assigned in
     *        the structure field.
     *        If id != 0 then the existing row in the table will be updated.
     */
    void save() {
        if (id == 0) {
            id = table(Meta::getSqlTable()).insertORM((Model&)*this);
        } else {
            table(Meta::getSqlTable()).updateORM((Model&)*this);
        }
    }

    /**
     * @brief Removes row from the table by ID.
     */
    void remove() {
        AUI_ASSERT(id != 0);
        table(Meta::getSqlTable()).removeORM((Model&)*this);
    }

    class IncompleteSelectRequest {
        friend struct ASqlModel<Model>;
    private:
        AString mSql;
        AString mWhereExpr;
        AVector<AVariant> mWhereParams;

        IncompleteSelectRequest(AString sql, const ASqlBuilder::WhereStatement::WhereExpr& expression):
            mSql(std::move(sql)) {
            where(expression);
        }

    public:
        IncompleteSelectRequest(const IncompleteSelectRequest&) = delete;
        ~IncompleteSelectRequest() = default;

        IncompleteSelectRequest& where(const ASqlBuilder::WhereStatement::WhereExpr& w) {
            auto[exprString, whereParams] = ASqlBuilder::WhereStatement::WhereExpr::unpack(w);
            if (!exprString.empty()) {
                if (mWhereExpr.empty()) {
                    mWhereExpr = "WHERE " + exprString;
                    mWhereParams = std::move(whereParams);
                } else {
                    mWhereExpr += "AND " + exprString;
                    mWhereParams.insertAll(whereParams);
                }
            }
            return *this;
        }

        /**
         * @brief Get query result in ORM.
         * @return query result in ORM
         */
        AVector<Model> get() {
            auto idField = AField<ASqlModel<Model>>::make(&ASqlModel<Model>::id);
            AVector<Model> result;
            result.reserve(0x100);

            mSql += " ";
            mSql += mWhereExpr;
            auto dbResult = Autumn::get<ASqlDatabase>()->query(mSql, mWhereParams);

            AVector<size_t> sqlColumnToModelFieldIndexMapping;
            AVector<_<AField<Model>>> fields;
            fields << AModelMeta<Model>::getFields().valueVector();

            for (auto& row : dbResult) {
                Model m;
                idField->set(m, row->getValue(0));
                for (size_t columnIndex = 1; columnIndex < dbResult->getColumns().size(); ++columnIndex) {
                    fields[columnIndex - 1]->set(m, row->getValue(columnIndex));
                }
                result << std::move(m);
            }

            return result;
        }

        /**
         * @brief Do query and get first row in ORM
         * \throws NoSuchRowException when database returned zero rows
         *         TooManyRowsException when database returned two or more rows
         * @return ORM structure
         */
        inline Model first() {
            auto result = get();
            if (result.size() == 0)
                throw NoSuchRowException();
            if (result.size() != 1)
                throw TooManyRowsException();
            return result.first();
        }
    };
    static _<IncompleteSelectRequest> where(const ASqlBuilder::WhereStatement::WhereExpr& expression) {
        AStringVector columnNames;
        columnNames << "id";
        columnNames << Meta::getFields().keyVector();
        return aui::ptr::manage_shared(new IncompleteSelectRequest("SELECT " + (columnNames.empty() ? '*'
            : columnNames.join(',')) + " FROM " + AModelMeta<Model>::getSqlTable(), expression));
    }

    /**
     * @brief Get a row from the table by ID.
     * @param id ID of the required string
     * @return the string table for the specified ID
     * \throws NoSuchRowException if no string was found for the specified ID
     */
    static Model byId(id_t id) {
        AStringVector columns;
        columns << "id";
        columns << Meta::getFields().keyVector();
        auto result = table(Meta::getSqlTable()).select(columns).where(col("id") == id)
                .template as<Model>();

        if (result.empty())
            throw NoSuchRowException();
        AUI_ASSERT(result.size() == 1);
        return result.first();
    }

    static _<IncompleteSelectRequest> all() {
        AStringVector columnNames;
        columnNames << "id";
        columnNames << Meta::getFields().keyVector();
        return aui::ptr::manage_shared(new IncompleteSelectRequest("SELECT " + (columnNames.empty() ? '*'
            : columnNames.join(',')) + " FROM " + AModelMeta<Model>::getSqlTable(), {}));
    }


    /**
     * @brief Creates a model and saves it to the database.
     * @tparam Args the types of constructor arguments
     * @param args the constructor arguments
     * @return the ORM structure
     */
    template<typename ... Args>
    static Model make(Args&&... args) {
        Model m = {0, std::forward<Args>(args)...};
        m.toJson();
        return m;
    }


    /**
     * @return name of the relation column for other tables.
     * @details
     * Example:
     * `struct User -> table users -> column user_id is the result.`
     */
    static AString getIdColumnNameInOtherTables() {
        AString tableName = AModelMeta<Model>::getSqlTable();
        if (tableName.endsWith("s")) {
            tableName = tableName.substr(0, tableName.length() - 1);
        }
        tableName += "_id";
        return tableName;
    }

protected:    /* ORM RELATIONSHIP */

    /**
     * @brief Implementation of one-to-many relation between ORM structures. Used with belongsTo.
     * <pre>
     * User::getPosts() -> hasMany<Post>()<br />
     * &nbsp;&nbsp;|-- Post::getAuthor() -> belongsTo<Post>()<br />
     * &nbsp;&nbsp;|-- Post::getAuthor() -> belongsTo<Post>()<br />
     * &nbsp;&nbsp;|-- Post::getAuthor() -> belongsTo<Post>()<br />
     * &nbsp;&nbsp;....
     *  </pre>
     * @tparam Other ORM model relation will created with
     * @return incomplete SQL request (see ASqlModel::IncompleteSelectRequest)
     * @see ASqlModel::IncompleteSelectRequest
     */
    template<typename Other>
    _<typename Other::IncompleteSelectRequest> hasMany() {
        const AString columnName = getIdColumnNameInOtherTables();
        return Other::where(col(columnName) == id);
    }

    template<typename Other>
    Other belongsTo(id_t desiredId) {
        return Other::where(col("id") == desiredId)->first();
    }
};

