// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#pragma once

#include <QEvent>
#include <QObject>
#include <QTimer>
#include <QWidget>
#include <qpoint.h>

class DragEventFilter : public QObject
{
    Q_OBJECT
public:
    enum class DragState {
        Locked,
        TimeCleared,
        DistanceCleared,
        Unlocked
    };
    Q_ENUMS(DragState)

    explicit DragEventFilter(QObject *parent);

    void installWidget(QWidget *widget);
    void uninstallWidget(QWidget *widget);
    bool eventFilter(QObject *object, QEvent *event) override;

protected:
    bool mousePressEvent(QObject *object, QMouseEvent *event);
    bool mouseMoveEvent(QObject *object, QMouseEvent *event);
    bool mouseReleaseEvent(QObject *object, QMouseEvent *event);
    bool allowDrag(QWidget *widget);
    void handleDrag();

    QTimer *m_dragTimer;
    QPointF m_clickPosition;
    QPointF m_currentPosition;
    DragState m_currentState;
};
