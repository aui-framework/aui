// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2007,2010 by Rick L. Vinyard, Jr.                       *
 *   rvinyard@cs.nmsu.edu                                                  *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#include <cstdint>
#include <ostream>

#ifndef DBUSCXX_ENUMS_H
#define DBUSCXX_ENUMS_H

namespace DBus {

enum class BusType {
    NONE = -1,
    SESSION,     /**< The login session bus */
    SYSTEM,      /**< The systemwide bus */
    STARTER     /**< The bus that started us, if any */
};

enum class PropertyUpdateType {
    /**
     * When this property changes, the PropertyChanged signal will be emitted
     * with the new value.
     */
    Updates,
    /**
     * When this property cahnges, the PropertyChanged signal will be emitted
     * but the new value will not be set in the signal.
     */
    Invalidates,
    /**
     * This property does not change during the lifetime.
     */
    Const,
    /**
     * The property does not emit the PropertyChanged signal whenever it changes.
     * This implies that when you query the property on a proxy, it will always
     * query the value over the bus.
     */
    DoesNotUpdate
};

enum class PropertyAccess {
    ReadOnly,
    WriteOnly,
    ReadWrite
};

enum class DataType {
    INVALID = '\0',
    BYTE = 'y',
    BOOLEAN = 'b',
    INT16 = 'n',
    UINT16 = 'q',
    INT32 = 'i',
    UINT32 = 'u',
    INT64 = 'x',
    UINT64 = 't',
    DOUBLE = 'd',
    STRING = 's',
    OBJECT_PATH = 'o',
    SIGNATURE = 'g',
    ARRAY = 'a',
    VARIANT = 'v',
    STRUCT = 'r',
    DICT_ENTRY = 'e',
    UNIX_FD = 'h',
};

enum class ContainerType {
    None,
    ARRAY = 'a',
    VARIANT = 'v',
    STRUCT = 'r',
    DICT_ENTRY = 'e',
};

enum class MessageType : int32_t {
    INVALID = 0,
    CALL    = 1,
    RETURN  = 2,
    ERROR   = 3,
    SIGNAL  = 4,
};

enum class DispatchStatus {
    DATA_REMAINS,
    COMPLETE,
    NEED_MEMORY,
};

enum class HandlerResult {
    /** This message was handled appropriately */
    Handled,
    /** This message had an invalid interface */
    Invalid_Interface,
    /** This message had an invalid method to call */
    Invalid_Method,
    /** This message had an invalid path */
    Invalid_Path,
    /** This message was not handled for some reason */
    Not_Handled,
};

enum class StartReply {
    FAILED = 0,
    SUCCESS,
    ALREADY_RUNNING,
};

enum class Endianess {
    Little,
    Big,
};

enum class RegistrationStatus {
    Success,
    /** Unable to register object: There is already an object exported on this path */
    Failed_Path_in_Use,
    /** Unable to register object: invalid object */
    Failed_Invalid_Object,
    /** Unable to register object: No thread dispatcher for the given thread found */
    Failed_No_Thread_Dispatcher,
};

/**
 * Gives hints to the connection as to which thread should be
 * the one to call the methods on the given object.
 */
enum class ThreadForCalling {
    /** Always call methods for this object from the dispatcher thread */
    DispatcherThread,
    /** Always call methods for this object from the current thread */
    CurrentThread,
};

enum class MessageHeaderFields {
    Invalid       = 0,
    Path          = 1,
    Interface     = 2,
    Member        = 3,
    Error_Name    = 4,
    Reply_Serial  = 5,
    Destination   = 6,
    Sender        = 7,
    Signature     = 8,
    Unix_FDs      = 9,
};

/**
 * Response status to attempting to register a name on the bus.
 */
enum class RequestNameResponse {
    /** The caller is now the primary owner */
    PrimaryOwner,
    /** The caller is in the queue */
    NameInQueue,
    /** The name exists and we cannot get it */
    NameExists,
    /** We are already the owner of the specified name */
    AlreadyOwner
};

enum class ReleaseNameResponse {
    /** The name has been released */
    NameReleased,
    /** The given name does not exist on the bus */
    NameNonExistant,
    /** We are not the owner, and we also were not in the queue waiting to own the name */
    NotOwner
};

inline uint8_t header_field_to_int( MessageHeaderFields header ) {
    switch( header ) {
    case MessageHeaderFields::Path:
        return 1;

    case MessageHeaderFields::Interface:
        return 2;

    case MessageHeaderFields::Member:
        return 3;

    case MessageHeaderFields::Error_Name:
        return 4;

    case MessageHeaderFields::Reply_Serial:
        return 5;

    case MessageHeaderFields::Destination:
        return 6;

    case MessageHeaderFields::Sender:
        return 7;

    case MessageHeaderFields::Signature:
        return 8;

    case MessageHeaderFields::Unix_FDs:
        return 9;

    default:
        return 0;
    }
}

inline MessageHeaderFields int_to_header_field( uint8_t val ) {
    switch( val ) {
    case 1:
        return MessageHeaderFields::Path;

    case 2:
        return MessageHeaderFields::Interface;

    case 3:
        return MessageHeaderFields::Member;

    case 4:
        return MessageHeaderFields::Error_Name;

    case 5:
        return MessageHeaderFields::Reply_Serial;

    case 6:
        return MessageHeaderFields::Destination;

    case 7:
        return MessageHeaderFields::Sender;

    case 8:
        return MessageHeaderFields::Signature;

    case 9:
        return MessageHeaderFields::Unix_FDs;
    }

    return MessageHeaderFields::Invalid;
}

inline std::ostream& operator<<( std::ostream& os, Endianess endian ) {
    if( endian == Endianess::Little ) {
        os << "Little";
    } else {
        os << "Big";
    }

    return os;
}

inline std::ostream& operator<<( std::ostream& os, MessageType type ) {
    os << "MessageType::";

    switch( type ) {
    case MessageType::INVALID:
        os << "INVALID";
        break;

    case MessageType::CALL:
        os << "CALL";
        break;

    case MessageType::RETURN:
        os << "RETURN";
        break;

    case MessageType::ERROR:
        os << "ERROR";
        break;

    case MessageType::SIGNAL:
        os << "SIGNAL";
        break;
    }

    return os;
}

inline std::ostream& operator<<( std::ostream& os, DispatchStatus status ) {
    os << "DispatchStatus::";

    switch( status ) {
    case DispatchStatus::DATA_REMAINS:
        os << "DATA_REMAINS";
        break;

    case DispatchStatus::COMPLETE:
        os << "COMPLETE";
        break;

    case DispatchStatus::NEED_MEMORY:
        os << "NEED_MEMORY";
        break;
    }

    return os;
}

inline std::ostream& operator<<( std::ostream& os, HandlerResult res ) {
    os << "HandlerResult::";

    switch( res ) {
    case HandlerResult::Handled:
        os << "Handled";
        break;

    case HandlerResult::Not_Handled:
        os << "Not_Handled";
        break;

    case HandlerResult::Invalid_Path:
        os << "Invalid_Path";
        break;

    case HandlerResult::Invalid_Method:
        os << "Invalid_Method";
        break;

    case HandlerResult::Invalid_Interface:
        os << "Invalid_Interface";
    }

    return os;
}

}

#endif

