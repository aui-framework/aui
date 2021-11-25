//
// Created by Alex2772 on 11/24/2021.
//

#include <AUI/Traits/callables.h>
#include <AUI/IO/StringStream.h>
#include <AUI/Xml/AXml.h>
#include "AText.h"
#include "AButton.h"
#include <stack>
#include <AUI/Util/kAUI.h>
#include <chrono>
#include <utility>
#include <AUI/Traits/strings.h>

struct State {
    AFontStyle fontStyle;
    bool bold = false;
    bool italic = false;
};



_<AText> AText::fromItems(std::initializer_list<std::variant<AString, _<AView>>> init) {
    auto text = aui::ptr::manage(new AText);
    AVector<_<AWordWrappingEngine::Entry>> entries;
    text->mWordEntries.reserve(init.size());
    entries.reserve(init.size());
    for (auto& item : init) {
        std::visit(aui::lambda_overloaded {
            [&](const AString& string) {
                for (auto& w : string.split(' ')) {
                    text->mWordEntries.emplace_back(text.get(), w);
                    entries << aui::ptr::fake(&text->mWordEntries.last());
                }
            },
            [&](const _<AView>& view) {
                text->addView(view);
                entries << _new<ViewEntry>(view);
            },
        }, item);
    }

    text->mEngine.setEntries(std::move(entries));
    return text;
}

_<AText> AText::fromHtml(const AString& html) {
    StringStream stringStream(html);

    struct: IXmlDocumentVisitor {

        struct CommonEntityVisitor: IXmlEntityVisitor {
            _<AText> text = aui::ptr::manage(new AText());
            AVector<_<AWordWrappingEngine::Entry>> entries;

            struct State {
                AFontStyle fontStyle;
                bool bold = false;
                bool italic = false;
            } currentState;
            std::stack<State> stateStack;

            CommonEntityVisitor() {}

            void visitAttribute(const AString& name, AString value) override {};
            _<IXmlEntityVisitor> visitEntity(AString entityName) override {

                struct ViewEntityVisitor: IXmlEntityVisitor {
                    CommonEntityVisitor& parent;
                    AString name;
                    AMap<AString, AString> attrs;

                    ViewEntityVisitor(CommonEntityVisitor& parent, AString name) : parent(parent), name(std::move(name)) {}

                    void visitAttribute(const AString& attributeName, AString value) override {
                        attrs[attributeName] = std::move(value);
                    }

                    _<IXmlEntityVisitor> visitEntity(AString entityName) override {
                        return nullptr;
                    }

                    void visitTextEntity(const AString& entity) override {
                        IXmlEntityVisitor::visitTextEntity(entity);
                    }

                    ~ViewEntityVisitor() override {
                        parent.entries << _new<ViewEntry>(_new<AButton>("ты пидор {}"_format(name)));
                    }
                };

                return _new<ViewEntityVisitor>(*this, std::move(entityName));
            };
            void visitTextEntity(const AString& entity) override {
                for (auto& w : entity.split(' ')) {
                    text->mWordEntries.emplace_back(text.get(), w);
                    entries << aui::ptr::fake(&text->mWordEntries.last());
                }
            };

        } entityVisitor;
        _<IXmlEntityVisitor> visitEntity(AString entityName) override {
            return aui::ptr::fake(&entityVisitor);
        }
    } visitor;
    AXml::read(aui::ptr::fake(&stringStream), aui::ptr::fake(&visitor));

    auto text = std::move(visitor.entityVisitor.text);
    text->mEngine.setEntries(std::move(visitor.entityVisitor.entries));

    return text;
}

int AText::getContentMinimumWidth() {
    return 10;
}

int AText::getContentMinimumHeight() {
    return mPrerenderedString ? mPrerenderedString->getHeight() : 0;
}

void AText::render() {
    AViewContainer::render();

    if (!mPrerenderedString) {
        mEngine.setTextAlign(TextAlign::JUSTIFY);
        mEngine.performLayout({ mPadding.left, mPadding.top }, getSize());
        {
            auto multiStringCanvas = Render::newMultiStringCanvas(getFontStyle());
            for (auto& wordEntry : mWordEntries) {
                multiStringCanvas->addString(wordEntry.getPosition(), wordEntry.getWord());
            }
        }
        {
            auto multiStringCanvas = Render::newMultiStringCanvas(getFontStyle());
            for (auto& wordEntry : mWordEntries) {
                multiStringCanvas->addString(wordEntry.getPosition(), wordEntry.getWord());
            }
            mPrerenderedString = multiStringCanvas->build();
        }
    }
    if (mPrerenderedString) {
        mPrerenderedString->draw();
    }
}

void AText::setSize(int width, int height) {
    AViewContainer::setSize(width, height);
    mPrerenderedString = nullptr;
}

glm::ivec2 AText::WordEntry::getSize() {
    return { mText->getFontStyle().getWidth(mWord) + mText->getFontStyle().getSpaceWidth(), mText->getFontStyle().size };
}

void AText::WordEntry::setPosition(const glm::ivec2& position) {
    mPosition = position;
}

Float AText::WordEntry::getFloat() const {
    return Float::NONE;
}


glm::ivec2 AText::ViewEntry::getSize() {
    return { mView->getMinimumWidth() + mView->getMargin().horizontal(), mView->getMinimumHeight() + mView->getMargin().vertical() };
}

void AText::ViewEntry::setPosition(const glm::ivec2& position) {
    mView->setGeometry(position + glm::ivec2{mView->getMargin().left, mView->getMargin().top},
                       mView->getMinimumSize());
}

Float AText::ViewEntry::getFloat() const {
    return Float::LEFT;
}

