project(kscope4)

# set version
set (KSCOPE_VERSION_MAJOR 1)
set (KSCOPE_VERSION_MINOR 7)
set (KSCOPE_VERSION_RELEASE 0)
set (KSCOPE_VERSION "${KSCOPE_VERSION_MAJOR}.${KSCOPE_VERSION_MINOR}.${KSCOPE_VERSION_RELEASE}" )
set (KSCOPE_VERSION_STRING "${KSCOPE_VERSION} (KScope 1.7)")

set( QT_USE_QT3SUPPORT TRUE )

# Find the required Libaries
find_package(KDE4 REQUIRED)
find_package(Qt4 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)
 
add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)
add_definitions (-DVERSION=\\\"${KSCOPE_VERSION}\\\")

include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${KDE4_INCLUDES}
)

add_subdirectory ( src )
add_subdirectory( doc )
