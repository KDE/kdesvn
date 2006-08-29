SET(helperssrc 
    ktranslateurl.cpp
    sshagent.cpp
    sub2qt.cpp)

KDE3_AUTOMOC(${helperssrc} )
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
ADD_LIBRARY(kdesvnhelpers STATIC ${helperssrc})

SET_TARGET_PROPERTIES(kdesvnhelpers
    PROPERTIES
    COMPILE_FLAGS ${CMAKE_SHARED_LIBRARY_CXX_FLAGS})
