# Standard libraries to link against
SET(MY_LIBS "")

IF(WIN32)
 SET(MY_LIBS ${MY_LIBS} ws2_32)
ELSE(WIN32)
 SET(MY_LIBS ${MY_LIBS} ncurses)
ENDIF(WIN32)
IF(UNIX AND NOT APPLE)
 SET(MY_LIBS ${MY_LIBS} rt)
ENDIF(UNIX AND NOT APPLE)

add_executable(Ltool Ltool.cpp)
target_link_libraries(Ltool lite ${MY_LIBS})

add_executable(ckinfo ckinfo.cpp)
target_link_libraries(ckinfo lite ${MY_LIBS})

add_executable(Lfirefly Lfirefly.cpp)
target_link_libraries(Lfirefly lite ${MY_LIBS})

add_executable(Lstarry Lstarry.cpp)
target_link_libraries(Lstarry lite ${MY_LIBS})

add_executable(Lsparkle Lsparkle.cpp)
target_link_libraries(Lsparkle lite ${MY_LIBS})

add_executable(testmix testmix.cpp)
target_link_libraries(testmix lite ${MY_LIBS})

