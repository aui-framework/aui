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

//
// Created by Alex2772 on 1/27/2023.
//

#include <AUI/Network/AIcmp.h>
#include "AUI/IO/AIOException.h"
#include "AUI/Platform/unix/UnixIoThread.h"
#include "AUI/Platform/ErrorToException.h"
#include "AUI/Common/AByteBuffer.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <csignal>

#include <netinet/ip_icmp.h>
#include <sys/time.h>

class IcmpImpl: public aui::noncopyable {
public:
    static constexpr int SOCKET_TYPE = SOCK_DGRAM;

    static void setEUid() {
#if AUI_PLATFORM_LINUX
        auto r = setuid(geteuid());
        assert(r >= 0);
#endif
    }
    static void setUid() {
#if AUI_PLATFORM_LINUX
        auto r = setuid(getuid());
        assert(r >= 0);
#endif
    }

    IcmpImpl(const AInet4Address& mDestination) : mDestination(mDestination) {}

    static inline void tvsub(struct timeval *out, struct timeval *in)
    {
        if ((out->tv_usec -= in->tv_usec) < 0) {
            --out->tv_sec;
            out->tv_usec += 1000000;
        }
        out->tv_sec -= in->tv_sec;
    }

    int isOurs(uint16_t id)
    {
        return SOCKET_TYPE == SOCK_DGRAM || id == mId;
    }

    static unsigned short calculateCheckum(const unsigned short *addr, int len, unsigned short csum) // iputils/ping/ping.c
    {
        int nleft = len;
        const unsigned short *w = addr;
        unsigned short answer;
        int sum = csum;

        /*
         *  Our algorithm is simple, using a 32 bit accumulator (sum),
         *  we add sequential 16 bit words to it, and at the end, fold
         *  back all the carry bits from the top 16 bits into the lower
         *  16 bits.
         */
        while (nleft > 1) {
            sum += *w++;
            nleft -= 2;
        }

        /* mop up an odd byte, if necessary */
        if (nleft == 1)
            sum += le16toh(*(unsigned char *)w); /* le16toh() may be unavailable on old systems */

        /*
         * add back carry outs from top 16 bits to low 16 bits
         */
        sum = (sum >> 16) + (sum & 0xffff);	/* add hi 16 to low 16 */
        sum += (sum >> 16);			/* add carry */
        answer = ~sum;				/* truncate to 16 bits */
        return (answer);
    }
    int ping4ParseReply(msghdr *msg, int cc, void *addr,
                        timeval *tv)
    {
        sockaddr_in* from = static_cast<sockaddr_in*>(addr);
        uint8_t* buf = static_cast<uint8_t*>(msg->msg_iov->iov_base);
        icmphdr *icp;
        iphdr *ip;
        int hlen;
        int csfailed;
        cmsghdr *cmsgh;
        int reply_ttl;
        uint8_t *opts, *tmp_ttl;
        int olen;
        int wrong_source = 0;

        /* Check the IP header */
        ip = (iphdr *)buf;
        if (SOCKET_TYPE == SOCK_RAW) {
            hlen = ip->ihl * 4;
            if (cc < hlen + 8 || ip->ihl < 5) {
                return 1;
            }
            reply_ttl = ip->ttl;
            opts = buf + sizeof(iphdr);
            olen = hlen - sizeof(iphdr);
        } else {
            hlen = 0;
            reply_ttl = 0;
            opts = buf;
            olen = 0;
            for (cmsgh = CMSG_FIRSTHDR(msg); cmsgh; cmsgh = CMSG_NXTHDR(msg, cmsgh)) {
                if (cmsgh->cmsg_level != SOL_IP)
                    continue;
                if (cmsgh->cmsg_type == IP_TTL) {
                    if (cmsgh->cmsg_len < sizeof(int))
                        continue;
                    tmp_ttl = (uint8_t *)CMSG_DATA(cmsgh);
                    reply_ttl = (int)*tmp_ttl;
                } else if (cmsgh->cmsg_type == IP_RETOPTS) {
                    opts = (uint8_t *)CMSG_DATA(cmsgh);
                    olen = cmsgh->cmsg_len;
                }
            }
        }

        /* Now the ICMP part */
        cc -= hlen;
        icp = (icmphdr *)(buf + hlen);
        csfailed = calculateCheckum((unsigned short*) icp, cc, 0);

        if (icp->type == ICMP_ECHOREPLY) {
            if (!isOurs(icp->un.echo.id))
                return 1;
            if (csfailed) {
                return 1;
            }

            if (from->sin_addr.s_addr != mDestination.addr().sin_addr.s_addr)
                return 0;

            const uint8_t *ptr = reinterpret_cast<uint8_t*>(icp + 8);
            if (cc >= (8 + sizeof(timeval))) {
                struct timeval tmp_tv;
                memcpy(&tmp_tv, ptr, sizeof(tmp_tv));

                tvsub(tv, &tmp_tv);
                auto tripTime = tv->tv_sec * 1000000 + tv->tv_usec;
                mResult.supplyResult(std::chrono::milliseconds(tripTime));
            }

        }

        return 0;
    }

    AFuture<std::chrono::high_resolution_clock::duration> send(std::chrono::milliseconds timeout) {
        try {
            setUid();
            setEUid();
            constexpr int SOCKET_TYPE = SOCK_DGRAM;
            mSocket = socket(AF_INET, SOCKET_TYPE, IPPROTO_ICMP);
            setUid();

            if (mSocket < 0) {
                throw AIOException(aui::impl::formatSystemError().description);
            }

            {
                int hold = 1;
                setsockopt(mSocket, SOL_IP, IP_RECVERR, &hold, sizeof(hold));
                setsockopt(mSocket, SOL_IP, IP_RECVTTL, &hold, sizeof(hold));
                setsockopt(mSocket, SOL_IP, IP_RETOPTS, &hold, sizeof(hold));
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
            auto r = ::connect(mSocket, reinterpret_cast<const sockaddr*>(&socketAddress), sizeof(sockaddr_in));

            if (r != 0) {
                throw AIOException(aui::impl::formatSystemError().description);
            }

            {
                char data[64];
                auto icp = (icmphdr*)data;
                icp->type = ICMP_ECHO;
                icp->code = 0;
                icp->checksum = 0;
                icp->un.echo.sequence = htons(1);
                icp->un.echo.id = -1;

                icp->checksum = calculateCheckum((unsigned short*) &icp, 64, 0);

                timeval tmp_tv;
                gettimeofday(&tmp_tv, NULL);
                memcpy(icp + 1, &tmp_tv, sizeof(tmp_tv));
                icp->checksum = calculateCheckum((unsigned short*) &tmp_tv, sizeof(tmp_tv), ~icp->checksum);

                r = sendto(mSocket, data, sizeof(data), 0, reinterpret_cast<const sockaddr*>(&socketAddress),
                           sizeof(socketAddress));
                if (r != sizeof(data)) {
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
            mIov.iov_base = mIovBuffer;
            mIov.iov_len = sizeof(mIovBuffer);
            memset(&mMsg, 0, sizeof(mMsg));
            mMsg.msg_name = mAddressBuffer;
            mMsg.msg_namelen = sizeof(mAddressBuffer);
            mMsg.msg_iov = &mIov;
            mMsg.msg_iovlen = 1;
            mMsg.msg_control = mAnsData;
            mMsg.msg_controllen = sizeof(mAnsData);

            int cc = recvmsg(mSocket, &mMsg, MSG_WAITALL);
            if (cc < 0) {
                return false;
            }
            cmsghdr* c;

            timeval* recv_timep = NULL;
            for (c = CMSG_FIRSTHDR(&mMsg); c; c = CMSG_NXTHDR(&mMsg, c)) {
                if (c->cmsg_level != SOL_SOCKET ||
                    c->cmsg_type != SO_TIMESTAMP)
                    continue;
                if (c->cmsg_len < CMSG_LEN(sizeof(struct timeval)))
                    continue;
                recv_timep = (struct timeval*) CMSG_DATA(c);
            }
            ping4ParseReply(&mMsg, cc, mAddressBuffer, recv_timep);
        } catch (...) {
            mResult.supplyException();
        }
        return true;
    }

    ~IcmpImpl() {
        shutdown(mSocket, 0);
    }

    int mSocket;

private:
    uint16_t mId = 0;
    AInet4Address mDestination;
    AFuture<std::chrono::high_resolution_clock::duration> mResult;

    iovec mIov;
    msghdr mMsg;
    char mIovBuffer[192];
    char mAddressBuffer[128];
    char mAnsData[4096];
};


AFuture<std::chrono::high_resolution_clock::duration> AIcmp::ping(AInet4Address destination, std::chrono::milliseconds timeout) noexcept {
    auto impl = _new<IcmpImpl>(destination);

    UnixIoThread::inst().registerCallback(impl->mSocket, POLLIN, [impl](int v) mutable {
        if (impl->receive()) {
            UnixIoThread::inst().unregisterCallback(impl->mSocket);
        }
    });


    return impl->send(timeout);
}
