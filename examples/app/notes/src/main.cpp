/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <AUI/Platform/AWindow.h>
#include <AUI/Platform/Entry.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AScrollArea.h>
#include <AUI/View/ASplitter.h>
#include <AUI/View/ATextArea.h>
#include <AUI/Model/AListModel.h>
#include "AUI/View/AButton.h"
#include "AUI/View/ADrawableView.h"

using namespace declarative;
using namespace ass;

struct Note {
    AProperty<AString> title;
    AProperty<AString> content;
};

class TitleTextArea : public ATextArea {
public:
    using ATextArea::ATextArea;
    void onCharEntered(char16_t c) override {
        if (c == '\r') {
            AWindow::current()->focusNextView();
            return;
        }
        ATextArea::onCharEntered(c);
    }
};

struct StringOneLinePreview {
    AString operator()(const AString& s) const {
        if (s.empty()) {
            return "Empty";
        }
        return s.restrictLength(100, "").replacedAll('\n', ' ');
    }
};

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

        setContents(Vertical {
          ASplitter::Horizontal()
                  .withItems({
                    Vertical {
                      Centered {
                        Button { Icon { ":img/new.svg" }, Label { "New Note" } }.connect(&AView::clicked, me::newNote),
                      },
                      AScrollArea::Builder()
                          .withContents(
                          AUI_DECLARATIVE_FOR(note, mNotes, AVerticalLayout) {
                              _<AView> view =
                                  Vertical {
                                      Label {} with_style { FontSize { 10_pt }, ATextOverflow::ELLIPSIS } &
                                          note->title.readProjected(StringOneLinePreview {}),
                                      Label {} with_style {
                                                ATextOverflow::ELLIPSIS,
                                                Opacity { 0.7f },
                                              } &
                                          note->content.readProjected(StringOneLinePreview {}),
                                  } with_style {
                                      Padding { 4_dp, 8_dp },
                                      BorderRadius { 8_dp },
                                      Margin { 4_dp, 8_dp },
                                  };
                              AObject::connect(view->clicked, [this, note] { mCurrentNote = note; });
                              AObject::connect(mCurrentNote, view, [note, &view = *view](const _<Note>& currentNote) {
                                  ALogger::info("main") << "currentNote == note " << currentNote << " == " << note;
                                  view.setAssName(".plain_bg", currentNote == note);
                              });
                              return view;
                          })
                          .build(),
                    } with_style { MinSize { 200_dp } },

                    Vertical::Expanding {
                      Centered {
                        Button { Icon { ":img/trash.svg" }, Label { "Delete" } }.connect(
                            &AView::clicked, me::deleteCurrentNote),
                      },
                      CustomLayout::Expanding {} &
                          mCurrentNote.readProjected([&](const _<Note>& currentNote) -> _<AView> {
                              if (currentNote == nullptr) {
                                  return Centered { Label { "No note selected" } };
                              }
                              return AScrollArea::Builder().withContents(
                                  Vertical {
                                    mTitleTextArea =
                                        _new<TitleTextArea>("Untitled") let {
                                            it->setCustomStyle({ FontSize { 14_pt }, Expanding { 1, 0 } });
                                            biConnect(currentNote->title, it->text());
                                        },
                                    _new<ATextArea>("Text") with_style { Expanding() } && currentNote->content,
                                  } with_style {
                                    Padding { 8_dp, 16_dp },
                                  });
                          }),
                    } with_style { MinSize { 200_dp } }
                        << ".plain_bg",
                  })
                  .build() with_style { Expanding() },
        });

        if (mNotes->empty()) {
            newNote();
        }
    }

    void newNote() {
        auto note = aui::ptr::manage(new Note { .title = "Untitled" });
        mNotes << note;
        mCurrentNote = std::move(note);
        mTitleTextArea->focus();
    }

    void deleteCurrentNote() {}

private:
    _<AListModel<_<Note>>> mNotes = _new<AListModel<_<Note>>>();
    AProperty<_<Note>> mCurrentNote;
    _<ATextArea> mTitleTextArea;
};

AUI_ENTRY {
    auto w = _new<MainWindow>();
    w->show();
    return 0;
}
