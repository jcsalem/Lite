add_library(lite STATIC
CKbuffer.cpp
CKdevice.cpp
Color.cpp
ComboBuffer.cpp
CursesBuffer.cpp
FilterBuffers.cpp
LBuffer.cpp
LFramework.cpp
Lobj.cpp
Lproc.cpp
LSparkle.cpp
StripBuffer.cpp
utils.cpp
utilsFile.cpp
utilsGPIO.cpp
utilsIP.cpp
utilsOptions.cpp
utilsRandom.cpp
utilsSocket.cpp
utilsStats.cpp
utilsTime.cpp
WinBuffer.cpp)

install(DIRECTORY "${PROJECT_SOURCE_DIR}/libsrc/" 
		  DESTINATION "${CMAKE_BINARY_DIR}/include"
		  FILES_MATCHING PATTERN "*.h")

