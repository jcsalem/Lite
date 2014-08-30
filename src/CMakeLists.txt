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

# Add SFML support
SET(MY_LIBS ${MY_LIBS} ${SFML_LIBRARIES})

# Raspberry Pi flag
IF(EXISTS "/etc/init.d/liteberryrun")
  SET(RASPBERRY_PI 1)
ENDIF()

# Mac Code Signing
if(APPLE)
   find_program(MAC_CODESIGN codesign)
endif(APPLE)

# Excutables 
set(PROGRAMS Ltool ckinfo Lfirefly Lflash Lstarry Lsparkle Lpov testmix)

foreach (PROG ${PROGRAMS})
  add_executable(${PROG} ${PROG}.cpp)
  target_link_libraries(${PROG} lite ${MY_LIBS})

  # Code signing on Mac
  if(MAC_CODESIGN)
	add_custom_command(TARGET ${PROG}
						POST_BUILD VERBATIM
						COMMENT "Code signing ${PROG}"
						COMMAND ${MAC_CODESIGN} -f -s - $<TARGET_FILE:${PROG}>
						)
  endif(MAC_CODESIGN)
  
  # To enable GPIO access, make binaries on the Raspberry PI setuid root 
  if(RASPBERRY_PI)
  	add_custom_command(TARGET ${PROG}
  						POST_BUILD VERBATIM
						COMMENT "Making ${PROG} setuid root"
  						COMMAND sudo chown root:root $<TARGET_FILE:${PROG}>
  						COMMAND sudo chmod 04755 $<TARGET_FILE:${PROG}>
  						)
  endif(RASPBERRY_PI)
endforeach(PROG) 

