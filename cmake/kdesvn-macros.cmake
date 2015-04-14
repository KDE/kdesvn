
macro(BUILD_TEST tname)
  set(${tname}-src ${tname}.cpp)
  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${tname}.h)
        set(${tname}-src ${${tname}-src} ${tname}.h)
  endif()
  add_executable(${tname} ${${tname}-src})
  target_link_libraries(${tname} svnqt Qt5::Core Qt5::Sql)
  add_test(${tname} ${CMAKE_CURRENT_BINARY_DIR}/${tname})
endmacro()

