/*
 * SPDX-FileCopyrightText: 2024 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#pragma once

#include <memory>
#include <optional>

#include <QObject>
#include <QVariant>

#include "Definition.h"
#include "Selector.h"
#include "properties/StyleProperty.h"

#include "union_export.h"

namespace Union
{

class StyleRulePrivate;

<<<<<<< HEAD
class StyleRuleInterface
{
public:
    virtual ~StyleRuleInterface()
    {
    }

    virtual QSizeF contentSize() const = 0;
    virtual QRectF boundingRect() const = 0;
    virtual QMarginsF borderSizes() const = 0;

    virtual std::optional<AreaDefinition> foreground() const = 0;
    virtual std::optional<AreaDefinition> background() const = 0;
    virtual std::optional<BorderDefinition> border() const = 0;
    virtual std::optional<CornersDefinition> corners() const = 0;
    virtual std::optional<ShadowDefinition> shadow() const = 0;
    virtual std::optional<BorderDefinition> outset() const = 0;
    virtual std::optional<SizeDefinition> margins() const = 0;
    virtual std::optional<SizeDefinition> padding() const = 0;
    virtual std::optional<TextDefinition> text() const = 0;
    virtual std::optional<IconDefinition> icon() const = 0;
};

=======
>>>>>>> 8bb3de3 (Remove now-obsolete code for old properties from StyleRule)
/**
 * A set of style properties that should be applied to a certain set of elements.
 *
 * This class defines a set of properties to apply to an element, along with a
 * list of selectors that should match for this style to apply.
 */
class UNION_EXPORT StyleRule : public QObject, public std::enable_shared_from_this<StyleRule>
{
    Q_OBJECT

public:
    using Ptr = std::shared_ptr<StyleRule>;

    StyleRule(std::unique_ptr<StyleRulePrivate> &&d);
    ~StyleRule() override;

    SelectorList selectors() const;
    void setSelectors(const SelectorList &selectors);

    const Properties::StyleProperty &properties() const;
    void setProperties(const Properties::StyleProperty &newProperties);

    static Ptr create();

private:
    const std::unique_ptr<StyleRulePrivate> d;
};

}

UNION_EXPORT QDebug operator<<(QDebug debug, std::shared_ptr<Union::StyleRule> style);
