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