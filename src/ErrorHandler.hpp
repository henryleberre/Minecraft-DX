#ifndef __MINECRAFT__ERROR_HANDLER_HPP
#define __MINECRAFT__ERROR_HANDLER_HPP

#include "Pch.hpp"

#ifdef _WIN32

    #define FATAL_ERROR(errorMsg) { MessageBoxA(NULL, errorMsg, "Minecraft: Fatal Error", MB_ICONERROR); std::exit(-1); }

#endif // _WIN32

#endif // __MINECRAFT__ERROR_HANDLER_HPP