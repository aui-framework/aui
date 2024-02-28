// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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
