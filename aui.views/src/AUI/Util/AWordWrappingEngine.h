#pragma once


#include <AUI/Common/AObject.h>
#include <AUI/Common/AVector.h>
#include <glm/glm.hpp>
#include <AUI/Enum/TextAlign.h>
#include <AUI/Enum/Float.h>

class API_AUI_VIEWS AWordWrappingEngine {
public:
    class Entry {
    public:
        virtual ~Entry() = default;

        virtual glm::ivec2 getSize() = 0;
        virtual void setPosition(const glm::ivec2& position) = 0;

        [[nodiscard]]
        virtual Float getFloat() const = 0;

        [[nodiscard]]
        bool isFloating() const {
            return getFloat() != Float::NONE;
        }
    };

private:
    AVector<_<Entry>> mEntries;
    float mLineHeight = 1.f;
    TextAlign mTextAlign = TextAlign::LEFT;

public:
    void setLineHeight(float lineHeight) {
        mLineHeight = lineHeight;
    }

    void setTextAlign(TextAlign textAlign) {
        mTextAlign = textAlign;
    }

    void setEntries(AVector<_<Entry>> entries) {
        mEntries = std::move(entries);
    }

    void performLayout(const glm::ivec2& offset, const glm::ivec2& size);
};


