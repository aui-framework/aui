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

#include <range/v3/action.hpp>
#include <range/v3/algorithm.hpp>
#include <range/v3/view.hpp>

#include "DevtoolsPerformanceTab.h"

#include <AUI/ASS/ASS.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AButton.h>
#include <chrono>
#include <variant>

#include "AUI/ASS/Property/FixedSize.h"
#include "AUI/Common/AObject.h"
#include "AUI/Common/AOptional.h"
#include "AUI/Enum/ImageRendering.h"
#include "AUI/Enum/Visibility.h"
#include "AUI/Image/AImage.h"
#include "AUI/Image/APixelFormat.h"
#include "AUI/Model/AListModel.h"
#include "AUI/Model/ATreeModelIndex.h"
#include "AUI/Model/ITreeModel.h"
#include "AUI/Performance/APerformanceFrame.h"
#include "AUI/Performance/APerformanceSection.h"
#include "AUI/Platform/ASurface.h"
#include "AUI/Platform/AInput.h"
#include "AUI/Platform/APlatform.h"
#include "AUI/Render/ABrush.h"
#include "AUI/Render/IRenderer.h"
#include "AUI/Render/ITexture.h"
#include "AUI/Traits/values.h"
#include "AUI/Render/ARenderContext.h"
#include "AUI/View/ASpacerFixed.h"
#include "AUI/View/ASplitter.h"
#include "AUI/View/ATabView.h"
#include "AUI/View/AViewContainer.h"
#include "Devtools.h"
#include "glm/vector_relational.hpp"

using namespace std::chrono;
using namespace std::chrono_literals;
using namespace ass;
using namespace declarative;

namespace {
#if AUI_PROFILING
    class GraphView: public AView {
    public:
        static constexpr auto DEFAULT_SIZE = 256;
        GraphView(): mImage({DEFAULT_SIZE, DEFAULT_SIZE}) {
            setExpanding();
            mImage.fill({0, 0, 0, 0});
            mFrames.resize(DEFAULT_SIZE);
        }

        void render(ARenderContext ctx) override {
            AView::render(ctx);
            if (mTexture == nullptr) {
                mTexture = ctx.render.getNewTexture();
            }

            ctx.render.rectangle(ATexturedBrush {
                .texture = mTexture,
                .imageRendering = ImageRendering::PIXELATED,
            }, {0, 0}, mImage.size() * plotScale());

            if (!mSelectionMode) {
                return;
            }

            if (mHoveredFrameIndex && !mSelectedFrameIndex) {
                ctx.render.rectangle(ASolidBrush {AColor::WHITE.transparentize(0.6f) }, {*mHoveredFrameIndex * plotScale(), 0}, {plotScale(), getSize().y});
            }

            if (mSelectedFrameIndex) {
                ctx.render.rectangle(ASolidBrush {AColor::WHITE.transparentize(0.5f) }, {*mSelectedFrameIndex * plotScale(), 0}, {plotScale(), getSize().y});
            }
        }

        void setSize(glm::ivec2 size) override {
            if (getSize() == size) {
                return;
            }
            AView::setSize(size);

            mImage = glm::uvec2{size} / plotScale();
            mImage.fill({0, 0, 0, 0});
            mFrames.resize(mImage.width());
        }

        void onPerformanceFrame(const APerformanceSection::Datas& sections) {
            if (glm::any(glm::equal(mImage.size(), glm::uvec2(0, 0)))) {
                return;
            }

            mFrames[mFrameIndex] = sections;

            // fade-out effect
            if (mFrameIndex % 10 == 0) {
                for (auto& c : mImage) {
                    if (c.r > 0 ) c.r -= 1;
                    if (c.g > 0 ) c.g -= 1;
                    if (c.b > 0 ) c.b -= 1;
                }
            }

            // transparentisze the column
            for (unsigned i = 0; i < mImage.size().y; ++i) {
                mImage.set(glm::uvec2{mFrameIndex, i}, AFormattedColorConverter(AColor(0.f, 0.f, 0.f, 0.f)));
            }

            SectionStatMap sectionStatsMap;
            populateSectionStat(sectionStatsMap, sections);

            auto sectionStatsList = sectionStatsMap
                                    | ranges::views::values
                                    | ranges::to_vector
                                    | ranges::actions::sort([](const auto& l, const auto& r) {
                                        return l.durationIncludingChildren > r.durationIncludingChildren;
                                      })
                                    ;

            unsigned y = 0;

            for (const auto& section : sectionStatsList) {
                const int times = timeToY(section.duration);
                for (int i = 0; i < times; ++i, ++y) {
                    if (y >= mImage.size().y) {
                        goto end;
                    }
                    mImage.set(glm::uvec2{mFrameIndex, mImage.size().y - y - 1}, AFormattedColorConverter(section.color));
                }
            }
            end:

            mImage.setWithPositionCheck(glm::uvec2{mFrameIndex, mImage.size().y - timeToY(16'600us) - 1}, AFormattedColorConverter(AColor::RED)); // 60 fps
            mImage.setWithPositionCheck(glm::uvec2{mFrameIndex, mImage.size().y - timeToY(6'250us) - 1}, AFormattedColorConverter(AColor::RED)); // 160 fps

            // nullsafe lol?
            AUI_NULLSAFE(mTexture)->setImage(mImage);
            mFrameIndex++;
            mFrameIndex %= mImage.size().x;
            redraw();
        }

        void setSelectionMode(bool isSelectionMode) {
            mSelectionMode = isSelectionMode;

            if (!isSelectionMode) {
                mHoveredFrameIndex.reset();
                mSelectedFrameIndex.reset();
            }
        }

        void onPointerMove(glm::vec2 pos, const APointerMoveEvent& event) override {
            AView::onPointerMove(pos, event);
            unsigned index = unsigned(pos.x) / plotScale();
            mHoveredFrameIndex = index;
            if (event.pointerIndex.isFinger() || AInput::isKeyDown(event.pointerIndex.button().valueOr(AInput::LBUTTON))) {
                setSelectedFrameIndex(index);
            }
        }

        void onPointerReleased(const APointerReleasedEvent& event) override {
            AView::onPointerReleased(event);
            setSelectedFrameIndex(unsigned(event.position.x) / plotScale());
        }

        void setSelectedFrameIndex(unsigned index) {
            if (mSelectedFrameIndex == index) {
                return;
            }
            index %= mFrames.size();
            mSelectedFrameIndex = index;
            emit selectionChanged(mFrames[index]);
        }

    signals:
        emits<APerformanceSection::Datas> selectionChanged;

    private:
        AOptional<unsigned> mHoveredFrameIndex;
        AOptional<unsigned> mSelectedFrameIndex;
        AVector<APerformanceSection::Datas> mFrames;
        bool mSelectionMode = false;
        struct SectionStat {
            AString name;
            AColor color;

            /**
             * Unlike APerformanceSection::Data::duration, this one excludes the duration of children sections.
             */
            high_resolution_clock::duration duration = high_resolution_clock::duration(0);

            high_resolution_clock::duration durationIncludingChildren = high_resolution_clock::duration(0);
        };
        using SectionStatMap = AMap<AString /* section name */, SectionStat>;

        AFormattedImage<APixelFormat::RGBA_BYTE> mImage;
        _<ITexture> mTexture;
        unsigned mFrameIndex = 0;

        [[nodiscard]]
        unsigned plotScale() const {
            return unsigned(APlatform::getDpiRatio() * 2.f);
        }

        static int timeToY(high_resolution_clock::duration t) {
            return duration_cast<microseconds>(t).count() / 100;
        }

        static high_resolution_clock::duration populateSectionStat(SectionStatMap& dst, const APerformanceSection::Datas& sections) {
            high_resolution_clock::duration accumulator = high_resolution_clock::duration(0);
            for (const auto& section : sections) {
                auto& sectionStat = dst.getOrInsert(section.name, [&] {
                    return SectionStat { .name = section.name, .color = section.color };
                });
                sectionStat.durationIncludingChildren += section.duration;
                auto childrenDuration = populateSectionStat(dst, section.children);
                //assert(("children duration bigger than parent?", section.duration >= childrenDuration));
                sectionStat.duration += section.duration - childrenDuration;
                accumulator += section.duration;
            }
            return accumulator;
        }

    };

    class PerformanceSectionsTreeView: public AViewContainerBase {
    public:
        static constexpr auto MAX_DEPTH = 30;
        PerformanceSectionsTreeView() {
        }

        void onPerformanceFrame(const APerformanceSection::Datas& sections) {
            _<AViewContainer> root = Vertical{};
            populate(*root, sections);
            setContents(Horizontal { root });
        }

        void setVerboseMode(bool verboseMode) {
            mVerboseMode = verboseMode;
        }

    private:
        bool mVerboseMode  = false;

        _<AView> makeChip(const APerformanceSection::Data& i) {
            return Horizontal {
                Label { i.name } AUI_WITH_STYLE {
                    TextColor { i.color.readableBlackOrWhite() },
                },
                Label { "{}μs"_format(duration_cast<microseconds>(i.duration).count()) } AUI_WITH_STYLE {
                    TextColor { AColor::WHITE.transparentize(0.3f) },
                    BackgroundSolid { AColor::BLACK },
                },
                mVerboseMode ? _new<ALabel>(i.verboseInfo) AUI_WITH_STYLE {
                    TextColor { AColor::WHITE.transparentize(0.3f) },
                    BackgroundSolid { AColor::BLACK },
                } : nullptr,
            } AUI_WITH_STYLE {
                BackgroundSolid { i.color },
                BorderRadius { 6_pt },
                Margin { 2_dp, 4_dp },
                FixedSize { 1_dp * glm::max(0.25f * float(duration_cast<microseconds>(i.duration).count()), 1.f), {} },
            };
        }

        void populate(AViewContainer& container, const APerformanceSection::Datas& sections, int remainingDepth = MAX_DEPTH) {
            if (remainingDepth == 0) {
                return;
            }
            for (const auto& section : sections) {
                _<AViewContainer> v = Vertical{} AUI_WITH_STYLE {
                    BorderLeft { 2_dp, section.color },
                };
                container.addView(v);
                v->addView(makeChip(section));
                if (section.children.empty()) {
                    continue;
                }
                _<AViewContainer> root = Vertical{} AUI_WITH_STYLE {
                    Padding { {}, {}, {}, 8_dp },
                };
                populate(*root, section.children, remainingDepth - 1);
                v->addView(Horizontal { root });
            }
        }
    };

#endif
}


DevtoolsPerformanceTab::DevtoolsPerformanceTab(ASurface* targetWindow) : mTargetWindow(targetWindow) {
    using namespace declarative;

#if AUI_PROFILING
    connect(targetWindow->performanceFrameComplete, [this](const APerformanceSection::Datas& frame) {
        if (!std::holds_alternative<Running>(*mState)) {
            return;
        }
        emit nextFrame(frame);
    });

    auto graphView = _new<GraphView>();
    auto treeView = _new<PerformanceSectionsTreeView>();

    connect(nextFrame, AUI_SLOT(graphView)::onPerformanceFrame);
    connect(nextFrame, treeView, [treeView = treeView.get()](const APerformanceSection::Datas& sections) {
        static high_resolution_clock::time_point lastTimeUpdated;
        auto now = high_resolution_clock::now();
        if ((now - lastTimeUpdated) < 1s) {
            return;
        }
        lastTimeUpdated = now;

        treeView->onPerformanceFrame(sections);
    }); 
    connect(graphView->selectionChanged, AUI_SLOT(treeView)::onPerformanceFrame);

    connect(mState, [=](const State& state) {
      bool isPaused = std::holds_alternative<Paused>(state);
      graphView->setSelectionMode(isPaused);
      treeView->setVerboseMode(isPaused);
    });

    struct ScrollPassthroughImpl: public AViewContainer {
    public:
        void onScroll(const AScrollEvent& e) override {
            getViews().last()->onScroll(e);
        }
    };

    using ScrollPassthrough = aui::ui_building::view_container_layout<AStackedLayout, ScrollPassthroughImpl>;

    setContents(Centered {
        ScrollPassthrough::Expanding {
            graphView,
            AScrollArea::Builder().withContents(
                Vertical::Expanding {
                    treeView,
                }
            ).build() AUI_WITH_STYLE { Visibility::UNREACHABLE },
        } AUI_LET {
            connect(mState, [=](const State& state) {
                using namespace ass;
                if (std::holds_alternative<Running>(state)) {
                    it->setCustomStyle({
                        Opacity { 0.7f },
                        Visibility::UNREACHABLE,
                    });
                } else {
                    it->setCustomStyle({
                        Opacity { 1.f },
                        Visibility::VISIBLE,
                        BackgroundSolid { AColor::BLACK.transparentize(0.5f) },
                    });
                }
            });
        },
        Vertical::Expanding {
            Centered {
                _new<AButton>(/* pause */) AUI_LET {
                    connect(mState, [=](const State& state) {
                      if (std::holds_alternative<Running>(state)) {
                          it->setText("Pause");
                      } else {
                          it->setText("Resume");
                      }
                    });
                    connect(it->clicked, me::toggleRunPause);
                },
            },
        },
    });  
#else
    setContents(Centered {
        Label { "Please set -DAUI_PROFILING=TRUE in CMake configure." }
    });
#endif
}
