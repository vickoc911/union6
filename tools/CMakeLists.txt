# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2024 Arjen Hiemstra <ahiemstra@heimr.nl>

add_subdirectory(ruleinspector)
add_subdirectory(styletool)

if (BUILD_OUTPUT_GTK)
    add_subdirectory(gtktheme)
endif()
