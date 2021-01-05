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
#endif

_<Dll> Dll::load(const AString& path)
{
#ifdef _MSC_VER
    auto fullpath = path + "." + getDllExtension();
#else
    auto fullpath = "lib" + path + "." + getDllExtension();
#endif
#if defined(_WIN32)
	auto lib = LoadLibrary(fullpath.c_str());
	if (!lib)
	{
		throw DllLoadException("Could not load shared library: " + fullpath + ": " + AString::number(int(GetLastError())));
	}
#elif defined(__ANDROID__)
	auto name = ("lib" + fullpath).toStdString();
	auto lib = dlopen(name.c_str(), RTLD_LAZY);
	if (!lib)
	{
		throw DllLoadException("Could not load shared library: " + fullpath + ": " + dlerror());
	}
#else
	char buf[0x2000];
	getcwd(buf, sizeof(buf));
	auto name = (AString(buf) + "/" + fullpath).toStdString();
	auto lib = dlopen(name.c_str(), RTLD_LAZY);
	if (!lib)
	{
	    // ../lib/

        name = (AString(buf) + "/../lib/" + fullpath).toStdString();
        lib = dlopen(name.c_str(), RTLD_LAZY);
        if (!lib) {
		    throw DllLoadException("Could not load shared library: " + fullpath + ": " + dlerror());
        }
	}
#endif
	return _<Dll>(new Dll(lib));
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
