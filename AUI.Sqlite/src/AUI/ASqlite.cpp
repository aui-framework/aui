#include "ASqlite.h"
#include <AUI/Data/ISqlDatabase.h>
#include "sqlite3.h"
#include <AUI/Common/AException.h>
#include <cassert>

class SqliteRow: public ISqlDriverRow {
private:
    sqlite3_stmt* mStmt;

public:
    SqliteRow(sqlite3_stmt* stmt) : mStmt(stmt) {}

    ~SqliteRow() override {

    }

    AVariant getValue(size_t index) override {
        switch (sqlite3_column_type(mStmt, index)) {
            case SQLITE_INTEGER:
                return sqlite3_column_int(mStmt, index);

            case SQLITE_FLOAT:
                return sqlite3_column_double(mStmt, index);

            case SQLITE_TEXT:
                return AString((const char*)sqlite3_column_text(mStmt, index));

            case SQLITE_NULL:
                return nullptr;
        }
        assert(0);
        return AVariant();
    }
};

class SqliteResult: public ISqlDriverResult {
    friend class SqliteDatabase;
private:
    sqlite3_stmt* mStmt;
    AVector<SqlColumn> mColumns;

    /**
     * это нужно для продления жизни строк.
     */
    AVector<std::string> mTempStrings;

public:
    SqliteResult() {}

    ~SqliteResult() override {
        sqlite3_finalize(mStmt);
    }

    const AVector<SqlColumn>& getColumns() override {
        if (mColumns.empty()) {
            for (unsigned i = 0; i < sqlite3_column_count(mStmt); ++i) {
                SqlColumn c;
                c.name = sqlite3_column_name(mStmt, i);
                switch (sqlite3_column_type(mStmt, i)) {
                    case SQLITE_INTEGER:
                        c.type = SqlType::ST_STRING;
                        break;
                }
                mColumns << c;
            }
        }
        return mColumns;
    }

    size_t rowCount() override {
        assert(0);
        return 0;
    }

    _<ISqlDriverRow> begin() override {
        return next(_new<SqliteRow>(mStmt));
    }

    _<ISqlDriverRow> next(const _<ISqlDriverRow>& previous) override {
        switch (sqlite3_step(mStmt)) {
            case SQLITE_DONE:
                return nullptr;
            case SQLITE_ROW:
                return previous;
            case SQLITE_ERROR:
                throw AException("could not fetch row");
        }
        assert(0);
        return nullptr;
    }
};

class SqliteDatabase: public ISqlDatabase {
private:
    sqlite3* mConnection;
public:
    SqliteDatabase(const AString& path) {
        sqlite3_open(path.toStdString().c_str(), &mConnection);
        if (!mConnection) {
            throw AException("could not open database: " + path);
        }
    }

    ~SqliteDatabase() override {
        sqlite3_close(mConnection);
    }

	SqlDriverType getDriverType() override {
        return DT_SQLITE;
    }

    _<ISqlDriverResult> query(const AString& query, const AVector<AVariant>& params) override {
        sqlite3_stmt* stmt;

        auto result = _new<SqliteResult>();

        result->mTempStrings.reserve(params.size() + 1);
        result->mTempStrings << query.toStdString();
        sqlite3_prepare_v2(mConnection, result->mTempStrings.back().c_str(), result->mTempStrings.back().length(), &stmt,
                           nullptr);
        result->mStmt = stmt;

        for (unsigned i = 0; i < params.size(); ++i) {
            switch (params[i].getType()) {
                case AVariantType::AV_NULL:
                    sqlite3_bind_null(stmt, i + 1);
                    break;
                case AVariantType::AV_INT:
                    sqlite3_bind_int(stmt, i + 1, params[i].toInt());
                    break;
                case AVariantType::AV_UINT:
                    sqlite3_bind_int(stmt, i + 1, params[i].toInt());
                    break;
                case AVariantType::AV_FLOAT:
                    sqlite3_bind_double(stmt, i + 1, params[i].toFloat());
                    break;
                case AVariantType::AV_DOUBLE:
                    sqlite3_bind_double(stmt, i + 1, params[i].toDouble());
                    break;
                case AVariantType::AV_STRING:
                    result->mTempStrings << params[i].toString().toStdString();
                    sqlite3_bind_text(stmt, i + 1, result->mTempStrings.back().c_str(), result->mTempStrings.back().length(),
                                      nullptr);
                    break;
                case AVariantType::AV_BOOL:
                    sqlite3_bind_int(stmt, i + 1, params[i].toBool());
                    break;
            }
        }
        if (!stmt) {
            throw AException(AString("could not execute query: ") + sqlite3_errmsg(mConnection));
        }

        return result;
    }

    int execute(const AString& query, const AVector<AVariant>& params) override {
        SqliteDatabase::query(query, params)->begin();
        return sqlite3_last_insert_rowid(mConnection);
    }
};

AString ASqlite::getDriverName() {
    return "sqlite";
}

_<ISqlDatabase> ASqlite::openDriverConnection(const AString& address, uint16_t port, const AString& databaseName,
                                         const AString& username, const AString& password) {
    return _new<SqliteDatabase>(address);
}