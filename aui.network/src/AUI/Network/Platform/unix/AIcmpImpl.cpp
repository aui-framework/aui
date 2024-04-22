// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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

//
// Created by Alex2772 on 1/27/2023.
//

extern "C" {
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <netinet/ip_icmp.h>
#include <sys/time.h>
}

#include <AUI/Network/AIcmp.h>
#include "AUI/Common/AByteBufferView.h"
#include "AUI/Common/AException.h"
#include "AUI/Common/AOptional.h"
#include "AUI/IO/AIOException.h"
#include "AUI/Logging/ALogger.h"
#include "AUI/Platform/unix/UnixIoThread.h"
#include "AUI/Platform/ErrorToException.h"
#include "AUI/Common/AByteBuffer.h"
#include <chrono>
#include <csignal>


namespace {
    static constexpr auto MESSAGE_BUFFER_SIZE = 192;

class IcmpImpl: public aui::noncopyable {
public:
    static constexpr int SOCKET_TYPE = SOCK_DGRAM;

    static void setEUid() {
#if AUI_PLATFORM_LINUX
        auto r = setuid(geteuid());
        AUI_ASSERT(r >= 0);
#endif
    }
    static void setUid() {
#if AUI_PLATFORM_LINUX
        auto r = setuid(getuid());
        AUI_ASSERT(r >= 0);
#endif
    }

    IcmpImpl(const AInet4Address& mDestination) : mSocket(socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP)), mDestination(mDestination) {
        if (mSocket < 0) {
            throw AIOException(aui::impl::formatSystemError().description);
        }
    }

    static uint16_t calculateCheckum(AByteBufferView buffer)
    {
        /* RFC 1071 - http://tools.ietf.org/html/rfc1071 */

        size_t i;
        uint64_t sum = 0;
        const char* buf = buffer.data();

        for (i = 0; i < buffer.size(); i += 2) {
            sum += *(uint16_t *)buf;
            buf += 2;
        }
        if (buffer.size() - i > 0)
            sum += *(uint8_t *)buf;

        while ((sum >> 16) != 0)
            sum = (sum & 0xffff) + (sum >> 16);

        return (uint16_t)~sum;
    }

    AFuture<std::chrono::high_resolution_clock::duration> send(std::chrono::milliseconds timeout) {
        try {
            {
                int hold = 1;
#if !AUI_PLATFORM_APPLE
                setsockopt(mSocket, SOL_IP, IP_RECVERR, &hold, sizeof(hold));
                setsockopt(mSocket, SOL_IP, IP_RECVTTL, &hold, sizeof(hold));
                setsockopt(mSocket, SOL_IP, IP_RETOPTS, &hold, sizeof(hold));
#endif
                setsockopt(mSocket, SOL_SOCKET, SO_TIMESTAMP, &hold, sizeof(hold));

                hold = 512;
                setsockopt(mSocket, SOL_SOCKET, SO_RCVBUF, (char *)&hold, sizeof(hold));
                setsockopt(mSocket, SOL_SOCKET, SO_SNDBUF, (char *)&hold, sizeof(hold));

                timeval tv;
                tv.tv_sec = 10;
                tv.tv_usec = 0;
                setsockopt(mSocket, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv, sizeof(tv));


                tv.tv_sec = std::chrono::duration_cast<std::chrono::seconds>(timeout).count();
                tv.tv_usec = std::chrono::duration_cast<std::chrono::microseconds>(timeout).count() % 1'000'000;

                setsockopt(mSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(tv));
            }

            const auto socketAddress = mDestination.addr();
            {
                icmp icp;
                aui::zero(icp);
                icp.icmp_type = ICMP_ECHO;
                icp.icmp_seq = htons(1);

                icp.icmp_cksum = calculateCheckum(AByteBufferView::fromRaw(icp));

                auto r = sendto(mSocket, reinterpret_cast<const char *>(&icp), sizeof(icp), 0,
                                reinterpret_cast<const sockaddr *>(&socketAddress), sizeof(socketAddress));
                mTime = std::chrono::high_resolution_clock::now();
                if (r != sizeof(icp)) {
                    throw AIOException("ping send error: {}"_format(aui::impl::formatSystemError().description));
                }
            }
        } catch (...) {
            mResult.supplyException();
        }

        return mResult;
    }

    bool receive() {
        try {
            char messageBuffer[MESSAGE_BUFFER_SIZE];
            char packetInfoBuffer[MESSAGE_BUFFER_SIZE];
            iovec messageBufferIov = {
                messageBuffer,
                sizeof(messageBuffer)
            };
            msghdr msg = {
                nullptr,
                0,
                &messageBufferIov,
                1,
                packetInfoBuffer,
                sizeof(packetInfoBuffer),
                0
            };

            const auto messageLength = (int)recvmsg(mSocket, &msg, 0);
            if (messageLength < 0) {
                aui::impl::lastErrorToException("recvmsg failed");
            }

            /*
             * For IPv4, we must take the length of the IP header into
             * account.
             *
             * Header length is stored in the lower 4 bits of the VHL field
             * (VHL = Version + Header Length).
             */
            const auto ipHeaderLength = ((*(uint8_t *)messageBuffer) & 0x0F) * 4;

            const auto reply = (icmp*)(messageBuffer + ipHeaderLength);
            const auto replySeq = ntohs(reply->icmp_seq);

            /*
             * Verify that this is indeed an echo reply packet.
             */
            if (reply->icmp_type != 0 /* ICMP_ECHO_REPLY */) {
                return false;
            }

            /*
             * Verify the sequence number to make sure that the reply
             * is associated with the current request.
             */
            if (replySeq != 1) {
                return false;
            }

            const auto replyChecksum = reply->icmp_cksum;
            reply->icmp_cksum = 0;

            /*
             * Verify the checksum.
             */
            if (replyChecksum != calculateCheckum(AByteBufferView::fromRaw(*reply))) {
                throw AException("bad checksum");
            }
            mResult.supplyValue(std::chrono::high_resolution_clock::now() - mTime);

            return true;

        } catch (...) {
            mResult.supplyException();
        }
        return true;
    }

    ~IcmpImpl() {
        close(mSocket);
    }

    int mSocket;

    AFuture<std::chrono::high_resolution_clock::duration> mResult;
private:
    AInet4Address mDestination;
    AFuture<std::chrono::high_resolution_clock::duration> mResult;

    std::chrono::high_resolution_clock::time_point mTime;
};
}

AFuture<std::chrono::high_resolution_clock::duration> AIcmp::ping(AInet4Address destination, std::chrono::milliseconds timeout) noexcept {
    auto impl = _new<IcmpImpl>(destination);
    auto timer = _new<ATimer>(timeout);
    AObject::connect(timer->fired, timer, [impl]() {
        impl->supplyException(AIOException("timeout"));
        UnixIoThread::inst().unregisterCallback(impl->mSocket);
    });

    UnixIoThread::inst().registerCallback(impl->mSocket, UnixPollEvent::IN, [impl, timer](ABitField<UnixPollEvent>) mutable {
        if (impl->receive()) {
            UnixIoThread::inst().unregisterCallback(impl->mSocket);
        }
    });
    timer->start();

    return impl->send(timeout);
}
