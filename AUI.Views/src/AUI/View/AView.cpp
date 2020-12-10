#include "AView.h"
#include "AUI/Render/Render.h"
#include "AUI/Util/ATokenizer.h"
#include "AUI/Platform/AWindow.h"
#include "AUI/Url/AUrl.h"
#include "AUI/Image/Drawables.h"
#include "AUI/Render/RenderHints.h"
#include "AUI/Animator/AAnimator.h"

#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <AUI/IO/StringStream.h>
#include <AUI/Util/kAUI.h>

#include "AUI/Platform/ADesktop.h"
#include "AUI/Render/AFontManager.h"
#include "AUI/Util/AMetric.h"
#include "AUI/Util/Factory.h"

// windows.h
#undef max
#undef min

AWindow* AView::getWindow()
{

	AView* parent = nullptr;

	for (AView* target = this; target; target = target->mParent) {
		parent = target;
	}

	return dynamic_cast<AWindow*>(parent);
}

AView::AView()
{
	AVIEW_CSS;
}

void AView::redraw()
{
	if (auto w = getWindow()) {
		w->flagRedraw();
	}
}

void AView::drawStencilMask()
{
    if (mBorderRadius > 0 && mPadding.horizontal() == 0 && mPadding.vertical() == 0) {
        Render::inst().drawRoundedRect(mPadding.left,
                                       mPadding.top,
                                           getWidth() - mPadding.horizontal(),
                                           getHeight() - mPadding.vertical(),
                                       mBorderRadius);
    } else {
        Render::inst().setFill(Render::FILL_SOLID);
        Render::inst().drawRect(mPadding.left, mPadding.top, getWidth() - mPadding.horizontal(),
                                    getHeight() - mPadding.vertical());
    }
}

void AView::postRender() {
    if (mAnimator)
        mAnimator->postRender(this);
    popStencilIfNeeded();
}

void AView::popStencilIfNeeded() {
    if (getOverflow() == OF_HIDDEN)
    {
        /*
         * Если у AView есть ограничение по Overflow, то он запушил свою маску в буфер трафарета, но он не может
         * вернуть буфер трафарета к прежнему состоянию из-за ограничений C++, поэтому заносить маску нужно
         * после обновлений преобразований (позиция, поворот), но перед непосредственным рендером содержимого AView
         * (то есть, в <code>AView::render</code>), а возвращать трафарет к предыдущему состоянию можно только
         * здесь, после того, как AView будет полностью отрендерен.
         */
        RenderHints::PushMask::popMask([&]() {
            drawStencilMask();
        });
    }
}
void AView::render()
{
    if (mAnimator)
        mAnimator->animate(this);

	{
		ensureCSSUpdated();

        for (auto& e : mBackgroundEffects)
        {
            e->draw([&]()
                    {
                        Render::inst().drawRect(0, 0, getWidth(), getHeight());
                    });
        }

        // список отрисовки.
        if (mHasTransitions) {
            mTransitionValue = glm::clamp(mTransitionValue, 0.f, 1.f);
            for (auto& item : mCssDrawListBack)
                item();

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            RenderHints::PushColor c;
            Render::inst().setColor({1, 1, 1, mTransitionValue });
            for (auto& item : mCssDrawListFront)
                item();

            auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::high_resolution_clock::now().time_since_epoch());
            mTransitionValue += (now - mLastFrameTime).count() / (1000.f * mTransitionDuration);
            if (mTransitionValue >= 1.f) {
                mHasTransitions = false;
            }
            else {
                AWindow::current()->flagRedraw();
                mLastFrameTime = now;
            }
        }
        else
        {
            for (auto& item : mCssDrawListFront)
                item();
        }
	}

    // stencil
    if (mOverflow == OF_HIDDEN)
    {
        RenderHints::PushMask::pushMask([&]() {
            drawStencilMask();
        });
    }
}

void AView::recompileCSS()
{
	// соберём полный список свойств CSS.
	ADeque<_<Stylesheet::Entry>> entries;
	entries.insert(entries.end(), mCssEntries.begin(), mCssEntries.end());

	for (auto& possibleEntry : mCssPossibleEntries)
	{
		if (possibleEntry->selectorMatches(this, false) == Stylesheet::Entry::M_MATCH)
		{
			entries << possibleEntry;
		}
	}
	if (mCustomStylesheet) {
        entries.insert(entries.end(), mCustomStylesheet->getEntries().begin(), mCustomStylesheet->getEntries().end());
    }

	// некоторая вспомогательная хрень
	auto processStylesheet = [&](css type, const std::function<void(property)>& callback)
	{

		for (auto i = entries.rbegin(); i != entries.rend(); ++i)
		{
			if (auto x = (*i)->getSheet().contains(type))
			{
				callback(x->second);
				break;
			}
		}
	};

	userProcessStyleSheet(processStylesheet);

	processStylesheet(css::T_TRANSITION, [&](property p)
	{
		if (p->getArgs().size() == 1)
		{
			auto string = p->getArgs()[0];
			if (string == "none")
			{
				mHasTransitions = false;
				return;
			}
			if (string.endsWith("s"))
			{
				string = string.mid(0, string.length() - 1);
			}
			auto duration = string.toFloat();
			if (duration > 0)
			{
				// господи, за что это дерьмо
				if (!mHasTransitions) {
					mHasTransitions = true;
					mCssDrawListBack = std::move(mCssDrawListFront);
					mTransitionValue = 1.f - glm::clamp(mTransitionValue, 0.f, 1.f);
					mLastFrameTime = std::chrono::duration_cast<std::chrono::milliseconds>(
						std::chrono::high_resolution_clock::now().time_since_epoch());
				}
				mTransitionDuration = duration;
			}
		}
	});

	if (!mHasTransitions)
	{
		mCssDrawListBack.clear();
		mTransitionValue = 1.f;
	}

	mCursor = ACursor::DEFAULT;
	mOverflow = OF_VISIBLE;
	mMargin = {};
	mMinSize = {};
    mBorderRadius = 0.f;
    //mForceStencilForBackground = false;
	mMaxSize = glm::ivec2(std::numeric_limits<int>::max(), std::numeric_limits<int>::max());
	mFontStyle = {AFontManager::inst().getDefault(), 12, false, ALIGN_LEFT, AColor(0, 0, 0, 1.f) };
	mBackgroundEffects.clear();

	// общий обработчик для margin и padding.
	auto fieldProcessor = [&](property p) -> ABoxFields
	{
		switch (p->getArgs().size())
		{
		case 1:
		{
			float all = AMetric(p->getArgs()[0]).getValuePx();
			return { all, all, all, all };
		}
		case 2:
		{
			float vertical = AMetric(p->getArgs()[0]).getValuePx();
			float horizontal = AMetric(p->getArgs()[1]).getValuePx();
			return { horizontal, horizontal, vertical ,vertical };
		}
		case 3:
		{
			float top = AMetric(p->getArgs()[0]).getValuePx();
			float horizontal = AMetric(p->getArgs()[1]).getValuePx();
			float bottom = AMetric(p->getArgs()[2]).getValuePx();
			return { horizontal, horizontal, top ,bottom };
		}
		case 4:
		{
			float top = AMetric(p->getArgs()[0]).getValuePx();
			float right = AMetric(p->getArgs()[1]).getValuePx();
			float bottom = AMetric(p->getArgs()[2]).getValuePx();
			float left = AMetric(p->getArgs()[3]).getValuePx();
			return { left, right, top ,bottom };
		}
		}
		return {};
	};
	processStylesheet(css::T_MARGIN, [&](property p)
	{
		mMargin = fieldProcessor(p);
	});
	processStylesheet(css::T_PADDING, [&](property p)
	{
		mPadding = fieldProcessor(p);
	});

	processStylesheet(css::T_OVERFLOW, [&](property p)
	{
		if (p->getArgs().size() == 1) {
			if (p->getArgs()[0] == "hidden")
				mOverflow = OF_HIDDEN;
		}
	});

	processStylesheet(css::T_WIDTH_MIN, [&](property p)
	{
		if (p->getArgs().size() == 1) {
			mMinSize.x = AMetric(p->getArgs()[0]).getValuePx();
		}
	});
	processStylesheet(css::T_HEIGHT_MIN, [&](property p)
	{
		if (p->getArgs().size() == 1) {
			mMinSize.y = AMetric(p->getArgs()[0]).getValuePx();
		}
	});

	processStylesheet(css::T_WIDTH_MAX, [&](property p)
	{
		if (p->getArgs().size() == 1) {
			mMaxSize.x = AMetric(p->getArgs()[0]).getValuePx();
		}
	});
	processStylesheet(css::T_HEIGHT_MAX, [&](property p)
	{
		if (p->getArgs().size() == 1) {
			mMaxSize.y = AMetric(p->getArgs()[0]).getValuePx();
		}
	});

	processStylesheet(css::T_WIDTH, [&](property p)
	{
		if (p->getArgs().size() == 1) {
			mFixedSize.x = AMetric(p->getArgs()[0]).getValuePx();
		}
	});
	processStylesheet(css::T_HEIGHT, [&](property p)
	{
		if (p->getArgs().size() == 1) {
			mFixedSize.y = AMetric(p->getArgs()[0]).getValuePx();
		}
	});

	processStylesheet(css::T_COLOR, [&](property p)
	{
		if (p->getArgs().size() == 1) {
			mFontStyle.color = p->getArgs()[0];
		}
	});
	processStylesheet(css::T_FONT_FAMILY, [&](property p)
	{
		if (p->getArgs().size() == 1) {
			mFontStyle.font = AFontManager::inst().get(p->getArgs()[0].trim('\'').trim('\"'));
		}
	});
	processStylesheet(css::T_FONT_SIZE, [&](property p)
	{
		if (p->getArgs().size() == 1) {
			mFontStyle.size = AMetric(p->getArgs()[0]).getValuePx();
		}
	});

	processStylesheet(css::T_CURSOR, [&](property p)
	{
		if (p->getArgs().size() == 1)
		{
			static AMap<AString, ACursor> cursors = {
				{"default", ACursor::DEFAULT},
				{"pointer", ACursor::POINTER},
				{"text", ACursor::TEXT},
			};
			if (auto c = cursors.contains(p->getArgs()[0]))
			{
				mCursor = c->second;
			}
		}
	});

	processStylesheet(css::T_TEXT_ALIGN, [&](property p)
	{
		if (p->getArgs().size() == 1) {
			if (p->getArgs()[0] == "left")
				mFontStyle.align = ALIGN_LEFT;
			else if (p->getArgs()[0] == "center")
				mFontStyle.align = ALIGN_CENTER;
			else if (p->getArgs()[0] == "right")
				mFontStyle.align = ALIGN_RIGHT;
		}
	});
	processStylesheet(css::T_AUI_FONT_RENDERING, [&](property p)
	{
		if (p->getArgs().size() == 1) {
			if (p->getArgs()[0] == "nearest")
				mFontStyle.fontRendering = FR_NEAREST;
			else if (p->getArgs()[0] == "antialiasing")
				mFontStyle.fontRendering = FR_ANTIALIASING;
			else if (p->getArgs()[0] == "subpixel")
#ifdef __ANDROID__
			    mFontStyle.fontRendering = FR_ANTIALIASING;
#else
			    mFontStyle.fontRendering = FR_SUBPIXEL;
#endif
		}
	});

	// составление списка отрисовки.
	mCssDrawListFront.clear();

    processStylesheet(css::T_AUI_SCALE, [&](property p)
    {
        float oX, oY;
        switch (p->getArgs().size()) {
            case 1:
                oX = oY = p->getArgs()[0].toFloat();
                break;
            case 2:
                oX = p->getArgs()[0].toFloat();
                oY = p->getArgs()[1].toFloat();
                break;
            default:
                return;
        }
        mCssDrawListFront << [&, oX, oY]() {
            Render::inst().setTransform(glm::scale(glm::mat4(1.f), glm::vec3{oX, oY, 1.0}));
        };
    });

    processStylesheet(css::T_AUI_OFFSET, [&](property p)
    {
        int oX, oY;
        switch (p->getArgs().size()) {
            case 1:
                oX = oY = AMetric(p->getArgs()[0]).getValuePx();
                break;
            case 2:
                oX = AMetric(p->getArgs()[0]).getValuePx();
                oY = AMetric(p->getArgs()[1]).getValuePx();
                break;
            default:
                return;
        }
        mCssDrawListFront << [&, oX, oY]() {
            Render::inst().setTransform(glm::translate(glm::mat4(1.f), glm::vec3{oX, oY, 0.0}));
        };
    });

	processStylesheet(css::T_BOX_SHADOW, [&](property p)
	{
        glm::vec2 offset;
        float radius = 0.f, stretch = 0.f;
        AColor color;
        switch (p->getArgs().size()) {
            case 4: // x y radius color
                offset.x = AMetric(p->getArgs()[0]).getValuePx();
                offset.y = AMetric(p->getArgs()[1]).getValuePx();
                radius = AMetric(p->getArgs()[2]).getValuePx();
                color = p->getArgs()[3];
                break;
            case 5: // x y radius stretch color
                offset.x = AMetric(p->getArgs()[0]).getValuePx();
                offset.y = AMetric(p->getArgs()[1]).getValuePx();
                radius = AMetric(p->getArgs()[2]).getValuePx();
                stretch = AMetric(p->getArgs()[3]).getValuePx();
                color = p->getArgs()[4];
                break;
            default:
                return;
        }
        mCssDrawListFront << [&, offset, radius, stretch, color]() {
            Render::inst().drawBoxShadow(offset.x - stretch, offset.y - stretch, getWidth() + stretch * 2,
                                             getHeight() + stretch * 2, radius, color);
        };
	});
	processStylesheet(css::T_BACKGROUND_COLOR, [&](property p)
	{
		AColor color = p->getArgs()[0];
		if (color.a > 0.001) {
            mCssDrawListFront << [&, color]() {
                RenderHints::PushColor x;

                Render::inst().setColor(color);
                if (mBorderRadius > 0) {
                    Render::inst().drawRoundedRectAntialiased(0, 0, getWidth(), getHeight(), mBorderRadius);
                } else  {
                    Render::inst().setFill(Render::FILL_SOLID);
                    Render::inst().drawRect(0, 0, getWidth(), getHeight());
                }
            };
        }
	});
	processStylesheet(css::T_BACKGROUND_EFFECT, [&](property p)
	{
		for (auto& a : p->getArgs()) {
			mBackgroundEffects << Autumn::get<Factory<IShadingEffect>>(a)->createObject();
		}
	});

	AColor backgroundImageOverlay(1.f);
	processStylesheet(css::T_AUI_BACKGROUND_OVERLAY, [&](property p)
	{
	    if (p->getArgs().size() == 1) {
            backgroundImageOverlay = AColor(p->getArgs()[0]);
	    }
	});

	processStylesheet(css::T_BACKGROUND, [&](property p)
	{
		auto& last = p->getArgs().back();
		if (last.last() == ')')
		{
			if (last.startsWith("url("))
			{
				auto urlString = last.mid(4, last.length() - 5);
				if ((urlString.startsWith("'") && urlString.endsWith("'")) ||
					(urlString.startsWith("\"") && urlString.endsWith("\"")))
				{
					urlString = urlString.mid(1, urlString.length() - 2);
				}
				if (auto drawable = Drawables::get(urlString))
				{
					AString sizing;
					processStylesheet(css::T_BACKGROUND_SIZE, [&](property p)
					{
						if (p->getArgs().size() == 1)
							sizing = p->getArgs()[0];
					});


                    Repeat repeat = REPEAT_NONE;

                    processStylesheet(css::T_BACKGROUND_REPEAT, [&](property p)
                    {
                        if (p->getArgs().size() == 1) {
                            if (p->getArgs()[0] == "repeat") {
                                repeat = REPEAT;
                            } else if (p->getArgs()[0] == "repeat-x") {
                                repeat = REPEAT_X;
                            } else if (p->getArgs()[0] == "repeat-y") {
                                repeat = REPEAT_Y;
                            }
                        }
                    });


                    auto drawableDrawWrapper = [repeat, drawable, backgroundImageOverlay](const glm::ivec2& size) {
                        RenderHints::PushColor c;
                        Render::inst().setColor(backgroundImageOverlay);
                        Render::inst().setRepeat(repeat);
                        drawable->draw(size);
                        Render::inst().setRepeat(REPEAT_NONE);
                    };

                    if (sizing == "fit")
					{
						mCssDrawListFront << [&, drawableDrawWrapper]()
						{
                            drawableDrawWrapper(getSize());
						};
					} else if (sizing == "fit-padding")
					{
						mCssDrawListFront << [&, drawableDrawWrapper]()
						{
                            RenderHints::PushMatrix m;
                            Render::inst().setTransform(
                                    glm::translate(glm::mat4(1.f), glm::vec3{mPadding.left, mPadding.top, 0.f}));
                            drawableDrawWrapper(getSize() - glm::ivec2{mPadding.horizontal(), mPadding.vertical()});
						};
					}
					else {
						mCssDrawListFront << [&, drawable, drawableDrawWrapper]()
						{
							auto imageSize = glm::vec2(drawable->getSizeHint());
							if (drawable->isDpiDependent())
								imageSize *= AWindow::current()->getDpiRatio();

							RenderHints::PushMatrix m;
                            Render::inst().setTransform(
								glm::translate(glm::mat4(1.f),
									glm::vec3(glm::ivec2((glm::vec2(mSize) - imageSize) / 2.f), 0.f)));
                            drawableDrawWrapper(imageSize);
						};
					}
				}
			}
		}
		else {
			AColor color = p->getArgs()[0];
			mCssDrawListFront << [&, color]() {
				RenderHints::PushColor x;
                Render::inst().setColor(color);

				if (mBorderRadius > 0) {
                    Render::inst().drawRoundedRectAntialiased(0, 0, getWidth(), getHeight(), mBorderRadius);
				} else  {
                    Render::inst().setFill(Render::FILL_SOLID);
                    Render::inst().drawRect(0, 0, getWidth(), getHeight());
                }
			};
		}
	});

	processStylesheet(css::T_BORDER, [&](property p)
	{
		int width = 1;
		AColor c;

		enum
		{
			B_SOLID,
			B_INSET,
			B_OUTSET,
		} style = B_SOLID;

		switch (p->getArgs().size())
		{
		case 0:
			return;
		case 1:
			if (p->getArgs()[0] == "none")
				return;
			width = AMetric(p->getArgs()[0]).getValuePx();
			break;

		case 3:
			width = AMetric(p->getArgs()[0]).getValuePx();
			c = AColor(p->getArgs()[2]);
			if (p->getArgs()[1] == "outset")
			{
				style = B_OUTSET;
			}
			else if (p->getArgs()[1] == "inset")
			{
				style = B_INSET;
			}
			break;

		case 2:
			c = AColor(p->getArgs()[1]);
			break;
		}
		switch (style)
		{
		case B_SOLID:
			mCssDrawListFront << [&, c, width]() {
                RenderHints::PushColor x;
                if (mBorderRadius > 0) {
                    Render::inst().setColor(c);
                    Render::inst().drawRoundedBorder(0, 0, getWidth(), getHeight(), mBorderRadius, width);
                } else {
                    Render::inst().setFill(Render::FILL_SOLID);
                    RenderHints::PushMask mask([&]() {
                        /*
                        if (mBorderRadius > 0) {
                            Render::inst().drawRoundedRect(width, width, getWidth() - width * 2,
                                            getHeight() - width * 2, glm::max(mBorderRadius - width, 0.f));
                        } else */ {
                        Render::inst().drawRect(width, width, getWidth() - width * 2,
                                                    getHeight() - width * 2);
                    }
                    });
                    RenderHints::PushMask::Layer maskLayer(RenderHints::PushMask::Layer::DECREASE);
                    Render::inst().setColor(c);
                    Render::inst().drawRect(0, 0, getWidth(), getHeight());
                }
			};
			break;
		case B_INSET:
			mCssDrawListFront << [&, c, width]() {
				RenderHints::PushColor x;
                Render::inst().setFill(Render::FILL_SOLID);
                Render::inst().setColor(c);
                Render::inst().drawRectBorder(0, 0, getWidth(), getHeight(), width);

                Render::inst().setColor({0.55f, 0.55f, 0.55f, 1 });
                Render::inst().drawRectBorderSide(0, 0, getWidth(), getHeight(), width, S_CORNER_TOPLEFT);
			};
			break;
		case B_OUTSET:
			mCssDrawListFront << [&, c, width]() {
				RenderHints::PushColor x;
                Render::inst().setFill(Render::FILL_SOLID);
                Render::inst().setColor(c);
                Render::inst().drawRectBorder(0, 0, getWidth(), getHeight(), width);

                Render::inst().setColor({0.55f, 0.55f, 0.55f, 1 });
                Render::inst().drawRectBorderSide(0, 0, getWidth(), getHeight(), width, S_CORNER_BOTTOMRIGHT);
			};
			break;
		}
		//mForceStencilForBackground = true;
	});
	processStylesheet(css::T_BORDER_BOTTOM, [&](property p)
	{
		float width = 1.f;
		AColor c;


		switch (p->getArgs().size())
		{
		case 0:
			return;
		case 1:
			if (p->getArgs()[0] == "none")
				return;
			width = AMetric(p->getArgs()[0]).getValuePx();
			break;

		case 3:
			width = AMetric(p->getArgs()[0]).getValuePx();
			c = AColor(p->getArgs()[2]);
			break;

		case 2:
			c = AColor(p->getArgs()[1]);
			break;
		}
        mCssDrawListFront << [&, c, width]() {
            Render::inst().setFill(Render::FILL_SOLID);
            RenderHints::PushColor x;
            RenderHints::PushMask mask([&]() {
                if (mBorderRadius > 0) {
                    Render::inst().drawRoundedRect(0, 0, getWidth(),
                                                       getHeight() - width, glm::max(mBorderRadius - width, 0.f));
                } else {
                    Render::inst().drawRect(0, 0, getWidth(),
                                                getHeight() - width);
                }
            });
            RenderHints::PushMask::Layer maskLayer(RenderHints::PushMask::Layer::DECREASE);
            Render::inst().setColor(c);
            Render::inst().drawRect(0, 0, getWidth(), getHeight());
        };
		//mForceStencilForBackground = true;
	});

    processStylesheet(css::T_BORDER_RADIUS, [&](property p)
    {
        if (p->getArgs().size() == 1) {
            //mForceStencilForBackground = true;
            mBorderRadius = AMetric(p->getArgs()[0]).getValuePx();
        }
    });
}

void AView::userProcessStyleSheet(const std::function<void(css, const std::function<void(property)>&)>& processor)
{
}


float AView::getTotalFieldHorizontal() const
{
	return mPadding.horizontal() + mMargin.horizontal();
}

float AView::getTotalFieldVertical() const
{
	return mPadding.vertical() + mMargin.vertical();
}

int AView::getContentMinimumWidth()
{
	return 0;
}

int AView::getContentMinimumHeight()
{
	return 0;
}

bool AView::hasFocus() const
{
	return mHasFocus;
}

int AView::getMinimumWidth()
{
	ensureCSSUpdated();
	return (mFixedSize.x == 0 ? ((glm::max)(getContentMinimumWidth(), mMinSize.x) + mPadding.horizontal()) : mFixedSize.x);
}

int AView::getMinimumHeight()
{
	ensureCSSUpdated();
	return (mFixedSize.y == 0 ? ((glm::max)(getContentMinimumHeight(), mMinSize.y) + mPadding.vertical()) : mFixedSize.y);
}

void AView::getTransform(glm::mat4& transform) const
{
	transform = glm::translate(transform, glm::vec3{ getPosition(), 0.f });
}

FontStyle& AView::getFontStyle()
{
	if (mFontStyle.font == nullptr)
		mFontStyle.font = AFontManager::inst().getDefault();
	return mFontStyle;
}


void AView::pack()
{
	setSize(getMinimumWidth(), getMinimumHeight());
}

const ADeque<AString>& AView::getCssNames() const
{
	return mCssNames;
}

void AView::addCssName(const AString& css)
{
	mCssNames << css;
	mCssHelper = nullptr;
}

void AView::ensureCSSUpdated()
{
	if (mCssHelper == nullptr)
	{
		mCssHelper = _new<ACSSHelper>();
		connect(customCssPropertyChanged, mCssHelper,
			&ACSSHelper::onCheckPossiblyMatchCss);
		connect(mCssHelper->invalidateViewCss, this, [&]()
		{
			mCssHelper = nullptr;
		});

		connect(AWindow::current()->dpiChanged, this, [&]()
		{
			mCssHelper = nullptr;
		});
		connect(mCssHelper->checkPossiblyMatchCss, this, &AView::recompileCSS);
		connect(mCssHelper->checkPossiblyMatchCss, this, &AView::redraw);
		mCssEntries.clear();

		for (auto& entry : Stylesheet::inst().getEntries())
		{
			switch (entry->selectorMatches(this))
			{
			case Stylesheet::Entry::M_MATCH:
				mCssEntries << entry;
				break;
			case Stylesheet::Entry::M_POSSIBLY_MATCH:
				mCssPossibleEntries << entry;
				break;
			}
		}

		recompileCSS();
	}
}

void AView::onMouseEnter()
{
    if (AWindow::shouldDisplayHoverAnimations()) {
        mHovered = true;
    }
}


void AView::onMouseMove(glm::ivec2 pos)
{
}

void AView::onMouseLeave()
{
    if (AWindow::shouldDisplayHoverAnimations()) {
        mHovered = false;
    }
}


void AView::onMousePressed(glm::ivec2 pos, AInput::Key button)
{
	mPressed = true;
	if (auto w = getWindow())
	{
		if (w != this) {
			connect(w->mouseReleased, this, [&]()
			{
				AThread::current()->enqueue([&]()
				{
					// чтобы быть точно уверенным, что isPressed будет равно false.
					mPressed = false;
				});
				disconnect();
			});
		}
	}
}

void AView::onMouseReleased(glm::ivec2 pos, AInput::Key button)
{
	mPressed = false;
	emit clickedButton(button);
	switch (button)
	{
	case AInput::LButton:
		emit clicked();
		break;
	case AInput::RButton:
		emit clickedRight();
		break;
	}
}

void AView::onMouseDoubleClicked(glm::ivec2 pos, AInput::Key button)
{
	emit doubleClicked(button);
}

void AView::onMouseWheel(glm::ivec2 pos, int delta) {

}

void AView::onKeyDown(AInput::Key key)
{
}

void AView::onKeyRepeat(AInput::Key key)
{
}

void AView::onKeyUp(AInput::Key key)
{
}

void AView::onFocusAcquired()
{
	mHasFocus = true;
}

void AView::onFocusLost()
{
	mHasFocus = false;
}

void AView::onCharEntered(wchar_t c)
{

}

void AView::getCustomCssAttributes(AMap<AString, AVariant>& map)
{
	if (mEnabled)
	{
		map["enabled"] = true;
	}
	else
	{
		map["disabled"] = true;
	}
}

void AView::setEnabled(bool enabled)
{
	mEnabled = enabled;
    emit customCssPropertyChanged();
	setSignalsEnabled(mEnabled);
	emit customCssPropertyChanged();
}

void AView::setDisabled(bool disabled)
{
    mEnabled = !disabled;
    emit customCssPropertyChanged();
    setSignalsEnabled(mEnabled);
    emit customCssPropertyChanged();
}

void AView::setAnimator(const _<AAnimator>& animator) {
    mAnimator = animator;
    if (mAnimator)
        mAnimator->setView(this);
}

glm::ivec2 AView::getPositionInWindow() {
    glm::ivec2 p(0);
    for (AView* i = this; i && i->getParent(); i = i->getParent()) {
        p += i->getPosition();
    }
    return p;
}


void AView::setPosition(const glm::ivec2& position) {
    mPosition = position;
}
void AView::setSize(int width, int height)
{
    if (mFixedSize.x != 0)
    {
        mSize.x = mFixedSize.x;
    }
    else
    {
        mSize.x = width;
        if (mMinSize.x != 0)
            mSize.x = glm::max(mMinSize.x, mSize.x);
    }
    if (mFixedSize.y != 0)
    {
        mSize.y = mFixedSize.y;
    }
    else
    {
        mSize.y = height;
        if (mMinSize.y != 0)
            mSize.y = glm::max(mMinSize.y, mSize.y);
    }
    mSize = glm::min(mSize, mMaxSize);
}

void AView::setGeometry(int x, int y, int width, int height) {
    setPosition({ x, y });
    setSize(width, height);
}

bool AView::consumesClick(const glm::ivec2& pos) {
    return true;
}

void AView::setCss(const AString& cssCode) {
    if (cssCode.empty()) {
        mCustomStylesheet = nullptr;
    } else {
        mCustomStylesheet = std::make_unique<Stylesheet::Cache>();
        mCustomStylesheet->load(Stylesheet::inst(), _new<StringStream>(cssCode), true);
    }
    recompileCSS();
}

_<AView> AView::determineSharedPointer() const {
    if (mParent) {
        for (auto& p : mParent->getViews()) {
            if (p.get() == this) {
                return p;
            }
        }
    }
    return nullptr;
}

void AView::focus() {
    uiX [&]() {
        auto s = determineSharedPointer();
        assert(s);
        AWindow::current()->setFocusedView(s);
    };
}

