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
// Created by alex2 on 5/31/2021.
//

#include "PackManual.h"
#include "Pack.h"
#include <AUI/Crypt/AHash.h>
#include <AUI/IO/APath.h>
#include <AUI/IO/AFileInputStream.h>
#include <AUI/IO/AFileOutputStream.h>
#include <AUI/Util/ATokenizer.h>
#include <AUI/Util/LZ.h>

void PackManual::run(Toolbox& t) {
    if (t.args.size() != 3)
    {
        throw IllegalArgumentsException("invalid argument count");
    }
    else
    {
        Pack::doPacking(t.args[0], t.args[1], t.args[2]);
    }
}

AString PackManual::getName() {
    return "pack_manual";
}

AString PackManual::getSignature() {
    return "<file to pack> <asset path for your application> <resulting cpp>";
}

AString PackManual::getDescription() {
    return "pack a file into the .cpp file with manually specifying asset path for your application";
}
