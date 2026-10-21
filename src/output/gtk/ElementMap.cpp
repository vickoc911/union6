// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Kylie <kyliepc@proton.me>

#include "ElementMap.h"

#include <properties/BorderPropertyGroup.h>
#include <properties/CornersPropertyGroup.h>
#include <properties/LayoutPropertyGroup.h>
#include <properties/LinePropertyGroup.h>
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
static constexpr auto ActiveFocus = State::ActiveFocus;
static constexpr auto Highlighted = State::Highlighted;

static const StateVariant Base{};
static const StateVariant Backdrop{.pseudo = u":backdrop"_s, .backdrop = true};

static StateVariant state(const QString &pseudo, Union::Element::States states)
{
    return StateVariant{.pseudo = pseudo, .states = states};
}

static StateVariant hint(const QString &pseudo, const QStringList &hints)
{
    return StateVariant{.pseudo = pseudo, .hints = hints};
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

// GTK entries show focus any time they have keyboard focus
static const QList EntryStates = {
    Base,
    state(u":hover"_s, Hovered),
    state(u":focus-within"_s, ActiveFocus | VisualFocus),
    state(u":disabled"_s, Disabled),
    Backdrop,
};

static const QList CheckStates = {
    Base,
    state(u":hover"_s, Hovered),
    state(u":checked"_s, Checked),
    state(u":checked:hover"_s, Checked | Hovered),
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

// Checks & radios that aren't in a check button (menus, tree views) carry the states themselves
static const QList IndicatorStates = {
    Base,
    state(u":checked"_s, Checked),
    state(u":disabled"_s, Disabled),
    state(u":checked:disabled"_s, Checked | Disabled),
};

static const QList CheckIndicatorStates = {
    Base,
    state(u":checked"_s, Checked),
    hint(u":indeterminate"_s, {u"indeterminate"_s}),
    state(u":disabled"_s, Disabled),
    state(u":checked:disabled"_s, Checked | Disabled),
};

static const QList CheckBoxIndicatorStates = {
    Base,
    state(u":hover"_s, Hovered),
    state(u":checked"_s, Checked),
    state(u":checked:hover"_s, Checked | Hovered),
    state(u":focus-visible"_s, VisualFocus),
    hint(u":indeterminate"_s, {u"indeterminate"_s}),
    StateVariant{.pseudo = u":indeterminate:disabled"_s, .states = Disabled, .hints = {u"indeterminate"_s}},
    state(u":disabled"_s, Disabled),
    state(u":checked:disabled"_s, Checked | Disabled),
    Backdrop,
};

static const QList BaseAndBackdrop = {Base, Backdrop};
static const QList BaseAndDisabled = {Base, state(u":disabled"_s, Disabled)};

static ElementSpec element(const QString &type, const QStringList &hints = {}, const QVariantMap &attributes = {})
{
    return ElementSpec{type, hints, attributes};
}

static qreal layoutValue(const std::shared_ptr<StylePropertyGroup> &properties, std::optional<qreal> (LayoutPropertyGroup::*getter)() const)
{
    if (!properties || !properties->layout()) {
        return 0.0;
    }
    return (properties->layout()->*getter)().value_or(0.0);
}

// GTK centres slider and switch handles in the trough. Negative margins let a handle that's bigger than the trough overflow it
static AdjustFunction overflowAdjust(ElementPath troughPath, Qt::Orientation orientation)
{
    return [troughPath, orientation](DeclarationList &declarations, const RuleContext &context, const StateVariant &) {
        const auto size = orientation == Qt::Horizontal ? &LayoutPropertyGroup::height : &LayoutPropertyGroup::width;
        const auto trough = context.query(troughPath);
        const auto handle = context.query(troughPath + ElementPath{element(u"Handle"_s)});
        const auto overflow = (layoutValue(handle, size) - layoutValue(trough, size)) / 2.0;
        if (overflow > 0.0) {
            declarations.set(u"margin"_s, orientation == Qt::Horizontal ? toPx(-overflow) + u" 0"_s : u"0 "_s + toPx(-overflow));
        }
    };
}

// GTK sizes a switch by its track, so the handle stuck out of it & rows taller than the track stretched it.
// Union sizes it by the handle, so add the overhang as a transparent border the background doesn't paint
static AdjustFunction trackBorderAdjust(ElementPath troughPath)
{
    return [troughPath](DeclarationList &declarations, const RuleContext &context, const StateVariant &) {
        const auto trough = context.query(troughPath);
        const auto handle = context.query(troughPath + ElementPath{element(u"Handle"_s)});
        const auto overflow = (layoutValue(handle, &LayoutPropertyGroup::height) - layoutValue(trough, &LayoutPropertyGroup::height)) / 2.0;
        if (overflow <= 0.0) {
            return;
        }

        declarations.set(u"border-style"_s, u"solid"_s);
        declarations.set(u"border-color"_s, u"transparent"_s);
        declarations.set(u"border-width"_s, toPx(overflow) + u" 0"_s);
        declarations.set(u"background-clip"_s, u"padding-box"_s);

        // corners are measured on the outside of the border, so they need the overhang only vertically
        if (trough && trough->corners()) {
            const auto radius = trough->corners()->radii().topLeft;
            declarations.set(u"border-radius"_s, toPx(radius) + u" / "_s + toPx(radius + overflow));
        }
    };
}

static QMarginsF borderWidths(const std::shared_ptr<StylePropertyGroup> &properties)
{
    const auto border = properties ? properties->border() : nullptr;
    if (!border) {
        return {};
    }
    const auto size = [](const LinePropertyGroup *line) {
        return line ? line->size().value_or(0.0) : 0.0;
    };
    return QMarginsF(size(border->left()), size(border->top()), size(border->right()), size(border->bottom()));
}

// Union places spin box indicators from the edge of the whole spin box but GTK puts the buttons inside its
// padding, which apps like GIMP change. Only keep the padding before the text & size it with min-height
static AdjustFunction spinBoxAdjust(ElementPath spinBox)
{
    return [spinBox](DeclarationList &declarations, const RuleContext &context, const StateVariant &) {
        const auto properties = context.query(spinBox);
        if (!properties || !properties->layout() || !properties->layout()->padding()) {
            return;
        }

        const auto border = borderWidths(properties);
        const auto padding = properties->layout()->padding()->toMargins();
        declarations.set(u"padding"_s, shorthand(u"0"_s, u"0"_s, u"0"_s, toPx(padding.left() - border.left())));
        declarations.set(u"min-height"_s, toPx(layoutValue(properties, &LayoutPropertyGroup::height) - border.top() - border.bottom()));
    };
}

static AdjustFunction spinButtonAdjust(ElementPath spinBox, ElementPath indicator, bool atEnd)
{
    return [spinBox, indicator, atEnd](DeclarationList &declarations, const RuleContext &context, const StateVariant &variant) {
        declarations.set(u"padding"_s, u"0"_s);
        declarations.set(u"box-shadow"_s, declarations.value(u"box-shadow"_s).value_or(u"none"_s));

        const auto border = borderWidths(context.query(spinBox));
        const auto properties = context.query(indicator, variant.states);
        const auto margins = properties && properties->layout() && properties->layout()->margins() ? properties->layout()->margins()->toMargins() : QMarginsF{};
        const auto end = atEnd ? toPx(margins.right() - border.right()) : u"0"_s;
        declarations.set(u"margin"_s, shorthand(toPx(margins.top() - border.top()), end, toPx(margins.bottom() - border.bottom()), u"0"_s));
    };
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

    specs.append({
        .selector = u"entry"_s,
        .path = {element(u"TextField"_s)},
        .features = Feature::Frame | Feature::TextColor | Feature::Font | Feature::Padding | Feature::MinHeight | Feature::Outline,
        .variants = EntryStates,
        .borderMode = BorderMode::Real,
    });

    specs.append({
        .selector = u"spinbutton:not(.vertical)"_s,
        .path = {element(u"SpinBox"_s)},
        .features = Feature::Frame | Feature::TextColor | Feature::Font | Feature::Padding | Feature::MinHeight | Feature::Outline,
        .variants = EntryStates,
        .borderMode = BorderMode::Real,
        .adjust = spinBoxAdjust({element(u"SpinBox"_s)}),
    });

    // Union spin boxes don't stack the buttons around the text, vertical ones just get the frame
    specs.append({
        .selector = u"spinbutton.vertical"_s,
        .path = {element(u"SpinBox"_s)},
        .features = Feature::Frame | Feature::TextColor | Feature::Outline,
        .variants = EntryStates,
        .borderMode = BorderMode::Real,
    });

    // GTK puts both buttons at the end so they both use the Union increase button
    // corners are squared unless they're hovered or pressed.
    const ElementPath spinBox{element(u"SpinBox"_s)};
    const ElementPath increase{element(u"SpinBox"_s), element(u"Indicator"_s, {u"increase"_s})};
    const auto spinButton = [&](const QString &selector, bool atEnd) {
        return RuleSpec{
            .selector = selector,
            .path = increase,
            .features =
                Feature::Background | Feature::Border | Feature::Corners | Feature::TextColor | Feature::IconColor | Feature::IconSource | Feature::MinWidth,
            .variants =
                {
                    Base,
                    state(u":hover"_s, Hovered),
                    state(u":active"_s, Pressed),
                    state(u":disabled"_s, Disabled),
                    Backdrop,
                },
            .parents = atEnd ? QStringList{u"button"_s, u"spinbutton:not(.vertical) > button"_s} : QStringList{u"button"_s},
            .adjust = spinButtonAdjust(spinBox, increase, atEnd),
        };
    };
    specs.append(spinButton(u"spinbutton:not(.vertical) > button"_s, false));
    specs.append(spinButton(u"spinbutton:not(.vertical) > button.up"_s, true));

    specs.append({
        .selector = u"spinbutton:not(.vertical) > button.down"_s,
        .path = {element(u"SpinBox"_s), element(u"Indicator"_s, {u"decrease"_s})},
        .features = Feature::IconSource,
        .variants = {Base},
        .parents = {u"spinbutton:not(.vertical) > button"_s},
        .partial = true,
    });

    // Vertical spin buttons stack +/- around the text with no frame of their
    // own, so they only get the indicator look while hovered or pressed
    specs.append({
        .selector = u"spinbutton.vertical > button"_s,
        .path = {element(u"SpinBox"_s), element(u"Indicator"_s)},
        .features = Feature::Background | Feature::Border | Feature::Corners | Feature::TextColor | Feature::IconColor,
        .variants =
            {
                Base,
                state(u":hover"_s, Hovered),
                state(u":active"_s, Pressed),
                state(u":disabled"_s, Disabled),
                Backdrop,
            },
        .parents = {u"button"_s},
        .partial = true,
    });

    specs.append({
        .selector = u"checkbutton"_s,
        .path = {element(u"CheckBox"_s)},
        .features = Feature::TextColor | Feature::Font | Feature::Padding,
        .variants = {Base, state(u":disabled"_s, Disabled), Backdrop},
    });

    specs.append({
        .selector = u"check"_s,
        .path = {element(u"CheckBox"_s), element(u"Indicator"_s)},
        .features = Feature::Frame | Feature::MinSize | Feature::IconSource,
        .variants = CheckIndicatorStates,
        .stateAnchor = 0,
    });

    specs.append({
        .selector = u"radio"_s,
        .path = {element(u"RadioButton"_s), element(u"Indicator"_s)},
        .features = Feature::Frame | Feature::MinSize | Feature::IconSource,
        .variants = IndicatorStates,
        .stateAnchor = 0,
    });

    specs.append({
        .selector = u"checkbutton{} > check"_s,
        .path = {element(u"CheckBox"_s), element(u"Indicator"_s)},
        .features = Feature::Frame | Feature::MinSize | Feature::Outline | Feature::IconSource,
        .variants = CheckBoxIndicatorStates,
        .stateAnchor = 0,
    });

    specs.append({
        .selector = u"switch"_s,
        .path = {element(u"Switch"_s), element(u"Indicator"_s)},
        .features = Feature::Frame | Feature::MinSize,
        .variants = CheckStates,
        .stateAnchor = 0,
        .adjust = trackBorderAdjust({element(u"Switch"_s), element(u"Indicator"_s)}),
    });

    specs.append({
        .selector = u"switch{} > slider"_s,
        .path = {element(u"Switch"_s), element(u"Indicator"_s), element(u"Handle"_s)},
        .features = Feature::Frame | Feature::MinSize | Feature::Outline,
        .variants = CheckStates,
        .stateAnchor = 0,
        .adjust = overflowAdjust({element(u"Switch"_s), element(u"Indicator"_s)}, Qt::Horizontal),
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
        .selector = u"checkbutton{} > radio"_s,
        .path = {element(u"RadioButton"_s), element(u"Indicator"_s)},
        .features = Feature::Frame | Feature::MinSize | Feature::Outline | Feature::IconSource,
        .variants = CheckStates,
        .stateAnchor = 0,
    });

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

    specs.append({
        .selector = u"entry{} > text > placeholder"_s,
        .path = {element(u"TextField"_s), element(u"Text"_s, {u"placeholder"_s})},
        .features = Feature::TextColor,
        .variants = BaseAndDisabled,
        .stateAnchor = 0,
    });
}

static void appendGtk3Rules(QList<RuleSpec> &specs)
{
    specs.append({
        .selector = u"radiobutton"_s,
        .path = {element(u"RadioButton"_s)},
        .features = Feature::TextColor | Feature::Font | Feature::Padding,
        .variants = {Base, state(u":disabled"_s, Disabled), Backdrop},
    });

    specs.append({
        .selector = u"radiobutton{} > radio"_s,
        .path = {element(u"RadioButton"_s), element(u"Indicator"_s)},
        .features = Feature::Frame | Feature::MinSize | Feature::Outline | Feature::IconSource,
        .variants = CheckStates,
        .stateAnchor = 0,
    });
}

QList<RuleSpec> ruleSpecs(GtkVersion version)
{
    QList<RuleSpec> specs;
    appendRules(specs);
    if (version == GtkVersion::Gtk4) {
        appendGtk4Rules(specs);
    } else {
        appendGtk3Rules(specs);
    }
    return specs;
}
}
