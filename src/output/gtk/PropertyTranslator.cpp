// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Kylie <kyliepc@proton.me>

#include "PropertyTranslator.h"

#include <algorithm>
#include <array>

#include <QUrl>

#include "ColorTranslator.h"
#include "ThemeAssets.h"

using namespace Qt::StringLiterals;
using namespace Union::Properties;

namespace Union::Gtk
{

enum Side {
    Top = 0,
    Right,
    Bottom,
    Left
};

struct Line {
    qreal width = 0.0;
    std::optional<QColor> color;
    QString expression;

    bool visible() const
    {
        return width > 0.0 && color && color->alpha() > 0;
    }
};

static Line resolveLine(const LinePropertyGroup *line)
{
    if (!line || line->style().value_or(LineStyle::Solid) == LineStyle::None) {
        return Line{};
    }

    return Line{
        .width = line->size().value_or(0.0),
        .color = resolveColor(line->color()),
        .expression = toColorExpression(line->color()).value_or(u"transparent"_s),
    };
}

template<typename Group>
static std::array<Line, 4> resolveLines(const Group *group)
{
    if (!group) {
        return {};
    }
    return {resolveLine(group->top()), resolveLine(group->right()), resolveLine(group->bottom()), resolveLine(group->left())};
}

static std::array<std::optional<qreal>, 4> sizeValues(const SizePropertyGroup *size)
{
    if (!size) {
        return {};
    }
    return {size->top(), size->right(), size->bottom(), size->left()};
}

static QString pxShorthand(const std::array<qreal, 4> &values)
{
    return shorthand(toPx(values[Top]), toPx(values[Right]), toPx(values[Bottom]), toPx(values[Left]));
}

static QString insetShadows(const std::array<Line, 4> &lines)
{
    const auto &top = lines[Top];
    const bool uniform = std::all_of(lines.begin(), lines.end(), [&top](const Line &line) {
        return line.visible() && line.width == top.width && line.expression == top.expression;
    });

    if (uniform) {
        return u"inset 0 0 0 %1 %2"_s.arg(toPx(top.width), top.expression);
    }

    QStringList shadows;
    // Offsetting an inset shadow towards the other side means it only shows on one side
    if (lines[Top].visible()) {
        shadows.append(u"inset 0 %1 0 0 %2"_s.arg(toPx(lines[Top].width), lines[Top].expression));
    }
    if (lines[Right].visible()) {
        shadows.append(u"inset %1 0 0 0 %2"_s.arg(toPx(-lines[Right].width), lines[Right].expression));
    }
    if (lines[Bottom].visible()) {
        shadows.append(u"inset 0 %1 0 0 %2"_s.arg(toPx(-lines[Bottom].width), lines[Bottom].expression));
    }
    if (lines[Left].visible()) {
        shadows.append(u"inset %1 0 0 0 %2"_s.arg(toPx(lines[Left].width), lines[Left].expression));
    }
    return shadows.join(u", "_s);
}

static std::optional<QString> dropShadow(const ShadowPropertyGroup *shadow)
{
    const auto color = resolveColor(shadow->color());
    const auto expression = toColorExpression(shadow->color());
    if (!color || !expression || color->alpha() == 0) {
        return std::nullopt;
    }

    const auto x = shadow->offset() ? shadow->offset()->horizontal().value_or(0.0) : 0.0;
    const auto y = shadow->offset() ? shadow->offset()->vertical().value_or(0.0) : 0.0;
    const auto blur = shadow->blur().value_or(0.0);
    const auto spread = shadow->size().value_or(0.0);

    if (x == 0.0 && y == 0.0 && blur == 0.0 && spread == 0.0) {
        return std::nullopt;
    }

    return u"%1 %2 %3 %4 %5"_s.arg(toPx(x), toPx(y), toPx(blur), toPx(spread), expression.value());
}

static QString toGtkIconName(const QString &unionIconName)
{
    // GTK's built-in icons
    static const QHash<QString, QString> names = {
        {u"arrow-down"_s, u"pan-down-symbolic"_s},
        {u"arrow-up"_s, u"pan-up-symbolic"_s},
        {u"arrow-left"_s, u"pan-start-symbolic"_s},
        {u"arrow-right"_s, u"pan-end-symbolic"_s},
        {u"spinbox-increase"_s, u"list-add-symbolic"_s},
        {u"spinbox-decrease"_s, u"list-remove-symbolic"_s},
        {u"search"_s, u"edit-find-symbolic"_s},
        {u"edit-clear"_s, u"edit-clear-symbolic"_s},
        {u"window-restore"_s, u"window-restore-symbolic"_s},
    };

    static const auto suffix = u"-symbolic"_s;
    const auto name = unionIconName.endsWith(suffix) ? unionIconName.chopped(suffix.size()) : unionIconName;
    return names.value(name, unionIconName);
}

static void translateFont(const FontPropertyGroup *font, DeclarationList &output)
{
    if (!font) {
        return;
    }

    if (auto family = font->family(); family && !family->isEmpty()) {
        output.set(u"font-family"_s, u"\"%1\""_s.arg(family.value()));
    }
    if (auto size = font->size(); size && size.value() > 0.0) {
        output.set(u"font-size"_s, toPx(size.value()));
    }
    if (auto weight = font->weight()) {
        output.set(u"font-weight"_s, QString::number(int(weight.value())));
    }
    if (auto style = font->style()) {
        output.set(u"font-style"_s, style.value() == QFont::StyleItalic ? u"italic"_s : style.value() == QFont::StyleOblique ? u"oblique"_s : u"normal"_s);
    }
    if (auto capitalization = font->capitalization()) {
        switch (capitalization.value()) {
        case QFont::AllUppercase:
            output.set(u"text-transform"_s, u"uppercase"_s);
            break;
        case QFont::AllLowercase:
            output.set(u"text-transform"_s, u"lowercase"_s);
            break;
        case QFont::Capitalize:
            output.set(u"text-transform"_s, u"capitalize"_s);
            break;
        case QFont::SmallCaps:
            output.set(u"font-variant-caps"_s, u"small-caps"_s);
            break;
        case QFont::MixedCase:
            output.set(u"text-transform"_s, u"none"_s);
            break;
        }
    }
    if (auto decorations = font->decorations()) {
        QStringList lines;
        if (decorations->testFlag(TextDecoration::Underline)) {
            lines.append(u"underline"_s);
        }
        if (decorations->testFlag(TextDecoration::Overline)) {
            lines.append(u"overline"_s);
        }
        if (decorations->testFlag(TextDecoration::StrikeThrough)) {
            lines.append(u"line-through"_s);
        }
        output.set(u"text-decoration-line"_s, lines.isEmpty() ? u"none"_s : lines.join(u' '));
    }
    if (auto spacing = font->letterSpacing()) {
        output.set(u"letter-spacing"_s, toPx(spacing.value()));
    }
}

QSet<QString> featureProperties(Features features)
{
    static const QList<std::pair<Feature, QStringList>> properties = {
        {Feature::Background, {u"background-color"_s, u"background-image"_s, u"background-repeat"_s, u"background-position"_s}},
        {Feature::Border, {u"border-style"_s, u"border-width"_s, u"border-color"_s, u"box-shadow"_s}},
        {Feature::Corners, {u"border-radius"_s}},
        {Feature::TextColor, {u"color"_s}},
        {Feature::Font,
         {u"font-family"_s,
          u"font-size"_s,
          u"font-weight"_s,
          u"font-style"_s,
          u"font-variant-caps"_s,
          u"text-transform"_s,
          u"text-decoration-line"_s,
          u"letter-spacing"_s}},
        {Feature::Padding, {u"padding"_s}},
        {Feature::Margin, {u"margin"_s}},
        {Feature::MinWidth, {u"min-width"_s}},
        {Feature::MinHeight, {u"min-height"_s}},
        {Feature::Shadow, {u"box-shadow"_s}},
        {Feature::Outline, {u"outline-style"_s, u"outline-width"_s, u"outline-color"_s, u"outline-offset"_s, u"-gtk-outline-radius"_s}},
        {Feature::IconColor, {u"color"_s}},
        {Feature::IconSize, {u"-gtk-icon-size"_s}},
        {Feature::IconSource, {u"-gtk-icon-source"_s, u"color"_s}},
        {Feature::Opacity, {u"opacity"_s}},
        {Feature::Spacing, {u"border-spacing"_s}},
    };

    QSet<QString> result;
    for (const auto &entry : properties) {
        if (features.testFlag(entry.first)) {
            result.unite(QSet(entry.second.begin(), entry.second.end()));
        }
    }
    return result;
}

static DeclarationList translateProperties(const StylePropertyGroup *properties, const TranslateOptions &options)
{
    DeclarationList output;
    if (!properties) {
        return output;
    }

    const auto features = options.features;

    const auto background = properties->background();
    std::optional<QString> backgroundImage;
    if (background && features.testFlag(Feature::Background)) {
        if (auto color = background->color()) {
            output.set(u"background-color"_s, toColorExpression(color).value_or(u"transparent"_s));
        }
    }

    // Masked images turn into symbolic images so GTK can recolour them
    std::optional<QString> recolor;
    if (background && background->image()) {
        const auto image = background->image();
        if (auto source = image->source(); source && !source->empty()) {
            const auto flags = image->flags().value_or(ImageFlags{});
            if (!flags.testAnyFlags(ImageFlag::Mask | ImageFlag::InvertedMask)) {
                backgroundImage = options.assets->addImage(source.value());
            } else if (features.testFlag(Feature::IconSource)) {
                recolor = toColorExpression(image->maskColor());
                backgroundImage = options.assets->addSymbolicImage(source.value());
            }
        }
    }

    if (features.testFlag(Feature::IconSource)) {
        const auto icon = properties->icon();
        const auto iconName = icon ? icon->name().value_or(QString{}) : QString{};
        if (backgroundImage && recolor) {
            output.set(u"-gtk-icon-source"_s, u"-gtk-recolor(url(\"%1\"))"_s.arg(backgroundImage.value()));
            output.set(u"color"_s, recolor.value());
        } else if (backgroundImage) {
            output.set(u"-gtk-icon-source"_s, u"url(\"%1\")"_s.arg(backgroundImage.value()));
        } else if (!iconName.isEmpty() && iconName != u"empty") {
            output.set(u"-gtk-icon-source"_s, u"-gtk-icontheme(\"%1\")"_s.arg(toGtkIconName(iconName)));
        } else if (icon && icon->source() && icon->source()->isLocalFile()) {
            if (auto path = options.assets->addImage(icon->source()->toLocalFile().toStdString())) {
                output.set(u"-gtk-icon-source"_s, u"url(\"%1\")"_s.arg(path.value()));
            }
        } else {
            output.set(u"-gtk-icon-source"_s, u"none"_s);
        }
    } else if (backgroundImage && features.testFlag(Feature::Background)) {
        output.set(u"background-image"_s, u"url(\"%1\")"_s.arg(backgroundImage.value()));
        output.set(u"background-repeat"_s, u"no-repeat"_s);
        output.set(u"background-position"_s, u"center"_s);
    }

    const auto lines = resolveLines(properties->border());
    std::array<qreal, 4> realBorder{};
    QStringList shadows;
    bool hasShadowValue = false;

    if (features.testFlag(Feature::Shadow) && properties->shadow()) {
        hasShadowValue = true;
        if (auto shadow = dropShadow(properties->shadow())) {
            shadows.append(shadow.value());
        }
    }

    if (features.testFlag(Feature::Border) && properties->border()) {
        if (options.borderMode == BorderMode::Real) {
            QStringList colors;
            bool anyVisible = false;
            for (int side = Top; side <= Left; ++side) {
                const auto &line = lines[side];
                realBorder[side] = line.visible() ? line.width : 0.0;
                colors.append(line.visible() ? line.expression : u"transparent"_s);
                anyVisible = anyVisible || line.visible();
            }
            output.set(u"border-style"_s, anyVisible ? u"solid"_s : u"none"_s);
            output.set(u"border-width"_s, pxShorthand(realBorder));
            if (anyVisible) {
                output.set(u"border-color"_s, shorthand(colors[Top], colors[Right], colors[Bottom], colors[Left]));
            }
        } else {
            hasShadowValue = true;
            const bool anyVisible = std::any_of(lines.begin(), lines.end(), [](const Line &line) {
                return line.visible();
            });
            if (anyVisible) {
                shadows.append(insetShadows(lines));
            }
        }
    }

    if (hasShadowValue) {
        output.set(u"box-shadow"_s, shadows.isEmpty() ? u"none"_s : shadows.join(u", "_s));
    }

    if (features.testFlag(Feature::Corners) && properties->corners()) {
        const auto radii = properties->corners()->radii();
        output.set(u"border-radius"_s, shorthand(toPx(radii.topLeft), toPx(radii.topRight), toPx(radii.bottomRight), toPx(radii.bottomLeft)));
    }

    // GTK only has one outline for every side
    if (features.testFlag(Feature::Outline) && properties->outline()) {
        const auto outlines = resolveLines(properties->outline());
        const auto outline = std::find_if(outlines.begin(), outlines.end(), [](const Line &line) {
            return line.visible();
        });
        if (outline != outlines.end()) {
            output.set(u"outline-style"_s, u"solid"_s);
            output.set(u"outline-width"_s, toPx(outline->width));
            output.set(u"outline-color"_s, outline->expression);
            output.set(u"outline-offset"_s, u"0"_s);

            // GTK 4 rounds the outline like the border but GTK 3 needs its radius, on the outside of the outline
            if (options.version == GtkVersion::Gtk3 && properties->corners()) {
                const auto radii = properties->corners()->radii();
                const auto width = outline->width;
                output.set(
                    u"-gtk-outline-radius"_s,
                    shorthand(toPx(radii.topLeft + width), toPx(radii.topRight + width), toPx(radii.bottomRight + width), toPx(radii.bottomLeft + width)));
            }
        } else {
            output.set(u"outline-width"_s, u"0"_s);
        }
    }

    // "color" covers both text and symbolic icons in GTK
    const auto text = properties->text();
    bool hasColor = false;
    if (text && features.testFlag(Feature::TextColor)) {
        if (auto color = toColorExpression(text->color())) {
            output.set(u"color"_s, color.value());
            hasColor = true;
        }
    }

    const auto icon = properties->icon();
    if (icon && features.testFlag(Feature::IconColor) && !hasColor) {
        if (auto color = toColorExpression(icon->color())) {
            output.set(u"color"_s, color.value());
        }
    }

    if (text && features.testFlag(Feature::Font)) {
        translateFont(text->font(), output);
    }

    if (icon && features.testFlag(Feature::IconSize)) {
        if (auto width = icon->width(); width && width.value() > 0.0) {
            output.set(u"-gtk-icon-size"_s, toPx(width.value()));
        }
    }

    if (features.testFlag(Feature::Opacity) && properties->display()) {
        if (auto opacity = properties->display()->opacity()) {
            output.set(u"opacity"_s, QString::number(opacity.value(), 'g', 3));
        }
    }

    // Union measures padding & inset different from GTK. The size is the background, insets go outside it
    // and padding starts at the outer edge. GTK puts margin, border and padding outside the min size
    //   margin = inset
    //   padding = padding - inset - border
    //   min size = size - border - padding on both sides
    const auto layout = properties->layout();
    if (!layout) {
        return output;
    }

    const bool useMargin = features.testFlag(Feature::Margin);
    const bool usePadding = features.testFlag(Feature::Padding);
    const auto padding = sizeValues(layout->padding());
    const auto inset = useMargin ? sizeValues(layout->inset()) : std::array<std::optional<qreal>, 4>{};

    std::array<qreal, 4> edges{};
    std::array<qreal, 4> gtkPadding{};
    std::array<qreal, 4> gtkMargin{};
    bool hasPadding = false;
    bool hasMargin = false;
    for (int side = Top; side <= Left; ++side) {
        const auto sideInset = inset[side].value_or(0.0);
        const auto innerEdge = sideInset + realBorder[side];
        hasPadding = hasPadding || padding[side].has_value();
        hasMargin = hasMargin || inset[side].has_value();
        gtkPadding[side] = std::max(0.0, padding[side].value_or(0.0) - innerEdge);
        edges[side] = realBorder[side] + (usePadding ? gtkPadding[side] : 0.0);
        gtkMargin[side] = sideInset;
    }

    if (usePadding && hasPadding) {
        output.set(u"padding"_s, pxShorthand(gtkPadding));
    }

    if (useMargin && hasMargin) {
        output.set(u"margin"_s, pxShorthand(gtkMargin));
    }

    if (features.testFlag(Feature::MinWidth)) {
        if (auto width = layout->width()) {
            output.set(u"min-width"_s, toPx(std::max(0.0, width.value() - edges[Left] - edges[Right])));
        }
    }

    if (features.testFlag(Feature::MinHeight)) {
        if (auto height = layout->height()) {
            output.set(u"min-height"_s, toPx(std::max(0.0, height.value() - edges[Top] - edges[Bottom])));
        }
    }

    if (features.testFlag(Feature::Spacing)) {
        if (auto spacing = layout->spacing()) {
            output.set(u"border-spacing"_s, toPx(spacing.value()));
        }
    }

    return output;
}

static bool isSupported(const QString &property, GtkVersion version)
{
    static const QSet gtk4Only = {
        u"-gtk-icon-size"_s,
        u"border-spacing"_s,
        u"font-variant-caps"_s,
        u"text-transform"_s,
    };
    return version == GtkVersion::Gtk4 || !gtk4Only.contains(property);
}

DeclarationList translate(const StylePropertyGroup *properties, const TranslateOptions &options)
{
    auto output = translateProperties(properties, options);
    for (const auto &entry : output.entries()) {
        if (!isSupported(entry.property, options.version)) {
            output.remove(entry.property);
        }
    }
    return output;
}
}
