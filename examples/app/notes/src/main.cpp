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

using namespace declarative;
using namespace ass;

struct Note {
    AProperty<AString> title;
    AProperty<AString> content;
};

class MainWindow : public AWindow {
public:
    MainWindow() : AWindow("Notes") {
        setContents(Vertical {
          ASplitter::Horizontal().withItems({
            Vertical {
              _new<AButton>("New Note").connect(&AView::clicked, me::newNote),
              AScrollArea::Builder()
                  .withContents(
                    AUI_DECLARATIVE_FOR(notes, mNotes, AVerticalLayout) {
                      return Vertical {
                          Label {} & notes->title,
                          Label {} & notes->content,
                      };
                    }).build(),
            } with_style {
              MinSize { 100_dp },
            },

            CustomLayout::Expanding {} & mCurrentNote.readProjected([](const _<Note>& currentNote) -> _<AView> {
                if (currentNote == nullptr) {
                    return Centered { Label { "No note selected" } };
                }
                return AScrollArea::Builder()
                  .withContents(Vertical {
                      (_new<ATextArea>("Untitled") with_style {
                          FontSize { 14_pt },
                      }) && currentNote->title,
                      (_new<ATextArea>("Text")) && currentNote->content,
                  });
            }),
          }),
        });

        if (mNotes->empty()) {
            newNote();
        }
    }

    void newNote() {
        auto note = aui::ptr::manage(new Note{ .title = "Untitled" });
        mNotes << note;
        mCurrentNote = std::move(note);
    }

private:
    _<AListModel<_<Note>>> mNotes = _new<AListModel<_<Note>>>();
    AProperty<_<Note>> mCurrentNote;
};

AUI_ENTRY {
    auto w = _new<MainWindow>();
    w->show();
    return 0;
}
