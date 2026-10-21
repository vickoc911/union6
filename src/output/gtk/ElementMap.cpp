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

static constexpr auto Hovered = State::Hovered;
static constexpr auto Pressed = State::Pressed;
static constexpr auto Checked = State::Checked;
static constexpr auto Disabled = State::Disabled;
static constexpr auto VisualFocus = State::VisualFocus;
static constexpr auto Highlighted = State::Highlighted;

static const StateVariant Base{};
static const StateVariant Backdrop{.pseudo = u":backdrop"_s, .backdrop = true};

static StateVariant state(const QString &pseudo, Union::Element::States states)
{
    return StateVariant{.pseudo = pseudo, .states = states};
}

static const QList ButtonStates = {
    Base,
    state(u":hover"_s, Hovered),
    state(u":active"_s, Pressed),
    state(u":checked"_s, Checked),
    state(u":checked:hover"_s, Checked | Hovered),
    state(u":checked:active"_s, Checked | Pressed),
    state(u":focus-visible"_s, VisualFocus),
    state(u":disabled"_s, Disabled),
    state(u":checked:disabled"_s, Checked | Disabled),
    Backdrop,
};

// GTK marks the combo box button as checked while the popup is open
static const QList ComboStates = {
    Base,
    state(u":hover"_s, Hovered),
    state(u":active"_s, Pressed),
    state(u":checked"_s, Pressed),
    state(u":focus-visible"_s, VisualFocus),
    state(u":disabled"_s, Disabled),
    Backdrop,
};

static const QList BaseAndBackdrop = {Base, Backdrop};
static ElementSpec element(const QString &type, const QStringList &hints = {}, const QVariantMap &attributes = {})
{
    return ElementSpec{type, hints, attributes};
}

// only the sides of the labels padding from the button
static AdjustFunction sidePaddingAdjust(ElementPath path)
{
    return [path](DeclarationList &declarations, const RuleContext &context, const StateVariant &) {
        const auto properties = context.query(path);
        if (!properties || !properties->layout() || !properties->layout()->padding()) {
            return;
        }

        const auto padding = properties->layout()->padding()->toMargins();
        declarations.set(u"padding"_s, shorthand(u"0"_s, toPx(padding.right()), u"0"_s, toPx(padding.left())));
    };
}

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

    specs.append({
        .selector = u"button"_s,
        .path = {element(u"Button"_s)},
        .features = Feature::Control,
        .variants = ButtonStates,
    });

    specs.append({
        .selector = u"button.flat"_s,
        .path = {element(u"Button"_s, {u"flat"_s})},
        .features = Feature::Control,
        .variants = ButtonStates,
        .parents = {u"button"_s},
    });

    // GIMP shrink the padding of every button, do this so stack switchers keep their side padding on the label
    specs.append({
        .selector = u"stackswitcher > button > label"_s,
        .path = {element(u"Button"_s)},
        .features = Feature::None,
        .variants = {Base},
        .adjust = sidePaddingAdjust({element(u"Button"_s)}),
    });

    specs.append({
        .selector = u"combobox button.combo"_s,
        .path = {element(u"ComboBox"_s)},
        .features = Feature::Control,
        .variants = ComboStates,
        .parents = {u"button"_s},
    });

    specs.append({
        .selector = u"combobox arrow"_s,
        .path = {element(u"ComboBox"_s), element(u"Indicator"_s)},
        .features = Feature::IconSource | Feature::MinSize,
        .variants = {Base},
    });

    // Suggested & destructive actions don't exist in Union. Use a selected item and an error message instead
    // FIXME: define the hover and pressed colours of these in breeze union, until then they look the same in every state
    const auto accentStates = [](Union::Element::States base) {
        return QList{
            state(QString{}, base),
            state(u":hover"_s, base | Hovered),
            state(u":active"_s, base | Pressed),
            state(u":checked"_s, base | Pressed),
        };
    };

    specs.append({
        .selector = u"button.suggested-action:not(:disabled){}, headerbar button.suggested-action:not(:disabled){}"_s,
        .path = {element(u"ItemDelegate"_s)},
        .features = Feature::Frame | Feature::TextColor | Feature::IconColor,
        .variants = accentStates(Highlighted),
        .parents = {u"button"_s},
        .partial = true,
    });

    specs.append({
        .selector = u"button.destructive-action:not(:disabled){}, headerbar button.destructive-action:not(:disabled){}"_s,
        .path = {element(u"InlineMessage"_s, {}, {{u"type"_s, u"error"_s}})},
        .features = Feature::Background | Feature::Border | Feature::Shadow | Feature::TextColor | Feature::IconColor,
        .variants = accentStates({}),
        .parents = {u"button"_s},
        .partial = true,
    });
}

static void appendGtk4Rules(QList<RuleSpec> &specs)
{
    specs.append({
        .selector = u"dropdown > button"_s,
        .path = {element(u"ComboBox"_s)},
        .features = Feature::Control,
        .variants = ComboStates,
        .parents = {u"button"_s},
    });

    specs.append({
        .selector = u"dropdown arrow, menubutton arrow.down"_s,
        .path = {element(u"ComboBox"_s), element(u"Indicator"_s)},
        .features = Feature::IconSource | Feature::MinSize,
        .variants = {Base},
    });
}

QList<RuleSpec> ruleSpecs(GtkVersion version)
{
    QList<RuleSpec> specs;
    appendRules(specs);
    if (version == GtkVersion::Gtk4) {
        appendGtk4Rules(specs);
    }
    return specs;
}
}
