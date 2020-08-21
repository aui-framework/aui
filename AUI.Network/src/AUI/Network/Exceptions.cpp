#include "Exceptions.h"


SocketException::SocketException(const AString& arg, const AInet4Address& addr) :
	IOException(arg + ": " + addr.toString()),
	mDestAddress(addr)
{

}

SocketException::SocketException(const AString& arg): IOException(arg)
{
}

SocketConnectionResetException::SocketConnectionResetException(const AString& arg,
                                                               const AInet4Address& addr) : SocketException(
		arg, addr) {}
