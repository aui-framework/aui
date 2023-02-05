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

#include <AUI/Autumn/Autumn.h>
#include <AUI/Common/AString.h>
#include <AUI/Common/AVariant.h>
#include <AUI/Common/AVector.h>
#include <AUI/Common/AMap.h>
#include <AUI/Common/AStringVector.h>
#include <AUI/Reflect/AField.h>
#include <AUI/Data/ASqlDatabase.h>
#include <AUI/Traits/parameter_pack.h>

template<typename ModelType>
struct ASqlModel;

template<class T>
struct AModelMeta;

class API_AUI_DATA ASqlBuilder {
private:
    AString mTableName;

public:
    class API_AUI_DATA Statement {
    protected:
        ASqlBuilder& mBuilder;
        AString mSql;

        Statement(ASqlBuilder& builder, const AString& sql);
        Statement(const Statement&) = delete;
    public:
        virtual ~Statement() = default;

        /**
         * @brief Does the another request to the same table.
         * \code
         * ASqlBuilder("users").insert({"username", "age"}, {"John", 23})
         *              .also().delete().where("username", '=', "Paul");
         * \endcode
         * @return appropriate ASqlBuilder.
         */
        ASqlBuilder& also() {
            return mBuilder;
        }
    };

    class API_AUI_DATA Insert: public Statement {
        friend class ASqlBuilder;
    private:
        AVector<AVariant> mData;
        bool mInsertDone = false;

        Insert(ASqlBuilder& builder, const AString& sql);
        int doInsert();

    public:
        Insert(const Insert&) = delete;
        Insert& row(const AVector<AVariant>& data);
        Insert& rows(const AVector<AVector<AVariant>>& data);
        id_t rowId();
        ~Insert() override;
    };

    class API_AUI_DATA WhereStatement: public Statement {
    public:
        ~WhereStatement() override = default;

        class API_AUI_DATA WhereExpr {
            friend class WhereStatement;
        private:
            AString mExprString;
            AVector<AVariant> mWhereParams;

        public:
            WhereExpr() = default;
            explicit WhereExpr(const AString& exprString);

            static std::tuple<AString, AVector<AVariant>> unpack(const WhereExpr& e) {
                return std::make_tuple(e.mExprString, e.mWhereParams);
            }

            inline WhereExpr& operator>(const AVariant& other) {
                mExprString += " > ?";
                mWhereParams << other;
                return *this;
            }
            inline WhereExpr& operator>=(const AVariant& other) {
                mExprString += " >= ?";
                mWhereParams << other;
                return *this;
            }
            inline WhereExpr& operator<(const AVariant& other) {
                mExprString += " < ?";
                mWhereParams << other;
                return *this;
            }
            inline WhereExpr& operator<=(const AVariant& other) {
                mExprString += " <= ?";
                mWhereParams << other;
                return *this;
            }
            inline WhereExpr& operator==(const AVariant& other) {
                mExprString += " = ?";
                mWhereParams << other;
                return *this;
            }
            inline WhereExpr& operator!=(const AVariant& other) {
                mExprString += " != ?";
                mWhereParams << other;
                return *this;
            }

            inline WhereExpr& operator&&(const WhereExpr& other) {
                mExprString += " and " + other.mExprString;
                mWhereParams.insertAll(other.mWhereParams);
                return *this;
            }
            inline WhereExpr& operator||(const WhereExpr& other) {
                mExprString += " or " + other.mExprString;
                mWhereParams.insertAll(other.mWhereParams);
                return *this;
            }
        };

    protected:
        AString mWhereExpr;
        AVector<AVariant> mWhereParams;

        WhereStatement(ASqlBuilder& builder, const AString& sql);
        void whereImpl(const WhereExpr& w);
    };

    class API_AUI_DATA Select: public WhereStatement {
        friend class ASqlBuilder;
    private:
        AVector<AVariant> mData;

        Select(ASqlBuilder& builder, const AString& sql);

    public:
        Select(const Select&) = delete;
        ~Select() override = default;

        Select& where(const WhereExpr& w) {
            whereImpl(w);
            return *this;
        }

        /**
         * @brief Gets query result.
         * @return query result
         */
        AVector<AVector<AVariant>> get();

        /**
         * @brief Gets query result in ORM.
         * @return query result in ORM
         */
        template<class Model>
        AVector<Model> as() {
            auto idField = AField<ASqlModel<Model>>::make(&ASqlModel<Model>::id);
            AVector<Model> result;
            result.reserve(0x100);

            mSql += " ";
            mSql += mWhereExpr;
            auto dbResult = Autumn::get<ASqlDatabase>()->query(mSql, mWhereParams);

            AVector<size_t> sqlColumnToModelFieldIndexMapping;
            AVector<_<AField<Model>>> fields;
            fields << AModelMeta<Model>::getFields().valueVector();
            /*
            {
                sqlColumnToModelFieldIndexMapping.resize(dbResult->getColumns().size());
                AMap<AString, size_t> columnNameToFieldIndexMapping;
                size_t counter = 0;
                for (auto& column : dbResult->getColumns()) {
                    columnNameToFieldIndexMapping[column.name] = counter++;
                }

                for (std::pair<AString, _<AField<Model>>>& field : AModelMeta<Model>::fields) {
                    sqlColumnToModelFieldIndexMapping.push_back(columnNameToFieldIndexMapping[field.first]);
                    fields << field.second;
                }
            }*/

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
    };

    class API_AUI_DATA Update: public WhereStatement {
        friend class ASqlBuilder;
    private:
        AMap<AString, AVariant> mValues;

        Update(const AMap<AString, AVariant>& values, ASqlBuilder& builder, const AString& sql);

    public:
        Update(const Update&) = delete;
        ~Update() override;

        Update& where(const WhereExpr& w) {
            whereImpl(w);
            return *this;
        }

        void set(const AString& key, const AVariant& value) {
            mValues[key] = value;
        }
    };

    class API_AUI_DATA Delete: public WhereStatement {
        friend class ASqlBuilder;
    private:
        AVector<AVariant> mData;

        Delete(ASqlBuilder& builder, const AString& sql);

    public:
        Delete(const Delete&) = delete;
        ~Delete() override;

        Delete& where(const WhereExpr& w) {
            whereImpl(w);
            return *this;
        }
    };

    ASqlBuilder(const AString& tableName);

    /**
     * @brief Does the INSERT query to DB.
     * @param columnNames column names
     * @return helper object for adding rows
     */
    Insert insert(const AStringVector& columnNames);


    /**
     * @brief Does the INSERT query to DB.
     * @param args... column names
     * @return helper object for adding rows
     */
    template<typename... Args>
    Insert ins(Args&&... args) {
        AStringVector v;
        aui::parameter_pack::for_each([&](const AString& s) {
            v << s;
        }, std::forward<Args>(args)...);
        return insert(v);
    }


    /**
     * @brief Does the SELECT query to DB.
     * @param columnNames column names
     * @return helper object for retrieving rows
     */
    Select select(const AStringVector& columnNames = {});

    /**
     * @brief Does the SELECT query to DB.
     * @param args... column names
     * @return helper object for retrieving rows
     */
    template<typename... Args>
    Select sel(Args&&... args) {
        AStringVector v;
        aui::parameter_pack::for_each([&](const AString& s) {
            v << s;
        }, std::forward<Args>(args)...);
        return select(v);
    }

    /**
     * @brief Does the UPDATE query to DB.
     * @param data data pairs {key, value}
     * @return helper object for adding data (additional data)
     */
    Update update(const AMap<AString, AVariant>& data = {});

    /**
     * @brief Does the DELETE query to DB.
     * @return helper object for WHERE condition
     */
    Delete remove();


    /**
     * @brief Inserts ORM object.
     * @tparam Model ORM
     * @param model ORM
     * @return row id
     */
    template<typename Model>
    id_t insertORM(const Model& model) {
        Insert insertStatement = insert(AModelMeta<Model>::getFields().keyVector());

        AVector<AVariant> row;
        for (std::pair<AString, _<AField<Model>>> field : AModelMeta<Model>::getFields()) {
            row << field.second->get(model);
        }
        insertStatement.row(row);
        return insertStatement.rowId();
    }

    /**
     * @brief Updates ORM object.
     * @tparam Model ORM
     * @param model ORM
     */
    template<typename Model>
    void updateORM(const Model& model) {
        Update updateStatement = update();

        AVector<AVariant> row;
        for (const std::pair<AString, _<AField<Model>>>& field : AModelMeta<Model>::getFields()) {
            if (field.first != "id")
                updateStatement.set(field.first, field.second->get(model));
        }
        updateStatement.where(WhereStatement::WhereExpr("id") == model.id);
    }

    /**
     * @brief Removes ORM object.
     * @tparam Model ORM
     * @param model ORM
     */
    template<typename Model>
    void removeORM(const Model& model) {
        remove().where(WhereStatement::WhereExpr("id") == model.id);
    }
};


typedef ASqlBuilder table;
typedef ASqlBuilder::WhereStatement::WhereExpr col;