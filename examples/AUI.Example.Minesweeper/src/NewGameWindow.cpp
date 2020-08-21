#include "NewGameWindow.h"


#include "AUI/Layout/AGridLayout.h"
#include "AUI/Layout/AHorizontalLayout.h"
#include "AUI/Layout/AVerticalLayout.h"
#include "AUI/View/AButton.h"
#include "AUI/View/ALabel.h"
#include "AUI/View/ANumberPicker.h"
#include "AUI/Util/UIBuildingHelpers.h"

int gWidth = 10;
int gHeight = 10;
int gMines = 10;

void NewGameWindow::updateMinesMax()
{
	mMines->setMax(mWidth->getValue() * mHeight->getValue() - 25);
}

void NewGameWindow::updateDifficultyLabel()
{
	int difficulty = mWidth->getValue() * mHeight->getValue() / (glm::max)(mMines->getValue(), 1);

	AString text = u8"Сложность: ";
	switch (difficulty)
	{
	case 0:
	case 1:
		text += u8"очень высокая";
		break;
	case 2:
	case 3:
		text += u8"высокая";
		break;
	case 4:
	case 5:
		text += u8"средняя";
		break;
	case 6:
	case 7:
	case 8:
		text += u8"низкая";
		break;

	default:
		text += u8"очень низкая";
	}
	mDifficultyLabel->setText(text);
}

NewGameWindow::NewGameWindow(MinesweeperWindow* minesweeper):
	AWindow(u8"Новая игра", 100, 100, minesweeper),
	mMinesweeper(minesweeper)
{
	setWindowStyle(WS_DIALOG);

	setLayout(_new<AVerticalLayout>());

	auto form = _new<AViewContainer>();
	form->setLayout(_new<AGridLayout>(2, 3));
	form->addView(_new<ALabel>(u8"Ширина поля:"));
	mWidth = _new<ANumberPicker>();
	mWidth->setMin(8);
	mWidth->setMax(50);
	form->addView(mWidth);
	
	form->addView(_new<ALabel>(u8"Высота поля:"));
	mHeight = _new<ANumberPicker>();
	mHeight->setMin(8);
	mHeight->setMax(50);
	form->addView(mHeight);

	form->addView(_new<ALabel>(u8"Количество мин:"));
	mMines = _new<ANumberPicker>();
	mMines->setMin(8);
	
	form->addView(mMines);
	addView(form);
	addView(mDifficultyLabel = _new<ALabel>());
	
	auto begin = _new<AButton>(u8"Начать игру");
	auto cancel = _new<AButton>(u8"Отмена");
	
	begin->setDefault();

	begin->setExpanding({ 1, 1 });
	cancel->setExpanding({ 1, 1 });

	connect(begin->clicked, this, [&]()
	{
		close();
		mMinesweeper->beginGame(gWidth = mWidth->getValue(), 
			gHeight = mHeight->getValue(), gMines = mMines->getValue());
	});
	connect(cancel->clicked, this, &AWindow::close);
	
	addView(_container<AHorizontalLayout>({
		begin,
		cancel
	}));

	mWidth->setValue(gWidth);
	mHeight->setValue(gHeight);
	
	updateMinesMax();
	
	mMines->setValue(gMines);
	
	updateDifficultyLabel();

	connect(mWidth->valueChanging, this, &NewGameWindow::updateMinesMax);
	connect(mWidth->valueChanging, this, &NewGameWindow::updateDifficultyLabel);
	connect(mHeight->valueChanging, this, &NewGameWindow::updateMinesMax);
	connect(mHeight->valueChanging, this, &NewGameWindow::updateDifficultyLabel);
	connect(mMines->valueChanging, this, &NewGameWindow::updateDifficultyLabel);
	
	pack();
}
