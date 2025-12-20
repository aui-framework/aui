/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <range/v3/all.hpp>

#include <AUI/Platform/AWindow.h>
#include <AUI/Platform/Entry.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AScrollArea.h>
#include <AUI/View/ASplitter.h>
#include <AUI/View/ATextArea.h>
#include <AUI/Model/AListModel.h>
#include "AUI/View/AButton.h"
#include "AUI/View/ADrawableView.h"
#include "AUI/Json/Conversion.h"
#include "AUI/IO/AFileInputStream.h"
#include "AUI/Reflect/for_each_field.h"
#include "AUI/Platform/AMessageBox.h"
#include "AUI/View/ASpacerFixed.h"
#include "AUI/View/Dynamic.h"
#include <AUI/View/AForEachUI.h>

static constexpr auto LOG_TAG = "Notes";

using namespace declarative;
using namespace ass;

struct Note {
    AProperty<AString> title;
    AProperty<AString> content;
};

AJSON_FIELDS(Note, AJSON_FIELDS_ENTRY(title) AJSON_FIELDS_ENTRY(content))

class TitleTextArea : public ATextArea {
public:
    using ATextArea::ATextArea;
    void onCharEntered(AChar c) override {
        if (c == '\r') {
            AWindow::current()->focusNextView();
            return;
        }
        ATextArea::onCharEntered(c);
    }
};

/// [notePreview]
_<AView> notePreview(const _<Note>& note) {
    auto stringOneLinePreview = [](const AString& s) -> AString {
        if (s.empty()) {
            return "Empty";
        }
        return s.restrictLength(100, "").replacedAll('\n', ' ');
    };

    return Vertical {
        Label {
          .text = AUI_REACT(stringOneLinePreview(note->title)),
        } AUI_OVERRIDE_STYLE { FontSize { 10_pt }, ATextOverflow::ELLIPSIS },
        Label {
          .text = AUI_REACT(stringOneLinePreview(note->content)),
        } AUI_OVERRIDE_STYLE {
              ATextOverflow::ELLIPSIS,
              Opacity { 0.7f },
            },
    } AUI_OVERRIDE_STYLE {
        Padding { 4_dp, 8_dp },
        BorderRadius { 8_dp },
        Margin { 4_dp, 8_dp },
    };
}
/// [notePreview]

/// [noteEditor]
_<AView> noteEditor(const _<Note>& note) {
    if (note == nullptr) {
        return Centered { Label { "No note selected" } };
    }
    return AScrollArea::Builder().withContents(
        Vertical {
          _new<TitleTextArea>("Untitled") AUI_LET {
                  it->setCustomStyle({ FontSize { 14_pt }, Expanding { 1, 0 } });
                  AObject::biConnect(note->title, it->text());
                  if (note->content.raw.empty()) {
                      it->focus();
                  }
              },
          _new<ATextArea>("Text") AUI_OVERRIDE_STYLE { Expanding() } && note->content,
        } AUI_OVERRIDE_STYLE {
          Padding { 8_dp, 16_dp },
        });
}
/// [noteEditor]

class MainWindow : public AWindow {
public:
    MainWindow() : AWindow("Notes") {
        setExtraStylesheet(AStylesheet {
          {
            c(".plain_bg"),
            BackgroundSolid { AColor::WHITE },
          },
          {
            t<AWindow>(),
            Padding { 0 },
          },
        });
        load();

        connect(mNotes.changed, me::markDirty);

        setContents(Vertical {
          ASplitter::Horizontal()
                  .withItems({
                    Vertical {
                      Centered {
                        Horizontal {
                          Button { Horizontal { Icon { ":img/save.svg" }, SpacerFixed { 2_dp }, Label { "Save" } },
                                   { me::save } } &
                              mDirty > &AView::setEnabled,
                          Button { Horizontal { Icon { ":img/new.svg" }, SpacerFixed { 2_dp }, Label { "New Note" } },
                                   { me::newNote } },
                        } AUI_OVERRIDE_STYLE { LayoutSpacing { 4_dp }, Padding { 4_dp } },
                      },
                      /// [scrollarea]
                      AScrollArea::Builder()
                          .withContents(
                          AUI_DECLARATIVE_FOR(note, *mNotes, AVerticalLayout) {
                              observeChangesForDirty(note);
                              return notePreview(note) AUI_LET {
                                  connect(it->clicked, [this, note] { mCurrentNote = note; });
                                  it& mCurrentNote > [note](AView& view, const _<Note>& currentNote) {
                                      ALOG_DEBUG(LOG_TAG) << "currentNote == note " << currentNote << " == " << note;
                                      view.setAssName(".plain_bg", currentNote == note);
                                  };
                              };
                          })
                          .build(),
                      /// [scrollarea]
                    } AUI_OVERRIDE_STYLE { MinSize { 200_dp } },

                    Vertical::Expanding {
                      Centered {
                        Button {
                          Horizontal { Icon { ":img/trash.svg" }, SpacerFixed { 2_dp }, Label { "Delete" } },
                          { me::deleteCurrentNote },
                        } AUI_LET {
                            connect(AUI_REACT(mCurrentNote != nullptr), AUI_SLOT(it)::setEnabled);
                        },
                      },
                      experimental::Dynamic { AUI_REACT(noteEditor(mCurrentNote)) } AUI_OVERRIDE_STYLE { Expanding() },
                    } << ".plain_bg" AUI_OVERRIDE_STYLE { MinSize { 200_dp } },
                  })
                  .build() AUI_OVERRIDE_STYLE { Expanding() },
        });

        if (mNotes->empty()) {
            newNote();
        }
    }

    ~MainWindow() {
        if (mDirty) {
            save();
        }
    }

    /// [load]
    void load() {
        try {
            if (!"notes.json"_path.isRegularFileExists()) {
                return;
            }
            aui::from_json(AJson::fromStream(AFileInputStream("notes.json")), mNotes);
        } catch (const AException& e) {
            ALogger::info(LOG_TAG) << "Can't load notes: " << e;
        }
    }
    /// [load]

    /// [save]
    void save() {
        AFileOutputStream("notes.json") << aui::to_json(*mNotes);
        mDirty = false;
    }
    /// [save]

    /// [newNote]
    void newNote() {
        auto note = aui::ptr::manage_shared(new Note { .title = "Untitled" });
        mNotes.writeScope()->push_back(note);
        mCurrentNote = std::move(note);
    }
    /// [newNote]

    /// [deleteCurrentNote]
    void deleteCurrentNote() {
        if (mCurrentNote == nullptr) {
            return;
        }
        if (AMessageBox::show(
                this, "Do you really want to delete this note?",
                "{}\n\nThis operation is irreversible!"_format((*mCurrentNote)->title), AMessageBox::Icon::NONE,
                AMessageBox::Button::OK_CANCEL) != AMessageBox::ResultButton::OK) {
            return;
        }

        auto it = ranges::find(*mNotes, *mCurrentNote);
        it = mNotes.writeScope()->erase(it);
        mCurrentNote = it != mNotes->end() ? *it : nullptr;
    }
    /// [deleteCurrentNote]

    void markDirty() { mDirty = true; }

    void observeChangesForDirty(const _<Note>& note) {
        aui::reflect::for_each_field_value(
            *note,
            aui::lambda_overloaded {
              [&](auto& field) {},
              [&](APropertyReadable auto& field) {
                  ALOG_DEBUG(LOG_TAG) << "Observing for changes " << &field;
                  AObject::connect(field.changed, me::markDirty);
              },
            });
    }

private:
    AProperty<AVector<_<Note>>> mNotes;
    AProperty<_<Note>> mCurrentNote;
    AProperty<bool> mDirty = false;
};

AUI_ENTRY {
    auto w = _new<MainWindow>();
    w->show();
    return 0;
}
