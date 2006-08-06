SET(helperssrc 
   ktranslateurl.cpp
   sshagent.cpp
   sub2qt.cpp)
KDE3_AUTOMOC(${helperssrc} )

ADD_LIBRARY(kdesvnhelpers ${helperssrc})
