// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#include "DragEventFilter.h"

#include <QDialog>
#include <QDockWidget>
#include <QMainWindow>
#include <QMenuBar>
#include <QMouseEvent>
#include <QToolBar>
#include <QWindow>

DragEventFilter::DragEventFilter(QObject *parent)
    : QObject(parent)
{
}

void DragEventFilter::installWidget(QWidget *widget)
{
    if (allowDrag(widget)) {
        widget->removeEventFilter(this);
        widget->installEventFilter(this);
    }
}

void DragEventFilter::uninstallWidget(QWidget *widget)
{
    if (widget) {
        widget->removeEventFilter(this);
    }
}

bool DragEventFilter::eventFilter(QObject *object, QEvent *event)
{
    switch (event->type()) {
    case QEvent::MouseButtonPress:
        // In case we are dragging mouse from ie. a toolbutton to draggable area, we are locked
        // so that the drag does not start immediately. User has to release the mouse
        // button and then click on area that can be dragged for the operation to start.
        m_dragLocked = true;
        break;
    case QEvent::MouseMove:
        return mouseMoveEvent(object, event);
    case QEvent::MouseButtonRelease:
        m_dragLocked = false;
        break;
    default:
        break;
    }
    return false;
}

bool DragEventFilter::mouseMoveEvent(QObject *object, [[maybe_unused]] QEvent *event)
{
    if (const auto widget = qobject_cast<QWidget *>(object)) {
        if (m_dragLocked && allowDrag(widget) && widget->window() && widget->window()->windowHandle()) {
            // We rely on startSystemMove to start and stop these drag actions properly
            widget->window()->windowHandle()->startSystemMove();
        }
        return true;
    }
    return false;
}

bool DragEventFilter::allowDrag(QWidget *widget)
{
    if (!widget) {
        return false;
    }

    // If we are already doing something like resizing a window, don't allow drag.
    if (widget->cursor().shape() != Qt::ArrowCursor) {
        return false;
    }

    // Accept menubars and toolbars
    if ((qobject_cast<QMenuBar *>(widget) || qobject_cast<QToolBar *>(widget))) {
        // However skip them if they're titlebars of a dockwidget
        if (auto dockWidget = qobject_cast<const QDockWidget *>(widget->parent())) {
            return widget != dockWidget->titleBarWidget();
        }
        return true;
    }

    return false;
}
