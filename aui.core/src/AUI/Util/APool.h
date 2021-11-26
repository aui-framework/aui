#pragma once

#include <functional>
#include "AUI/Common/AQueue.h"

template<typename T>
class APool {
private:
    std::function<T*()> mFactory;
    AQueue<T*> mQueue;

public:
    APool(const std::function<T*()>& factory) : mFactory(factory) {}

    ~APool() {
        while (!mQueue.empty()) {
            delete mQueue.front();
            mQueue.pop();
        }
    }

    _<T> get() {
        T* t;
        if (mQueue.empty()) {
           t = mFactory();
        } else {
            t = mQueue.front();
            mQueue.pop();
        }
        return aui::ptr::manage(t, [&](T* t) {
            mQueue.push(t);
        });
    }
};

