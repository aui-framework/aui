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

#include <cassert>
#include "Dll.h"
#include "AUI/Common/AString.h"

#if defined(_WIN32)
#else
#include <unistd.h>
#include <AUI/IO/APath.h>
#include <AUI/Common/AStringVector.h>
#include <AUI/Traits/arrays.h>

#endif

_<Dll> Dll::load(const AString& path)
{
#ifdef _MSC_VER
    auto fullname = path + "." + getDllExtension();
#else
    auto doLoad = [](const APath& fp) -> _<Dll> {
        auto name = fp.toStdString();
        auto lib = dlopen(name.c_str(), RTLD_LAZY);
        if (lib) {
            return _<Dll>(new Dll(lib));
        }
        return nullptr;
    };

    if (APath(path).isRegularFileExists()) {
        return doLoad(path);
    }
    auto fullname = "lib" + path + "." + getDllExtension();
#endif
#if defined(_WIN32)
	auto lib = LoadLibrary(fullname.c_str());
	if (!lib)
	{
		throw DllLoadException("Could not load shared library: " + fullname + ": " + AString::number(int(GetLastError())));
	}
	return _<Dll>(new Dll(lib));
#elif defined(__ANDROID__)
	auto name = ("lib" + fullname).toStdString();
	auto lib = dlopen(name.c_str(), RTLD_LAZY);
	if (!lib)
	{
		throw DllLoadException("Could not load shared library: " + fullname + ": " + dlerror());
	}
	return _<Dll>(new Dll(lib));
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

AString Dll::getDllExtension()
{
#if defined(_WIN32)
	return "dll";
#else
	return "so";
#endif
}

void(*Dll::getProcAddressRawPtr(const AString& name) const noexcept)()
{
#if defined(_WIN32)
	auto r = reinterpret_cast<void(*)()>(
		GetProcAddress(mHandle, name.toStdString().c_str()));
#else
    auto r = reinterpret_cast<void(*)()>(
            dlsym(mHandle, name.toStdString().c_str()));
#endif
    assert(r);
    return r;
}

DllLoadException::~DllLoadException() = default;
