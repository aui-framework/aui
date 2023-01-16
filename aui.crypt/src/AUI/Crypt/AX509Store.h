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

#pragma once
#include "AUI/Rsa.h"
#include "AUI/Common/AByteBuffer.h"
#include "AX509.h"

/**
 * @brief x509 certificate store object.
 * @ingroup crypt
 */
class API_AUI_CRYPT AX509Store: public aui::noncopyable
{
private:
	void* mX509Store;
	ADeque<_<AX509>> mCertificates;
	
public:
	AX509Store();
	~AX509Store();

	bool validate(_<AX509> cert);
	void addCert(_<AX509> cert);
};
