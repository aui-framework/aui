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

#include "AMysql.h"
#include "AUI/Data/ISqlDatabase.h"

#include <mysql.h>
#include "AUI/Data/SQLException.h"
#include <cassert>
#include <AUI/Logging/ALogger.h>

class MysqlStmtResult : public ISqlDriverResult
{
private:
	MYSQL_STMT* mResult;
	AVector<SqlColumn> mColumns;
	AVector<enum enum_field_types> mColumnTypes;

public:
	class Row: public ISqlDriverRow
	{
	private:		
		MysqlStmtResult* mStmtResult;
		AVector<MYSQL_BIND> mResultData;
		AVector<AVariant> mResultVariant;
		
	public:
		explicit Row(MysqlStmtResult* result)
			: mStmtResult(result)
		{
			mResultData.resize(mysql_stmt_field_count(mStmtResult->mResult));
			if (!mResultData.empty()) {
				mResultVariant.reserve(mResultData.size());

				size_t index = 0;
				for (auto& m : mResultData)
				{
					m.buffer_type = mStmtResult->mColumnTypes[index];

					switch (m.buffer_type)
					{
					case MYSQL_TYPE_LONG: {
                        auto o = _new<AVariantHelper<int>>(0);
                        m.buffer = (void*)&o->getData();
                        m.buffer_length = sizeof(o->getData());
                        mResultVariant << o;
                        break;
                    }
					case MYSQL_TYPE_LONGLONG: {
                        auto o = _new<AVariantHelper<uint64_t>>(0);
                        m.buffer = (void*)&o->getData();
                        m.buffer_length = sizeof(o->getData());
                        mResultVariant << o;
                        break;
                    }
					case MYSQL_TYPE_STRING:
					case MYSQL_TYPE_VAR_STRING:
					{
						auto o = _new<AVariantHelper<std::string>>("");
						o->getData().reserve(m.buffer_length = 0x400);
						m.buffer = o->getData().data();
						auto rawPtr = (char*)&o->getData();
						m.length = (unsigned long*)(rawPtr + 20);
						mResultVariant << o;
						break;
					}

					default:
						assert(0);
					}

					index++;
				}
				mysql_stmt_bind_result(mStmtResult->mResult, mResultData.data());
			}
		}

		~Row() override = default;
		AVariant getValue(size_t index) override
		{
			return mResultVariant.at(index);
		}
	};


	explicit MysqlStmtResult(MYSQL_STMT* result) noexcept
		: mResult(result)
	{
		MYSQL_RES* r = mysql_stmt_result_metadata(result);
		
		if (result) {
			mColumns.reserve(mysql_stmt_field_count(result));
			mColumnTypes.reserve(mysql_stmt_field_count(result));
			while (auto field = mysql_fetch_field(r))
			{
				SqlType type;

				switch (field->type)
				{
				case MYSQL_TYPE_VAR_STRING:
					type = ST_STRING;
					break;

				default:
					type = ST_STRING;
					//assert(0);
				}

				mColumns << SqlColumn{ field->name, type };
				mColumnTypes << field->type;
			}
		}
		mysql_free_result(r);
	}


	const AVector<SqlColumn>& getColumns() override
	{
		return mColumns;
	}

	~MysqlStmtResult() override
	{
		if (mResult)
			mysql_stmt_free_result(mResult);
	}

	size_t rowCount() override
	{
		if (mResult)
			return mysql_stmt_num_rows(mResult);
		return 0;
	}

	_<ISqlDriverRow> begin() override
	{
		return next(_new<Row>(this));
	}

	_<ISqlDriverRow> next(const _<ISqlDriverRow>& previous) override
	{
		int r = mysql_stmt_fetch(mResult);
		switch (r)
		{
		case 0:
			return previous;
		case 1:
			throw SQLException(AString("Could not fetch next row: ") + mysql_stmt_error(mResult));
		case MYSQL_NO_DATA:
			return nullptr;
		case MYSQL_DATA_TRUNCATED:
			throw SQLException(AString("SQL driver returned MYSQL_DATA_TRUNCATED; Data truncation is not supported"));
		}

		throw SQLException("mysql_stmt_fetch returned unknown value: " + AString::number(r));
	}
};

class MysqlDatabase : public ISqlDatabase
{
private:
    AString mAddress;
    uint64_t mPort;
    AString mDatabaseName;
    AString mUsername;
    AString mPassword;
    AMap<AAbstractThread*, MYSQL> mMysqls;

	struct STMT
	{
	public:
		MYSQL_STMT* mHandle;

		STMT(MYSQL* mysql)
		{
			mHandle = mysql_stmt_init(mysql);
		}
		~STMT()
		{
			if (mHandle)
				mysql_stmt_close(mHandle);
		}
	};


	void doRequest(STMT& s, const AString& query, const AVector<AVariant>& params)
	{
        auto& mysql = getMysql();
		auto stdQuery = query.toStdString();
		if (mysql_stmt_prepare(s.mHandle, stdQuery.c_str(), stdQuery.length()))
			throw SQLException(AString("Could not prepare statement: ") + mysql_error(&mysql));

		AVector<MYSQL_BIND> b;
		AVector<std::string> strings;
		if (!params.empty()) {
			b.resize(params.size());

			strings.reserve(params.size());

			for (auto& p : params)
				strings << p.toString().toStdString();

			size_t index = 0;
			for (auto& p : strings)
			{
				auto& bindParam = b[index++];

				bindParam.buffer_type = MYSQL_TYPE_VAR_STRING;
				bindParam.buffer = (char*)p.c_str();
				bindParam.buffer_length = p.length();
			}

			mysql_stmt_bind_param(s.mHandle, b.data());
		}
		if (mysql_stmt_execute(s.mHandle))
		{
		    ALogger::debug("error in query:" + query + ": " + mysql_error(&getMysql()));
			throw SQLException(AString("Could not execute prepared statement: ") + mysql_error(&mysql));
		}
	}

	MYSQL& getMysql() {
	    if (auto c = mMysqls.contains(AThread::current().get())) {
	        return const_cast<MYSQL&>(c->second);
	    }
	    auto& mysql = mMysqls[AThread::current().get()];

        if (!mysql_init(&mysql))
            throw SQLException(AString("Could not init mysql"));

        auto r = mysql_set_character_set(&mysql, "utf8");

        if (!mysql_real_connect(&mysql, mAddress.toStdString().c_str(), mUsername.toStdString().c_str(),
                                mPassword.toStdString().c_str(), mDatabaseName.toStdString().c_str(), mPort,
                                nullptr, 0))
            throw SQLException(AString("Could not connect to the database: ") + mysql_error(&mysql));
        return mysql;
	}
	
public:
	MysqlDatabase(const AString& address, uint16_t port, const AString& databaseName,
	              const AString& username, const AString& password):
	              mAddress(address),
	              mPort(port),
	              mDatabaseName(databaseName),
	              mUsername(username),
                  mPassword(password)
	{
	}

	~MysqlDatabase() override
	{
	    for (auto& a : mMysqls) {
            auto mysql = a.second;
	        a.first->enqueue([mysql]() {
	            auto m = mysql;
                mysql_close(&m);
            });
        }
	}


	int execute(const AString& query, const AVector<AVariant>& params) override
	{
		STMT s(&getMysql());

		doRequest(s, query, params);

		return mysql_stmt_affected_rows(s.mHandle);
	}

	_<ISqlDriverResult> query(const AString& query, const AVector<AVariant>& params) override
	{
	    auto& mysql = getMysql();
		STMT s(&mysql);

		doRequest(s, query, params);

		if (mysql_stmt_field_count(s.mHandle) == 0)
			return nullptr;

		auto r = _new<MysqlStmtResult>(s.mHandle);
		s.mHandle = nullptr;
        mysql_store_result(&mysql);
		return r;
	}

	SqlDriverType getDriverType() override {
		return DT_MYSQL;
	}
};

AString AMysql::getDriverName()
{
	return "mysql";
}

_<ISqlDatabase> AMysql::openDriverConnection(const AString& address, uint16_t port, const AString& databaseName,
                                             const AString& username, const AString& password)
{
	return _new<MysqlDatabase>(address, port, databaseName, username, password);
}
