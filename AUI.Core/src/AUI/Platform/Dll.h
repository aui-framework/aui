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
	 * \param Путь до библиотеки (без расширения).
	 * \brief Загрузить динамическую библиотеку.
	 */
	static _<Dll> load(const AString& path);

	/**
	 * \brief Расширение динамических библиотек текущей платформы
	 * \return dll для Windows, so для Linux и т. д.
	 */
	static AString getDllExtension();
};