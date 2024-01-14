// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

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
#include "AUI/Platform/ABaseWindow.h"
#include "AUI/Platform/APlatform.h"
#include "AUI/Render/ABrush.h"
#include "AUI/Render/ARender.h"
#include "AUI/Render/ITexture.h"
#include "AUI/Traits/values.h"
#include "AUI/Util/ClipOptimizationContext.h"
#include "AUI/View/ASpacerFixed.h"
#include "AUI/View/ASplitter.h"
#include "AUI/View/ATabView.h"
#include "AUI/View/AViewContainer.h"
#include "Devtools.h"
#include "glm/vector_relational.hpp"

using namespace std::chrono;
using namespace std::chrono_literals;
using namespace ass;

namespace {
#if AUI_PROFILING
    class GraphView: public AView {
    public:
        GraphView(): mImage({256, 256}) {
            setExpanding();
            mTexture = ARender::getNewTexture();
            mImage.fill({0, 0, 0, 0});
        }

        void render(ClipOptimizationContext c) override {
            AView::render(c);

            ARender::rect(ATexturedBrush {
                .texture = mTexture,
                .imageRendering = ImageRendering::PIXELATED,
            }, {0, 0}, mImage.size() * plotScale());
        }

        void setSize(glm::ivec2 size) override {
            if (getSize() == size) {
                return;
            }
            AView::setSize(size);

            mImage = glm::uvec2{size} / plotScale();
            mImage.fill({0, 0, 0, 0});
        }

        void onPerformanceFrame(const APerformanceSection::Datas& sections) {
            if (glm::any(glm::equal(mImage.size(), glm::uvec2(0, 0)))) {
                return;
            }

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
                                    | ranges::view::values
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

            mImage.set(glm::uvec2{mFrameIndex, mImage.size().y - timeToY(16'600us) - 1}, AFormattedColorConverter(AColor::RED)); // 60 fps
            mImage.set(glm::uvec2{mFrameIndex, mImage.size().y - timeToY(6'250us) - 1}, AFormattedColorConverter(AColor::RED)); // 160 fps

            mTexture->setImage(mImage);
            mFrameIndex++;
            mFrameIndex %= mImage.size().x;
            redraw();
        }

        void setSelectionMode(bool isSelectionMode) {
            mSelectionMode = isSelectionMode;
        }
    private:
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

    class PerformanceSectionsTreeView: public AViewContainer {
    public:
        PerformanceSectionsTreeView() {
        }

        void onPerformanceFrame(const APerformanceSection::Datas& sections) {
            auto now = high_resolution_clock::now();
            if ((now - mLastTimeUpdated) < 1s) {
                return;
            }
            mLastTimeUpdated = now;
            _<AViewContainer> root = Vertical{};
            populate(*root, sections);
            setContents(Horizontal { root });
        }

    private:
        high_resolution_clock::time_point mLastTimeUpdated;

        static _<AView> makeChip(const APerformanceSection::Data& i) {
            return Horizontal {
                _new<ALabel>(i.name) with_style {
                    TextColor { i.color.readableBlackOrWhite() },
                }
            } with_style {
                BackgroundSolid { i.color },
                BorderRadius { 6_pt },
                Margin { 2_dp, 4_dp },
                FixedSize { 1_dp * glm::max(0.25f * float(duration_cast<microseconds>(i.duration).count()), 1.f), {} },
            };
        }

        static void populate(AViewContainer& container, const APerformanceSection::Datas& sections) {
            for (const auto& section : sections) {
                _<AViewContainer> v = Vertical{} with_style {
                    BorderLeft { 2_dp, section.color },
                };
                container.addView(v);
                v->addView(makeChip(section));
                if (section.children.empty()) {
                    continue;
                }
                _<AViewContainer> root = Vertical{} with_style {
                    Padding { {}, {}, {}, 8_dp },
                };
                populate(*root, section.children);
                v->addView(Horizontal { root });
            }
        }
    };

#endif
}


DevtoolsPerformanceTab::DevtoolsPerformanceTab(ABaseWindow* targetWindow) : mTargetWindow(targetWindow) {
    using namespace declarative;

#if AUI_PROFILING
    connect(targetWindow->performanceFrameComplete, [this](const APerformanceSection::Datas& frame) {
        if (!std::holds_alternative<Model::Running>(mModel->state)) {
            return;
        }
        emit nextFrame(frame);
    });

    auto graphView = _new<GraphView>() let {
        connect(nextFrame, slot(it)::onPerformanceFrame);
        
    };

    auto treeView = _new<PerformanceSectionsTreeView>() let { connect(nextFrame, slot(it)::onPerformanceFrame); };

    mModel.addObserver(&Model::state, [graphView](const Model::State& state) {
        bool isPaused = std::holds_alternative<Model::Paused>(state);
        graphView->setSelectionMode(isPaused);
    });

    setContents(Centered { 
        Centered::Expanding {
            graphView,
            Vertical::Expanding {
                treeView,
            },
        } let {
            mModel.addObserver(&Model::state, [it](const Model::State& state) {
                using namespace ass;
                if (std::holds_alternative<Model::Running>(state)) {
                    it->setCustomStyle({
                        Opacity { 0.7f },
                        Visibility::UNREACHABLE,
                    });
                } else {
                    it->setCustomStyle({
                        Opacity { 1.f },
                        Visibility::VISIBLE,
                    });
                }
            });
        },
        Vertical::Expanding {
            Centered {
                _new<AButton>(/* pause */) let {
                    mModel.addObserver(&Model::state, [it](const Model::State& state) {
                        if (std::holds_alternative<Model::Running>(state)) {
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
    mModel.setModel(Model{});
#else
    setContents(Centered {
        Label { "Please set -DAUI_PROFILING=TRUE in CMake configure." }
    });
#endif
}
