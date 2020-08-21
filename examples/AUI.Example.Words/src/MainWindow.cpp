//
// Created by alex2772 on 2020-08-11.
//

#include <AUI/View/ALabel.h>
#include <AUI/Layout/AVerticalLayout.h>
#include <AUI/View/ASpacer.h>
#include "MainWindow.h"
#include <AUI/Render/AFontManager.h>
#include <AUI/Animator/ARotationAnimator.h>
#include <AUI/Layout/AStackedLayout.h>
#include <AUI/Util/kAUI.h>
#include <AUI/Data/ASqlDatabase.h>
#include <AUI/Data/AMigrationManager.h>
#include <AUI/Json/AJson.h>
#include <AUI/IO/FileInputStream.h>
#include <AUI/Data/ASqlBlueprint.h>
#include <AUI/Util/UIBuildingHelpers.h>

#include <Model/Course.h>
#include <AUI/Layout/AHorizontalLayout.h>
#include <AUI/Url/AUrl.h>

MainWindow::MainWindow() : AWindow("Words", 300, 400) {
    setLayout(_new<AStackedLayout>());

    auto
    loadingScreen = _container<AVerticalLayout>({
                    _container<AHorizontalLayout>({
                        _new<ASpacer>(),
                        _new<ALabel>(u8"\uf1ce")
                              (&ALabel::addCssName, ".icon")
                              (&ALabel::setFont,
                               AFontManager::instance().get(":words/fas.otf"))
                              (&ALabel::setFontSize, 50dp)
                              (&ALabel::setExpanding, glm::ivec2(0))
                              (&ALabel::setAnimator,
                               _new<ARotationAnimator>()
                                       (&ARotationAnimator::setRepeating, true)
                                       (&ARotationAnimator::setDuration, 1.5f)),
                        _new<ASpacer>()
                    }),
                    _new<ALabel>(u8"Запуск...")
            });
            addView(loadingScreen);

            asyncX[&, loadingScreen]() {
                Autumn::put(ASqlDatabase::connect("sqlite", "data.db", 0, {}, {}, {}));
                AMigrationManager mm;
                mm.registerMigration("initial", [&]() {
                    ASqlBlueprintTable t("courses");
                    t.varchar("name");
                    t.text("description");

                    auto json = AJson::read(AUrl(":words/courses.json").open());
                    for (auto &course : json.asArray()) {
                        Course::make(course["courseName"].asString(), course["courseDescription"].asString());
                    }

                });

                mm.doMigration();

                uiX [&, loadingScreen]() {
                    //addView(_new<ALabel>(u8"ты лох)))))"));
                    updateLayout();
                };
            };
            }
