//
// Created by alex2772 on 9/8/22.
//

#include "AOptional.h"
#include "AException.h"


// avoid of cycling includes is the reason why the throw implementation is dedicated to cpp file
void aui::impl::optional::throwException(const char* message) {
    throw AException(message);
}
