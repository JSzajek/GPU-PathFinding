include "vendor/opencl_lib/opencllink.lua"
include "vendor/opencv_lib/opencv4link.lua"

IncludeDir = {}
LibraryDir = {}

Library = {}

-- Windows
Library["WinSock"] = "Ws2_32.lib"
Library["WinMM"] = "Winmm.lib"
Library["WinVersion"] = "Version.lib"