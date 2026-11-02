// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#pragma once

#include <QEvent>
#include <QObject>
#include <QWidget>

class DragEventFilter : public QObject
{
    Q_OBJECT

public:
    explicit DragEventFilter(QObject *parent);

    void installWidget(QWidget *widget);
    void uninstallWidget(QWidget *widget);
    bool eventFilter(QObject *object, QEvent *event) override;

protected:
    bool mouseMoveEvent(QObject *object, QEvent *event);
    bool allowDrag(QWidget *widget);

    bool m_dragLocked = false;
};