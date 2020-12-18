#pragma once

#include <AUI/Util/ACursorSelectable.h>
#include "AView.h"
#include "AUI/Common/ATimer.h"
#include <AUI/Render/Render.h>

class API_AUI_VIEWS AAbstractTextField : public AView, public ACursorSelectable
{
private:
	AString mContents;
	Render::PrerenderedString mPrerenderedString;

	static ATimer& blinkTimer();
	
	unsigned mCursorBlinkCount = 0;
	bool mCursorBlinkVisible = true;
	bool mTextChangedFlag = false;
    bool mIsPasswordTextField = false;
    bool mIsMultiline = false;

	int mHorizontalScroll = 0;
	size_t mMaxTextLength = 0x200;

	void updateCursorBlinking();
	void updateCursorPos();
	void invalidatePrerenderedString() {
        mPrerenderedString.mVao = nullptr;
	}

    AString getContentsPasswordWrap();
	
protected:
	virtual bool isValidText(const AString& text) = 0;

    glm::ivec2 getMouseSelectionPadding() override;
    glm::ivec2 getMouseSelectionScroll() override;
    FontStyle getMouseSelectionFont() override;
    AString getMouseSelectionText() override;

    void doRedraw() override;

public:
	AAbstractTextField();
	virtual ~AAbstractTextField();

	int getContentMinimumHeight() override;

	void onKeyDown(AInput::Key key) override;
	void onKeyRepeat(AInput::Key key) override;

	void onCharEntered(wchar_t c) override;
	void render() override;


	void onFocusLost() override;
	void onMousePressed(glm::ivec2 pos, AInput::Key button) override;
	void onMouseMove(glm::ivec2 pos) override;
	void onMouseReleased(glm::ivec2 pos, AInput::Key button) override;

	void setText(const AString& t);

	[[nodiscard]] const AString& getText() const override
	{
		return mContents;
	}

    void setPasswordMode(bool isPasswordMode) {
        mIsPasswordTextField = isPasswordMode;
    }

	void setMaxTextLength(size_t newTextLength) {
	    mMaxTextLength = newTextLength;
	}

    bool handlesNonMouseNavigation() override;
    void onFocusAcquired() override;

signals:
	/**
	 * \brief изменение текстового поля завершено
	 * \note этот сигнал порождается в том числе при вызове функции AAbstractTextField::setText
	 */
	emits<AString> textChanged;

	/**
	 * \brief содержимое текстового поля изменяется пользователем.
	 */
	emits<AString> textChanging;
};
