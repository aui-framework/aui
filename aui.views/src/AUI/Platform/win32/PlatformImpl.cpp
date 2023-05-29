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

#include "AUI/Platform/APlatform.h"
#include "AUI/Common/AString.h"
#include "AUI/IO/APath.h"
#include <AUI/Util/kAUI.h>

#include <Windows.h>
void ADesktop::playSystemSound(SystemSound s)
{
    switch (s)
    {
    case S_QUESTION:
        PlaySound(L"SystemQuestion", nullptr, SND_ASYNC);
        break;

    case S_ASTERISK:
        PlaySound(L"SystemAsterisk", nullptr, SND_ASYNC);
        break;

    }
}

float APlatform::getDpiRatio()
{
    typedef UINT(WINAPI *GetDpiForSystem_t)();
    static auto GetDpiForSystem = (GetDpiForSystem_t)GetProcAddress(GetModuleHandleA("User32.dll"), "GetDpiForSystem");
    if (GetDpiForSystem) {
        return GetDpiForSystem() / 96.f;
    }
    return 1.f;
}

void APlatform::openUrl(const AUrl& url) {
    ShellExecute(nullptr, L"open", url.c_str(), nullptr, nullptr, SW_NORMAL);
}
