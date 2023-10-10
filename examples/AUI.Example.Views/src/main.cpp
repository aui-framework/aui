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

#include "ExampleWindow.h"
#include "AUI/IO/AFileInputStream.h"
#include <AUI/Platform/Entry.h>
#include <AUI/i18n/AI18n.h>
#include <AUI/ASS/AStylesheet.h>

AUI_ENTRY
{
   //    Stylesheet::inst().load(AUrl(":views/style.css").open());
	auto w = _new<ExampleWindow>();
	w->show();
	return 0;
}