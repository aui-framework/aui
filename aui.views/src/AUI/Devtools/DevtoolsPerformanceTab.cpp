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

#include "DevtoolsPerformanceTab.h"

#include <AUI/ASS/ASS.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AButton.h>
#include <chrono>

#include "AUI/Common/AObject.h"
#include "AUI/Image/AImage.h"
#include "AUI/Image/APixelFormat.h"
#include "AUI/Model/ATreeModelIndex.h"
#include "AUI/Model/ITreeModel.h"
#include "AUI/Performance/APerformanceFrame.h"
#include "AUI/Platform/ABaseWindow.h"
#include "AUI/Render/ABrush.h"
#include "AUI/Render/ARender.h"
#include "AUI/Render/ITexture.h"
#include "AUI/Traits/values.h"
#include "AUI/Util/ClipOptimizationContext.h"
#include "AUI/View/ASplitter.h"
#include "AUI/View/ATabView.h"
#include "AUI/View/AViewContainer.h"
#include "Devtools.h"

using namespace std::chrono;

namespace {
#if AUI_PROFILING
    class GraphView: public AView {
    public:
        GraphView(ABaseWindow* targetWindow): mImage({256, 256}) {
            setExpanding();
            connect(targetWindow->performanceFrameComplete, me::onPerformanceFrame);
            mTexture = ARender::getNewTexture();
            mImage.fill({0, 0, 0, 0});
        }

        void render(ClipOptimizationContext c) override {
            AView::render(c);

            ARender::rect(ATexturedBrush {
                .texture = mTexture,
            }, {0, 0}, mImage.size());
        }

        void setSize(glm::ivec2 size) override {
            if (getSize() == size) {
                return;
            }
            AView::setSize(size);

            mImage = glm::uvec2{size};
            mImage.fill({0, 0, 0, 0});
        }

    private:
        AFormattedImage<APixelFormat::RGBA_BYTE> mImage;
        _<ITexture> mTexture;
        unsigned mFrameIndex = 0;

        void onPerformanceFrame(const APerformanceFrame::Sections& sections) {
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

            unsigned y = 0;
            for (const auto& section : sections) {
                const int times = duration_cast<microseconds>(section.duration).count() / 100;
                for (int i = 0; i < times; ++i, ++y) {
                    if (y >= mImage.size().y) {
                        goto end;
                    }
                    mImage.set(glm::uvec2{mFrameIndex, mImage.size().y - y - 1}, AFormattedColorConverter(section.color));
                }
            }

            end:
            mTexture->setImage(mImage);
            mFrameIndex++;
            mFrameIndex %= mImage.size().x;
            redraw();
        }
    };
#endif
}


DevtoolsPerformanceTab::DevtoolsPerformanceTab(ABaseWindow* targetWindow) : mTargetWindow(targetWindow) {
    using namespace declarative;

#if AUI_PROFILING
    setContents(Centered { _new<GraphView>(targetWindow) });
#else
    setContents(Centered {
        Label { "Please set -DAUI_PROFILING=TRUE in CMake configure." }
    });
#endif
}
