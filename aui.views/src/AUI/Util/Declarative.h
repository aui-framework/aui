#pragma once

#include <AUI/View/AViewContainer.h>
#include <AUI/Util/kAUI.h>


namespace aui::ui_building {
    template<typename ViewFactory>
    struct view_helper {
    public:
        view_helper(ViewFactory& factory): mFactory(factory) {}

        operator _<AView>() const {
            return mFactory();
        }
        operator _<AViewContainer>() const {
            return mFactory();
        }
        auto operator<<(const AString& assEntry) const {
            return mFactory() << assEntry;
        }
        template<typename T>
        auto operator^(const T& t) const {
            return mFactory() ^ t;
        }
        template<typename T>
        auto operator+(const T& t) const {
            return mFactory() + t;
        }

        template<typename T>
        auto operator^(T&& t) const {
            return mFactory() ^ std::forward<T>(t);
        }
        template<typename T>
        auto operator+(T&& t) const {
            return mFactory() + std::forward<T>(t);
        }

        auto operator->() const {
            return mFactory();
        }


        template<typename SignalField, typename Object, typename Function>
        auto connect(SignalField&& signalField, Object&& object, Function&& function) {
            return mFactory().connect(std::forward<SignalField>(signalField), std::forward<Object>(object), std::forward<Function>(function));
        }

        template<typename Object, typename Function>
        auto clicked(Object&& object, Function&& function) {
            return connect(&AView::clicked, std::forward<Object>(object), std::forward<Function>(function));
        }

        template<typename SignalField, typename Function>
        auto connect(SignalField&& signalField, Function&& function) {
            return mFactory().connect(std::forward<SignalField>(signalField), std::forward<Function>(function));
        }

    private:
        ViewFactory& mFactory;
    };

    template<typename View>
    struct view {

    public:
        template<typename... Args>
        view(Args&&... args): mView(_new<View>(std::forward<Args>(args)...)) {}

        operator _<AView>() {
            return std::move(mView);
        }

    private:
        _<View> mView;
    };

    template<typename Layout, typename Container = AViewContainer>
    struct layouted_container_factory_impl {
    private:
        AVector<_<AView>> mViews;

    public:
        struct Expanding: layouted_container_factory_impl<Layout, Container>, view_helper<Expanding> {
        public:
            Expanding(std::initializer_list<_<AView>> views): layouted_container_factory_impl<Layout>(views),
                                                              view_helper<Expanding>(*this) {

            }

            _<AViewContainer> operator()() {
                return layouted_container_factory_impl<Layout>::operator()() let {
                    it->setExpanding();
                };
            }
        };

        layouted_container_factory_impl(std::initializer_list<_<AView>> views) {
            mViews.reserve(views.size());
            for (const auto& v : views) {
                if (v) {
                    mViews << std::move(const_cast<_<AView>&>(v));
                }
            }
        }

        _<AViewContainer> operator()() {
            auto c = _new<Container>();
            c->setLayout(_new<Layout>());
            c->setViews(std::move(mViews));
            return c;
        }
    };


    template<typename Layout, typename Container = AViewContainer>
    struct layouted_container_factory: layouted_container_factory_impl<Layout, Container>, view_helper<layouted_container_factory<Layout, Container>> {

        layouted_container_factory(std::initializer_list<_<AView>> views): layouted_container_factory_impl<Layout, Container>(views),
                                                                           view_helper<layouted_container_factory<Layout, Container>>(*this) {

        }

    };


}
