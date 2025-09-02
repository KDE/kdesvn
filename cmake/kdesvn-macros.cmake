
macro(BUILD_TEST tname)
  set(${tname}-src ${tname}.cpp)
  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${tname}.h)
        set(${tname}-src ${${tname}-src} ${tname}.h)
  endif()
  ecm_add_test(
    ${${tname}-src}
    TEST_NAME ${tname}
    LINK_LIBRARIES svnqt Qt::Core Qt::Sql
  )
endmacro()

