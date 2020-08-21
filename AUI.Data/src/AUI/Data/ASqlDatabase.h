#pragma once

#include <AUI/Data.h>
#include "AUI/Common/SharedPtrTypes.h"
#include "AUI/Common/AMap.h"
#include "ISqlDriver.h"
#include "AUI/Common/AVector.h"
#include "AUI/Common/AVariant.h"
#include "ASqlQueryResult.h"
#include "ASqlDriverType.h"

class AString;

class API_AUI_DATA ASqlDatabase
{
private:
	static AMap<AString, _<ISqlDriver>>& getDrivers();

	_<ISqlDatabase> mDriverInterface;

	explicit ASqlDatabase(const _<ISqlDatabase>& driver_interface, const AString& driverName)
		: mDriverInterface(driver_interface)
	{
	}

public:
	~ASqlDatabase();

	/**
	 * \brief Выполнить запрос с получением результата (SELECT).
	 *
	 * \param query SQL запрос
	 * \param params аргументы запроса
	 * \return результат запроса
	 * \throws SQLException при возникновении какой-либо ошибки
	 */
	_<ASqlQueryResult> query(const AString& query, const AVector<AVariant>& params = {});

	/**
	 * \brief Выполнить запрос без результата (UPDATE, INSERT, DELETE и т. д.)
	 * 
	 * \param query SQL запрос
	 * \param params аргументы запроса
	 * \return количество затронутых строк (affected rows)
	 * \throws SQLException при возникновении какой-либо ошибки
	 */
	int execute(const AString& query, const AVector<AVariant>& params = {});


	/**
	 * \brief Подключиться к базе данных, используя указанные реквизиты и драйвер.
	 * \param driverName название драйвера БД. Если драйвер не загружен, то будет предпринята попытка загрузить драйвер
	 *                   по шаблону AUI.НАЗВАНИЕДРАЙВЕРА.dll. Если драйвер не получилось загрузить, то будет выплюнут
	 *                   SQLException.
	 *			         
	 * \param address хост сервера (IP адрес или домен)
	 * \param port порт сервера
	 * \param databaseName название БД
	 * \param username имя пользователя; в некоторых БД - необязательно
	 * \param password пароль пользователя; в некоторых БД - необязательно
	 * \return объект для связи с БД; в некоторых БД - необязательно
	 * \throws SQLException при возникновении какой-либо ошибки
	 */
	static _<ASqlDatabase> connect(const AString& driverName, const AString& address, uint16_t port = 0,
	                               const AString& databaseName = {}, const AString& username = {},
	                               const AString& password = {});

	/**
	 * \brief Тип драйвера. Требуется для коррекции запросов в БД в связи с различием драйверов.
	 * \return тип драйвера
	 */
	SqlDriverType getDriverType();

	static void registerDriver(_<ISqlDriver> driver);
};
