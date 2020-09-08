//
// Created by alex2772 on 9/3/20.
//

#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/View/AButton.h>
#include "NoteView.h"

CourseView::CourseView() {
    setLayout(_new<AHorizontalLayout>());
    addView(_container<AVerticalLayout>({
        _new<ALabel>("Заголовок") by(ALabel, {
            setFontSize(18dp);
        }),
        _new<ALabel>("Описание")
    }));

    addView(_new<ASpacer>());

    addView(_container<AStackedLayout>({
        _new<AButton>("Изучать")
                .connect(&AButton::clicked, this, &CourseView::learn)
    }));
}

void CourseView::learn() {

}
