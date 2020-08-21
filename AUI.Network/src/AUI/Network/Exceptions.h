#pragma once
#include "AInet4Address.h"
#include "AUI/IO/IOException.h"


class AInet4Address;
class AString;

class SocketException : public IOException {
public:
	AInet4Address mDestAddress;
	SocketException(const AString& arg, const AInet4Address& addr);
	SocketException(const AString& arg);
};
class SocketConnectionResetException : public SocketException {
public:
	SocketConnectionResetException(const AString& arg, const AInet4Address& addr);
};
