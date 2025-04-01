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
