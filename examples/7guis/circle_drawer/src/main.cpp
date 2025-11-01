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

#include <AUI/Platform/Entry.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include "AUI/View/AProgressBar.h"
#include "AUI/View/ASlider.h"
#include "AUI/View/AButton.h"

using namespace declarative;

struct Circle {
    glm::vec2 position;
    AProperty<float> radius = 10_dp;
};

class IAction {   // IAction: candidate to be committed to the framework
public:
    virtual ~IAction() = default;
    virtual void undo() = 0;
    virtual void redo() = 0;
};

class UndoStack {
public:
    using Container = std::list<_unique<IAction>>;
    using Iterator = Container::const_iterator;

private:
    Container mStack;

public:
    void undo() {
        if (nextAction == mStack.begin()) {
            return;
        }
        nextAction = std::prev(*nextAction);
        (**nextAction)->undo();
    }

    void redo() {
        if (nextAction == mStack.end()) {
            return;
        }
        (**nextAction)->redo();
        nextAction = std::next(*nextAction);
    }

    void add(_unique<IAction> action) {
        action->redo();
        nextAction = std::next(mStack.insert(mStack.erase(*nextAction, mStack.end()), std::move(action)));
        nextAction.notify();
    }

    Iterator begin() const { return mStack.begin(); }

    Iterator end() const { return mStack.end(); }

    AProperty<Iterator> nextAction = mStack.end();
};

struct State {
    AProperty<std::list<Circle>> circles;
    UndoStack history;
};

static constexpr auto MAX_RADIUS = 128.f;

class CircleDrawArea : public AView {
public:
    CircleDrawArea(_<State> state) : mState(std::move(state)) {
        setCustomStyle({
          Expanding(),
          BackgroundSolid(AColor::WHITE),
          Border(1_px, AColor::GRAY),
          AOverflow::HIDDEN_FROM_THIS,
        });
        connect(mState->circles.changed, me::redraw);
        connect(mHoveredCircle.changed, me::redraw);
    }

    void render(ARenderContext ctx) override {
        AView::render(ctx);

        for (const auto& circle : *mState->circles) {
            if (&circle == mHoveredCircle) {
                ctx.render.roundedRectangle(
                    ASolidBrush { AColor::GRAY }, circle.position - *circle.radius, glm::vec2(circle.radius * 2.f),
                    circle.radius);
            }
            ctx.render.roundedRectangleBorder(
                ASolidBrush { AColor::BLACK }, circle.position - *circle.radius, glm::vec2(circle.radius * 2.f),
                circle.radius, 1);
        }
    }

    void onPointerMove(glm::vec2 pos, const APointerMoveEvent& event) override {
        AView::onPointerMove(pos, event);
        mHoveredCircle = [&] {
            Circle* result = nullptr;
            float nearestDistanceToCursor = std::numeric_limits<float>::max();
            for (auto& circle : mState->circles.raw) {
                float distanceToCursor = glm::distance2(circle.position, pos);
                if (distanceToCursor > nearestDistanceToCursor) {
                    continue;
                }
                if (distanceToCursor > circle.radius * circle.radius) {
                    continue;
                }
                result = &circle;
                nearestDistanceToCursor = distanceToCursor;
            }
            return result;
        }();
    }

protected:
    AMenuModel composeContextMenu() override {
        auto circle = *mHoveredCircle;
        if (circle == nullptr) {
            return {};
        }
        return {
            {},
            {
              .name = "Adjust radius...",
              .onAction =
                  [this, circle] {
                      auto radiusPopup = _new<AWindow>(
                          "", 200_dp, 50_dp, dynamic_cast<AWindow*>(AWindow::current()), WindowStyle::MODAL);
                      radiusPopup->setContents(Vertical {
                        Label { "Adjust diameter of circle at {}."_format(circle->position) },
                        Slider {
                          .value = AUI_REACT(circle->radius / MAX_RADIUS),
                          .onValueChanged =
                              [this, circle](aui::float_within_0_1 s) {
                                  circle->radius = s * MAX_RADIUS;
                                  mState->circles.notify();
                              },
                        },
                      });
                      connect(radiusPopup->closed, [this, circle, oldRadius = circle->radius] {
                          if (oldRadius == circle->radius) {
                              return;
                          }
                          class ActionChangeRadius : public IAction {
                          public:
                              ActionChangeRadius(Circle* circle, float prevRadius, float newRadius)
                                : mCircle(circle), mPrevRadius(prevRadius), mNewRadius(newRadius) {}

                              ~ActionChangeRadius() override = default;

                              void undo() override { mCircle->radius = mPrevRadius; }

                              void redo() override { mCircle->radius = mNewRadius; }

                          private:
                              Circle* mCircle;
                              float mPrevRadius;
                              float mNewRadius;
                          };
                          mState->history.add(std::make_unique<ActionChangeRadius>(circle, oldRadius, circle->radius));
                      });
                      radiusPopup->show();
                  },
            },
        };
    }

public:
    void onPointerReleased(const APointerReleasedEvent& event) override {
        AView::onPointerReleased(event);
        if (event.asButton != AInput::LBUTTON) {
            return;
        }
        class ActionAddCircle : public IAction {
        public:
            ActionAddCircle(_<State> state, Circle circle) : mState(std::move(state)), mCircle(std::move(circle)) {}
            ~ActionAddCircle() override = default;
            void undo() override { mState->circles.writeScope()->pop_back(); }
            void redo() override { mState->circles.writeScope()->push_back(mCircle); }

        private:
            _<State> mState;
            Circle mCircle;
        };
        mState->history.add(std::make_unique<ActionAddCircle>(mState, Circle { .position = event.position }));
    }

private:
    _<State> mState;
    AProperty<Circle*> mHoveredCircle = nullptr;
};

class CircleDrawerWindow : public AWindow {
public:
    CircleDrawerWindow() : AWindow("AUI - 7GUIs - Circle Drawer", 300_dp, 250_dp) {
        setContents(
            Vertical {
              Centered {
                Horizontal {
                  Button { Label { "Undo" }, { me::undo } } AUI_LET {
                          connect(
                              AUI_REACT(mState.history.nextAction != mState.history.begin()), AUI_SLOT(it)::setEnabled);
                      },
                  Button { Label { "Redo" }, { me::redo } } AUI_LET {
                          connect(
                              AUI_REACT(mState.history.nextAction != mState.history.end()), AUI_SLOT(it)::setEnabled);
                      },
                },
              },
              _new<CircleDrawArea>(aui::ptr::fake_shared(&mState)),
            } AUI_WITH_STYLE { LayoutSpacing { 4_dp } });
    }

private:
    State mState;

    void undo() { mState.history.undo(); }

    void redo() { mState.history.redo(); }
};

AUI_ENTRY {
    _new<CircleDrawerWindow>()->show();
    return 0;
}