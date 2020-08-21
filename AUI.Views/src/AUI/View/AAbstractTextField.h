#pragma once

#include "AView.h"
#include "AUI/Common/ATimer.h"

class API_AUI_VIEWS AAbstractTextField : public AView
{
public:
	struct Selection
	{
		unsigned begin;
		unsigned end;
	};
private:
	AString mContents;

	static ATimer& blinkTimer();
	unsigned mCursorIndex = 0;
	unsigned mCursorSelection = -1;
	
	unsigned mCursorBlinkCount = 0;
	bool mCursorBlinkVisible = true;
	bool mTextChangedFlag = false;

	int mHorizontalScroll = 0;

	void updateCursorBlinking();
	void updateCursorPos();
	unsigned getCursorIndexByPos(int posX);
	int getPosByIndex(int index);
	bool hasSelection();
	
protected:
	virtual bool isValidText(const AString& text) = 0;
	
public:
	AAbstractTextField();
	virtual ~AAbstractTextField();

	Selection getSelection();
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

	const AString& getText() const
	{
		return mContents;
	}

signals:
	/**
	 * \brief содержимое текстового поля изменено (в том числе при
	 *        вызове функции \code AAbstractTextField::setText \endcode)
	 */
	emits<> textChanged;

	/**
	 * \brief содержимое текстового поля изменяется пользователем.
	 */
	emits<> textChanging;
};
