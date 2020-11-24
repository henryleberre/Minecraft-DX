#include "Pch.hpp"

#ifdef _WIN32

    #define FATAL_ERROR(errorMsg) { MessageBoxA(NULL, errorMsg, "Minecraft: Fatal Error", MB_ICONERROR); std::exit(-1); }

#endif // _WIN32