// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Akseli Lahtinen <akselmo@akselmo.dev>

#include "DragEventFilter.h"

#include <QApplication>
#include <QDialog>
#include <QDockWidget>
#include <QMainWindow>
#include <QMenuBar>
#include <QMouseEvent>
#include <QToolBar>
#include <QWindow>

DragEventFilter::DragEventFilter(QObject *parent)
    : QObject(parent)
    , m_dragTimer(new QTimer(this))
    , m_clickPosition(QPointF())
    , m_currentPosition(QPointF())
    , m_currentState(DragState::Unlocked)
{
    m_dragTimer->setInterval(QApplication::startDragTime());
    m_dragTimer->setSingleShot(true);
    connect(m_dragTimer, &QTimer::timeout, this, &DragEventFilter::handleDrag);
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
        return mousePressEvent(object, event);
    case QEvent::MouseMove:
        return mouseMoveEvent(object, event);
    case QEvent::MouseButtonRelease:
        return mouseReleaseEvent(object, event);
    default:
        break;
    }
    return false;
}

bool DragEventFilter::mousePressEvent([[maybe_unused]] QObject *object, QEvent *event)
{
    // In case we are dragging mouse from ie. a toolbutton to draggable area, we are locked
    // so that the drag does not start immediately. User has to release the mouse
    // button and then click on area that can be dragged for the operation to start.
    if (const auto &mouseEvent = static_cast<QMouseEvent *>(event)) {
        m_currentState = DragState::Locked;
        m_clickPosition = mouseEvent->position();
        m_dragTimer->start();
    }
    return false;
}

bool DragEventFilter::mouseMoveEvent(QObject *object, QEvent *event)
{
    // We want to start dragging when either time is cleared or the distance is cleared,
    // like in Breeze QStyle
    switch (m_currentState) {
    case DragState::Locked:
    case DragState::TimeCleared:
        if (const auto &mouseEvent = static_cast<QMouseEvent *>(event)) {
            m_currentPosition = mouseEvent->position();
            // Check if we have moved over the threshold
            if ((m_clickPosition - m_currentPosition).manhattanLength() >= QApplication::startDragDistance()) {
                m_currentState = DragState::DistanceCleared;
            }
        }
        break;
    case DragState::DistanceCleared:
        if (const auto widget = qobject_cast<QWidget *>(object)) {
            if (allowDrag(widget) && widget->window() && widget->window()->windowHandle()) {
                // We rely on startSystemMove to start and stop these drag actions properly
                widget->window()->windowHandle()->startSystemMove();
            }
            return true;
        }
        break;
    case DragState::Unlocked:
        break;
    }

    return false;
}

bool DragEventFilter::mouseReleaseEvent([[maybe_unused]] QObject *object, [[maybe_unused]] QEvent *event)
{
    m_currentState = DragState::Unlocked;
    m_dragTimer->stop();
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

void DragEventFilter::handleDrag()
{
    if (m_currentState == DragState::Locked) {
        m_currentState = DragState::TimeCleared;
    }
}
