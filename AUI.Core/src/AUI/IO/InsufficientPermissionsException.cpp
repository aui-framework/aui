//
// Created by alex2 on 31.10.2020.
//

#include "InsufficientPermissionsException.h"

InsufficientPermissionsException::InsufficientPermissionsException() {}

InsufficientPermissionsException::InsufficientPermissionsException(const AString& message) : IOException(message) {

}
