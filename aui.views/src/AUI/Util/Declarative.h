#pragma once

#include <AUI/View/AViewContainer.h>
#include <AUI/Util/kAUI.h>


namespace aui::ui_building {
    template<typename ContainerFactory>
    struct container_helper {
    public:
        container_helper(ContainerFactory& factory): mFactory(factory) {}

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

        auto operator->() const {
            return mFactory();
        }

    private:
        ContainerFactory& mFactory;
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
    struct layouted_container_factory: container_helper<layouted_container_factory<Layout, Container>> {
    private:
        AVector<_<AView>> mViews;

    public:
        struct Expanding: layouted_container_factory<Layout> {
        public:
            Expanding(std::initializer_list<_<AView>> views): layouted_container_factory<Layout>(views) {

            }

            _<AViewContainer> operator()() {
                return layouted_container_factory<Layout>::operator()() let { it->setExpanding(); };
            }
        };

        layouted_container_factory(std::initializer_list<_<AView>> views): container_helper<layouted_container_factory<Layout, Container>>(*this) {
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
}
