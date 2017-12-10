// This is the main function for the NATIVE version of this project.

#include <iostream>

#include "../box-world.h"

int main(int argc, char* argv[])
{
    BoxWorld world;

    world.config.Read("BoxConfig.cfg");

    auto args = emp::cl::ArgManager(argc, argv);
    if (args.ProcessConfigOptions(world.config, std::cout, "BoxConfig.cfg", "Box-macros.h") == false) exit(0);
    if (args.TestUnknown() == false) exit(0);  // If there are leftover args, throw an error.

    world.Setup();

    world.Run();

}
