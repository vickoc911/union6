# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>

add_library(UnionWidgetsStyle MODULE)
target_sources(UnionWidgetsStyle PRIVATE
        elements/AbstractElement.cpp
        elements/AbstractElement.h
        elements/ButtonElement.cpp
        elements/ButtonElement.h
        elements/CheckElement.cpp
        elements/CheckElement.h
        elements/ComboBoxElement.cpp
        elements/ComboBoxElement.h
        elements/GroupBoxElement.cpp
        elements/GroupBoxElement.h
        elements/HeaderElement.cpp
        elements/HeaderElement.h
        elements/ItemViewElement.cpp
        elements/ItemViewElement.h
        elements/SpinBoxElement.cpp
        elements/SpinBoxElement.h
        elements/TabElement.cpp
        elements/TabElement.h
        elements/ToolButtonElement.cpp
        elements/ToolButtonElement.h
        elements/MenuItemElement.cpp
        elements/MenuItemElement.h
        elements/ProgressBarElement.cpp
        elements/ProgressBarElement.h
        BackgroundDrawing.cpp
        BackgroundDrawing.h
        StyleUtils.cpp
        StyleUtils.h
        UnionStyle.cpp
        UnionStyle.h
        UnionStylePlugin.cpp
        UnionStylePlugin.h
)
target_link_libraries(UnionWidgetsStyle
        PUBLIC
        Qt::Widgets
        Union::Union
)

set_target_properties(UnionWidgetsStyle PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
)

install(TARGETS UnionWidgetsStyle DESTINATION ${KDE_INSTALL_QTPLUGINDIR}/styles/)
install(FILES union.themerc DESTINATION ${KDE_INSTALL_DATADIR}/kstyle/themes)

