#pragma once

#include "AModelMeta.h"
#include <AUI/Data/ASqlBuilder.h>

#include <utility>

/**
 * \brief Тип, определеяющий модель, которая может хранится в базе данных SQL.
 *        Реализовывает для этого типа запросы в базу данных (insert, update, select, delete)
 * \tparam Model модель (тип).
 *                   Класс модели должен реализовывать AModelMeta (см. AUI/Data/AModelMeta.h)
 */
template<typename Model>
struct ASqlModel {
    /**
     * \brief Выплёвывается, когда ожидается получение одной строки, но база данных не вернула ни одной строки.
     */
    class NoSuchRowException: public AException {};

    /**
     * \brief Выплёвывается, когда ожидается получение одной строки, но база данных вернула больше одной строки.
     */
    class TooManyRowsException: public AException {};


    typedef AModelMeta<Model> Meta;
    id_t id = 0;

    /**
     * \brief Сохраняет в БД эту модель.
     *        Если id = 0, то будет создана новая строка в таблице, а id созданной строки присвоится в поле структуры.
     *        Если id != 0, то будет обновлена существующая строка в таблице.
     */
    void save() {
        if (id == 0) {
            id = table(Meta::getSqlTable()).insertORM((Model&)*this);
        } else {
            table(Meta::getSqlTable()).updateORM((Model&)*this);
        }
    }

    /**
     * \brief Удаляет строку из БД по ID.
     */
    void remove() {
        assert(id != 0);
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
            if (mWhereExpr.empty()) {
                mWhereExpr = "WHERE " + exprString;
                mWhereParams = whereParams;
            } else {
                mWhereExpr += "AND " + exprString;
                mWhereParams << whereParams;
            }
            return *this;
        }

        /**
         * \brief Получить результат запроса в виде ORM.
         * \return результат запроса
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
         * \brief Выполнить запрос и получить первый элемент
         * \throws NoSuchRowException, когда база данных не вернула ни одной строки
         *         TooManyRowsException, когда база данных вернула больше одной строки
         * \return ORM структура
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
        return _<IncompleteSelectRequest>(new IncompleteSelectRequest("SELECT " + (columnNames.empty() ? '*' : columnNames.join(',')) + " FROM " + AModelMeta<Model>::getSqlTable(), expression));
    }

    /**
     * \brief Получить строку из таблицы по ID.
     * \param id ID требуемой строки
     * \return строка таблицы по указанному ID
     * \throws NoSuchRowException, если не найдена строка по указанному ID
     */
    static Model byId(id_t id) {
        AStringVector columns;
        columns << "id";
        columns << Meta::getFields().keyVector();
        auto result = table(Meta::getSqlTable()).select(columns).where(col("id") == id)
                .template as<Model>();

        if (result.empty())
            throw NoSuchRowException();
        assert(result.size() == 1);
        return result.first();
    }


    /**
     * \brief Создаёт модель и сохраняет её в базу данных.
     * \tparam Args типы аргументов конструктора
     * \param args аргументы конструктора
     * \return ORM структура
     */
    template<typename ... Args>
    static Model make(Args&&... args) {
        Model m = {0, std::forward<Args>(args)...};
        m.save();
        return m;
    }


    /**
     * \return название relation столбца.
     * \example struct User -> таблица users -> столбец user_id - возвращаемое значение.
     */
    static AString getIdColumnNameInOtherTables() {
        AString tableName = AModelMeta<Model>::getSqlTable();
        if (tableName.endsWith("s")) {
            tableName = tableName.mid(0, tableName.length() - 1);
        }
        tableName += "_id";
        return tableName;
    }

protected:    /* ORM RELATIONSHIP */

    /**
     * \brief Реализация one-to-many связи между ORM структурами. Используется в паре с belongsTo.
     * <pre>
     * User::getPosts() -> hasMany<Post>()<br />
     * &nbsp;&nbsp;|-- Post::getAuthor() -> belongsTo<Post>()<br />
     * &nbsp;&nbsp;|-- Post::getAuthor() -> belongsTo<Post>()<br />
     * &nbsp;&nbsp;|-- Post::getAuthor() -> belongsTo<Post>()<br />
     * &nbsp;&nbsp;....
     *  </pre>
     * \tparam Other ORM-тип, с которым нужно установить связь
     * \return незавершённый SQL запрос (см. ASqlModel::IncompleteSelectRequest)
     * \see ASqlModel::IncompleteSelectRequest
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

