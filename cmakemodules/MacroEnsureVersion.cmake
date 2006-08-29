# LGPL-v2, David Faure
#
# This macro compares version numbers of the form "x.y.z"
# MACRO_ENSURE_VERSION( ${FOO_MIN_VERSION} ${FOO_VERSION_FOUND} FOO_VERSION_OK)
# will set FOO_VERSIN_OK to true if FOO_VERSION_FOUND >= FOO_MIN_VERSION
#
MACRO(MACRO_ENSURE_VERSION requested_version found_version var_too_old)

    # parse the parts of the version string
    STRING(REGEX REPLACE "([0-9]+)\\.[0-9]+\\.[0-9]+" "\\1" req_major_vers "${requested_version}")
    STRING(REGEX REPLACE "[0-9]+\\.([0-9])+\\.[0-9]+" "\\1" req_minor_vers "${requested_version}")
    STRING(REGEX REPLACE "[0-9]+\\.[0-9]+\\.([0-9]+)" "\\1" req_patch_vers "${requested_version}")
   
    STRING(REGEX REPLACE "([0-9]+)\\.[0-9]+\\.[0-9]+.*" "\\1" found_major_vers "${found_version}")
    STRING(REGEX REPLACE "[0-9]+\\.([0-9])+\\.[0-9]+.*" "\\1" found_minor_vers "${found_version}")
    STRING(REGEX REPLACE "[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" found_patch_vers "${found_version}")
   
    # compute an overall version number which can be compared at once
    MATH(EXPR req_vers_num "${req_major_vers}*10000 + ${req_minor_vers}*100 + ${req_patch_vers}")
    MATH(EXPR found_vers_num "${found_major_vers}*10000 + ${found_minor_vers}*100 + ${found_patch_vers}")

    if (found_vers_num LESS req_vers_num)
        set( ${var_too_old} FALSE )
    else (found_vers_num LESS req_vers_num)
        set( ${var_too_old} TRUE )
    endif (found_vers_num LESS req_vers_num)

ENDMACRO(MACRO_ENSURE_VERSION)
