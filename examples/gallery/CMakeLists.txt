add_executable(union_gallery)
ecm_add_qml_module(union_gallery URI "org.kde.union.gallery" GENERATE_PLUGIN_SOURCE)

target_sources(union_gallery PRIVATE main.cpp)

ecm_target_qml_sources(union_gallery SOURCES
    Main.qml
)

set(_configured_files
    Buttons.qml
    Checkable.qml
)

set_property(DIRECTORY PROPERTY CMAKE_CONFIGURE_DEPENDS "${_configured_files}")

set(_styles
    "org.kde.union"
    "org.kde.breeze"
    "org.kde.desktop"
)

foreach(_file ${_configured_files})
    foreach(_style ${_styles})
        get_filename_component(_filename "${_file}" NAME_WE)
        file(READ "${_file}" _file_contents)
        string(REPLACE "QtQuick.Controls" "${_style}" _file_contents "${_file_contents}")
        file(GENERATE OUTPUT "${_filename}_${_style}.qml" CONTENT "${_file_contents}")
        ecm_target_qml_sources(union_gallery SOURCES "${CMAKE_CURRENT_BINARY_DIR}/${_filename}_${_style}.qml")
    endforeach()
endforeach()

target_link_libraries(union_gallery PRIVATE Qt6::Quick Qt6::Widgets)

install(TARGETS union_gallery DESTINATION ${KDE_INSTALL_BINDIR})

