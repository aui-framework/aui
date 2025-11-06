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

#include <AUI/Platform/Entry.h>
#include "AUI/Platform/AWindow.h"
#include "AUI/Util/UIBuildingHelpers.h"
#include "AUI/View/AScrollArea.h"
#include "AUI/View/ASpinnerV2.h"
#include "AUI/View/AForEachUI.h"
#include "AUI/Model/AListModel.h"
#include "AUI/Thread/AAsyncHolder.h"
#include "AUI/View/AText.h"
#include "AUI/View/ADrawableView.h"

using namespace declarative;
using namespace ass;

static auto headerWithContents(_<AView> content) {
    auto result = Stacked {
        ScrollArea { .content = content } AUI_WITH_STYLE {
              Expanding(),
              Padding { 80_dp, 0, 0 },
            } AUI_LET { it->setExtraStylesheet(
                     AStylesheet {
                         {
                             t<AScrollAreaViewport>(),
                             AOverflow::VISIBLE,
                         },
                     }
                     ); },
        Vertical::Expanding {
          Centered {
            Horizontal {
                  Label { "boll" } AUI_WITH_STYLE {
                      FixedSize { 60_dp },
                      BorderRadius { 60_dp / 2.f },
                      Padding { 0 },
                      ATextAlign::CENTER,
                      Backdrop {
                          Backdrop::LiquidFluid {},
                      },
//                      Border { 1_dp, AColor::GRAY.transparentize(0.7f) },
                      BoxShadow { 0, 32_dp, 32_dp, AColor::BLACK.transparentize(0.8f) },
                  },
                  Label { "boll" } AUI_WITH_STYLE {
                      FixedSize { 60_dp },
                      BorderRadius { 60_dp / 2.f },
                      Padding { 0 },
                      ATextAlign::CENTER,
                      AOverflow::HIDDEN_FROM_THIS,
                      AOverflowMask::ROUNDED_RECT,
                      Backdrop {
                          Backdrop::LiquidFluid {},
                          Backdrop::GaussianBlur { 5_dp },
                      },
//                      Border { 1_dp, AColor::GRAY.transparentize(0.6f) },
                      BoxShadow { 0, 32_dp, 32_dp, AColor::BLACK.transparentize(0.8f) },
                      BackgroundSolid { AColor::WHITE.transparentize(0.5f) },
                  },
              }
          } AUI_WITH_STYLE { Padding { 50_dp } },
        },
    };
    return result;
}

AUI_ENTRY {
    auto window = _new<AWindow>("Backdrop test", 600_dp, 300_dp);

    window->setContents(headerWithContents(
        Centered {
          Vertical::Expanding {
            Centered {
              Icon { ":logo.svg" } AUI_WITH_STYLE { FixedSize { 128_dp } },
            } AUI_WITH_STYLE { FixedSize { 500_dp, {} } },
            AText::fromString(
                "    — Eh bien, mon prince. Gênes et Lucques ne sont plus que des apanages, des "
                "поместья, de la famille Buonaparte. Non, je vous préviens que si vous ne me dites pas "
                "que nous avons la guerre, si vous vous permettez encore de pallier toutes les infamies, "
                "toutes les atrocités de cet Antichrist (ma parole, j'y crois) — je ne vous connais "
                "plus, vous n'êtes plus mon ami, vous n'êtes plus мой верный раб, comme vous dites 1. "
                "Ну, здравствуйте, здравствуйте. Je vois que je vous fais peur, садитесь и "
                "рассказывайте."),
            AText::fromString(
                "    Так говорила в июле 1805 года известная Анна Павловна Шерер, фрейлина и "
                "приближенная императрицы Марии Феодоровны, встречая важного и чиновного князя Василия, "
                "первого приехавшего на ее вечер. Анна Павловна кашляла несколько дней, у нее был грипп, "
                "как она говорила (грипп был тогда новое слово, употреблявшееся только редкими). В "
                "записочках, разосланных утром с красным лакеем, было написано без различия во всех:"),
            AText::fromString(
                "    «Si vous n'avez rien de mieux à faire, Monsieur le comte (или mon prince), et si la "
                "perspective de passer la soirée chez une pauvre malade ne vous effraye pas trop, je "
                "serai charmée de vous voir chez moi entre 7 et 10 heures. Annette Scherer» 3."),
            AText::fromString(
                "    — Dieu, quelle virulente sortie! 4 — отвечал, нисколько не смутясь такою встречей, "
                "вошедший князь, в придворном, шитом мундире, в чулках, башмаках и звездах, с светлым "
                "выражением плоского лица."),
            AText::fromString(
                "    Он говорил на том изысканном французском языке, на котором не только говорили, но и "
                "думали наши деды, и с теми, тихими, покровительственными интонациями, которые "
                "свойственны состаревшемуся в свете и при дворе значительному человеку. Он подошел к "
                "Анне Павловне, поцеловал ее руку, подставив ей свою надушенную и сияющую лысину, и "
                "покойно уселся на диване.") } AUI_WITH_STYLE { MaxSize { 550_dp, {} }, Padding { 16_dp } },
        }));

    window->show();

    return 0;
}