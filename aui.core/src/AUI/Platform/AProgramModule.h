/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

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