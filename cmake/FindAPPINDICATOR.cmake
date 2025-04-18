# https://github.com/transmission/transmission/blob/main/cmake/FindAPPINDICATOR.cmake

find_package(PkgConfig QUIET)

pkg_check_modules(PC_AYATANA_APPINDICATOR QUIET ayatana-appindicator3-0.1)
find_path(AYATANA_APPINDICATOR_INCLUDE_DIR
        NAMES libayatana-appindicator/app-indicator.h
        HINTS ${PC_AYATANA_APPINDICATOR_INCLUDE_DIRS})
find_library(AYATANA_APPINDICATOR_LIBRARY
        NAMES
        ayatana-appindicator3
        ayatana-appindicator
        HINTS ${PC_AYATANA_APPINDICATOR_LIBRARY_DIRS})

if(AYATANA_APPINDICATOR_INCLUDE_DIR AND AYATANA_APPINDICATOR_LIBRARY)
    set(APPINDICATOR_INCLUDE_DIR ${AYATANA_APPINDICATOR_INCLUDE_DIR})
    set(APPINDICATOR_LIBRARY ${AYATANA_APPINDICATOR_LIBRARY})
    set(APPINDICATOR_IS_AYATANA ON)
else()
    pkg_check_modules(PC_APPINDICATOR QUIET appindicator3-0.1)
    find_path(APPINDICATOR_INCLUDE_DIR
            NAMES libappindicator/app-indicator.h
            HINTS ${PC_APPINDICATOR_INCLUDE_DIRS})
    find_library(APPINDICATOR_LIBRARY
            NAMES
            appindicator3
            appindicator
            HINTS ${PC_APPINDICATOR_LIBRARY_DIRS})
    set(APPINDICATOR_IS_AYATANA OFF)
endif()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(APPINDICATOR
        REQUIRED_VARS
        APPINDICATOR_LIBRARY
        APPINDICATOR_INCLUDE_DIR)

mark_as_advanced(AYATANA_APPINDICATOR_INCLUDE_DIR AYATANA_APPINDICATOR_LIBRARY)
mark_as_advanced(APPINDICATOR_INCLUDE_DIR APPINDICATOR_LIBRARY)

set(APPINDICATOR_INCLUDE_DIRS ${APPINDICATOR_INCLUDE_DIR})
set(APPINDICATOR_LIBRARIES ${APPINDICATOR_LIBRARY})
