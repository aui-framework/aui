#pragma once


#include <AUI/Util/AWordWrappingEngine.h>
#include "AViewContainer.h"
#include <initializer_list>
#include <variant>

class API_AUI_VIEWS AText: public AViewContainer {
private:
    class WordEntry: public AWordWrappingEngine::Entry {
    private:
        AText* mText;
        AString mWord;
        _<AFont> mFont;
        glm::ivec2 mPosition;

    public:
        WordEntry(AText* text, const AString& word, const _<AFont>& font = nullptr) : mText(text), mWord(word), mFont(font) {}

        glm::ivec2 getSize() override;

        void setPosition(const glm::ivec2& position) override;

        Float getFloat() const override;

        const glm::ivec2& getPosition() const {
            return mPosition;
        }

        const AString& getWord() const {
            return mWord;
        }
    };

    class ViewEntry: public AWordWrappingEngine::Entry {
    private:
        _<AView> mView;

    public:
        ViewEntry(const _<AView>& view) : mView(view) {}

        glm::ivec2 getSize() override;
        void setPosition(const glm::ivec2& position) override;
        Float getFloat() const override;

        ~ViewEntry() override = default;
    };

    AWordWrappingEngine mEngine;
    ADeque<WordEntry> mWordEntries;

    Render::PrerenderedString mPrerenderedString;


    AText() = default;

public:
    static _<AText> fromItems(std::initializer_list<std::variant<AString, _<AView>>> init);
    static _<AText> fromHtml(const AString& html);

    void render() override;

    void setSize(int width, int height) override;

    int getContentMinimumWidth() override;

    int getContentMinimumHeight() override;

    void prerenderString();
};


