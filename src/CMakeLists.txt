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
IF(EXISTS "/etc/init.d/lightberryrun")
  SET(RASPBERRY_PI 1)
ENDIF()

# Excutables 
set(PROGRAMS Ltool ckinfo Lfirefly Lflash Lstarry Lsparkle testmix)

foreach (PROG ${PROGRAMS})
  add_executable(${PROG} ${PROG}.cpp)
  target_link_libraries(${PROG} lite ${MY_LIBS})
  
  # To enable GPIO access, make binaries on the Raspberry PI setuid root 
  if(RASPBERRY_PI)
  	add_custom_command(TARGET ${PROG}
  						POST_BUILD VERBATIM
  						COMMAND sudo chown root $<TARGET_FILE:${PROG}>
  						COMMAND sudo chmod 04755 $<TARGET_FILE:${PROG}>
  						COMMENT "Making ${PROG} setuid root"
  						)
  endif(RASPBERRY_PI)
endforeach(PROG) 

