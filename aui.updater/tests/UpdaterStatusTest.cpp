#include <gmock/gmock.h>
#include <AUI/UITest.h>

#ifdef Status
// leaked from xlib
#undef Status
#endif

#include <AUI/Updater/AUpdater.h>
#include "AUI/Util/UIBuildingHelpers.h"
#include "AUI/View/AButton.h"
#include "AUI/View/AProgressBar.h"

using namespace ass;
using namespace declarative;
using namespace std::chrono_literals;

class UpdaterStatusTest: public testing::UITest {
public:
protected:

    void SetUp() override {
        UITest::SetUp();

        mTestWindow = _new<AWindow>();
        mTestWindow->show();
    }

    void TearDown() override {
        mTestWindow = nullptr;
        UITest::TearDown();
    }

    _<AWindow> mTestWindow;
};



namespace {

class MyUpdater: public AUpdater {
public:
    MyUpdater() {
        ON_CALL(*this, checkForUpdatesImpl()).WillByDefault([] {
            AFuture<void> f;
            f.supplyValue();
            return f;
        });

        ON_CALL(*this, downloadUpdateImpl(testing::_)).WillByDefault([] {
          AFuture<void> f;
          f.supplyValue();
          return f;
        });
    }

    MOCK_METHOD(AFuture<void>, checkForUpdatesImpl, (), (override));
    MOCK_METHOD(AFuture<void>, downloadUpdateImpl, (const APath& unpackedUpdateDir), (override));
};

}

// AUI_DOCS_OUTPUT: doxygen/intermediate/updater_status_test.h
// @property AUpdater::status

TEST_F(UpdaterStatusTest, Test1) {
    auto mUpdater = _new<MyUpdater>();

    //
    // Typical observer of status is a UI projection displaying its value. You can even display controls in it:
    mTestWindow->setContents(Vertical {
      // AUI_DOCS_CODE_BEGIN
      CustomLayout {} & mUpdater->status.readProjected([&updater = mUpdater](const AUpdater::Status& status) {
          return std::visit(
              aui::lambda_overloaded {
                [&](const AUpdater::StatusIdle&) -> _<AView> {
                    return _new<AButton>("Check for updates").connect(&AView::clicked, slot(updater)::checkForUpdates);
                },
                [&](const AUpdater::StatusCheckingForUpdates&) -> _<AView> {
                    return Label { "Checking for updates..." };
                },
                [&](const AUpdater::StatusDownloading& s) -> _<AView> {
                    return Vertical {
                        Label { "Downloading..." },
                        _new<AProgressBar>() let { it->setValue(s.progress); },
                    };
                },
                [&](const AUpdater::StatusWaitingForApplyAndRestart&) -> _<AView> {
                    return _new<AButton>("Apply update and restart")
                        .connect(&AView::clicked, slot(updater)::applyUpdateAndRestart);
                },
                [&](const AUpdater::StatusNotAvailable&) -> _<AView> {
                    return nullptr;
                },
              },
              status);
      }),
      // AUI_DOCS_CODE_END
    });
    mUpdater->status = AUpdater::StatusIdle{};

    EXPECT_CALL(*mUpdater, checkForUpdatesImpl());
    By::text("Check for updates").perform(click());

    AThread::sleep(100ms);
    AThread::processMessages();

    EXPECT_CALL(*mUpdater, downloadUpdateImpl(testing::_));
    mUpdater->downloadUpdate();

    AThread::sleep(100ms);
    AThread::processMessages();
}
