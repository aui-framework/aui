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

#include <ctre.hpp>
#include <AUI/Platform/Entry.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include "AUI/View/ADropdownList.h"
#include "AUI/Model/AListModel.h"
#include "AUI/View/ATextField.h"
#include "AUI/Platform/AMessageBox.h"

using namespace declarative;
using namespace std::chrono;

constexpr auto REGEX_DATE = ctre::match<"([0-9]+)\\.([0-9]+)\\.([0-9]{4})">;

struct DateTextFieldState {
    AProperty<AOptional<system_clock::time_point>> parsed;
};

auto formatDate(system_clock::time_point date) { return "{0:%d}.{0:%m}.{0:%G}"_format(date); }

auto dateTextField(DateTextFieldState& state) {
    return _new<ATextField>() AUI_LET {
        AObject::biConnect(
            state.parsed.biProjected(aui::lambda_overloaded {
              [](const AOptional<system_clock::time_point>& v) -> AString {
                  if (!v) {
                      return "";
                  }
                  return formatDate(*v);
              },
              [](const AString& s) -> AOptional<system_clock::time_point> {
                  auto std = s.toStdString();
                  auto match = REGEX_DATE.match(std);
                  if (!match) {
                      return std::nullopt;
                  }
                  year_month_day ymd(
                      year(std::stoi(match.get<3>().str())), month(std::stoi(match.get<2>().str())),
                      day(std::stoi(match.get<1>().str())));
                  if (!ymd.ok()) {
                      return std::nullopt;
                  }
                  return sys_days(ymd);
              },
            }),
            it->text());
        it & state.parsed > [](AView& textField, const AOptional<system_clock::time_point>& value) {
            textField.setAssName(".red", !value.hasValue());
        };
    };
}

class FlightBookerWindow : public AWindow {
public:
    FlightBookerWindow() : AWindow("AUI - 7GUIs - Book Flight", 150_dp, 50_dp) {
        setExtraStylesheet(AStylesheet { {
          ass::c(".red"),
          ass::BackgroundSolid { AColor::RED },
        } });
        setContents(Centered {
          Vertical {
            _new<ADropdownList>(AListModel<AString>::make({ "one-way flight", "return flight" })) AUI_LET {
                    connect(it->selectionId().readProjected([](int selectionId) { return selectionId == 1; }),
                            mIsReturnFlight);
                },
            dateTextField(mDepartureDate),
            dateTextField(mReturnDate) AUI_LET { connect(mIsReturnFlight, AUI_SLOT(it)::setEnabled); },
            _new<AButton>("Book") AUI_LET {
                    connect(it->clicked, me::book);
                    connect(mIsValid, AUI_SLOT(it)::setEnabled);
                },
          } AUI_WITH_STYLE { LayoutSpacing { 4_dp } },
        });
    }

private:
    DateTextFieldState mDepartureDate { system_clock::now() }, mReturnDate { system_clock::now() };
    AProperty<bool> mIsReturnFlight;
    APropertyPrecomputed<bool> mIsValid = [&] {
        if (!mDepartureDate.parsed->hasValue()) {
            return false;
        }
        if (!mIsReturnFlight) {
            return true;
        }
        if (!mReturnDate.parsed->hasValue()) {
            return false;
        }
        if (mDepartureDate.parsed->value() > mReturnDate.parsed->value()) {
            return false;
        }
        return true;
    };

    void book() {
        AString msg = "Departure - {}"_format(formatDate(mDepartureDate.parsed->value()));
        if (mIsReturnFlight) {
            msg += "\nReturn - {}"_format(formatDate(mReturnDate.parsed->value()));
        }
        AMessageBox::show(this, "You've booked the flight", msg);
    }
};

AUI_ENTRY {
    _new<FlightBookerWindow>()->show();
    return 0;
}
