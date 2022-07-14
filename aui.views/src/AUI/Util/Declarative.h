#pragma once

#include <AUI/View/AViewContainer.h>
#include <AUI/Util/kAUI.h>
#include <AUI/Traits/callables.h>
#include <AUI/Traits/parameter_pack.h>


namespace aui::ui_building {

    using View = _<AView>;
    using ViewContainer = _<AViewContainer>;
    using ViewGroup = AVector<_<AView>>;
    using ViewOrViewGroup = std::variant<_<AView>, AVector<_<AView>>>;

    template<typename ViewFactory>
    struct view_helper {
    public:
        view_helper(ViewFactory& factory): mFactory(factory) {}

        operator View() const {
            return mFactory();
        }
        operator ViewContainer() const {
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

    static_assert(std::is_convertible_v<view<AView>, View>, "declarative view wrapper is not convertible to _<AView>");

    template<typename Layout, typename Container = AViewContainer>
    struct layouted_container_factory_impl {
    private:
        AVector<View> mViews;

    public:
        struct Expanding: layouted_container_factory_impl<Layout, Container>, view_helper<Expanding> {
        public:
            template<typename... Views>
            Expanding(Views&&... views): layouted_container_factory_impl<Layout>(std::forward<Views>(views)...),
                                         view_helper<Expanding>(*this) {

            }

            ViewContainer operator()() {
                return layouted_container_factory_impl<Layout>::operator()() let {
                    it->setExpanding();
                };
            }
        };

        template<typename... Views>
        layouted_container_factory_impl(Views&&... views) {
            mViews.reserve(sizeof...(views));
            aui::parameter_pack::for_each([this](auto&& item) {
                using type = decltype(item);
                constexpr bool isViewGroup = std::is_convertible_v<type, ViewGroup>;
                constexpr bool isView = std::is_convertible_v<type, View>;

                static_assert(isViewGroup || isView, "the item is neither convertible to View nor ViewGroup");

                if constexpr (isViewGroup) {
                    auto asViewGroup = ViewGroup(item);
                    mViews << std::move(asViewGroup);
                } else if constexpr (isView) {
                    auto asView = View(item);
                    mViews << std::move(asView);
                }
            }, std::forward<Views>(views)...);
        }

        ViewContainer operator()() {
            auto c = _new<Container>();
            c->setLayout(_new<Layout>());
            c->setViews(std::move(mViews));
            return c;
        }
    };


    template<typename Layout, typename Container = AViewContainer>
    struct layouted_container_factory: layouted_container_factory_impl<Layout, Container>, view_helper<layouted_container_factory<Layout, Container>> {

        template<typename... Views>
        layouted_container_factory(Views&&... views): layouted_container_factory_impl<Layout, Container>(std::forward<Views>(views)...),
                                                      view_helper<layouted_container_factory<Layout, Container>>(*this) {

        }

    };


}
