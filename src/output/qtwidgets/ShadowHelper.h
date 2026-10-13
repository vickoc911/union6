/*
 * SPDX-FileCopyrightText: 2014 Hugo Pereira Da Costa <hugo.pereira@free.fr>
 * SPDX-FileCopyrightText: 2020 Vlad Zahorodnii <vlad.zahorodnii@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "TileSet.h"

#include <KWindowShadow>

#include "properties/CornersPropertyGroup.h"
#include <QColor>
#include <QMap>
#include <QMargins>
#include <QObject>
#include <QPointer>
#include <QSet>

// Adjust the shadow so that its slightly under the widget to
// avoid any gaps. Take this into account when drawing the shadow.
constexpr int SHADOW_ADJUSTMENT = 2;

struct Shadow {
    Shadow() = default;

    Shadow(const QPoint &offset, qreal blur, qreal spread, const QColor &color)
        : offset(offset)
        , blur(blur)
        , spread(spread)
        , color(color)
    {
    }

    QPoint offset;
    qreal blur;
    qreal spread;
    QColor color;
    Union::Properties::CornersPropertyGroup::CornerRadii cornerRadius;
};

//* handle shadow pixmaps passed to window manager via X property
class ShadowHelper : public QObject
{
    Q_OBJECT

public:
    //* constructor
    explicit ShadowHelper();

    //* destructor
    ~ShadowHelper() override;

    //* shadow params from size enum
    static Shadow lookupShadowParams(QWidget *widget);

    //* reset
    void reset();

    //* register widget
    bool registerWidget(QWidget *, bool force = false);

    //* unregister widget
    void unregisterWidget(QWidget *);

    //* event filter
    bool eventFilter(QObject *, QEvent *) override;

    //* shadow tiles
    /** is public because it is also needed for mdi windows */
    TileSet shadowTiles(QWidget *);

protected Q_SLOTS:

    //* unregister widget
    void widgetDeleted(QObject *);

    //* unregister window
    void windowDeleted(QObject *);

protected:
    //* true if widget is a menu
    bool isMenu(QWidget *) const;

    //* true if widget is a tooltip
    bool isToolTip(QWidget *) const;

    //* dock widget
    bool isDockWidget(QWidget *) const;

    //* toolbar
    bool isToolBar(QWidget *) const;

    //* accept widget
    bool acceptWidget(QWidget *) const;

    // create shared shadow tiles from tileset
    const QVector<KWindowShadowTile::Ptr> &createShadowTiles();

    // create shadow tile from pixmap
    KWindowShadowTile::Ptr createTile(const QPixmap &);

    //* installs shadow on given widget in a platform independent way
    void installShadows(QWidget *);

    //* uninstalls shadow on given widget in a platform independent way
    void uninstallShadows(QWidget *);

    //* gets the shadow margins for the given widget
    QMargins shadowMargins(QWidget *) const;

private:
    //* registered widgets
    QSet<QWidget *> _widgets;

    //* managed shadows
    QMap<QWindow *, KWindowShadow *> _shadows;

    //* tileset
    TileSet _shadowTiles;

    //* number of tiles
    enum {
        numTiles = 8
    };

    //* shared shadow tiles
    QVector<KWindowShadowTile::Ptr> _tiles;
};
