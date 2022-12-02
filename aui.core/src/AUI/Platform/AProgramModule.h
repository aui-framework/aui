// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
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

#pragma once
#include <stdexcept>
#include <string>
#include <AUI/IO/APath.h>
#include <AUI/Common/SharedPtr.h>
#include <AUI/Common/AException.h>

#if AUI_PLATFORM_WIN
#include <windows.h>
#else
#include <dlfcn.h>

typedef void* HMODULE;
#endif
class AString;

class API_AUI_CORE AProgramModuleLoadException: public AException
{
public:
	explicit AProgramModuleLoadException(const AString& message)
		: AException(message)
	{
	}
	virtual ~AProgramModuleLoadException();
};

class API_AUI_CORE AProgramModule
{
private:
	HMODULE mHandle;

	AProgramModule(const AProgramModule&) = delete;
	
public:
    AProgramModule(HMODULE handle) noexcept
            : mHandle(handle)
    {
    }

	[[nodiscard]]
	void(*getProcAddressRawPtr(const AString& name) const noexcept)();

	template <typename Function>
	[[nodiscard]]
	Function* getProcAddress(const AString& name) const noexcept
	{
		return reinterpret_cast<Function*>(getProcAddressRawPtr(name));
	}
	
	/**
	 * @brief Loads a dynamic load library (shared object).
	 * @param path Path to a dynamic load library without extension (extension is added based on current platform)
	 */
	static _<AProgramModule> load(const AString& path);

	/**
	 * @brief Extension of a dynamic load library of current platform
	 * @return dll for Windows, so for Linux, dylib for Apple
	 */
	static AString getDllExtension();

    static _<AProgramModule> self();
};