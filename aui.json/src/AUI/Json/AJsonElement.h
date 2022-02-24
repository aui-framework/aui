/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

#pragma once

#include "AUI/Common/SharedPtrTypes.h"
#include "AUI/Common/AMap.h"
#include "AUI/Common/AVector.h"
#include <AUI/IO/IOutputStream.h>
#include <AUI/Thread/AThreadPool.h>
#include <AUI/Model/IListModel.h>
#include <AUI/Model/AListModel.h>
#include <AUI/Common/AStringVector.h>

#ifdef API_AUI_DATA
#include <AUI/Data/AModelMeta.h>
#endif

class AJsonElement;
class AJsonObject: public AMap<AString, AJsonElement> {

};

class AJsonArray: public AVector<AJsonElement> {

};

class AJsonElement: public std::variant<int, float, bool, std::nullptr_t, AString, AJsonArray, AJsonObject> {
public:
};

#include <AUI/Json/AJsonSerialization.h>
