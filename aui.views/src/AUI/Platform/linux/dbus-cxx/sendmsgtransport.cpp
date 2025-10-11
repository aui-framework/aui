// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2020 by Robert Middleton                                *
 *   robert.middleton@rm5248.com                                           *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#include "sendmsgtransport.h"

#include "dbus-cxx-private.h"
#include "utility.h"
#include "validator.h"
#include "message.h"

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <mutex>

#include <sys/socket.h>
#include <sys/types.h>

using DBus::priv::SendmsgTransport;

static const char* LOGGER_NAME = "DBus.priv.SendmsgTransport";

#define RECEIVE_BUFFER_SIZE 1024
#define SEND_BUFFER_SIZE    2048
#define CONTROL_BUFFER_SIZE 512

#ifdef _WIN32
class SendmsgTransport::priv_data {
public:
    priv_data( int fd ) :
        m_fd( fd ),
        m_ok( false ),
        rx_capacity( RECEIVE_BUFFER_SIZE ),
        rx_control_capacity( CONTROL_BUFFER_SIZE ),
        lpWSARecvMsg( NULL ) {
        ::memset( &rx_msg, 0, sizeof( WSAMSG ) );
        ::memset( &tx_msg, 0, sizeof( WSAMSG ) );
        m_sendBuffer.reserve( SEND_BUFFER_SIZE );
    }

    ~priv_data() {
        free( rx_msg.lpBuffers[0].buf );
        free( rx_msg.Control.buf );
        free( tx_msg.Control.buf );
    }

    int m_fd;
    bool m_ok;
    std::vector<uint8_t> m_sendBuffer;

    WSAMSG rx_msg;
    WSABUF rx_buf;
    int rx_capacity;
    int rx_control_capacity;

    WSAMSG tx_msg;
    WSABUF tx_buf;

    LPFN_WSARECVMSG lpWSARecvMsg;

    void init() {
        // Setup the RX data msghdr
        rx_msg.lpBuffers = &rx_buf;
        rx_msg.lpBuffers[0].buf = ( PCHAR ) ::malloc( rx_capacity );
        rx_msg.lpBuffers[0].len = rx_capacity;
        rx_msg.dwBufferCount = 1;
        rx_msg.Control.buf = ( PCHAR ) ::malloc( rx_control_capacity );
        rx_msg.Control.len = rx_control_capacity;

        // Setup the TX data msghdr
        tx_msg.lpBuffers = &tx_buf;
        tx_msg.dwBufferCount = 1;

        GUID g = WSAID_WSARECVMSG;
        DWORD dwBytesReturned = 0;

        if( WSAIoctl( m_fd, SIO_GET_EXTENSION_FUNCTION_POINTER, &g, sizeof( g ), &lpWSARecvMsg, sizeof( lpWSARecvMsg ), &dwBytesReturned, NULL, NULL ) != 0 ) {
            SIMPLELOGGER_DEBUG( LOGGER_NAME, "Unable to obtain a pointer to WSARecvMsg function" );
            m_ok = false;
        }
    }

    uint8_t* rx_byte_buf_data() {
        return reinterpret_cast<uint8_t*>( rx_msg.lpBuffers[0].buf );
    }

    ssize_t rx_size() {
        return rx_msg.lpBuffers[0].len;
    }

    ssize_t rx_control_size() {
        return rx_msg.Control.len;
    }

    void set_rx_capacity( ssize_t capacity ) {
        free( rx_msg.lpBuffers[0].buf );
        rx_msg.lpBuffers[0].buf = ( PCHAR ) ::malloc( capacity );
        rx_capacity = capacity;
    }

    int peek( ssize_t size ) {
        return receive( size, 0, 0, MSG_PEEK );
    }

    int send() {
        tx_buf.buf = ( PCHAR )m_sendBuffer.data();
        tx_buf.len = m_sendBuffer.size();
        tx_msg.Control.buf = nullptr;
        tx_msg.Control.len = 0;

        int result = WSASendMsg( m_fd, &tx_msg, 0, NULL, NULL, NULL );

        if( result == SOCKET_ERROR ) {
            wsa_errno( result );
        }

        return result;
    }

    int receive( ssize_t size, ssize_t control_size, ssize_t name_size, DWORD flags ) {
        rx_msg.lpBuffers[0].len = size;
        rx_msg.namelen = name_size;
        rx_msg.Control.len = control_size;

        rx_msg.dwFlags = flags;
        int result = lpWSARecvMsg( m_fd, &rx_msg, NULL, NULL, NULL );

        if( result == SOCKET_ERROR ) {
            wsa_errno( result );
        }

        return result;
    }
};
#else /* POSIX */
class SendmsgTransport::priv_data {
public:
    priv_data( int fd ) :
        m_fd( fd ),
        m_ok( false ),
        rx_capacity( RECEIVE_BUFFER_SIZE ),
        rx_control_capacity( CONTROL_BUFFER_SIZE ),
        tx_control_data( nullptr ),
        tx_control_capacity( CONTROL_BUFFER_SIZE )
        {
        ::memset( &rx_msg, 0, sizeof( struct msghdr ) );
        ::memset( &tx_msg, 0, sizeof( struct msghdr ) );
        m_sendBuffer.reserve( SEND_BUFFER_SIZE );
    }

    ~priv_data() {
        free( rx_msg.msg_iov[0].iov_base );
        free( rx_msg.msg_control );
        free( tx_control_data );
    }

    int m_fd;
    bool m_ok;
    std::vector<uint8_t> m_sendBuffer;

    struct msghdr rx_msg;
    struct iovec rx_buf;
    int rx_capacity;
    int rx_control_capacity;

    struct msghdr tx_msg;
    struct iovec tx_buf;
    void* tx_control_data;
    int tx_control_capacity;

    std::mutex m_send_mutex;

    void init() {
        // Setup the RX data msghdr
        rx_msg.msg_iov = &rx_buf;
        rx_msg.msg_iov[0].iov_base = ::malloc( rx_capacity );
        rx_msg.msg_iov[0].iov_len = rx_capacity;
        rx_msg.msg_iovlen = 1;
        rx_msg.msg_control = ::malloc( rx_control_capacity );

        // Setup the TX data msghdr
        tx_msg.msg_iov = &tx_buf;
        tx_msg.msg_iovlen = 1;
        tx_control_data = ::malloc( tx_control_capacity );
    }

    uint8_t* rx_byte_buf_data() {
        return static_cast<uint8_t*>( rx_msg.msg_iov[0].iov_base );
    }

    ssize_t rx_size() {
        return rx_msg.msg_iov[0].iov_len;
    }

    ssize_t rx_control_size() {
        return rx_msg.msg_controllen;
    }

    void set_rx_capacity( ssize_t capacity ) {
        free( rx_msg.msg_iov[0].iov_base );
        rx_msg.msg_iov[0].iov_base = ::malloc( capacity );
        rx_capacity = capacity;
    }

    int peek( ssize_t size ) {
        return receive( size, 0, 0, MSG_PEEK );
    }

    int send() {
        tx_buf.iov_base = m_sendBuffer.data();
        tx_buf.iov_len = m_sendBuffer.size();

        return sendmsg( m_fd, &tx_msg, 0 );
    }

    int receive( ssize_t size, ssize_t control_size, ssize_t name_size, int flags ) {
        rx_msg.msg_iov[0].iov_len = size;
        rx_msg.msg_controllen = control_size;
        rx_msg.msg_namelen = name_size;

        return recvmsg( m_fd, &rx_msg, flags );
    }
};
#endif

SendmsgTransport::SendmsgTransport( int fd, bool initialize ) :
    m_priv( std::make_unique<priv_data>( fd ) ) {
    uint8_t nulbyte = 0;
    m_priv->m_ok = true;
    int my_errno = 0;

    if( initialize ) {
        if( ::write( m_priv->m_fd, &nulbyte, 1 ) < 0 ) {
            my_errno = errno;
            std::string errmsg = strerror( my_errno );
            SIMPLELOGGER_DEBUG( LOGGER_NAME, "Unable to write nul byte: " + errmsg );
            m_priv->m_ok = false;
        }
    }

    if( m_priv->m_ok ) {
        m_priv->init();
    }

    if( !m_priv->m_ok ) {
        close( m_priv->m_fd );
        errno = my_errno;
        return;
    }
}

SendmsgTransport::~SendmsgTransport() {
    close( m_priv->m_fd );
}

std::shared_ptr<SendmsgTransport> SendmsgTransport::create( int fd, bool initialize ) {
    return std::shared_ptr<SendmsgTransport>( new SendmsgTransport( fd, initialize ) );
}

ssize_t SendmsgTransport::writeMessage( std::shared_ptr<const DBus::Message> message, uint32_t serial )
{
    std::scoped_lock lock(m_priv->m_send_mutex);
#ifdef _WIN32
    const std::vector<int> filedescriptors = message->filedescriptors();

    if( filedescriptors.size() > 0 ) {
        SIMPLELOGGER_ERROR( LOGGER_NAME, "Sending/receiving file descriptors over sockets is not supported on Windows" );
        return -1;
    }

    std::ostringstream debug_str;
    ssize_t ret;

    m_priv->m_sendBuffer.clear();

    if( !message->serialize_to_vector( &m_priv->m_sendBuffer, serial ) ) {
        return 0;
    }

    debug_str << "Going to send the following bytes: " << std::endl;
    DBus::hexdump( &m_priv->m_sendBuffer, &debug_str );
    SIMPLELOGGER_TRACE( LOGGER_NAME, debug_str.str() );
#else /* POSIX */
    const std::vector<int> filedescriptors = message->filedescriptors();
    struct cmsghdr* cmsg;
    int fd_space_needed = CMSG_SPACE( sizeof( int ) * filedescriptors.size() );
    std::ostringstream debug_str;
    ssize_t ret;

    m_priv->m_sendBuffer.clear();

    if( !message->serialize_to_vector( &m_priv->m_sendBuffer, serial ) ) {
        return 0;
    }

    debug_str << "Going to send the following bytes: " << std::endl;
    DBus::hexdump( &m_priv->m_sendBuffer, &debug_str );
    SIMPLELOGGER_TRACE( LOGGER_NAME, debug_str.str() );

    m_priv->tx_msg.msg_control = nullptr;
    m_priv->tx_msg.msg_controllen = 0;

    if( m_priv->tx_control_capacity < fd_space_needed ) {
        free( m_priv->tx_control_data );
        m_priv->tx_control_data = ::malloc( fd_space_needed );
        m_priv->tx_control_capacity = fd_space_needed;
    }

    /* Fill in our FD array(ancillary data) */
    if( filedescriptors.size() > 0 ) {
        m_priv->tx_msg.msg_control = m_priv->tx_control_data;
        m_priv->tx_msg.msg_controllen = fd_space_needed;
        cmsg = CMSG_FIRSTHDR( &m_priv->tx_msg );
        cmsg->cmsg_level = SOL_SOCKET;
        cmsg->cmsg_type = SCM_RIGHTS;
        cmsg->cmsg_len = CMSG_LEN( sizeof( int ) * filedescriptors.size() );

        int* data = ( int* )CMSG_DATA( cmsg );

        for( int fd : filedescriptors ) {
            *data = fd;
            data++;
        }
    }

#endif /* WIN32 */

    /* Now we finally send the data! */
    ret = m_priv->send();

    if( ret < 0 ) {
        int my_errno = errno;
        debug_str.str( "" );
        debug_str.clear();

        debug_str << "Can't send message: " << strerror( my_errno );

        SIMPLELOGGER_ERROR( LOGGER_NAME, debug_str.str() );
        m_priv->m_ok = false;
    }

    return ret;
}

std::shared_ptr<DBus::Message> SendmsgTransport::readMessage() {
    uint32_t header_array_len;
    ssize_t body_len;
    ssize_t total_len;
    ssize_t ret;
    uint8_t* header_raw = m_priv->rx_byte_buf_data();
    std::vector<int> fds;
#ifndef _WIN32
    ssize_t num_fds;
    struct cmsghdr* cmsg;
#endif

    /* Do a peek of the data to determine if our arrays are large enough or not */
    ret = m_priv->peek( 16 );

    if( ret < 0 ) {
        return std::shared_ptr<DBus::Message>();
    }

    if( header_raw[0] == 'l' ) {
        /* Little-endian */
        body_len = header_raw[ 7 ] << 24 |
            header_raw[ 6 ] << 16 |
            header_raw[ 5 ] << 8 |
            header_raw[ 4 ] << 0;

        header_array_len = header_raw[ 15 ] << 24 |
            header_raw[ 14 ] << 16 |
            header_raw[ 13 ] << 8 |
            header_raw[ 12 ] << 0;
    } else if( header_raw[0] == 'B' ) {
        /* Big-endian */
        body_len = header_raw[ 4 ] << 24 |
            header_raw[ 5 ] << 16 |
            header_raw[ 6 ] << 8 |
            header_raw[ 7 ] << 0;

        header_array_len = header_raw[ 12 ] << 24 |
            header_raw[ 13 ] << 16 |
            header_raw[ 14 ] << 8 |
            header_raw[ 15 ] << 0;
    } else {
        m_priv->m_ok = false;
        return std::shared_ptr<DBus::Message>();
    }

    if( (body_len + header_array_len + 12 + 4) >
            DBus::Validator::maximum_message_size() ){
        // Invalid message: it can't be that big!
        // purge our reading buffer and reset to a known state.
        purgeData();
        return std::shared_ptr<DBus::Message>();
    }

    if( 0 != header_array_len % 8 ) {
        header_array_len += 8 - ( header_array_len % 8 );
    }

    total_len = 12 + ( 4 + header_array_len ) + body_len;

    /* Check to see if our receive buffer capacity is big enough - expand if not */
    if( m_priv->rx_capacity < total_len ) {
        m_priv->set_rx_capacity( total_len );
        header_raw = m_priv->rx_byte_buf_data();
    }

    /* Do the real reading of the data */
    ret = m_priv->receive( total_len, m_priv->rx_control_capacity, 0, 0 );

    if( ret <= 0 ) {
        m_priv->m_ok = false;
        return std::shared_ptr<DBus::Message>();
    }

    SIMPLELOGGER_DEBUG( LOGGER_NAME, "Have " << m_priv->rx_control_size() << " bytes of control after real reading" );

#ifndef _WIN32

    /* Need to figure out how many FDs we have to create our array */
    for( cmsg = CMSG_FIRSTHDR( &m_priv->rx_msg );
        cmsg != nullptr;
        cmsg = CMSG_NXTHDR( &m_priv->rx_msg, cmsg ) ) {
        if( cmsg->cmsg_level == SOL_SOCKET &&
            cmsg->cmsg_type == SCM_RIGHTS ) {
            /* This is our FD array */
            num_fds = CMSG_LEN( cmsg->cmsg_len ) / sizeof( int );
            SIMPLELOGGER_DEBUG( LOGGER_NAME, "Have " << num_fds << " fds to extract from CMSGHDR" );
            int* fd_array = reinterpret_cast<int*>( CMSG_DATA( cmsg ) );

            for( ssize_t current = 0; current < num_fds; current++ ) {
                fds.push_back( *fd_array );
                fd_array++;
            }
        }
    }

#endif

    std::shared_ptr<DBus::Message> retmsg =
        DBus::Message::create_from_data( header_raw, m_priv->rx_size(), fds );

    return retmsg;
}

bool SendmsgTransport::is_valid() const {
    return m_priv->m_ok;
}

int SendmsgTransport::fd() const {
    return m_priv->m_fd;
}

void SendmsgTransport::purgeData(){
//    If a message is
//    too long to fit in the supplied buffers, and MSG_PEEK is not set in
//    the flags argument, the excess bytes shall be discarded....
    // https://man7.org/linux/man-pages/man3/recvmsg.3p.html
    m_priv->receive( 0, m_priv->rx_control_capacity, 0, 0 );
}
