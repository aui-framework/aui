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
    ASpinlockMutex userChangesText;
};

auto formatDate(system_clock::time_point date) { return "{0:%d}.{0:%m}.{0:%G}"_format(date); }

AOptional<system_clock::time_point> parseDate(AStringView s) {
    auto std = s.bytes();
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
}

auto dateTextField(_<DateTextFieldState> state) {
    return _new<ATextField>() AUI_LET {
        AObject::connect(state->parsed, it, [&it = *it, &state = *state](const AOptional<system_clock::time_point>& value) {
            if (!value) {
                return;
            }
            if (state.userChangesText.is_locked()) {
                return;
            }

            it.text() = formatDate(*value);
        });
        AObject::connect(it->text().changed, state->parsed, [&it = *it, &state = *state](const AString& s) {
            std::unique_lock lock(state.userChangesText);
            state.parsed = parseDate(s);
        });
        AObject::connect(AUI_REACT(ass::PropertyList { state->parsed->hasValue() ? BackgroundSolid(AColor::WHITE) : BackgroundSolid(AColor::RED) }), AUI_SLOT(it)::setCustomStyle);
    };
}

class FlightBookerWindow : public AWindow {
public:
    FlightBookerWindow() : AWindow("AUI - 7GUIs - Book Flight", 150_dp, 50_dp) {
        setContents(Centered {
          Vertical {
            _new<ADropdownList>(AListModel<AString>::make({ "one-way flight", "return flight" })) AUI_LET {
                    AObject::connect(AUI_REACT(mState->isReturnFlight ? 1 : 0), it->selectionId().assignment());
                    AObject::connect(it->selectionId().changed, [this](int newSelection) {
                        mState->isReturnFlight = newSelection == 1;
                    });
                },
            dateTextField(AUI_PTR_ALIAS(mState, departureDate)),
            dateTextField(AUI_PTR_ALIAS(mState, returnDate)) AUI_LET { connect(mState->isReturnFlight, AUI_SLOT(it)::setEnabled); },
            _new<AButton>("Book") AUI_LET {
                    connect(it->clicked, me::book);
                    connect(AUI_REACT(mState->isValid()), AUI_SLOT(it)::setEnabled);
                },
          } AUI_WITH_STYLE { LayoutSpacing { 4_dp } },
        });
    }

private:
    struct State {
        DateTextFieldState departureDate { system_clock::now() }, returnDate { system_clock::now() };
        AProperty<bool> isReturnFlight = false;
        bool isValid() const {
            if (!departureDate.parsed->hasValue()) {
                return false;
            }
            if (!isReturnFlight) {
                return true;
            }
            if (!returnDate.parsed->hasValue()) {
                return false;
            }
            if (departureDate.parsed->value() > returnDate.parsed->value()) {
                return false;
            }
            return true;
        };
    };
    _<State> mState = _new<State>();

    void book() {
        AString msg = "Departure - {}"_format(formatDate(mState->departureDate.parsed->value()));
        if (mState->isReturnFlight) {
            msg += "\nReturn - {}"_format(formatDate(mState->returnDate.parsed->value()));
        }
        AMessageBox::show(this, "You've booked the flight", msg);
    }
};

AUI_ENTRY {
    _new<FlightBookerWindow>()->show();
    return 0;
}
