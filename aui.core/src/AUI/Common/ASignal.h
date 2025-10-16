/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <algorithm>
#include <functional>
#include "AUI/Common/ADeque.h"
#include "AUI/Common/AObject.h"
#include "AUI/Thread/AMutex.h"
#include "AAbstractSignal.h"
#include "AUI/Traits/values.h"
#include "AUI/Util/ARaiiHelper.h"
#include "AUI/Util/AEvaluationLoopException.h"

namespace aui::detail::signal {

template <typename... Args>
std::tuple<Args...> makeTupleOfCopies(std::tuple<const Args&...> args) {
    return args;
}

static_assert(requires(const int& v) {
    { makeTupleOfCopies(std::tie(v)) } -> aui::same_as<std::tuple<int>>;
});

template <size_t I, typename TupleInitial, typename TupleAll>
auto resizeTuple(TupleInitial initial, TupleAll all) {
    if constexpr (I == 0) {
        return initial;
    } else {
        return resizeTuple<I - 1>(
            std::tuple_cat(initial, std::tie(std::get<std::tuple_size_v<TupleInitial>>(all))), all);
    }
}

template <aui::not_overloaded_lambda Lambda, typename... Args>
inline void callIgnoringExcessArgs(Lambda&& lambda, const Args&... args) {
    static constexpr size_t EXPECTED_ARG_COUNT = std::tuple_size_v<typename lambda_info<std::decay_t<Lambda>>::args>;
    auto smallerTuple = resizeTuple<EXPECTED_ARG_COUNT>(std::make_tuple(), std::tie(args...));
    std::apply(lambda, smallerTuple);
}

template <typename Projection>
struct projection_info {
    static_assert(
        aui::reflect::pointer_to_member<Projection> || aui::not_overloaded_lambda<Projection> ||
            aui::function_pointer<Projection>,
        "====================> ASignal: projection is required to be an pointer-to-member or not overloaded lambda or function pointer");
};

template <aui::reflect::pointer_to_member Projection>
struct projection_info<Projection> {
private:
    template <typename... T>
    struct cat;

    template <typename First, typename... T>
    struct cat<First, std::tuple<T...>> {
        using type = std::tuple<First, T...>;
    };

public:
    using info = typename aui::reflect::member<Projection>;
    using return_t = typename info::return_t;
    using args = typename cat<typename info::clazz&, typename info::args>::type;
};

template <aui::not_overloaded_lambda Projection>
struct projection_info<Projection> {
    using info = typename aui::lambda_info<Projection>;
    using return_t = typename info::return_t;
    using args = typename info::args;
};

template <aui::function_pointer Projection>
struct projection_info<Projection> {
    using info = typename aui::function_info<Projection>;
    using return_t = typename info::return_t;
    using args = typename info::args;
};

template <
    typename AnySignal,   // can't use AAnySignal here, as concept would depend on itself
    typename Projection>
struct ProjectedSignal {
    friend class ::AObject;

    AnySignal& base;
    std::decay_t<Projection> projection;

    ProjectedSignal(AnySignal& base, Projection projection) : base(base), projection(std::move(projection)) {}

    using projection_info_t = aui::detail::signal::projection_info<Projection>;

    using projection_returns_t = typename projection_info_t::return_t;

    static constexpr bool IS_PROJECTION_RETURNS_TUPLE = aui::is_tuple<projection_returns_t>;

    using emits_args_t =
        std::conditional_t<IS_PROJECTION_RETURNS_TUPLE, projection_returns_t, std::tuple<projection_returns_t>>;

    template <convertible_to<AObjectBase*> Object, not_overloaded_lambda Lambda>
    void connect(Object objectBase, Lambda&& lambda) {
        base.connect(
            objectBase,
            tuple_visitor<typename projection_info_t::args>::for_each_all([&]<typename... ProjectionArgs>() {
                return [invocable = std::forward<Lambda>(lambda),
                        projection = projection](const std::decay_t<ProjectionArgs>&... args) {
                    auto result = std::invoke(projection, args...);
                    if constexpr (IS_PROJECTION_RETURNS_TUPLE) {
                        std::apply(invocable, std::move(result));
                    } else {
                        std::invoke(invocable, std::move(result));
                    }
                };
            }));
    }

    operator bool() const { return bool(base); }

    bool isAtSignalEmissionState() const noexcept {
        return base.isAtSignalEmissionState();
    }

private:
    template <not_overloaded_lambda Lambda>
    auto makeRawInvocable(Lambda&& lambda) const {
        return tuple_visitor<emits_args_t>::for_each_all([&]<typename... ProjectionResult>() {
            return [lambda = std::forward<Lambda>(lambda)](const std::decay_t<ProjectionResult>&... args) {
                aui::detail::signal::callIgnoringExcessArgs(lambda, args...);
            };
        });
    }
};

}   // namespace aui::detail::signal

/**
 * @brief Signal field which can be subscribed to.
 * @tparam Args Arguments of the signal.
 * @details
 * <!-- aui:index_alias emits -->
 */
template <typename... Args>
class ASignal final : public AAbstractSignal {
    static_assert(
        (std::is_object_v<Args> && ...),
        "// ====================> ASignal: there's no effect of specifying of non value arguments for the signal."
        "Consider removing const and reference modifiers.");

    /* ASignal <-> AObject implementation stuff */
    friend class AObject;

    /* tests */
    friend class PropertyPrecomputedTest_APropertyPrecomputed_Complex_Test;
    friend class SignalSlotTest;
    friend class PropertyTest;
    friend class PropertyPrecomputedTest;

    template <typename AnySignal, typename Projection>
    friend struct aui::detail::signal::ProjectedSignal;

    template <typename T>
    friend class AWatchable;

public:
    using func_t = std::function<void(const Args&...)>;
    using emits_args_t = std::tuple<Args...>;

    template <typename Projection>
    auto projected(Projection&& projection) const {
        return aui::detail::signal::ProjectedSignal(const_cast<ASignal&>(*this), std::forward<Projection>(projection));
    }

    struct call_wrapper {
        ASignal& signal;
        std::tuple<const Args&...> args;

        void invokeSignal(AObject* sender) { signal.invokeSignal(sender, args); }
    };

    call_wrapper operator()(const Args&... args) { return { *this, std::make_tuple(std::cref(args)...) }; }

    ASignal() = default;
    ASignal(ASignal&&) noexcept = default;
    ASignal(const ASignal&) noexcept {
        // mOutgoingConnections are not borrowed on copy operation.
    }

    ASignal& operator=(ASignal&&) noexcept = default;
    ASignal& operator=(const ASignal&) noexcept {
        // mOutgoingConnections are not borrowed on copy operation.
        return *this;
    }

    virtual ~ASignal() noexcept = default;

    /**
     * @brief Check whether signal contains any connected slots or not.
     * @details
     * It's very useful then signal argument values calculation is expensive and you do not want to calculate them if no
     * slots connected to the signal.
     * @return true, if slot contains any connected slots, false otherwise.
     */
    operator bool() const { return hasOutgoingConnections(); }

    void clearAllOutgoingConnections() const noexcept override { mOutgoingConnections.clear(); }
    void clearAllOutgoingConnectionsWith(aui::no_escape<AObjectBase> object) const noexcept override {
        clearOutgoingConnectionsIf([&](const _<ConnectionImpl>& p) { return p->receiverBase == object.ptr(); });
    }

    [[nodiscard]] bool hasOutgoingConnections() const noexcept {
        return !mOutgoingConnections.empty();
    }

    [[nodiscard]] bool hasOutgoingConnectionsWith(aui::no_escape<AObjectBase> object) const noexcept override {
        return std::any_of(
            mOutgoingConnections.begin(), mOutgoingConnections.end(),
            [&](const SenderConnectionOwner& s) { return s.value->receiverBase == object.ptr(); });
    }

    [[nodiscard]]
    bool isAtSignalEmissionState() const noexcept {
        return mLoopGuard.is_locked();
    }

private:
    struct ConnectionImpl final : Connection {
        friend class ASignal;

        void disconnect() override {
            std::unique_lock lock(AObjectBase::SIGNAL_SLOT_GLOBAL_SYNC);
            unlinkInSenderSideOnly(lock);
            if (!lock.owns_lock()) lock.lock();
            unlinkInReceiverSideOnly(lock);

            receiverBase = nullptr;
            receiver = nullptr;
            sender = nullptr;
        }

    private:
        /**
         * @brief Pointer to the sender signal.
         * Guaranteed to be valid until set to null.
         */
        ASignal* sender = nullptr;

        /**
         * @brief Pointer to the receiver object.
         * @details
         * Guaranteed to be valid until set to null.
         */
        AObjectBase* receiverBase = nullptr;

        /**
         * @brief Points to the same object as `receiverBase`.
         * @details
         * `receiverBase` object is likely a child of AObject. However, in some cases (i.e., receiver is a property) it
         * is not. In such case, `receiver` is nullptr. Whether or not the receiver is a AObject is determined during
         * connection creation, when actual type is known at compile time. Hence, we can avoid RTTI overhead.
         *
         * This receiver pointer is used to determine shared_ptr of the object. Thus, this enables queued (interthread)
         * connections.
         */
        AObject* receiver = nullptr;

        /**
         * @brief Receiver's signal handler.
         */
        func_t func;

        /**
         * @brief Whether is connection to be removed.
         * @details
         * When disconnected, this variable is set to `false`. This means the connection should be removed fromm ingoing
         * and outgoing connections arrays (of `AObject` and `ASignal`, respectively). Moreover, the connection marked
         * to be removed must not invoke its handler.
         */
        bool toBeRemoved = false;

        /**
         * @brief Breaks connection in the receiver side.
         * @details
         * Called when `ASignal` has cleaned its connection instance.
         *
         * This cleanup function assumes that an appropriate clean action for the sender side is taken.
         */
        void unlinkInReceiverSideOnly(std::unique_lock<ASpinlockMutex>& lock) {
            toBeRemoved = true;

            auto receiverLocal = std::exchange(receiverBase, nullptr);
            if (!receiverLocal) {
                return;
            }
            receiver = nullptr;
            removeIngoingConnectionIn(receiverLocal, *this, lock);
        }

        void unlinkInSenderSideOnly(std::unique_lock<ASpinlockMutex>& lock) {
            toBeRemoved = true;
            auto localSender = std::exchange(sender, nullptr);
            if (!localSender) {
                return;
            }

            // As we marked toBeRemoved, we are not required to do anything further. However, we can perform a cheap
            // operation to clean the connection right now. If we fail at some point we can leave it as is.
            // invokeSignal will clean the connection for us at some point.
            auto it = std::find_if(
                localSender->mOutgoingConnections.begin(), localSender->mOutgoingConnections.end(),
                [&](const SenderConnectionOwner& o) { return o.value.get() == this; });
            if (it == localSender->mOutgoingConnections.end()) {
                // It can happen probably when another thread is performing invocation on this signal and stole the
                // mOutgoingConnections array.
                return;
            }
            // it->value may be unique owner of this, let's steal the ownership before erasure to keep things safe.
            auto self = std::exchange(it->value, nullptr);
            localSender->mOutgoingConnections.erase(it);
            lock.unlock();
        }

        void onBeforeReceiverSideDestroyed() override {
            std::unique_lock lock(AObjectBase::SIGNAL_SLOT_GLOBAL_SYNC);
            // this function can be called by receiver's AObject cleanup functions (presumably, destructor), so we
            // assume receiver (and thus receiverBase) are invalid.
            receiver = nullptr;
            receiverBase = nullptr;
            unlinkInSenderSideOnly(lock);
        }
    };

    /**
     * @brief Connection owner which destroys the connection from the sender side in destructor.
     */
    struct SenderConnectionOwner {
        _<ConnectionImpl> value = nullptr;

        SenderConnectionOwner() = default;
        explicit SenderConnectionOwner(_<ConnectionImpl> connection) noexcept : value(std::move(connection)) {}
        SenderConnectionOwner(const SenderConnectionOwner&) = default;
        SenderConnectionOwner(SenderConnectionOwner&&) noexcept = default;
        SenderConnectionOwner& operator=(const SenderConnectionOwner& rhs) {
            if (this == &rhs) {
                return *this;
            }
            release();
            value = rhs;
            return *this;
        }
        SenderConnectionOwner& operator=(SenderConnectionOwner&& rhs) noexcept {
            if (this == &rhs) {
                return *this;
            }
            release();
            value = std::move(rhs.value);
            return *this;
        }

        ~SenderConnectionOwner() {
            release();
        }

    private:
        void release() noexcept {
            if (!value) {
                return;
            }
            std::unique_lock lock(AObjectBase::SIGNAL_SLOT_GLOBAL_SYNC);
            // this destructor can be called in ASignal destructor, so it's worth to reset the sender as well.
            value->sender = nullptr;
            value->unlinkInReceiverSideOnly(lock);
            value = nullptr;
        }
    };

    mutable AVector<SenderConnectionOwner> mOutgoingConnections;
    ASpinlockMutex mLoopGuard;

    void invokeSignal(AObject* sender, std::tuple<const Args&...> args = {});

    template <aui::convertible_to<AObjectBase*> Object, aui::not_overloaded_lambda Lambda>
    const _<ConnectionImpl>& connect(Object objectBase, Lambda&& lambda) {
        AObject* object = nullptr;
        if constexpr (std::is_base_of_v<AObject, std::remove_pointer_t<Object>>) {
            object = objectBase;
        }
        const auto& connection = [&]() -> _<ConnectionImpl>& {
            auto conn = _new<ConnectionImpl>();
            conn->sender = this;
            conn->receiverBase = objectBase;
            conn->receiver = object;
            conn->func = makeRawInvocable(std::forward<Lambda>(lambda));
            std::unique_lock lock(AObjectBase::SIGNAL_SLOT_GLOBAL_SYNC);

            std::erase_if(mOutgoingConnections, [](const SenderConnectionOwner& o) {
                return o.value == nullptr;
            });

            return mOutgoingConnections.emplace_back(std::move(conn)).value;
        }();
        if (objectBase != AObject::GENERIC_OBSERVER) {
            addIngoingConnectionIn(objectBase, connection);
        }
        return connection;
    }

    _<Connection> addGenericObserver(AObjectBase* receiver, std::function<void()> observer) override {
        return connect(receiver, [observer = std::move(observer)] { observer(); });
    }

    template <aui::not_overloaded_lambda Lambda>
    auto makeRawInvocable(Lambda&& lambda) const {
        return [lambda = std::forward<Lambda>(lambda)](const Args&... args) mutable {
            aui::detail::signal::callIgnoringExcessArgs(lambda, args...);
        };
    }

private:
    template <typename Predicate>
    void clearOutgoingConnectionsIf(Predicate&& predicate) const noexcept {
        /*
         * Removal of connections before end of execution of clearOutgoingConnectionsIf may cause this ASignal
         * destruction, causing undefined behaviour. Destructing these connections after mSlotsLock unlocking solves the
         * problem.
         */
        AVector<SenderConnectionOwner> slotsToRemove;

        slotsToRemove.reserve(mOutgoingConnections.size());
        mOutgoingConnections.removeIf([&slotsToRemove, predicate = std::move(predicate)](SenderConnectionOwner& p) {
            if (predicate(p.value)) {
                slotsToRemove << std::move(p);
                return true;
            }
            return false;
        });

        slotsToRemove.clear();
    }
};
#include <AUI/Thread/AThread.h>

template <typename... Args>
void ASignal<Args...>::invokeSignal(AObject* sender, std::tuple<const Args&...> args) {
    if (mOutgoingConnections.empty())
        return;

    _<AObject> senderPtr, receiverPtr;

    if (sender != nullptr) {
        if (auto sharedPtr = weakPtrFromObject(sender).lock()) {   // avoid sender removal during signal processing
            senderPtr = std::move(static_cast<_<AObject>>(sharedPtr));
        }
    }

    std::unique_lock lock(AObjectBase::SIGNAL_SLOT_GLOBAL_SYNC);
    std::unique_lock lock2(mLoopGuard, std::try_to_lock);
    if (!lock2.owns_lock()) {
        throw AEvaluationLoopException();
    }
    auto outgoingConnections = std::move(mOutgoingConnections);   // needed to safely iterate through the slots
    ARaiiHelper returnBack = [&] {
        if (!lock.owns_lock()) lock.lock();
        AUI_MARK_AS_USED(senderPtr);
        AUI_MARK_AS_USED(receiverPtr);

        if (mOutgoingConnections.empty()) {
            mOutgoingConnections = std::move(outgoingConnections);
        } else {
            // mSlots might have been modified by a single threaded signal call. In this case merge two vectors
            mOutgoingConnections.insert(
                mOutgoingConnections.begin(), std::make_move_iterator(outgoingConnections.begin()),
                std::make_move_iterator(outgoingConnections.end()));
        }
    };
    for (auto i = outgoingConnections.begin(); i != outgoingConnections.end();) {
        _<ConnectionImpl>& outgoingConnection = i->value;
        if (!lock.owns_lock()) lock.lock();
        if (outgoingConnection->toBeRemoved) {
            lock.unlock();
            i = outgoingConnections.erase(i);
            continue;
        }
        _weak<AObject> receiverWeakPtr;
        if (outgoingConnection->receiver != nullptr) {
            receiverWeakPtr = weakPtrFromObject(outgoingConnection->receiver);
            if (outgoingConnection->receiver->isSlotsCallsOnlyOnMyThread() &&
                outgoingConnection->receiver->getThread() != AThread::current()) {
                // perform crossthread call; should make weak ptr to the object and queue call to thread message queue

                /*
                 * shared_ptr counting mechanism is used when doing a crossthread call.
                 * It could not track the object existence without shared_ptr block.
                 * Also, receiverWeakPtr.lock() may be null here because object is in different thread and being
                 * destructed by shared_ptr but have not reached clearAllIngoingConnections() yet.
                 */
                if (receiverWeakPtr.lock() != nullptr) {
                    outgoingConnection->receiver->getThread()->enqueue(
                        [senderWeakPtr = senderPtr.weak(),
                         receiverWeakPtr = std::move(receiverWeakPtr),
                         connection = outgoingConnection,
                         args = aui::detail::signal::makeTupleOfCopies(args)] {
                            static_assert(
                                std::is_same_v<std::tuple<std::decay_t<Args>...>, std::remove_const_t<decltype(args)>>,
                                "when performing a cross thread call, args is expected to hold values "
                                "instead of references");
                            auto receiverPtr = receiverWeakPtr.lock();
                            if (!receiverPtr) {
                                // receiver was destroyed while we were transferring the call to another thread.
                                return;
                            }

                            AObject::isDisconnected() = false;
                            ARaiiHelper h = [&] {
                                if (AObject::isDisconnected()) {
                                    connection->disconnect();
                                }
                            };
                            try {
                                (std::apply)(connection->func, args);
                            } catch (...) {
                                if (auto senderPtr = senderWeakPtr.lock()) {
                                    senderPtr->handleSlotException(std::current_exception());
                                }
                            }
                        });
                }
                ++i;
                continue;
            }
        }
        AObject::isDisconnected() = false;

        if (auto sharedPtr = receiverWeakPtr.lock()) {   // avoid receiver removal during signal processing
            receiverPtr = std::move(sharedPtr);
        }

        if constexpr (std::tuple_size_v<std::remove_cvref_t<decltype(args)>> > 0) {
            auto check_and_assert_lambda =
              []<typename TupleType>(TupleType&& nonEmptyTupleArgs) {
                using FirstType = decltype(std::get<0>(std::forward<TupleType>(nonEmptyTupleArgs)));
                static_assert(
                    std::is_reference_v<FirstType> && std::is_const_v<std::remove_reference_t<FirstType>>,
                    "when performing a non-threading call, args is expected to hold const references "
                    "instead of values");
            };

            check_and_assert_lambda(args);
        }
        lock.unlock();
        try {
            (std::apply)(outgoingConnection->func, args);
        } catch (...) {
            if (senderPtr) {
                senderPtr->handleSlotException(std::current_exception());
            }
        }
        if (AObject::isDisconnected()) {
            i = outgoingConnections.erase(i);
            continue;
        }
        ++i;
    }
}

/**
 * @brief A signal declaration.
 * @tparam Args signal arguments
 * @ingroup signal_slot
 * See [signal_slot] "signal-slot system" for more info.
 */
template <typename... Args>
using emits = ASignal<Args...>;

#define signals public


/*

// UNCOMMENT THIS to test ProjectedSignal

static_assert(requires (aui::detail::signal::ProjectedSignal<emits<int>, decltype([](int) { return double(0);})> t) {
    requires !decltype(t)::IS_PROJECTION_RETURNS_TUPLE;
    { decltype(t)::base } -> aui::same_as<ASignal<int>&>;
    { decltype(t)::projection } -> aui::not_overloaded_lambda;
    { decltype(t)::projection_returns_t{} } -> aui::same_as<double>;
    { decltype(t)::emits_args_t{} } -> aui::same_as<std::tuple<double>>;
    { decltype(t)::projection_info_t::args{} } -> aui::same_as<std::tuple<int>>;
    { decltype(t)::projection_info_t::return_t{} } -> aui::same_as<double>;
});

static_assert(requires (aui::detail::signal::ProjectedSignal<emits<AString>, decltype(&AString::length)> t) {
    requires !decltype(t)::IS_PROJECTION_RETURNS_TUPLE;
    { decltype(t)::base } -> aui::same_as<ASignal<AString>&>;
    { decltype(t)::emits_args_t{} } -> aui::same_as<std::tuple<size_t>>;
    { decltype(t)::projection_returns_t{} } -> aui::same_as<size_t>;
});
*/
