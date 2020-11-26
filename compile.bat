"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.27.29110\bin\Hostx64\x64\cl.exe" src/Minecraft.cpp /fp:fast /arch:AVX /Fe:Minecraft-MD.exe /EHsc /MD /O2 /std:c++17 /I "C:\Code\tools\vcpkg\installed\x86-windows\include" /link /LIBPATH:"C:\Code\tools\vcpkg\installed\x86-windows\lib"
REM cl.exe src/Minecraft.cpp /fp:fast /arch:AVX /Fe:Minecraft-MT.exe /EHsc /MT /O2 /std:c++17 /I "C:\Code\tools\vcpkg\installed\x86-windows\include" /link /LIBPATH:"C:\Code\tools\vcpkg\installed\x86-windows\lib"

del Minecraft.obj