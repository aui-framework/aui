#pragma once

#include <Windows.h>
#include <type_traits>

/**
 * @brief Helper class implementing COM AddRef Release, and QueryInterface.
 * @ingroup windows_specific
 */
template<typename Child, typename Base>
class AComBase: public Base {
public:
    virtual ~AComBase() = default;

    ULONG AddRef(void) override {
        ++mRefCounter;
        return 0;
    }

    ULONG Release(void) override {
        if (--mRefCounter == 0) {
            delete this;
        }
        return 0;
    }

    HRESULT QueryInterface(const IID& riid, void** ppv) override {
        static const QITAB qit[] = {
                QITABENT(Child, Base),
                { 0 },
        };
        return QISearch(this, qit, riid, ppv);
    }

private:
    std::atomic_uint mRefCounter = 1;
};


