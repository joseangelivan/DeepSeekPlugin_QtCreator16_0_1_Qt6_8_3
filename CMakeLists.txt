cmake_minimum_required(VERSION 3.16)

# Remove when sharing with others.
list(APPEND CMAKE_PREFIX_PATH "/opt/qtcreator-16.0.1")

project(DeepSeekPlugin_QtCreator16_0_1_Qt6_8_3)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Configura las rutas de inclusión específicas
# include_directories(
#     "/opt/qtcreator-16.0.1/include/qtcreator"
#     "/opt/qtcreator-16.0.1/include/qtcreator/src"
#     "/opt/qtcreator-16.0.1/include/qtcreator/src/plugins"
# )

find_package(QtCreator REQUIRED COMPONENTS Core ProjectExplorer TextEditor)
find_package(Qt6 COMPONENTS Widgets Network REQUIRED)

# Add a CMake option that enables building your plugin with tests.
# You don't want your released plugin binaries to contain tests,
# so make that default to 'NO'.
# Enable tests by passing -DWITH_TESTS=ON to CMake.
option(WITH_TESTS "Builds with tests" NO)

if(WITH_TESTS)
  # Look for QtTest
  find_package(Qt6 REQUIRED COMPONENTS Test)

  # Tell CMake functions like add_qtc_plugin about the QtTest component.
  set(IMPLICIT_DEPENDS Qt::Test)

  # Enable ctest for auto tests.
  enable_testing()
endif()


set(RESOURCE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/resources.qrc")

add_qtc_plugin(DeepSeekPlugin_QtCreator16_0_1_Qt6_8_3
  PLUGIN_DEPENDS
    QtCreator::Core
    QtCreator::ProjectExplorer
    QtCreator::TextEditor
  DEPENDS
    Qt::Widgets
    Qt::Network
    QtCreator::ExtensionSystem
    QtCreator::Utils
  SOURCES
   ${DeepSeekPlugin_RESOURCES}
    README.md
    deepseekplugin_qtcreator16_0_1_qt6_8_3.cpp
    deepseekplugin_qtcreator16_0_1_qt6_8_3constants.h
    deepseekplugin_qtcreator16_0_1_qt6_8_3tr.h
    deepseekoptionspage.cpp
    deepseekoptionspage.h
    deepseekoptionspage.ui
    deepseeknavigationchat.cpp
    deepseeknavigationchat.h
    deepseeksettings.h
    deepseeksettings.cpp
    singleton.h

)

# # Agrega las rutas específicas al target
# target_include_directories(DeepSeekPlugin_QtCreator16_0_1_Qt6_8_3 PRIVATE
#     "/opt/qtcreator-16.0.1/include/qtcreator"
#     "/opt/qtcreator-16.0.1/include/qtcreator/src"
#     "/opt/qtcreator-16.0.1/include/qtcreator/src/plugins"
# )


target_sources(DeepSeekPlugin_QtCreator16_0_1_Qt6_8_3
  PRIVATE
    ${RESOURCE_FILE}
)

# Enable the Run button in Qt Creator
get_target_property(QtCreatorCorePath QtCreator::Core LOCATION)
find_program(QtCreatorExecutable
  NAMES
    qtcreator "Qt Creator"
  PATHS
    "${QtCreatorCorePath}/../../../bin"
    "${QtCreatorCorePath}/../../../MacOS"
  NO_DEFAULT_PATH
)
if (QtCreatorExecutable)
  add_custom_target(RunQtCreator
    COMMAND ${QtCreatorExecutable} -pluginpath $<TARGET_FILE_DIR:DeepSeekPlugin_QtCreator16_0_1_Qt6_8_3>
    DEPENDS DeepSeekPlugin_QtCreator16_0_1_Qt6_8_3
  )
  set_target_properties(RunQtCreator PROPERTIES FOLDER "qtc_runnable")
endif()
