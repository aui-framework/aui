#include <cassert>
#include <AUI/Network/AUdpSocket.h>

#include "Exceptions.h"

#if defined(_WIN32)
#include <ws2tcpip.h>
#else

#include <sys/socket.h>
#include <netinet/in.h>

#endif



/**
 * \brief Запуск сокета
 * \param source_port
 */
AUdpSocket::AUdpSocket(uint16_t source_port) :
	mSelf({ 0, 0, 0, 0 }, source_port) {

    auto addr = mSelf.addr();
	if (::bind(getHandle(), (sockaddr*)&addr, sizeof(sockaddr_in)) < 0) {
		throw IOException(
			(AString("Failed to bind ASocket. Error code: ") + getErrorString()).c_str());
	}
}

/**
 * \brief Запуск сокета с рандомным портом (используется для "клиентского" сокета)
 */
AUdpSocket::AUdpSocket() :
	AUdpSocket(static_cast<uint16_t>((rand() % 20000) + 30000)) {
}


/**
 * \brief Отправляет данные по адресу. Плюёт исключение, если что-то пошло не так
 * \param buf Буфер
 * \param dst Адрес доставки
 */
void AUdpSocket::write(const AByteBuffer& buf, const AInet4Address& dst) {
	assert(buf.getSize() < 32768);
	//static boost::mutex m;
	//boost::unique_lock lock(m);
	auto addr = dst.addr();
	if (sendto(getHandle(), buf.getBuffer(), static_cast<int>(buf.getSize()), 0, (sockaddr*)&addr, sizeof(sockaddr_in)) <=
		0) {
		throw IOException(AString("sendto error ") + getErrorString());
	}
}

/**
 * \brief
 * \param buf
 * \param dst
 */
void AUdpSocket::read(AByteBuffer& buf, AInet4Address& dst) {
	buf.reserve(32768);
	sockaddr_in from;
	memset(&from, 0, sizeof(sockaddr_in));
	socklen_t  l = sizeof(from);
	int res;
	for (;;) {
		res = recvfrom(getHandle(), buf.getBuffer(), 32768, 0, (sockaddr*)& from, (socklen_t *)&l);
		if (res <= 0) {
			AString msg = AString("recvfrom error ") + getErrorString();
#if defined(_WIN32)
			switch (WSAGetLastError()) {
			case WSAEINTR:
				throw AThread::AInterrupted();
			case WSAECONNRESET:
				// https://stackoverflow.com/questions/30749423/is-winsock-error-10054-wsaeconnreset-normal-with-udp-to-from-localhost
				continue; // чё за херня кто это придумал??? 
			default:
				throw SocketException(msg, dst);
#else
            switch (res) {
                case EINTR:
                    throw AThread::AInterrupted();
                default:
                    throw SocketException(msg, dst);
#endif
			}
		}
		dst = from;
		break;
	}
	buf.setSize(static_cast<size_t>(res));
}

int AUdpSocket::createSocket()
{
	return socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
}
