#pragma once

#include <AUI/Thread/AThreadPool.h>


#define async AThreadPool::global() * [=]()
#define ui (*getThread()) * [=]()
#define asyncX AThreadPool::global() *
#define uiX (*getThread()) *
#define repeat(times) for(auto repeatStubIndex = 0; repeatStubIndex < times; ++repeatStubIndex)
#define repeat_async(times) for(auto repeatStubIndex = 0; repeatStubIndex < times; ++repeatStubIndex) AThreadPool::global() << [=]()