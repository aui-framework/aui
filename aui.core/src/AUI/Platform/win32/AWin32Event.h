// AUI Framework - Declarative UI toolkit for modern C++20
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

#pragma once

#include <AUI/Common/AObject.h>

#include <WIndows.h>

/**
 * @brief Encapsulates calls to CreateEvent/
 */
class API_AUI_CORE AWin32Event: public aui::noncopyable {
public:

    /**
     * @brief Creates or opens a named or unnamed event object.
     *
     * @param securityAttributes A pointer to a SECURITY_ATTRIBUTES structure. If this parameter is NULL, the handle
     *        cannot be inherited by child processes. The lpSecurityDescriptor member of the structure specifies a
     *        security descriptor for the new event. If lpEventAttributes is NULL, the event gets a default security
     *        descriptor. The ACLs in the default security descriptor for an event come from the primary or
     *        impersonation token of the creator.
     *
     * @param manualReset If this parameter is TRUE, the function creates a manual-reset event object, which requires
     *        the use of the ResetEvent function to set the event state to nonsignaled. If this parameter is FALSE, the
     *        function creates an auto-reset event object, and system automatically resets the event state to
     *        nonsignaled after a single waiting thread has been release.
     *
     * @param initialState If this parameter is TRUE, the initial state of the event object is signaled; otherwise, it
     *        is nonsignaled.
     *
     * @param name The name of the event object. The name is limited to MAX_PATH characters. Name comparison is case
     *        sensitive.
     */
    AWin32Event(LPSECURITY_ATTRIBUTES securityAttributes, bool manualReset, bool initialState, LPCWSTR name);
    AWin32Event(): AWin32Event(nullptr, false, false, nullptr) {}

    ~AWin32Event();

    operator HANDLE() const noexcept {
        return mEvent;
    }

private:
    HANDLE mEvent;
};
