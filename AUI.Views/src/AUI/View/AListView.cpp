#include "AListView.h"
#include "ALabel.h"
#include "AUI/Layout/AVerticalLayout.h"
#include "AUI/Platform/AWindow.h"



class AListItem: public ALabel
{
private:
	bool mSelected = false;

public:
	AListItem()
	{
		AVIEW_CSS;
	}

	AListItem(const AString& text)
		: ALabel(text)
	{
	}

	virtual ~AListItem() = default;


	void getCustomCssAttributes(AMap<AString, AVariant>& map) override
	{
		ALabel::getCustomCssAttributes(map);
		if (mSelected)
			map["selected"] = true;
	}

	void setSelected(const bool selected)
	{
		mSelected = selected;
		emit customCssPropertyChanged;
	}
};



int AListView::getContentMinimumHeight()
{
	return 40;
}

void AListView::onMousePressed(glm::ivec2 pos, AInput::Key button)
{
	AViewContainer::onMousePressed(pos, button);

	auto target = _cast<AListItem>(getViewAt(pos));
	if (target) {
	    if (!AInput::isKeyDown(AInput::LControl)) {
	        for (auto& s : mSelectionModel) {
                _cast<AListItem>(getViews()[s.getRow()])->setSelected(false);
	        }

	        mSelectionModel.clear();
	    }
	    mSelectionModel << AModelIndex(getViews().indexOf(target));
	    target->setSelected(true);

        emit selectionChanged(getSelectionModel());
	}
}

AListView::~AListView()
{
}

AListView::AListView(const _<IListModel<AString>>& model) {
    AVIEW_CSS;
    setModel(model);
}

void AListView::setModel(const _<IListModel<AString>>& model) {
    mModel = model;
    setLayout(_new<AVerticalLayout>());

    if (mModel) {
        for (size_t i = 0; i < model->listSize(); ++i) {
            addView(_new<AListItem>(model->listItemAt(i)));
        }

        connect(mModel->dataInserted, this, [&](const AModelRange<AString>& data) {
            for (const auto& row : data) {
                addView(_new<AListItem>(row));
            }
            updateLayout();
        });
    }
    updateLayout();
    AWindow::current()->flagRedraw();
}
