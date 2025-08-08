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
#include <AUI/View/AButton.h>
#include <AUI/View/AScrollArea.h>
#include <AUI/View/ATextArea.h>

using namespace declarative;
using namespace ass;
using namespace std::chrono_literals;

enum class CellState : std::uint8_t {
    DEAD,
    ALIVE,
};

constexpr CellState operator!(CellState s) {
    return s == CellState::ALIVE ? CellState::DEAD : CellState::ALIVE;
}

using CellsImage = AFormattedImage<APixelFormat::RGBA_BYTE>;

class Cells : public AObject {
public:
    Cells(glm::ivec2 size) {
        mSize = size;
        for (auto s : { &mStorage, &mNextPopulation }) {
            s->resize(size.x * size.y);
        }

        connect(mTimer->fired, me::frame);
        connect(isRunning.changed, AUI_SLOT(mTimer)::setRunning);
    }

    void frame() {
        mFrame = AThreadPool::global() * [&] {
            for (int y = 0; y < mSize.y; ++y) {
                for (int x = 0; x < mSize.x; ++x) {
                    glm::ivec2 i { x, y };
                    get(mNextPopulation, i) = [&] {
                        auto around = cellsAround(i);
                        switch (around) {
                            default:
                                return CellState::DEAD;
                            case 2:
                                return get(mStorage, i);
                            case 3:
                                return CellState::ALIVE;
                        }
                    }();
                }
            }
            std::swap(mStorage, mNextPopulation);

            emit frameComplete;
        };
    }

    [[nodiscard]] glm::ivec2 size() const noexcept { return mSize; }

    [[nodiscard]] const AVector<CellState>& storage() const noexcept { return mStorage; }

    [[nodiscard]] CellState& operator[](glm::ivec2 position) { return get(mStorage, position); }

    void randomize() {
        ranges::generate(mStorage, [&] {
            static std::default_random_engine re;
            static std::uniform_int_distribution d(0, 10);
            return d(re) > 7 ? CellState::ALIVE : CellState::DEAD;
        });
        emit frameComplete;
    }

    AProperty<bool> isRunning = false;
    emits<> frameComplete;

private:
    _<ATimer> mTimer = _new<ATimer>(100ms);
    AFuture<> mFrame;
    glm::ivec2 mSize {};
    AVector<CellState> mStorage;
    AVector<CellState> mNextPopulation;

    CellState& get(AVector<CellState>& storage, glm::ivec2 position) {
        position %= mSize;
        if (position.x < 0) {
            position.x = mSize.x + position.x;
        }
        if (position.y < 0) {
            position.y = mSize.y + position.y;
        }
        return storage[position.y * mSize.x + position.x];
    }

    unsigned cellsAround(glm::ivec2 position) {
        unsigned accumulator = 0;
        for (int y = -1; y <= 1; ++y) {
            for (int x = -1; x <= 1; ++x) {
                if (x == 0 && y == 0) {
                    continue;
                }
                accumulator += get(mStorage, position + glm::ivec2 { x, y }) == CellState::ALIVE ? 1 : 0;
            }
        }
        return accumulator;
    }
}; /// end

class CellsView : public AView {
public:
    static constexpr auto SCALE = 8_dp;

    CellsView(_<Cells> cells) : mCells(std::move(cells)) { connect(mCells->frameComplete, me::updateTexture); }

    void render(ARenderContext ctx) override {
        AView::render(ctx);
        if (mTexture) {
            ctx.render.rectangle(ATexturedBrush { mTexture }, { 0, 0 }, float(SCALE) * glm::vec2(mCells->size()));
        }
        auto drawGrid = [&] {
            ASmallVector<std::pair<glm::vec2, glm::vec2>, 128 * 2> points;
            for (int i = 1; i < mCells->size().x; ++i) {
                points << std::make_pair(glm::vec2(i * SCALE, 0.f), glm::vec2(i * SCALE, getSize().y));
            }
            for (int i = 1; i < mCells->size().y; ++i) {
                points << std::make_pair(glm::vec2(0.f, i * SCALE), glm::vec2(getSize().x, i * SCALE));
            }
            ctx.render.lines(ASolidBrush { AColor::GRAY }, points);
        };
        drawGrid();
    }

    void onPointerPressed(const APointerPressedEvent& event) override {
        AView::onPointerPressed(event);
        glm::ivec2 position = glm::ivec2(event.position / float(SCALE));
        (*mCells)[position] = !(*mCells)[position];
        updateTexture();
    }

    int getContentMinimumWidth() override { return mCells->size().x * SCALE; }

    int getContentMinimumHeight() override { return mCells->size().y * SCALE; }

private:
    _<Cells> mCells;
    _<ITexture> mTexture;

    void updateTexture() {
        if (!mTexture) {
            mTexture = AWindow::current()->getRenderingContext()->renderer().getNewTexture();
        }

        CellsImage image(mCells->size());
        for (unsigned y = 0; y < image.height(); ++y) {
            for (unsigned x = 0; x < image.width(); ++x) {
                image.set(
                    { x, y },
                    AFormattedColorConverter(
                        (*mCells)[glm::ivec2(x, y)] == CellState::ALIVE
                            ? AColor::WHITE
                            : AColor::TRANSPARENT_BLACK));
            }
        }
        mTexture->setImage(image);
        redraw();
    }
}; /// end

class GameOfLifeWindow : public AWindow {
public:
    GameOfLifeWindow() : AWindow("Game of Life") {
        setContents(Vertical {
          Centered {
            Horizontal {
              _new<AButton>("Randomize") AUI_LET {
                      connect(it->clicked, AUI_SLOT(mCells)::randomize);
                  },
              _new<AButton>() AUI_LET {
                      it & mCells.isRunning > [](AButton& b, bool isRunning) {
                          b.setText(isRunning ? "Pause" : "Run");
                      };
                      connect(it->clicked, [&] { mCells.isRunning = !mCells.isRunning; });
                  },
            },
          },
          Centered {
            _new<CellsView>(aui::ptr::fake_shared(&mCells)) AUI_WITH_STYLE {
                  Expanding(),
                  BackgroundSolid(AColor::BLACK),
                },
          },
        });
    }

private:
    Cells mCells { { 64, 64 } };
}; /// end

AUI_ENTRY {
    auto w = _new<GameOfLifeWindow>();
    w->pack();
    w->show();
    return 0;
}
