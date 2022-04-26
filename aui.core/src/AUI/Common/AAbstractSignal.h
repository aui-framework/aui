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

#include <exception>

#include <AUI/Core.h>
#include <AUI/Common/SharedPtrTypes.h>

class AObject;

class API_AUI_CORE AAbstractSignal
{
private:
    bool mDestroyed = false;

protected:
	void linkSlot(AObject* object) noexcept;
	void unlinkSlot(AObject* object) noexcept;


	static bool& isDisconnected();

    static _weak<AObject> weakPtrFromObject(AObject* object);
	
public:
	virtual void clearAllConnectionsWith(AObject* object) = 0;
	virtual void clearAllConnections() = 0;
	virtual ~AAbstractSignal() {
	    mDestroyed = true;
	}

    [[nodiscard]] bool isDestroyed() const {
        return mDestroyed;
    }
};

#include <AUI/Common/AObject.h>

inline bool& AAbstractSignal::isDisconnected() {
    return AObject::isDisconnected();
}

inline _weak<AObject> AAbstractSignal::weakPtrFromObject(AObject* object) { // AAbstractSignal is a friend of AObject
    return object->objectWeakPtr();
}
