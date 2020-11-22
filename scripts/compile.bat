cl.exe Minecraft.cpp /fp:fast /arch:AVX /Fe:Minecraft-MD.exe /EHsc /MD /O2 /std:c++17 /I "C:\Code\tools\vcpkg\installed\x86-windows\include" /link /LIBPATH:"C:\Code\tools\vcpkg\installed\x86-windows\lib"
cl.exe Minecraft.cpp /fp:fast /arch:AVX /Fe:Minecraft-MT.exe /EHsc /MT /O2 /std:c++17 /I "C:\Code\tools\vcpkg\installed\x86-windows\include" /link /LIBPATH:"C:\Code\tools\vcpkg\installed\x86-windows\lib"

rm Minecraft.obj