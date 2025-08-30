#include <gmock/gmock.h>
#include <AUI/UITest.h>

#ifdef Status
/* leaked from xlib */
#undef Status
#endif

#include <AUI/Updater/AUpdater.h>
#include "AUI/Util/UIBuildingHelpers.h"
#include "AUI/View/AButton.h"
#include "AUI/View/AProgressBar.h"

using namespace ass;
using namespace declarative;
using namespace std::chrono_literals;

class UpdaterStatusTest : public testing::UITest {
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

class MyUpdater : public AUpdater {
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


TEST_F(UpdaterStatusTest, Test1) {
    auto mUpdater = _new<MyUpdater>();

    //
    // Typical observer of status is a UI projection displaying its value. You can even display controls in it:
    mTestWindow->setContents(Vertical {
      // AUI_DOCS_CODE_BEGIN
      CustomLayout {} & mUpdater->status.readProjected([&updater = mUpdater](const std::any& status) -> _<AView> {
          if (std::any_cast<AUpdater::StatusIdle>(&status)) {
              return _new<AButton>("Check for updates").connect(&AView::clicked, AUI_SLOT(updater)::checkForUpdates);
          }
          if (std::any_cast<AUpdater::StatusCheckingForUpdates>(&status)) {
              return Label { "Checking for updates..." };
          }
          if (auto downloading = std::any_cast<AUpdater::StatusDownloading>(&status)) {
              return Vertical {
                  Label { "Downloading..." },
                  _new<AProgressBar>() & downloading->progress,
              };
          }
          if (std::any_cast<AUpdater::StatusWaitingForApplyAndRestart>(&status)) {
              return _new<AButton>("Apply update and restart")
                  .connect(&AView::clicked, AUI_SLOT(updater)::applyUpdateAndRestart);
          }
          return nullptr;
      }),
      // AUI_DOCS_CODE_END
    });
    mUpdater->status = AUpdater::StatusIdle {};

    EXPECT_CALL(*mUpdater, checkForUpdatesImpl());
    By::text("Check for updates").perform(click());

    AThread::sleep(100ms);
    AThread::processMessages();

    EXPECT_CALL(*mUpdater, downloadUpdateImpl(testing::_));
    mUpdater->downloadUpdate();

    AThread::sleep(100ms);
    AThread::processMessages();

    mUpdater->status = AUpdater::StatusDownloading { .progress = 0.25 };
    EXPECT_DOUBLE_EQ(*_cast<AProgressBar>(By::type<AProgressBar>().one())->value(), 0.25f);

    {
        auto statusDownloading = std::any_cast<AUpdater::StatusDownloading>(&*mUpdater->status);
        statusDownloading->progress = 0.5f;
        EXPECT_DOUBLE_EQ(*_cast<AProgressBar>(By::type<AProgressBar>().one())->value(), 0.5f);
    }
}
