/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

#include <AUI/Platform/Entry.h>
#include <Command/Pack.h>
#include <Command/Lang.h>
#include <Command/Css2ass.h>
#include <Command/PackManual.h>

#include "Toolbox.h"

Toolbox toolbox;

template<typename T>
void registerCommand() {
    ICommand* t = new T;
    toolbox.commands[t->getName()] = t;
}
Toolbox::Toolbox() {
    registerCommand<Pack>();
    registerCommand<Lang>();
    registerCommand<Css2ass>();
    registerCommand<PackManual>();
}
Toolbox::~Toolbox() {
    for (auto& c : commands) {
        delete c.second;
    }
}

void printAllCommands() {
    std::cout << "Available commands:" << std::endl;
    for (auto& c : toolbox.commands) {
        c.second->printHelp();
    }
}

AUI_ENTRY
{
	std::cout << "Advanced Universal Interface toolbox" << std::endl;
	if (args.size() <= 1)
	{
		printAllCommands();
	}
	else
	{
	    /*
	    std::cout << "Command line:" << std::endl;
	    for (int i = 0; i < args.size(); ++i) {
            std::cout << args[i] << " ";
        }
	    std::cout << std::endl;*/
		for (int i = 2; i < args.size(); ++i)
		{
			toolbox.args << args[i];
		}

		ICommand* command;
		try {
            if (auto c = toolbox.commands.contains(args[1])) {
                command = c->second;
                command->run(toolbox);
            } else {
                printAllCommands();
                return -1;
            }
        } catch (const IllegalArgumentsException& e) {
		    std::cerr << std::endl << e.getMessage() << std::endl;
		    command->printHelp();
		    return -1;
        } catch (const AException& e) {
		    std::cerr << std::endl << e.getMessage() << std::endl;
		    return -1;
        } catch (...) {
		    return -1;
        }
	}

	return 0;
}
