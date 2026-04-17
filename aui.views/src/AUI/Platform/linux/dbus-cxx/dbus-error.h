// SPDX-License-Identifier: LGPL-3.0-or-later OR BSD-3-Clause
/***************************************************************************
 *   Copyright (C) 2020 by Robert Middleton                                *
 *   robert.middleton@rm5248.com                                           *
 *                                                                         *
 *   This file is part of the dbus-cxx library.                            *
 ***************************************************************************/
#ifndef DBUSCXX_DBUS_ERROR_H
#define DBUSCXX_DBUS_ERROR_H

/*****************************
 * This file contains copy/pasted macros from the DBus reference implementation,
 * but renamed to be DBUSCXX_ERROR to avoid any potential #define problems.
 */

/** A generic error; "something went wrong" - see the error message for more. */
#define DBUSCXX_ERROR_FAILED                     "org.freedesktop.DBus.Error.Failed"
/** There was not enough memory to complete an operation. */
#define DBUSCXX_ERROR_NO_MEMORY                  "org.freedesktop.DBus.Error.NoMemory"
/** The bus doesn't know how to launch a service to supply the bus name you wanted. */
#define DBUSCXX_ERROR_SERVICE_UNKNOWN            "org.freedesktop.DBus.Error.ServiceUnknown"
/** The bus name you referenced doesn't exist (i.e. no application owns it). */
#define DBUSCXX_ERROR_NAME_HAS_NO_OWNER          "org.freedesktop.DBus.Error.NameHasNoOwner"
/** No reply to a message expecting one, usually means a timeout occurred. */
#define DBUSCXX_ERROR_NO_REPLY                   "org.freedesktop.DBus.Error.NoReply"
/** Something went wrong reading or writing to a socket, for example. */
#define DBUSCXX_ERROR_IO_ERROR                   "org.freedesktop.DBus.Error.IOError"
/** A D-Bus bus address was malformed. */
#define DBUSCXX_ERROR_BAD_ADDRESS                "org.freedesktop.DBus.Error.BadAddress"
/** Requested operation isn't supported (like ENOSYS on UNIX). */
#define DBUSCXX_ERROR_NOT_SUPPORTED              "org.freedesktop.DBus.Error.NotSupported"
/** Some limited resource is exhausted. */
#define DBUSCXX_ERROR_LIMITS_EXCEEDED            "org.freedesktop.DBus.Error.LimitsExceeded"
/** Security restrictions don't allow doing what you're trying to do. */
#define DBUSCXX_ERROR_ACCESS_DENIED              "org.freedesktop.DBus.Error.AccessDenied"
/** Authentication didn't work. */
#define DBUSCXX_ERROR_AUTH_FAILED                "org.freedesktop.DBus.Error.AuthFailed"
/** Unable to connect to server (probably caused by ECONNREFUSED on a socket). */
#define DBUSCXX_ERROR_NO_SERVER                  "org.freedesktop.DBus.Error.NoServer"
/** Certain timeout errors, possibly ETIMEDOUT on a socket.
 * Note that #DBUSCXX_ERROR_NO_REPLY is used for message reply timeouts.
 * @warning this is confusingly-named given that #DBUSCXX_ERROR_TIMED_OUT also exists. We can't fix
 * it for compatibility reasons so just be careful.
 */
#define DBUSCXX_ERROR_TIMEOUT                    "org.freedesktop.DBus.Error.Timeout"
/** No network access (probably ENETUNREACH on a socket). */
#define DBUSCXX_ERROR_NO_NETWORK                 "org.freedesktop.DBus.Error.NoNetwork"
/** Can't bind a socket since its address is in use (i.e. EADDRINUSE). */
#define DBUSCXX_ERROR_ADDRESS_IN_USE             "org.freedesktop.DBus.Error.AddressInUse"
/** The connection is disconnected and you're trying to use it. */
#define DBUSCXX_ERROR_DISCONNECTED               "org.freedesktop.DBus.Error.Disconnected"
/** Invalid arguments passed to a method call. */
#define DBUSCXX_ERROR_INVALID_ARGS               "org.freedesktop.DBus.Error.InvalidArgs"
/** Missing file. */
#define DBUSCXX_ERROR_FILE_NOT_FOUND             "org.freedesktop.DBus.Error.FileNotFound"
/** Existing file and the operation you're using does not silently overwrite. */
#define DBUSCXX_ERROR_FILE_EXISTS                "org.freedesktop.DBus.Error.FileExists"
/** Method name you invoked isn't known by the object you invoked it on. */
#define DBUSCXX_ERROR_UNKNOWN_METHOD             "org.freedesktop.DBus.Error.UnknownMethod"
/** Object you invoked a method on isn't known. */
#define DBUSCXX_ERROR_UNKNOWN_OBJECT             "org.freedesktop.DBus.Error.UnknownObject"
/** Interface you invoked a method on isn't known by the object. */
#define DBUSCXX_ERROR_UNKNOWN_INTERFACE          "org.freedesktop.DBus.Error.UnknownInterface"
/** Property you tried to access isn't known by the object. */
#define DBUSCXX_ERROR_UNKNOWN_PROPERTY           "org.freedesktop.DBus.Error.UnknownProperty"
/** Property you tried to set is read-only. */
#define DBUSCXX_ERROR_PROPERTY_READ_ONLY         "org.freedesktop.DBus.Error.PropertyReadOnly"
/** Certain timeout errors, e.g. while starting a service.
 * @warning this is confusingly-named given that #DBUSCXX_ERROR_TIMEOUT also exists. We can't fix
 * it for compatibility reasons so just be careful.
 */
#define DBUSCXX_ERROR_TIMED_OUT                  "org.freedesktop.DBus.Error.TimedOut"
/** Tried to remove or modify a match rule that didn't exist. */
#define DBUSCXX_ERROR_MATCH_RULE_NOT_FOUND       "org.freedesktop.DBus.Error.MatchRuleNotFound"
/** The match rule isn't syntactically valid. */
#define DBUSCXX_ERROR_MATCH_RULE_INVALID         "org.freedesktop.DBus.Error.MatchRuleInvalid"
/** While starting a new process, the exec() call failed. */
#define DBUSCXX_ERROR_SPAWN_EXEC_FAILED          "org.freedesktop.DBus.Error.Spawn.ExecFailed"
/** While starting a new process, the fork() call failed. */
#define DBUSCXX_ERROR_SPAWN_FORK_FAILED          "org.freedesktop.DBus.Error.Spawn.ForkFailed"
/** While starting a new process, the child exited with a status code. */
#define DBUSCXX_ERROR_SPAWN_CHILD_EXITED         "org.freedesktop.DBus.Error.Spawn.ChildExited"
/** While starting a new process, the child exited on a signal. */
#define DBUSCXX_ERROR_SPAWN_CHILD_SIGNALED       "org.freedesktop.DBus.Error.Spawn.ChildSignaled"
/** While starting a new process, something went wrong. */
#define DBUSCXX_ERROR_SPAWN_FAILED               "org.freedesktop.DBus.Error.Spawn.Failed"
/** We failed to setup the environment correctly. */
#define DBUSCXX_ERROR_SPAWN_SETUP_FAILED         "org.freedesktop.DBus.Error.Spawn.FailedToSetup"
/** We failed to setup the config parser correctly. */
#define DBUSCXX_ERROR_SPAWN_CONFIG_INVALID       "org.freedesktop.DBus.Error.Spawn.ConfigInvalid"
/** Bus name was not valid. */
#define DBUSCXX_ERROR_SPAWN_SERVICE_INVALID      "org.freedesktop.DBus.Error.Spawn.ServiceNotValid"
/** Service file not found in system-services directory. */
#define DBUSCXX_ERROR_SPAWN_SERVICE_NOT_FOUND    "org.freedesktop.DBus.Error.Spawn.ServiceNotFound"
/** Permissions are incorrect on the setuid helper. */
#define DBUSCXX_ERROR_SPAWN_PERMISSIONS_INVALID  "org.freedesktop.DBus.Error.Spawn.PermissionsInvalid"
/** Service file invalid (Name, User or Exec missing). */
#define DBUSCXX_ERROR_SPAWN_FILE_INVALID         "org.freedesktop.DBus.Error.Spawn.FileInvalid"
/** Tried to get a UNIX process ID and it wasn't available. */
#define DBUSCXX_ERROR_SPAWN_NO_MEMORY            "org.freedesktop.DBus.Error.Spawn.NoMemory"
/** Tried to get a UNIX process ID and it wasn't available. */
#define DBUSCXX_ERROR_UNIX_PROCESS_ID_UNKNOWN    "org.freedesktop.DBus.Error.UnixProcessIdUnknown"
/** A type signature is not valid. */
#define DBUSCXX_ERROR_INVALID_SIGNATURE          "org.freedesktop.DBus.Error.InvalidSignature"
/** A file contains invalid syntax or is otherwise broken. */
#define DBUSCXX_ERROR_INVALID_FILE_CONTENT       "org.freedesktop.DBus.Error.InvalidFileContent"
/** Asked for SELinux security context and it wasn't available. */
#define DBUSCXX_ERROR_SELINUX_SECURITY_CONTEXT_UNKNOWN    "org.freedesktop.DBus.Error.SELinuxSecurityContextUnknown"
/** Asked for ADT audit data and it wasn't available. */
#define DBUSCXX_ERROR_ADT_AUDIT_DATA_UNKNOWN     "org.freedesktop.DBus.Error.AdtAuditDataUnknown"
/** There's already an object with the requested object path. */
#define DBUSCXX_ERROR_OBJECT_PATH_IN_USE         "org.freedesktop.DBus.Error.ObjectPathInUse"
/** The message meta data does not match the payload. e.g. expected
    number of file descriptors were not sent over the socket this message was received on. */
#define DBUSCXX_ERROR_INCONSISTENT_MESSAGE       "org.freedesktop.DBus.Error.InconsistentMessage"
/** The message is not allowed without performing interactive authorization,
 * but could have succeeded if an interactive authorization step was
 * allowed. */
#define DBUSCXX_ERROR_INTERACTIVE_AUTHORIZATION_REQUIRED "org.freedesktop.DBus.Error.InteractiveAuthorizationRequired"

#endif /* DBUSCXX_DBUS_ERROR_H */
