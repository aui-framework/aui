// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
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
#include "AUI/Common/SharedPtrTypes.h"
#include "AUI/Common/AByteBuffer.h"

/**
 * @brief Stores public and/or private RSA key.
 * @ingroup crypt
 */
class API_AUI_CRYPT ARsa: public aui::noncopyable
{
private:
	void* mRsa = nullptr;


	explicit ARsa(void* rsa);

public:
	~ARsa();

	AByteBuffer encrypt(AByteBufferView in);
	AByteBuffer decrypt(AByteBufferView in);

	size_t getKeyLength() const;

	AByteBuffer getPrivateKeyPEM() const;
	AByteBuffer getPublicKeyPEM() const;
	
	static _<ARsa> generate(int bits = 0x800);
	static _<ARsa> fromPrivateKeyPEM(AByteBufferView buffer);
	static _<ARsa> fromPublicKeyPEM(AByteBufferView buffer);
};