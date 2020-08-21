#pragma once

#include <AUI/Common/AVariant.h>

template<class Klass>
class AField {
public:
    virtual void set(Klass& object, const AVariant& value) = 0;
    virtual AVariant get(const Klass& object) = 0;

    template<typename T>
    inline static _<AField<Klass>> make(T(Klass::*field));
};

namespace aui::detail {
    template<typename Klass, typename T>
    class AFieldImpl: public AField<Klass> {
    private:
        typedef T(Klass::*field_t);
        field_t mFieldPtr;

    public:
        AFieldImpl(field_t fieldPtr) : mFieldPtr(fieldPtr) {}

        void set(Klass& object, const AVariant& value) override {
            object.*mFieldPtr = value.to<T>();
        }

        AVariant get(const Klass& object) override {
            return object.*mFieldPtr;
        }
    };
}


template<class Klass>
template<typename T>
_<AField<Klass>> AField<Klass>::make(T(Klass::*field)) {
    return _new<aui::detail::AFieldImpl<Klass, T>>(field);
}