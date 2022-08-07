#pragma once
/* Base, empty configuration file!

     To override, place a file in your include paths of the form:

. (your include path here)
| sol (directory, or equivalent)
  | config.hpp (your config.hpp file)

     So that when sol2 includes the file

#include <sol/config.hpp>

     it gives you the configuration values you desire. Configuration values can be
seen in the safety.rst of the doc/src, or at
https://sol2.readthedocs.io/en/latest/safety.html ! You can also pass them through
the build system, or the command line options of your compiler.

*/

#ifdef _DEBUG
#define SOL_ALL_SAFETIES_ON 1
#define SOL_PRINT_ERRORS 0
#define SOL_USING_CXX_LUA 1
#endif
