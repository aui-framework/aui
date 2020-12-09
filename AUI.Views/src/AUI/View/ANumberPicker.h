#pragma once
#include "AViewContainer.h"
#include "ATextField.h"

/**
 * \brief текстовое поле для ввода чисел с кнопками увеличения/
 *		  уменьшения числа.
 */
class API_AUI_VIEWS ANumberPicker: public AViewContainer
{
private:
	class ANumberPickerField: public AAbstractTextField
	{
	private:
		ANumberPicker& mPicker;
	public:
		ANumberPickerField(::ANumberPicker& picker)
			: mPicker(picker)
		{
		}

		virtual ~ANumberPickerField() = default;

        void onKeyRepeat(AInput::Key key) override;

    protected:
		bool isValidText(const AString& text) override;
	};
	
	_<ANumberPickerField> mTextField;

	int mMin = 0;
	int mMax = 100;
	
public:
	ANumberPicker();

	int getContentMinimumHeight() override;

	void setValue(int v);
	int getValue() const;


	[[nodiscard]] int getMin() const
	{
		return mMin;
	}

	[[nodiscard]] int getMax() const
	{
		return mMax;
	}


    void setMin(const int min);
	void setMax(const int max);

signals:
	/**
	 * \brief число изменено
	 */
	emits<int> valueChanged;

	/**
	 * \brief число меняется
	 */
	emits<> valueChanging;
};
