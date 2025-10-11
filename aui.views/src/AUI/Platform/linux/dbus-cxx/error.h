// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2007,2008,2009,2010 by Rick L. Vinyard, Jr.             *
 *   rvinyard@cs.nmsu.edu                                                  *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#include <exception>
#include <string>
#include <AUI/Platform/linux/dbus-cxx/dbus-error.h>

#ifndef DBUSCXX_ERROR_H
#define DBUSCXX_ERROR_H

namespace DBus {

class Message;

/**
 * @defgroup errors Errors
 *
 * A hierarchy of error objects descended from std::exception thrown when
 * exceptional conditions arise.
 */

/**
 * Base class of a hierarchy of error objects thrown when exceptional conditions occur
 *
 * @ingroup core
 *
 * @ingroup errors
 *
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
class Error : public std::exception {
public:
    Error();

    Error( const char* name, const char* message = nullptr );

    Error( const char* name, std::string message );

    Error( std::string name, std::string message );

    ~Error() noexcept;

    const char* what() const noexcept;

    std::string name() const;

    std::string message() const;

protected:
    std::string m_name;
    std::string m_message;

};

#define DBUSCXX_ERROR( CPPTYPE, DBUS_ERROR_CODE )            \
    class CPPTYPE : public Error {                             \
    public:                                                  \
        CPPTYPE( const char* message = nullptr )                 \
            : Error( DBUS_ERROR_CODE, message ) {}               \
        CPPTYPE( std::string message )                           \
            : Error( DBUS_ERROR_CODE, message ) {}               \
    }

/**
 * @class ErrorFailed
 * @ingroup errors
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
DBUSCXX_ERROR( ErrorFailed, DBUSCXX_ERROR_FAILED );

/**
 * @class ErrorNoMemory
 * @ingroup errors
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
DBUSCXX_ERROR( ErrorNoMemory, DBUSCXX_ERROR_NO_MEMORY );

/**
 * @class ErrorServiceUnknown
 * @ingroup errors
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
DBUSCXX_ERROR( ErrorServiceUnknown, DBUSCXX_ERROR_SERVICE_UNKNOWN );

/**
 * @class ErrorNameHasNoOwner
 * @ingroup errors
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
DBUSCXX_ERROR( ErrorNameHasNoOwner, DBUSCXX_ERROR_NAME_HAS_NO_OWNER );

/**
 * @class ErrorNoReply
 * @ingroup errors
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
DBUSCXX_ERROR( ErrorNoReply, DBUSCXX_ERROR_NO_REPLY );

/**
 * @class ErrorIOError
 * @ingroup errors
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
DBUSCXX_ERROR( ErrorIOError, DBUSCXX_ERROR_IO_ERROR );

/**
 * @class ErrorBadAddress
 * @ingroup errors
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
DBUSCXX_ERROR( ErrorBadAddress, DBUSCXX_ERROR_BAD_ADDRESS );

/**
 * @class ErrorNotSupported
 * @ingroup errors
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
DBUSCXX_ERROR( ErrorNotSupported, DBUSCXX_ERROR_NOT_SUPPORTED );

/**
 * @class ErrorLimitsExceeded
 * @ingroup errors
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
DBUSCXX_ERROR( ErrorLimitsExceeded, DBUSCXX_ERROR_LIMITS_EXCEEDED );

/**
 * @class ErrorAccessDenied
 * @ingroup errors
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
DBUSCXX_ERROR( ErrorAccessDenied, DBUSCXX_ERROR_ACCESS_DENIED );

/**
 * @class ErrorAuthFailed
 * @ingroup errors
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
DBUSCXX_ERROR( ErrorAuthFailed, DBUSCXX_ERROR_AUTH_FAILED );

/**
 * @class ErrorInteractiveAuthorizationRequired
 * @ingroup errors
 *
 * This error is caused attempting to call a D-Bus method which expects
 * `ALLOW_INTERACTIVE_AUTHORIZATION` to be set when it isn't. See
 * <https://dbus.freedesktop.org/doc/dbus-specification.html#message-protocol-messages>.
 *
 * This flag can be enabled with @ref
 * DBus::MethodProxyBase::enable_interactive_authorization().
 */
DBUSCXX_ERROR( ErrorInteractiveAuthorizationRequired, DBUSCXX_ERROR_INTERACTIVE_AUTHORIZATION_REQUIRED );

/**
 * @class ErrorNoServer
 * @ingroup errors
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
DBUSCXX_ERROR( ErrorNoServer, DBUSCXX_ERROR_NO_SERVER );

/**
 * @class ErrorTimeout
 * @ingroup errors
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
DBUSCXX_ERROR( ErrorTimeout, DBUSCXX_ERROR_TIMEOUT );

/**
 * @class ErrorNoNetwork
 * @ingroup errors
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
DBUSCXX_ERROR( ErrorNoNetwork, DBUSCXX_ERROR_NO_NETWORK );

/**
 * @class ErrorAddressInUse
 * @ingroup errors
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
DBUSCXX_ERROR( ErrorAddressInUse, DBUSCXX_ERROR_ADDRESS_IN_USE );

/**
 * @class ErrorDisconnected
 * @ingroup errors
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
DBUSCXX_ERROR( ErrorDisconnected, DBUSCXX_ERROR_DISCONNECTED );

/**
 * @class ErrorInvalidArgs
 * @ingroup errors
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
DBUSCXX_ERROR( ErrorInvalidArgs, DBUSCXX_ERROR_INVALID_ARGS );

/**
 * @class ErrorFileNotFound
 * @ingroup errors
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
DBUSCXX_ERROR( ErrorFileNotFound, DBUSCXX_ERROR_FILE_NOT_FOUND );

/**
 * @class ErrorFileExists
 * @ingroup errors
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
DBUSCXX_ERROR( ErrorFileExists, DBUSCXX_ERROR_FILE_EXISTS );

/**
 * @class ErrorUnknownMethod
 * @ingroup errors
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
DBUSCXX_ERROR( ErrorUnknownMethod, DBUSCXX_ERROR_UNKNOWN_METHOD );

/**
 * @class ErrorUnknownObject
 * @ingroup errors
 */
DBUSCXX_ERROR( ErrorUnknownObject, DBUSCXX_ERROR_UNKNOWN_OBJECT );

/**
 * @class ErrorUnknownInterface
 * @ingroup errors
 */
DBUSCXX_ERROR( ErrorUnknownInterface, DBUSCXX_ERROR_UNKNOWN_INTERFACE );

/**
 * @class ErrorUnknownProperty
 * @ingroup errors
 */
DBUSCXX_ERROR( ErrorUnknownProperty, DBUSCXX_ERROR_UNKNOWN_PROPERTY );

/**
 * @class ErrorPropertyReadOnly
 * @ingroup errors
 */
DBUSCXX_ERROR( ErrorPropertyReadOnly, DBUSCXX_ERROR_PROPERTY_READ_ONLY );

/**
 * @class ErrorTimedOut
 * @ingroup errors
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
DBUSCXX_ERROR( ErrorTimedOut, DBUSCXX_ERROR_TIMED_OUT );

/**
 * @class ErrorMatchRuleNotFound
 * @ingroup errors
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
DBUSCXX_ERROR( ErrorMatchRuleNotFound, DBUSCXX_ERROR_MATCH_RULE_NOT_FOUND );

/**
 * @class ErrorMatchRuleInvalid
 * @ingroup errors
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
DBUSCXX_ERROR( ErrorMatchRuleInvalid, DBUSCXX_ERROR_MATCH_RULE_INVALID );

/**
 * @class ErrorSpawnExecFailed
 * @ingroup errors
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
DBUSCXX_ERROR( ErrorSpawnExecFailed, DBUSCXX_ERROR_SPAWN_EXEC_FAILED );

/**
 * @class ErrorSpawnForkFailed
 * @ingroup errors
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
DBUSCXX_ERROR( ErrorSpawnForkFailed, DBUSCXX_ERROR_SPAWN_FORK_FAILED );

/**
 * @class ErrorSpawnChildExited
 * @ingroup errors
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
DBUSCXX_ERROR( ErrorSpawnChildExited, DBUSCXX_ERROR_SPAWN_CHILD_EXITED );

/**
 * @class ErrorSpawnChildSignaled
 * @ingroup errors
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
DBUSCXX_ERROR( ErrorSpawnChildSignaled, DBUSCXX_ERROR_SPAWN_CHILD_SIGNALED );

/**
 * @class ErrorSpawnFailed
 * @ingroup errors
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
DBUSCXX_ERROR( ErrorSpawnFailed, DBUSCXX_ERROR_SPAWN_FAILED );

/**
 * @class ErrorUnixProcessIdUnknown
 * @ingroup errors
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
DBUSCXX_ERROR( ErrorUnixProcessIdUnknown, DBUSCXX_ERROR_UNIX_PROCESS_ID_UNKNOWN );

/**
 * @class ErrorInvalidSignature
 * @ingroup errors
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
DBUSCXX_ERROR( ErrorInvalidSignature, DBUSCXX_ERROR_INVALID_SIGNATURE );

/**
 * @class ErrorInvalidFileContent
 * @ingroup errors
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
DBUSCXX_ERROR( ErrorInvalidFileContent, DBUSCXX_ERROR_INVALID_FILE_CONTENT );

/**
 * @class ErrorSELinuxSecurityContextUnknown
 * @ingroup errors
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
DBUSCXX_ERROR( ErrorSELinuxSecurityContextUnknown, DBUSCXX_ERROR_SELINUX_SECURITY_CONTEXT_UNKNOWN );

/**
 * @class ErrorInvalidCObject
 * @ingroup errors
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
DBUSCXX_ERROR( ErrorInvalidCObject, "Invalid C Object" );

/**
 * @class ErrorInvalidTypecast
 * @ingroup errors
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
DBUSCXX_ERROR( ErrorInvalidTypecast, "Invalid Typecast" );

/**
 * @class ErrorInvalidConnection
 * @ingroup errors
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
DBUSCXX_ERROR( ErrorInvalidConnection, "DBus connection is invalid" );

/**
 * @class ErrorInvalidMessageType
 * @ingroup errors
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
DBUSCXX_ERROR( ErrorInvalidMessageType, "DBus message type does not match the C++ Message object type" );

/**
 * @class ErrorNoConnection
 * @ingroup errors
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
DBUSCXX_ERROR( ErrorNoConnection, "No connection specified and operation requires a DBus connection" );

/**
 * @class ErrorDeadlock
 * @ingroup errors
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
DBUSCXX_ERROR( ErrorDeadlock, "Locking a mutex a dbus-cxx thread already owns" );

/**
 * @class ErrorNotOwner
 * @ingroup errors
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
DBUSCXX_ERROR( ErrorNotOwner, "Unlocking a mutex a dbus-cxx thread doesn't own" );

/**
 * @class ErrorNotInitialized
 * @ingroup errors
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
DBUSCXX_ERROR( ErrorNotInitialized, "DBus::init(bool threadsafe=true) not called" );

/**
 * @class ErrorInvalidAdaptee
 * @ingroup errors
 * @author Rick L Vinyard Jr <rvinyard@cs.nmsu.edu>
 */
DBUSCXX_ERROR( ErrorInvalidAdaptee, "DBus adapter method called, but adaptee is invalid" );

/**
 * @class ErrorDispatcherInitFailed
 * @ingroup errors
 */
DBUSCXX_ERROR( ErrorDispatcherInitFailed, "DBus::Dispatcher initialization failed" );

/**
 * @class ErrorInvalidSharedPtr
 * @ingroup errors
 */
DBUSCXX_ERROR( ErrorInvalidSharedPtr, "shared_ptr was invalid" );

/**
 * @class ErrorPollFailed
 * @ingroup errors
 */
DBUSCXX_ERROR( ErrorPollFailed, "poll() syscall failed" );

/**
 * This error may be thrown if there is an invalid return value from a DBus function
 * that should never have an invalid return value.
 */
DBUSCXX_ERROR( ErrorInvalidReturn, "dbuscxx.Error.InvalidReturnValue" );

/**
 * This error may be thrown if we receive an unexpected response
 */
DBUSCXX_ERROR( ErrorUnexpectedResponse, "dbuscxx.Error.UnexpectedResponse" );

class ErrorIncorrectDispatchThread : public Error {
public:
    ErrorIncorrectDispatchThread( const char* message = nullptr )
        : Error( nullptr, message ) {}
};

class ErrorRemoteException : public Error {
public:
    ErrorRemoteException( const char* message = nullptr )
        : Error( nullptr, message ) {}
};

class ErrorUnknown : public Error {
public:
    ErrorUnknown( const char* message = nullptr )
        : Error( nullptr, message ) {}
};

class ErrorBadVariantCast : public Error {
public:
    ErrorBadVariantCast( const char* message = nullptr )
        : Error( nullptr, message ) {}
};

/**
 * We were unable to parse(something is unimplemented?)
 */
class ErrorUnableToParse : public Error {
public:
    ErrorUnableToParse( const char* message = nullptr )
        : Error( nullptr, message ) {}
};

/**
 * An invalid message was tried to be parsed.
 */
class ErrorInvalidMessage : public Error {
public:
    ErrorInvalidMessage( const char* message = nullptr )
        : Error( nullptr, message ) {}
};

class ErrorMessageNestedTooDeep : public Error {
public:
    ErrorMessageNestedTooDeep( const char* message = nullptr )
        : Error( nullptr, message ) {}
};

}

#endif
