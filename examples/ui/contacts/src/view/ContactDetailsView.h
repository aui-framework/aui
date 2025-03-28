/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <model/Contact.h>
#include <AUI/View/AViewContainer.h>

class ContactDetailsView : public AViewContainerBase {
public:
    ContactDetailsView(_<Contact> contact);

signals:
    emits<> deleteAction;

private:
    _<Contact> mContact;
    _<Contact> mOriginalContact;
    AProperty<bool> mEditorMode = false;

    template<typename T>
    _<AView> presentation(AProperty<T>& property);

    template<typename T>
    _<AView> row(AString title, AProperty<T>& property);

    void drop();
    void toggleEdit();
};

