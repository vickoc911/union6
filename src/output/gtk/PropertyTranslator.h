// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Kylie <kyliepc@proton.me>

#pragma once

#include <QFlags>
#include <QSet>

#include <properties/StylePropertyGroup.h>

#include "CssDeclarations.h"

namespace Union
{
namespace Gtk
{
class ThemeAssets;

enum class Feature : uint32_t {
    None = 0,
    Background = 1 << 0,
    Border = 1 << 1,
    Corners = 1 << 2,
    TextColor = 1 << 3,
    Font = 1 << 4,
    Padding = 1 << 5,
    Margin = 1 << 6,
    MinWidth = 1 << 7,
    MinHeight = 1 << 8,
    Shadow = 1 << 9,
    Outline = 1 << 10,
    IconColor = 1 << 11,
    IconSize = 1 << 12,
    IconSource = 1 << 13,
    Opacity = 1 << 14,
    Spacing = 1 << 15,

    MinSize = MinWidth | MinHeight,
    Frame = Background | Border | Corners | Shadow,
    Control = Frame | TextColor | Font | Padding | MinSize | Outline | IconSize | Opacity,
};
Q_DECLARE_FLAGS(Features, Feature)

enum class BorderMode {
    InsetShadow,
    Real,
};

enum class GtkVersion {
    Gtk3 = 3,
    Gtk4 = 4
};

struct TranslateOptions {
    Features features = Feature::None;
    BorderMode borderMode = BorderMode::InsetShadow;
    ThemeAssets *assets = nullptr;
    GtkVersion version = GtkVersion::Gtk4;
};

QSet<QString> featureProperties(Features features);

DeclarationList translate(const Union::Properties::StylePropertyGroup *properties, const TranslateOptions &options);
}
}

Q_DECLARE_OPERATORS_FOR_FLAGS(Union::Gtk::Features)
