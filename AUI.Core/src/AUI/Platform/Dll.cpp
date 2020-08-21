#include "Dll.h"
#include "AUI/Common/AString.h"

#ifdef _WIN32
#else
#include <unistd.h>
#endif

_<Dll> Dll::load(const AString& path)
{
	auto fullpath = path + "." + getDllExtension();
#ifdef _WIN32
	auto lib = LoadLibrary(fullpath.c_str());
	if (!lib)
	{
		throw DllLoadException("Could not load shared library: " + fullpath);
	}
#else
	char buf[0x2000];
	getcwd(buf, sizeof(buf));
	auto name = (AString(buf) + "/lib" + fullpath).toStdString();
	auto lib = dlopen(name.c_str(), RTLD_LAZY);
	if (!lib)
	{
		throw DllLoadException("Could not load shared library: " + fullpath + ": " + dlerror());
	}
#endif
	return _<Dll>(new Dll(lib));
}

AString Dll::getDllExtension()
{
#ifdef _WIN32
	return "dll";
#else
	return "so";
#endif
}

void(*Dll::getProcAddressRawPtr(const AString& name) const noexcept)()
{
#ifdef _WIN32
	return reinterpret_cast<void(*)()>(
		GetProcAddress(mHandle, name.toStdString().c_str()));
#else
    return reinterpret_cast<void(*)()>(
            dlsym(mHandle, name.toStdString().c_str()));
#endif
}
