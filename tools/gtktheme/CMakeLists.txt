# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2026 Kylie <kyliepc@proton.me>

add_executable(union-gtk-theme main.cpp)
target_link_libraries(union-gtk-theme PRIVATE UnionGtkOutput)
install(TARGETS union-gtk-theme ${KDE_INSTALL_DEFAULT_ARGS})
