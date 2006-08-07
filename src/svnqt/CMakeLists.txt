SET(SOURCES
    apr.cpp
    client_annotate.cpp
    client_cat.cpp
    client.cpp
    client_diff.cpp
    client_ls.cpp
    client_modify.cpp
    client_property.cpp client_status.cpp
    context.cpp datetime.cpp dirent.cpp entry.cpp exception.cpp log_entry.cpp path.cpp
    pool.cpp revision.cpp status.cpp targets.cpp url.cpp wc.cpp version_check.cpp
    lock_entry.cpp client_lock.cpp info_entry.cpp client_impl.cpp contextdata.cpp
    commititem.cpp repository.cpp repositorydata.cpp
    repositorylistener.cpp svnstream.cpp svnfilestream.cpp)

FILE(GLOB HEADERS *.hpp)

ADD_LIBRARY(svnqt SHARED ${SOURCES} )
SET_TARGET_PROPERTIES(svnqt PROPERTIES SOVERSION 2 VERSION 2.2.2)
SET(ALL_LINKFLAGS ${APR_EXTRA_LIBFLAGS})
SET(ALL_LINKFLAGS "${ALL_LINKFLAGS} ${APU_EXTRA_LIBFLAGS}")
SET_TARGET_PROPERTIES(svnqt PROPERTIES LINK_FLAGS "${ALL_LINKFLAGS} ${LINK_NO_UNDEFINED}")
target_link_libraries(svnqt ${QT_LIBRARIES} ${SUBVESION_ALL_LIBS})

# Just a small linking test
ADD_EXECUTABLE(testlink testmain.cpp)
target_link_libraries(testlink svnqt)

# install rules
INSTALL(TARGETS svnqt
    LIBRARY DESTINATION lib
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE 
                GROUP_READ GROUP_EXECUTE
                WORLD_READ WORLD_EXECUTE)
install_files(/include/svnqt FILES ${HEADERS})
