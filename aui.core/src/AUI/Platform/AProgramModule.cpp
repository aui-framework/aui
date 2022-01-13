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
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

#include <cassert>
#include "AProgramModule.h"
#include "AUI/Common/AString.h"

#if AUI_PLATFORM_WIN
#else
#include <unistd.h>
#include <AUI/IO/APath.h>
#include <AUI/Common/AStringVector.h>
#include <AUI/Traits/arrays.h>

#endif

_<AProgramModule> AProgramModule::load(const AString& path)
{
#if AUI_PLATFORM_WIN
    auto fullname = path + "." + getDllExtension();
#else
    auto doLoad = [](const APath& fp) -> _<Dll> {
        auto name = fp.toStdString();
        auto lib = dlopen(name.c_str(), RTLD_LAZY);
        if (lib) {
            return aui::ptr::manage(new Dll(lib));
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
		throw DllLoadException("Could not load shared library: " + fullname + ": " + dlerror());
	}
	return aui::ptr::manage(new Dll(lib));
#else
	char buf[0x1000];
	getcwd(buf, sizeof(buf));

	APath paths[] = {
            ""_as,
            AString(buf) + "/",
            AString(buf) + "/../lib/",
            "/usr/local/lib/"_as,
            "/usr/lib/"_as,
            "/lib/"_as,
    };

	std::string dlErrors[aui::array_length(paths)];

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

    throw DllLoadException(diagnostic);
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
