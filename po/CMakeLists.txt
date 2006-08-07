ADD_POFILES("kdesvn" gmofiles)

ADD_CUSTOM_TARGET(translations ALL DEPENDS ${gmofiles})