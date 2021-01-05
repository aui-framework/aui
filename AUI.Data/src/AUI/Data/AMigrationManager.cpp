#include "AMigrationManager.h"
#include "AMeta.h"
#include "AUI/Common/AException.h"
#include "AUI/Util/AError.h"

void AMigrationManager::registerMigration(const AString& description, const std::function<void()>& migrationCode)
{
	mMigrations << std::pair<AString, std::function<void()>>{description, migrationCode};
}

void AMigrationManager::doMigration()
{
	int migrationNumber = AMeta::get("migration").toInt();
	unsigned index = 0;
	
	
	try {
		for (auto& m : mMigrations)
		{
			if (migrationNumber == 0)
			{
				m.second();
			ALogger::info("Successful migration "_i18n + AString::number(index) + " (" + mMigrations.at(index).first + ")");
			}
			else
			{
				migrationNumber -= 1;
			}
			++index;
		}
	} catch (const AException& e)
	{
		AError::handle(e, "Could not finish migration "_i18n + AString::number(index) + " (" + mMigrations.at(index).first + ")");
	}

	AMeta::set("migration", index);
}
