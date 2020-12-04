#include <AUI/Platform/Entry.h>
#include <Command/Pack.h>
#include <Command/Lang.h>

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
	std::cout << "Alex2772 Universal Interface toolbox" << std::endl;
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
        }
	}

	return 0;
}
