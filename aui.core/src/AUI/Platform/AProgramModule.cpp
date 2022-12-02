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

#include <cassert>
#include "AProgramModule.h"
#include "AUI/Common/AString.h"

#if AUI_PLATFORM_WIN
#else
#include <unistd.h>
#include <AUI/IO/APath.h>
#include <AUI/Common/AStringVector.h>


#endif

_<AProgramModule> AProgramModule::load(const AString& path)
{
#if AUI_PLATFORM_WIN
    auto fullname = path + "." + getDllExtension();
#else
    auto doLoad = [](const APath& fp) -> _<AProgramModule> {
        auto name = fp.toStdString();
        auto lib = dlopen(name.c_str(), RTLD_LAZY);
        if (lib) {
            return aui::ptr::manage(new AProgramModule(lib));
        }
        return nullptr;
    };

    if (APath(path).isRegularFileExists()) {
        return doLoad(path);
    }
    auto fullname = "lib" + path + "." + getDllExtension();
#endif
#if AUI_PLATFORM_WIN
	auto lib = LoadLibrary(fullname.c_str());
	if (!lib)
	{
		throw AProgramModuleLoadException("Could not load shared library: " + fullname + ": " + AString::number(int(GetLastError())));
	}
	return aui::ptr::manage(new AProgramModule(lib));
#elif AUI_PLATFORM_ANDROID
	auto name = ("lib" + fullname).toStdString();
	auto lib = dlopen(name.c_str(), RTLD_LAZY);
	if (!lib)
	{
		throw AProgramModuleLoadException("Could not load shared library: " + fullname + ": " + dlerror());
	}
	return aui::ptr::manage(new AProgramModule(lib));
#else
	char buf[0x1000];
	getcwd(buf, sizeof(buf));

	APath paths[] = {
            ""_as,
            "lib/"_as,
            AString(buf) + "/",
            AString(buf) + "/../lib/",
            "/usr/local/lib/"_as,
            "/usr/lib/"_as,
            "/lib/"_as,
    };

	std::string dlErrors[std::size(paths)];

	size_t counter = 0;
	for (auto& fp : paths) {
	    try {
            fp = APath(fp + fullname).absolute();
	    } catch (...) {
	        fp = fp + fullname;
        }
	    try {
            if (fp.isRegularFileExists()) {
                if (auto dll = doLoad(fp)) {
                    return dll;
                }
                dlErrors[counter] = dlerror();
            }
        } catch (...) {

        }
	    ++counter;
	}

	AString diagnostic = "Could not load shared library: " + fullname + "\n";
	counter = 0;
	for (auto& fp : paths) {
	    auto& dlError = dlErrors[counter];
	    diagnostic += " - " + fp + " -> " + (dlError.empty() ? "not found" : dlError) + "\n";
        ++counter;
	}

    throw AProgramModuleLoadException(diagnostic);
#endif
}

AString AProgramModule::getDllExtension()
{
#if AUI_PLATFORM_WIN
	return "dll";
#else
	return "so";
#endif
}

void(*AProgramModule::getProcAddressRawPtr(const AString& name) const noexcept)()
{
#if AUI_PLATFORM_WIN
	auto r = reinterpret_cast<void(*)()>(
		GetProcAddress(mHandle, name.toStdString().c_str()));
#else
    auto r = reinterpret_cast<void(*)()>(
            dlsym(mHandle, name.toStdString().c_str()));
#endif
    return r;
}

AProgramModuleLoadException::~AProgramModuleLoadException() = default;

_<AProgramModule> AProgramModule::self() {
#if AUI_PLATFORM_WIN
    return aui::ptr::manage(new AProgramModule(GetModuleHandle(nullptr)));
#else
    return aui::ptr::manage(new AProgramModule(nullptr));
#endif
}
