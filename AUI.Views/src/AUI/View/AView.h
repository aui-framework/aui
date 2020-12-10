#pragma once
#include <glm/glm.hpp>

#include "AUI/Common/ABoxFields.h"
#include "AUI/Common/AObject.h"
#include "AUI/Common/ADeque.h"
#include "AUI/Common/SharedPtr.h"
#include "AUI/Platform/AInput.h"
#include "AUI/Render/ACSSHelper.h"
#include "AUI/Render/Stylesheet.h"
#include "AUI/Reflect/AClass.h"
#include "AUI/Util/Watchable.h"

#include <chrono>
#include <functional>

#include "AUI/Common/AVariant.h"
#include "AUI/Platform/ACursor.h"
#include "AUI/Render/FontStyle.h"
#include "AUI/Util/IShadingEffect.h"

class Render;
class AWindow;
class AViewContainer;
class AAnimator;

/**
 * \brief Класс, описывающий минимальную единицу, которая может быть
 *        помещена в контейнер (в том числе в окно), занимающая некоторое
 *        пространство на экране, реагирующее на изменения размера, положения,
 *        перемещения курсора, нажатия/отпускания клавиш и кнопок, движения
 *        колёсика мыши и т. д.
 *
 *        Аналог QWidget, View.
 *
 * \note  Для корректной работы CSS в конструкторах всех ваших
 *        классов, наследующих AView следует написать AVIEW_CSS
 */
class API_AUI_VIEWS AView: public AObject
{
	friend class AViewContainer;
	friend class Stylesheet::Entry;
public:
	enum Overflow
	{
		OF_VISIBLE,
		OF_HIDDEN
	};

	enum Visibility {
		/**
		 * AView виден и активен
		 */
		V_VISIBLE,

		/**
		 * AView невидим, но мышкой кликнуть можно
		 */
		 V_INVISIBLE,

		 /**
		  * AView невидим и мышкой попасть невозможно
		  */
		  V_GONE
	};

private:
	/**
	 * \brief Вспомогательный объект для получения оповещений об
	 *	      обновлении CSS от других View.
	 */
	_<ACSSHelper> mCssHelper;

	/**
	 * \brief Дополнительная анимация.
	 */
	_<AAnimator> mAnimator;

	/**
	 * \brief Определяет, отображать ли выходящую за границу AView
	 *		  графику, или нет
	 */
	Overflow mOverflow = OF_VISIBLE;

	/**
	 * \brief Определяет, виден ли и можно ли тыкнуть мышкой в
	 *        этот AView.
	 */
	Visibility mVisibility = V_VISIBLE;

	/**
	 * \brief Список отрисовки, или запекание команд отрисовки,
	 *        чтобы каждый раз не парсить CSS.
	 */
	ADeque<std::function<void()>> mCssDrawListFront;

	/**
	 * \brief При включённых анимациях (mHasTransitions = true)
	 *		  содержит список отрисовки предыдущего состояния.
	 */
	ADeque<std::function<void()>> mCssDrawListBack;

	/**
	 * \brief Эффекты фона
	 */
	ADeque<_<IShadingEffect>> mBackgroundEffects;

	/**
	 * \brief Коэффициент времени текущей анимации.
	 *		  = 0 - начало анимации (mCssDrawListBack)
	 *		  = 1 - конец анимации (mCssDrawListFront)
	 */
	float mTransitionValue = 0.f;

	/**
	 * \brief Время предыдущего кадра для определения, какой
	 *		  длительности должен быть шаг анимации.
	 */
	std::chrono::milliseconds mLastFrameTime;

	/**
	 * \brief Длительность текущей анимации (в сек.)
	 */
	float mTransitionDuration;

	/**
	 * \brief Свидетельствует о наличии анимации (перехода).
	 */
	bool mHasTransitions = false;

	/**
	 * \brief border-radius, заданный в CSS
	 */
    float mBorderRadius = 0;

	/**
	 * \brief Стиль шрифта для этого AView.
	 */
	FontStyle mFontStyle;

	_unique<Stylesheet::Cache> mCustomStylesheet;


protected:
	/**
	 * \brief Родительский элемент
	 */
	AViewContainer* mParent = nullptr;

	/**
	 * \brief Определяет, какую форму должен принять указатель,
	 *		  когда он находится над этим AView.
	 */
	ACursor mCursor = ACursor::DEFAULT;

	/**
	 * \brief Позиция этого View относительно верхнего
	 *        левого угла родителя.
	 */
	glm::ivec2 mPosition;

	/**
	 * \brief Размер этого View, включая content area,
	 *        padding и border.
	 */
	glm::ivec2 mSize = glm::ivec2(20, 20);

	/**
	 * \brief Коэффициент расширения. Подсказывает менеджеру
	 *        компоновки, настолько нужно расширять этот
	 *        View относительно других View. (x;y)
	 */
	glm::ivec2 mExpanding = {0, 0};

	/**
	 * \brief Значения минимального размера CSS.
	 */
	glm::ivec2 mMinSize = {0, 0};

	/**
	 * \brief Значения максимального размера CSS.
	 */
	glm::ivec2 mMaxSize = {0x7fffffff, 0x7fffffff};

	/**
	 * \brief Значения абсолютного размера CSS (и не только).
	 */
	glm::ivec2 mFixedSize = {0, 0};

	/**
	 * \brief CSS margin. Подсказывает менеджеру компоновки,
	 *		  на каком расстоянии от этого View должны располагаться
	 *		  другие View.
	 */
	ABoxFields mMargin;

	/**
	 * \brief CSS padding. Обрабатывается реализацией AView
	 *		  индивидуально. Определяет расстояние от краёв View
	 *		  до content area.
	 */
	ABoxFields mPadding;

	/**
	 * \brief Правила CSS, которые подходят этому View и
	 *		  на 100% не зависят от состояния этого или других
	 *		  View.
	 */
	ADeque<_<Stylesheet::Entry>> mCssEntries;


	/**
	 * \brief Правила CSS, которые подходят этому View и
	 *		  зависят от состояния (:hover, :focus) этого
	 *		  или других View.
	 */
	ADeque<_<Stylesheet::Entry>> mCssPossibleEntries;

	/**
	 * \brief CSS названия классов.
	 */
	ADeque<AString> mCssNames;

	/**
	 * \brief Найти окно, которому принадлежит этот AView.
	 * \return окно, которому принадлежит этот AView.
	 *         Может быть nullptr.
	 */
	AWindow* getWindow();

	/**
	 * \brief Пересобирает mCssDrawList и вытаскивает значения
	 *		  маргинов, паддингов и прочих бордеров из таблицы
	 *		  стилей.
	 */
	virtual void recompileCSS();

	using property = const _<Stylesheet::Entry::Property>&;

	/**
	 * \brief Пользовательский AView может определить поведение
	 *		  на правила CSS
	 * \param processor интерфейс, который ищет требуемое правило
	 *		  с учётом весов.
	 */
	virtual void userProcessStyleSheet(const std::function<void(css, const std::function<void(property)>&)>& processor);


	/**
	 * \brief Попробовать определить std::shared_ptr для этого объекта.
	 */
    _<AView> determineSharedPointer() const;

public:
	AView();
	virtual ~AView() = default;

	/**
	 * \brief Попросить рендерер перерисовать этот AView.
	 */
	void redraw();


	virtual void drawStencilMask();

	/**
	 * \brief Отрисовка этого AView. Эта фукнция не должна
	 *		  вызываться никем, кроме как рендерером.
	 */
	virtual void render();

	virtual void postRender();

	void popStencilIfNeeded();

	/**
	 * \return позиция этого AView относительно левого верхнего
	 *	       края родителя (в пикселях).
	 */
	const glm::ivec2& getPosition() const
	{
		return mPosition;
	}

	/**
	 * \return размер этого AView (в пикселях).
	 */
	const glm::ivec2& getSize() const
	{
		return mSize;
	}

	Overflow getOverflow() const
	{
		return mOverflow;
	}

	/**
	 * \return ширина этого AView (в пикселях).
	 */
	int getWidth() const
	{
		return mSize.x;
	}

	/**
	 * \return высота этого AView (в пикселях).
	 */
	int getHeight() const
	{
		return mSize.y;
	}

	/**
	 * \return количество пикселей, занимаемое этим AView по горизонтали, с учётом размера, паддинга и маргина.
	 */
	float getTotalOccupiedWidth() const
	{
		return mSize.x + getTotalFieldHorizontal();
	}

    /**
     * \return количество пикселей, занимаемое этим AView по вертикали, с учётом размера, паддинга и маргина.
     */
    float getTotalOccupiedHeight() const
	{
        return mSize.y + getTotalFieldVertical();
	}

	/**
	 * \note каждый менеджер компоновки должен
	 *		 обрабатывать этот отступ.
	 * \return внешние отступы.
	 */
	[[nodiscard]]
	const ABoxFields& getMargin() const
	{
		return mMargin;
	}

	virtual bool consumesClick(const glm::ivec2& pos);


	/**
	 * \note каждый <class ?: AView> должен сам
	 *		 обрабатывать этот отступ.
	 * \return внешние отступы.
	 */
	[[nodiscard]]
	const ABoxFields& getPadding() const
	{
		return mPadding;
	}

	/**
	 * \brief расчитывает ширину, занимаемой этим
	 *	      AView.
	 *
	 * \return ширина, занимаемая этим AView.
	 */
	[[nodiscard]]
	float getTotalFieldHorizontal() const;


	/**
	 * \brief расчитывает высоту, занимаемой этим
	 *	      AView.
	 *
	 * \return высота, занимаемая этим AView.
	 */
	[[nodiscard]]
	float getTotalFieldVertical() const;

	AViewContainer* getParent() const
	{
		return mParent;
	}

	ACursor getCursor() const
	{
		return mCursor;
	}


	/**
	 * \brief расчитывает минимальную ширину контента.
	 *
	 * \return минимальная ширина контента.
	 */
	virtual int getContentMinimumWidth();

	/**
	 * \brief расчитывает минимальную высоту контента.
	 *
	 * \return минимальная высота контента.
	 */
	virtual int getContentMinimumHeight();


	bool hasFocus() const;

	virtual int getMinimumWidth();
    virtual int getMinimumHeight();

	glm::ivec2 getMinimumSize() {
	    return {getMinimumWidth(), getMinimumHeight()};
	}

	[[nodiscard]] const glm::ivec2& getMaxSize() const
	{
		return mMaxSize;
	}

	int getContentWidth() const
	{
		return static_cast<int>(mSize.x - mPadding.horizontal());
	}
	int getContentHeight() const
	{
		return static_cast<int>(mSize.y - mPadding.vertical());
	}

	void setExpanding(const glm::ivec2& expanding)
	{
		mExpanding = expanding;
	}

    const _<AAnimator>& getAnimator() const {
	    return mAnimator;
	}
    void setAnimator(const _<AAnimator>& animator);

    void getTransform(glm::mat4& transform) const;
	int getExpandingHorizontal() const
	{
		return mExpanding.x;
	}
	int getExpandingVertical() const
	{
		return mExpanding.y;
	}

	FontStyle& getFontStyle();

	virtual void setPosition(const glm::ivec2& position);
	virtual void setSize(int width, int height);
    virtual void setGeometry(int x, int y, int width, int height);

	void setFixedSize(const glm::ivec2& size) {
	    mFixedSize = size;
	}

	bool isMouseHover() const
	{
		return mHovered;
	}
	bool isMousePressed() const
	{
		return mPressed;
	}
	bool isEnabled() const
	{
		return mEnabled;
	}

	Visibility getVisibility() const
	{
		return mVisibility;
	}

	void setVisibility(Visibility visibility)
	{
		mVisibility = visibility;
		redraw();
	}

	/**
	 * \brief Выставить минимально возможный размер AView.
	 */
	void pack();


	/**
	 * \brief Выставить фокус на этот AView.
	 */
	 void focus();

	/**
	 * \return координаты этого AView относительно левого верхнего угла окна
	 */
    [[nodiscard]] glm::ivec2 getPositionInWindow();

	const ADeque<AString>& getCssNames() const;
	void addCssName(const AString& css);

	/**
	 * \brief добавить CSS класс к AView. Эта функция служит для упрощения создания элементов с кастомными классами
	 * \example
	 * <code>
	 * ...
	 * _new<ALabel>("Компоненты) << ".components_title"
	 * ...
	 * </code>
	 * \param cssName класс CSS, который нужно добавить к этому AView
	 */
	inline AView& operator<<(const AString& cssName) {
	    addCssName(cssName);
	    return *this;
	}

	void setCss(const AString& cssCode);
	void ensureCSSUpdated();

	virtual void onMouseEnter();
	virtual void onMouseMove(glm::ivec2 pos);
	virtual void onMouseLeave();

	virtual void onMousePressed(glm::ivec2 pos, AInput::Key button);
	virtual void onMouseReleased(glm::ivec2 pos, AInput::Key button);
    virtual void onMouseDoubleClicked(glm::ivec2 pos, AInput::Key button);
    virtual void onMouseWheel(glm::ivec2 pos, int delta);
    virtual void onKeyDown(AInput::Key key);
	virtual void onKeyRepeat(AInput::Key key);
	virtual void onKeyUp(AInput::Key key);
	virtual void onFocusAcquired();
	virtual void onFocusLost();

	virtual void onCharEntered(wchar_t c);

	virtual void getCustomCssAttributes(AMap<AString, AVariant>& map);

	void setEnabled(bool enabled = true);
	void setDisabled(bool disabled = true);

	void enable()
	{
		setEnabled(true);
	}
	void disable()
	{
		setEnabled(false);
	}

signals:
	emits<bool> hoveredState;
	emits<> mouseEnter;
	emits<> mouseLeave;

	emits<bool> pressedState;
	emits<> mousePressed;
	emits<> mouseReleased;

	emits<bool> enabledState;
	emits<> enabled;
	emits<> disabled;

	/**
	 * \brief щёлчок какой-то кнопкой мыши.
	 */
	emits<AInput::Key> clickedButton;

	/**
	 * \brief щёлчок левой кнопкой мыши.
	 */
	emits<> clicked;

	/**
	 * \brief щёлчок правой кнопкой мыши.
	 */
	emits<> clickedRight;

	emits<AInput::Key> doubleClicked;

	emits<> customCssPropertyChanged;

	emits<bool> focusState;
	emits<> focusAcquired;
	emits<> focusLost;

private:
	Watchable<bool> mHovered = Watchable<bool>(hoveredState, mouseEnter, mouseLeave);
	Watchable<bool> mPressed = Watchable<bool>(pressedState, mousePressed, mouseReleased);
	//Watchable<bool> mFocused = Watchable<bool>(pressedState, mousePressed, mouseReleased);
	Watchable<bool> mEnabled = Watchable<bool>(enabledState, enabled, disabled, true);
	Watchable<bool> mHasFocus = Watchable<bool>(focusState, focusAcquired, focusLost, false);
};

#define AVIEW_CSS addCssName(AClass<decltype(*this)>::name())