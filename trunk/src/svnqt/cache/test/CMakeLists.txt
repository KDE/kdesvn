SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

MACRO(BUILD_TEST tname)
  SET(${tname}-src ${tname}.cpp)
  IF (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${tname}.h)
        SET(${tname}-src ${${tname}-src} ${tname}.h)
  ENDIF (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${tname}.h)
  ADD_EXECUTABLE(${tname} ${${tname}-src})
  TARGET_LINK_LIBRARIES(${tname} ${svnqt-name} ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY})
  ADD_TEST(${tname} ${CMAKE_CURRENT_BINARY_DIR}/${tname})
ENDMACRO(BUILD_TEST)

IF (BUILD_TESTS)
    CONFIGURE_FILE(
        ${CMAKE_CURRENT_SOURCE_DIR}/testconfig.h.in
        ${CMAKE_CURRENT_BINARY_DIR}/testconfig.h
    )
    BUILD_TEST(sqlite)
ENDIF(BUILD_TESTS)
