project(kscope4)

# set version
set (KSCOPE_VERSION_MAJOR 1)
set (KSCOPE_VERSION_MINOR 8)
set (KSCOPE_VERSION_RELEASE 0)
set (KSCOPE_VERSION "${KSCOPE_VERSION_MAJOR}.${KSCOPE_VERSION_MINOR}.${KSCOPE_VERSION_RELEASE}" )
set (KSCOPE_VERSION_STRING "${KSCOPE_VERSION} (KScope 1.8)")

# Start: packaging additions - bms
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "KDE 4 CScope front end")
SET(CPACK_PACKAGE_VENDOR "KScope Maintainer")
SET(CPACK_PACKAGE_CONTACT "KScope Maintainer <kscope@unknown.net>")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
SET(CPACK_PACKAGE_VERSION_MAJOR "1")
SET(CPACK_PACKAGE_VERSION_MINOR "8")
SET(CPACK_PACKAGE_VERSION_PATCH "0")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "CMake ${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}")
SET(CPACK_GENERATOR "DEB")
SET(CPACK_SOURCE_GENERATOR "")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.14), libkdecore5, libkdeui5, libkfile4, libkio5, libkparts4, libktexteditor4, libplasma3, libqjson0 (>= 0.7.1), libqt4-dbus (>= 4:4.5.3), libqt4-sql (>= 4:4.5.3), libqt4-xml (>= 4:4.5.3), libqtcore4 (>= 4:4.8.0), libqtgui4 (>= 4:4.8.0), libstdc++6 (>= 4.1.1)")
include(CPack)
# End: packaging additions - bms

# Find the required Libaries
find_package(KDE4 REQUIRED)
find_package(Qt4 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)
 
add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
add_definitions (-DVERSION=\\\"${KSCOPE_VERSION}\\\")

include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${KDE4_INCLUDES}
)

add_subdirectory ( src )
add_subdirectory( doc )
