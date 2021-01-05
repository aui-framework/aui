/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

#pragma once
#include <stdexcept>
#include <string>

#include "AUI/Common/SharedPtr.h"
#include "AUI/Common/AException.h"

#if defined(_WIN32)
#include <windows.h>
#else
#include <dlfcn.h>

typedef void* HMODULE;
#endif
class AString;

class API_AUI_CORE DllLoadException: public AException
{
public:


	explicit DllLoadException(const AString& message)
		: AException(message)
	{
	}
};

class API_AUI_CORE Dll
{
private:
	HMODULE mHandle;

	Dll(const Dll&) = delete;
	
public:
    Dll(HMODULE handle) noexcept
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
	 * \brief Loads a dynamic load library (shared object).
	 * \param path Path to a dynamic load library without extension (extension is added based on current platform)
	 */
	static _<Dll> load(const AString& path);

	/**
	 * \brief Extension of a dynamic load library of current platform
	 * \return dll for Windows, so for Linux, dylib for Apple
	 */
	static AString getDllExtension();
};