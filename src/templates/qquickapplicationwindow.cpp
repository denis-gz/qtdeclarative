/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Labs Templates module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qquickapplicationwindow_p.h"
#include "qquickoverlay_p.h"
#include "qquickcontrol_p_p.h"

#include <QtCore/private/qobject_p.h>
#include <QtQuick/private/qquickitem_p.h>
#include <QtQuick/private/qquickitemchangelistener_p.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype ApplicationWindow
    \inherits Window
    \instantiates QQuickApplicationWindow
    \inqmlmodule Qt.labs.controls
    \ingroup qtlabscontrols-containers
    \brief Provides a top-level application window.

    ApplicationWindow is a \l Window which makes it convenient to add
    a \l header and \l footer item to the window.

    \image qtlabscontrols-applicationwindow-wireframe.png

    \note By default, an ApplicationWindow is not visible.

    \sa {Container Controls}
*/

class QQuickApplicationWindowPrivate : public QQuickItemChangeListener
{
    Q_DECLARE_PUBLIC(QQuickApplicationWindow)

public:
    QQuickApplicationWindowPrivate()
        : complete(false)
        , contentItem(Q_NULLPTR)
        , header(Q_NULLPTR)
        , footer(Q_NULLPTR)
        , overlay(Q_NULLPTR)
    { }

    void relayout();

    void itemImplicitWidthChanged(QQuickItem *item) Q_DECL_OVERRIDE;
    void itemImplicitHeightChanged(QQuickItem *item) Q_DECL_OVERRIDE;

    void updateFont(const QFont &);
    inline void setFont_helper(const QFont &f) {
        if (font.resolve() == f.resolve() && font == f)
            return;
        updateFont(f);
    }
    void resolveFont();

    bool complete;
    QQuickItem *contentItem;
    QQuickItem *header;
    QQuickItem *footer;
    QQuickOverlay *overlay;
    QFont font;
    QQuickApplicationWindow *q_ptr;
};

void QQuickApplicationWindowPrivate::relayout()
{
    Q_Q(QQuickApplicationWindow);
    QQuickItem *content = q->contentItem();
    qreal hh = header ? header->height() : 0;
    qreal fh = footer ? footer->height() : 0;

    content->setY(hh);
    content->setWidth(q->width());
    content->setHeight(q->height() - hh - fh);

    if (overlay) {
        overlay->setWidth(q->width());
        overlay->setHeight(q->height());
        overlay->stackAfter(content);
    }

    if (header) {
        header->setY(-hh);
        QQuickItemPrivate *p = QQuickItemPrivate::get(header);
        if (!p->widthValid) {
            header->setWidth(q->width());
            p->widthValid = false;
        }
    }

    if (footer) {
        footer->setY(content->height());
        QQuickItemPrivate *p = QQuickItemPrivate::get(footer);
        if (!p->widthValid) {
            footer->setWidth(q->width());
            p->widthValid = false;
        }
    }
}

void QQuickApplicationWindowPrivate::itemImplicitWidthChanged(QQuickItem *item)
{
    Q_UNUSED(item);
    relayout();
}

void QQuickApplicationWindowPrivate::itemImplicitHeightChanged(QQuickItem *item)
{
    Q_UNUSED(item);
    relayout();
}

QQuickApplicationWindow::QQuickApplicationWindow(QWindow *parent) :
    QQuickWindowQmlImpl(parent), d_ptr(new QQuickApplicationWindowPrivate)
{
    d_ptr->q_ptr = this;
}

QQuickApplicationWindow::~QQuickApplicationWindow()
{
    Q_D(QQuickApplicationWindow);
    if (d->header)
        QQuickItemPrivate::get(d->header)->removeItemChangeListener(d, QQuickItemPrivate::ImplicitWidth | QQuickItemPrivate::ImplicitHeight);
    if (d->footer)
        QQuickItemPrivate::get(d->footer)->removeItemChangeListener(d, QQuickItemPrivate::ImplicitWidth | QQuickItemPrivate::ImplicitHeight);
}

/*!
    \qmlproperty Item Qt.labs.controls::ApplicationWindow::header

    A header item for the window, for example a title bar, menu or tool-bar.
    By default this property is empty, no header will be shown.

    \sa footer
*/
QQuickItem *QQuickApplicationWindow::header() const
{
    Q_D(const QQuickApplicationWindow);
    return d->header;
}

void QQuickApplicationWindow::setHeader(QQuickItem *header)
{
    Q_D(QQuickApplicationWindow);
    if (d->header != header) {
        delete d->header;
        d->header = header;
        if (header) {
            header->setParentItem(contentItem());
            QQuickItemPrivate *p = QQuickItemPrivate::get(header);
            p->addItemChangeListener(d, QQuickItemPrivate::ImplicitWidth | QQuickItemPrivate::ImplicitHeight);
            if (qFuzzyIsNull(header->z()))
                header->setZ(1);
            if (isComponentComplete())
                d->relayout();
        }
        emit headerChanged();
    }
}

/*!
    \qmlproperty Item Qt.labs.controls::ApplicationWindow::footer

    A footer item for the window, for example a status bar or menu.
    By default this property is empty, no footer will be shown.

    \sa header
*/
QQuickItem *QQuickApplicationWindow::footer() const
{
    Q_D(const QQuickApplicationWindow);
    return d->footer;
}

void QQuickApplicationWindow::setFooter(QQuickItem *footer)
{
    Q_D(QQuickApplicationWindow);
    if (d->footer != footer) {
        delete d->footer;
        d->footer = footer;
        if (footer) {
            footer->setParentItem(contentItem());
            QQuickItemPrivate *p = QQuickItemPrivate::get(footer);
            p->addItemChangeListener(d, QQuickItemPrivate::ImplicitWidth | QQuickItemPrivate::ImplicitHeight);
            if (qFuzzyIsNull(footer->z()))
                footer->setZ(1);
            if (isComponentComplete())
                d->relayout();
        }
        emit footerChanged();
    }
}

QQmlListProperty<QObject> QQuickApplicationWindow::contentData()
{
    return QQuickItemPrivate::get(contentItem())->data();
}

QQuickItem *QQuickApplicationWindow::contentItem() const
{
    QQuickApplicationWindowPrivate *d = const_cast<QQuickApplicationWindowPrivate *>(d_func());
    if (!d->contentItem) {
        d->contentItem = new QQuickItem(QQuickWindow::contentItem());
        d->relayout();
    }
    return d->contentItem;
}

QQuickItem *QQuickApplicationWindow::overlay() const
{
    QQuickApplicationWindowPrivate *d = const_cast<QQuickApplicationWindowPrivate *>(d_func());
    if (!d->overlay) {
        d->overlay = new QQuickOverlay(QQuickWindow::contentItem());
        d->relayout();
    }
    return d->overlay;
}

/*!
    \qmlproperty font Qt.labs.controls::ApplicationWindow::font

    This property holds the font currently set for the window.
*/
QFont QQuickApplicationWindow::font() const
{
    Q_D(const QQuickApplicationWindow);
    return d->font;
}

void QQuickApplicationWindow::setFont(const QFont &f)
{
    Q_D(QQuickApplicationWindow);
    if (d->font == f)
        return;

    QFont resolvedFont = f.resolve(QQuickControlPrivate::themeFont(QPlatformTheme::SystemFont));
    d->setFont_helper(resolvedFont);
}

void QQuickApplicationWindow::resetFont()
{
    setFont(QFont());
}

void QQuickApplicationWindowPrivate::resolveFont()
{
    QFont resolvedFont = font.resolve(QQuickControlPrivate::themeFont(QPlatformTheme::SystemFont));
    setFont_helper(resolvedFont);
}

void QQuickApplicationWindowPrivate::updateFont(const QFont &f)
{
    Q_Q(QQuickApplicationWindow);
    font = f;

    QQuickControlPrivate::updateFontRecur(q->contentItem(), f);

    emit q->fontChanged();
}

QQuickApplicationWindowAttached *QQuickApplicationWindow::qmlAttachedProperties(QObject *object)
{
    return new QQuickApplicationWindowAttached(object);
}

bool QQuickApplicationWindow::isComponentComplete() const
{
    Q_D(const QQuickApplicationWindow);
    return d->complete;
}

void QQuickApplicationWindow::componentComplete()
{
    Q_D(QQuickApplicationWindow);
    d->complete = true;
    QQuickWindowQmlImpl::componentComplete();
}

void QQuickApplicationWindow::resizeEvent(QResizeEvent *event)
{
    Q_D(QQuickApplicationWindow);
    QQuickWindowQmlImpl::resizeEvent(event);
    d->relayout();
}

class QQuickApplicationWindowAttachedPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QQuickApplicationWindowAttached)

public:
    QQuickApplicationWindowAttachedPrivate() : window(Q_NULLPTR) { }

    void windowChange(QQuickWindow *wnd);

    QQuickApplicationWindow *window;
};

void QQuickApplicationWindowAttachedPrivate::windowChange(QQuickWindow *wnd)
{
    Q_Q(QQuickApplicationWindowAttached);
    QQuickApplicationWindow *newWindow = qobject_cast<QQuickApplicationWindow *>(wnd);
    if (window != newWindow) {
        QQuickApplicationWindow *oldWindow = window;
        if (oldWindow) {
            QObject::disconnect(oldWindow, &QQuickApplicationWindow::activeFocusItemChanged,
                                q, &QQuickApplicationWindowAttached::activeFocusItemChanged);
            QObject::disconnect(oldWindow, &QQuickApplicationWindow::headerChanged,
                                q, &QQuickApplicationWindowAttached::headerChanged);
            QObject::disconnect(oldWindow, &QQuickApplicationWindow::footerChanged,
                                q, &QQuickApplicationWindowAttached::footerChanged);
        }
        if (newWindow) {
            QObject::connect(newWindow, &QQuickApplicationWindow::activeFocusItemChanged,
                             q, &QQuickApplicationWindowAttached::activeFocusItemChanged);
            QObject::connect(newWindow, &QQuickApplicationWindow::headerChanged,
                             q, &QQuickApplicationWindowAttached::headerChanged);
            QObject::connect(newWindow, &QQuickApplicationWindow::footerChanged,
                             q, &QQuickApplicationWindowAttached::footerChanged);
        }

        window = newWindow;
        emit q->windowChanged();
        emit q->contentItemChanged();
        emit q->overlayChanged();

        if ((oldWindow && oldWindow->activeFocusItem()) || (newWindow && newWindow->activeFocusItem()))
            emit q->activeFocusItemChanged();
        if ((oldWindow && oldWindow->header()) || (newWindow && newWindow->header()))
            emit q->headerChanged();
        if ((oldWindow && oldWindow->footer()) || (newWindow && newWindow->footer()))
            emit q->footerChanged();
    }
}

QQuickApplicationWindowAttached::QQuickApplicationWindowAttached(QObject *parent)
    : QObject(*(new QQuickApplicationWindowAttachedPrivate), parent)
{
    Q_D(QQuickApplicationWindowAttached);
    QQuickItem *item = qobject_cast<QQuickItem *>(parent);
    if (item) {
        d->windowChange(item->window());
        QObjectPrivate::connect(item, &QQuickItem::windowChanged, d, &QQuickApplicationWindowAttachedPrivate::windowChange);
    }
}

/*!
    \qmlattachedproperty ApplicationWindow Qt.labs.controls::ApplicationWindow::window

    This attached property holds the application window. The property can be attached
    to any item. The value is \c null if the item is not in an ApplicationWindow.
*/
QQuickApplicationWindow *QQuickApplicationWindowAttached::window() const
{
    Q_D(const QQuickApplicationWindowAttached);
    return d->window;
}

/*!
    \qmlattachedproperty Item Qt.labs.controls::ApplicationWindow::contentItem

    This attached property holds the window content item. The property can be attached
    to any item. The value is \c null if the item is not in an ApplicationWindow.
*/
QQuickItem *QQuickApplicationWindowAttached::contentItem() const
{
    Q_D(const QQuickApplicationWindowAttached);
    return d->window ? d->window->contentItem() : Q_NULLPTR;
}

/*!
    \qmlattachedproperty Item Qt.labs.controls::ApplicationWindow::activeFocusItem

    This attached property holds the active focus item. The property can be attached
    to any item. The value is \c null if the item is not in an ApplicationWindow, or
    the window has no active focus.

    \sa Window::activeFocusItem
*/
QQuickItem *QQuickApplicationWindowAttached::activeFocusItem() const
{
    Q_D(const QQuickApplicationWindowAttached);
    return d->window ? d->window->activeFocusItem() : Q_NULLPTR;
}

/*!
    \qmlattachedproperty Item Qt.labs.controls::ApplicationWindow::header

    This attached property holds the window header item. The property can be attached
    to any item. The value is \c null if the item is not in an ApplicationWindow, or
    the window has no header item.
*/
QQuickItem *QQuickApplicationWindowAttached::header() const
{
    Q_D(const QQuickApplicationWindowAttached);
    return d->window ? d->window->header() : Q_NULLPTR;
}

/*!
    \qmlattachedproperty Item Qt.labs.controls::ApplicationWindow::footer

    This attached property holds the window footer item. The property can be attached
    to any item. The value is \c null if the item is not in an ApplicationWindow, or
    the window has no footer item.
*/
QQuickItem *QQuickApplicationWindowAttached::footer() const
{
    Q_D(const QQuickApplicationWindowAttached);
    return d->window ? d->window->footer() : Q_NULLPTR;
}

/*!
    \qmlattachedproperty Item Qt.labs.controls::ApplicationWindow::overlay

    This attached property holds the window overlay item. The property can be attached
    to any item. The value is \c null if the item is not in an ApplicationWindow.
*/
QQuickItem *QQuickApplicationWindowAttached::overlay() const
{
    Q_D(const QQuickApplicationWindowAttached);
    return d->window ? d->window->overlay() : Q_NULLPTR;
}

QT_END_NAMESPACE
