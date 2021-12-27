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
#include <AUI/Platform/AWindow.h>

struct State {
    AFontStyle fontStyle;
    bool bold = false;
    bool italic = false;
};

_<AText> AText::fromString(const AString& string) {
    auto text = aui::ptr::manage(new AText);
    AVector<_<AWordWrappingEngine::Entry>> entries;
    auto splt = string.split(' ');
    entries.reserve(splt.size());
    for (auto& w : splt) {
        text->mWordEntries.emplace_back(text.get(), w);
        entries << aui::ptr::fake(&text->mWordEntries.last());
    }

    text->mEngine.setEntries(std::move(entries));
    return text;
}

_<AText> AText::fromItems(std::initializer_list<std::variant<AString, _<AView>>> init) {
    auto text = aui::ptr::manage(new AText);
    AVector<_<AWordWrappingEngine::Entry>> entries;
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
    struct CommonEntityVisitor: IXmlDocumentVisitor {
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
                    auto view = _new<AButton>("hello {}"_format(name));
                    parent.text->addView(view);
                    parent.entries << _new<ViewEntry>(view);
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
    AXml::read(aui::ptr::fake(&stringStream), aui::ptr::fake(&entityVisitor));

    auto text = std::move(entityVisitor.text);
    text->mEngine.setEntries(std::move(entityVisitor.entries));

    return text;
}

int AText::getContentMinimumWidth() {
    return 10;
}

int AText::getContentMinimumHeight() {
    return mPrerenderedString ? mPrerenderedString->getHeight() : 0;
}

void AText::render() {
    if (!mPrerenderedString) {
        prerenderString();
    }

    AViewContainer::render();

    if (mPrerenderedString) {
        mPrerenderedString->draw();
    }
}

void AText::prerenderString() {
    mEngine.setTextAlign(getFontStyle().align);
    mEngine.setLineHeight(getFontStyle().lineSpacing);
    mEngine.performLayout({mPadding.left, mPadding.top }, getSize());
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
        mPrerenderedString = multiStringCanvas->finalize();
    }
}

void AText::setSize(int width, int height) {
    bool widthDiffers = width != getWidth();
    int prevContentMinimumHeight = getContentMinimumHeight();
    AViewContainer::setSize(width, height);
    if (widthDiffers) {
        prerenderString();

        AThread::current()->enqueue([&]()
                                    {
                                        if (auto p = getParent())
                                            p->updateLayout();
                                    });
    }
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
    return Float::NONE;
}

