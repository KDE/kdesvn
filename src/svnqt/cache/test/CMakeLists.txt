set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

macro(BUILD_TEST tname)
  set(${tname}-src ${tname}.cpp)
  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${tname}.h)
        set(${tname}-src ${${tname}-src} ${tname}.h)
  endif()
  add_executable(${tname} ${${tname}-src})
  target_link_libraries(${tname} ${svnqt-name} ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY})
  add_test(${tname} ${CMAKE_CURRENT_BINARY_DIR}/${tname})
endmacro()

if(BUILD_TESTS)
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/testconfig.h.in
        ${CMAKE_CURRENT_BINARY_DIR}/testconfig.h
    )
    build_test(sqlite)
endif()
