#pragma once

namespace aui {
    struct parameter_pack {
        template<typename Callable, typename... Args>
        inline static void for_each(Callable c, Args&&... args) {
            invoke(c, std::forward<Args>(args)...);
        }

    private:

        template<typename Callable, typename Arg1>
        inline static void invoke(Callable c, Arg1&& arg1) {
            c(arg1);
        }
        template<typename Callable, typename Arg1, typename... Args>
        inline static void invoke(Callable c, Arg1&& arg1, Args&&... args) {
            c(arg1);
            invoke(c, args...);
        }
    };
}