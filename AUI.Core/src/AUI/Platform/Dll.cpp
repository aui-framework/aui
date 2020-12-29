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
