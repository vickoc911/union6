# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2026 Kylie <kyliepc@proton.me>

add_library(UnionGtkOutput STATIC)

ecm_qt_declare_logging_category(UnionGtkOutput
    HEADER gtk_logging.h
    IDENTIFIER UNION_GTK
    CATEGORY_NAME union.gtk
    DESCRIPTION "Union - GTK Output"
    EXPORT Union
)

target_sources(UnionGtkOutput PRIVATE
    ColorTranslator.cpp
    ColorTranslator.h
    CssDeclarations.cpp
    CssDeclarations.h
    ElementMap.cpp
    ElementMap.h
    PropertyTranslator.cpp
    PropertyTranslator.h
    ThemeAssets.cpp
    ThemeAssets.h
    ThemeGenerator.cpp
    ThemeGenerator.h
)

qt_add_resources(UnionGtkOutput union_gtk_stylesheets
    PREFIX /union-gtk
    BASE resources
    FILES
        resources/common/base.css
        resources/common/overrides.css
        resources/gtk3/base.css
        resources/gtk3/overrides.css
        resources/gtk4/base.css
        resources/gtk4/overrides.css
)

target_include_directories(UnionGtkOutput PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(UnionGtkOutput
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Union::Union
)
