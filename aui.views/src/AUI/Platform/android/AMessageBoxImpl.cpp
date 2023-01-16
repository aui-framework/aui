// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

#include "AUI/Platform/AMessageBox.h"


#include <AUI/Platform/android/OSAndroid.h>
#include <AUI/Platform/AMessageBox.h>

AMessageBox::ResultButton AMessageBox::show(AWindow* parent, const AString& title, const AString& message, AMessageBox::Icon icon, AMessageBox::Button b) {

    auto j = AAndroid::getJNI();
    auto klazzAUI = j->FindClass("com/github/aui/android/AUI");
    auto methodShowMessageBox = j->GetStaticMethodID(klazzAUI, "showMessageBox", "(Ljava/lang/String;Ljava/lang/String;)V");
    auto strTitle = j->NewStringUTF(title.toStdString().c_str());
    auto strMessage = j->NewStringUTF(message.toStdString().c_str());

    j->CallStaticVoidMethod(klazzAUI, methodShowMessageBox, strTitle, strMessage);

    j->DeleteLocalRef(strTitle);
    j->DeleteLocalRef(strMessage);

    return AMessageBox::ResultButton::INVALID;
}
