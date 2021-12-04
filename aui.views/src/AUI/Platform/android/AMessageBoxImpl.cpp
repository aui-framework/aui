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

#include "AUI/Platform/AMessageBox.h"


#include <AUI/Platform/OSAndroid.h>
#include <AUI/Platform/AMessageBox.h>

AMessageBox::ResultButton AMessageBox::show(AWindow* parent, const AString& title, const AString& message, AMessageBox::Icon icon, AMessageBox::Button b) {

    auto j = AAndroid::getJNI();
    auto klazzAUI = j->FindClass("ru/alex2772/aui/AUI");
    auto methodShowMessageBox = j->GetStaticMethodID(klazzAUI, "showMessageBox", "(Ljava/lang/String;Ljava/lang/String;)V");
    auto strTitle = j->NewStringUTF(title.toStdString().c_str());
    auto strMessage = j->NewStringUTF(message.toStdString().c_str());

    j->CallStaticVoidMethod(klazzAUI, methodShowMessageBox, strTitle, strMessage);

    j->DeleteLocalRef(strTitle);
    j->DeleteLocalRef(strMessage);

    return AMessageBox::ResultButton::INVALID;
}
