#pragma once


#include <AUI/Util/AWordWrappingEngine.h>
#include "AViewContainer.h"
#include <initializer_list>
#include <variant>
#include <AUI/Enum/WordBreak.h>

class API_AUI_VIEWS AText: public AViewContainer {
public:
    using Flags = AVector<std::variant<WordBreak>>;
    struct ParsedFlags {
        WordBreak wordBreak = WordBreak::NORMAL;
    };

private:
    class CharEntry: public AWordWrappingEngine::Entry {
    private:
        AText* mText;
        char32_t mChar;
        glm::ivec2 mPosition;

    public:
        CharEntry(AText* text, char32_t ch)
            : mText(text), mChar(ch) {}

        glm::ivec2 getSize() override;

        void setPosition(const glm::ivec2& position) override;

        Float getFloat() const override;

        const glm::ivec2& getPosition() const {
            return mPosition;
        }

        char32_t getChar() const {
            return mChar;
        }
    };
    class WordEntry: public AWordWrappingEngine::Entry {
    private:
        AText* mText;
        AString mWord;
        glm::ivec2 mPosition;

    public:
        WordEntry(AText* text, AString word)
            : mText(text), mWord(std::move(word)){}

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

    class WhitespaceEntry: public AWordWrappingEngine::Entry {
    private:
        AText* mText;

    public:
        WhitespaceEntry(AText* text) : mText(text) {}

        glm::ivec2 getSize() override;
        void setPosition(const glm::ivec2& position) override;
        Float getFloat() const override;

        bool escapesEdges() override;

        ~WhitespaceEntry() override = default;
    } mWhitespaceEntry;

    AWordWrappingEngine mEngine;
    ADeque<WordEntry> mWordEntries;
    ADeque<CharEntry> mCharEntries;

    Render::PrerenderedString mPrerenderedString;
    ParsedFlags mParsedFlags;


    AText(): mWhitespaceEntry(this) {}
    void pushWord(AVector<_<AWordWrappingEngine::Entry>>& entries,
                  const AString& word,
                  const ParsedFlags& flags);

    static ParsedFlags parseFlags(const Flags& flags);

public:
    static _<AText> fromItems(std::initializer_list<std::variant<AString, _<AView>>> init, const Flags& flags = {});
    static _<AText> fromHtml(const AString& html, const Flags& flags = {});
    static _<AText> fromString(const AString& string, const Flags& flags = {});

    void render() override;
    void setSize(int width, int height) override;
    int getContentMinimumWidth() override;
    int getContentMinimumHeight() override;
    void prerenderString();
};


