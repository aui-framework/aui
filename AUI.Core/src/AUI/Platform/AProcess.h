//
// Created by alex2 on 31.10.2020.
//

#pragma once


#include <AUI/IO/APath.h>

class API_AUI_CORE AProcess {
public:
    static void execute(const AString& command, const AString& args = {}, const APath& workingDirectory = {});
};


