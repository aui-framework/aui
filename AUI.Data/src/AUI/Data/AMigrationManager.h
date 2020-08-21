#pragma once
#include <functional>
#include <AUI/Common/ADeque.h>
#include <AUI/Data.h>
#include "AUI/Common/AString.h"


class API_AUI_DATA AMigrationManager
{
private:
	ADeque<std::pair<AString, std::function<void()>>> mMigrations;
	
public:
	void registerMigration(const AString& description, const std::function<void()>& migrationCode);

	void doMigration();
};
