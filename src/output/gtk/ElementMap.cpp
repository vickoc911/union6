// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Kylie <kyliepc@proton.me>

#include "ElementMap.h"

#include <properties/LayoutPropertyGroup.h>
#include <properties/SizePropertyGroup.h>

using namespace Qt::StringLiterals;
using namespace Union::Properties;

namespace Union::Gtk
{

using State = Union::Element::State;

static const StateVariant Base{};
static const StateVariant Backdrop{.pseudo = u":backdrop"_s, .backdrop = true};

static const QList BaseAndBackdrop = {Base, Backdrop};
// Parent rules need to come first
static void appendRules(QList<RuleSpec> &specs)
{
    specs.append({
        .selector = u"window.background"_s,
        .path = {},
        .features = Feature::Background | Feature::TextColor | Feature::Font,
        .variants = BaseAndBackdrop,
        .borderMode = BorderMode::Real,
    });
}

QList<RuleSpec> ruleSpecs(GtkVersion)
{
    QList<RuleSpec> specs;
    appendRules(specs);
    return specs;
}
}
