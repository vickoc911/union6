// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Kylie <kyliepc@proton.me>

#pragma once

#include <functional>
#include <memory>

#include <QList>
#include <QString>
#include <QStringList>
#include <QVariantMap>

#include <Element.h>
#include <properties/StylePropertyGroup.h>

#include "CssDeclarations.h"
#include "PropertyTranslator.h"

namespace Union
{
namespace Gtk
{

struct ElementSpec {
    QString type;
    QStringList hints = {};
    QVariantMap attributes = {};
};

using ElementPath = QList<ElementSpec>;

// A GTK state of a rule and the Union states it corresponds to.
struct StateVariant {
    QString pseudo = {};
    Union::Element::States states = {};
    QStringList hints = {};
    bool backdrop = false;
    // Override RuleSpec::stateAnchor & RuleSpec::gtkMarker unless -1
    int unionAnchor = -1;
    int gtkMarker = -1;
};

class RuleContext
{
public:
    virtual ~RuleContext() = default;

    virtual std::shared_ptr<Union::Properties::StylePropertyGroup> query(const ElementPath &path, Union::Element::States states = {}) const = 0;
};

using AdjustFunction = std::function<void(DeclarationList &, const RuleContext &, const StateVariant &)>;

// A generated GTK rule and the Union element it is generated from.
struct RuleSpec {
    // State pseudo-classes go at "{}", or on the end if there isn't one
    QString selector;
    ElementPath path;
    Features features;
    QList<StateVariant> variants;
    BorderMode borderMode = BorderMode::InsetShadow;
    // Which element in path & which "{}" in selector get the states, -1 is the last one
    int stateAnchor = -1;
    int gtkMarker = -1;
    QStringList parents = {};
    // Keep the parents' values for properties outside of features
    bool partial = false;
    AdjustFunction adjust = {};
};

QList<RuleSpec> ruleSpecs(GtkVersion version);
}
}
