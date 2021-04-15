#include <AUI/Platform/Entry.h>
#include <AUI/Platform/AWindow.h>
#include <AUI/Util/UIBuildingHelpers.h>
#include <AUI/Util/ADataBinding.h>

struct MyModel {
    AString data;
};

class MyWindow: public AWindow {
private:
    _<ADataBinding<MyModel>> mBinding = _new<ADataBinding<MyModel>>();
    const MyModel mModel;

public:

    MyWindow(const MyModel& model):
        AWindow("Hello world", 300_dp, 200_dp),
        mModel(model)
    {
        mBinding->setModel(mModel);

        setContents(
            Stacked {
                _new<ALabel>("Hello world!") && mBinding(&MyModel::data, &ALabel::setText)
            }
        );
    }
};

AUI_ENTRY {
    _new<MyWindow>({"DATA!!!!"})->show();

    return 0;
}