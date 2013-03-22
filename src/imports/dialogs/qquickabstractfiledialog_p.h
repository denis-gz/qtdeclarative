/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtQuick.Dialogs module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QQUICKABSTRACTFILEDIALOG_P_H
#define QQUICKABSTRACTFILEDIALOG_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtQml>
#include <QQuickView>
#include <QtGui/qpa/qplatformdialoghelper.h>
#include <qpa/qplatformtheme.h>
#include "qquickabstractdialog_p.h"

QT_BEGIN_NAMESPACE

class QQuickAbstractFileDialog : public QQuickAbstractDialog
{
    Q_OBJECT
    Q_PROPERTY(bool selectExisting READ selectExisting WRITE setSelectExisting NOTIFY fileModeChanged)
    Q_PROPERTY(bool selectMultiple READ selectMultiple WRITE setSelectMultiple NOTIFY fileModeChanged)
    Q_PROPERTY(bool selectFolder READ selectFolder WRITE setSelectFolder NOTIFY fileModeChanged)
    Q_PROPERTY(QString folder READ folder WRITE setFolder NOTIFY folderChanged)
    Q_PROPERTY(QStringList nameFilters READ nameFilters WRITE setNameFilters NOTIFY nameFiltersChanged)
    Q_PROPERTY(QString selectedNameFilter READ selectedNameFilter WRITE selectNameFilter NOTIFY filterSelected)
    Q_PROPERTY(QUrl fileUrl READ fileUrl NOTIFY selectionAccepted)
    Q_PROPERTY(QList<QUrl> fileUrls READ fileUrls NOTIFY selectionAccepted)

public:
    QQuickAbstractFileDialog(QObject *parent = 0);
    virtual ~QQuickAbstractFileDialog();

    virtual QString title() const;
    bool selectExisting() const { return m_selectExisting; }
    bool selectMultiple() const { return m_selectMultiple; }
    bool selectFolder() const { return m_selectFolder; }
    QString folder();
    QStringList nameFilters() const { return m_options->nameFilters(); }
    QString selectedNameFilter();
    QUrl fileUrl();
    virtual QList<QUrl> fileUrls();

public Q_SLOTS:
    void setVisible(bool v);
    void setTitle(QString t);
    void setSelectExisting(bool s);
    void setSelectMultiple(bool s);
    void setSelectFolder(bool s);
    void setFolder(QString f);
    void setNameFilters(const QStringList &f);
    void selectNameFilter(QString f);

Q_SIGNALS:
    void folderChanged();
    void nameFiltersChanged();
    void filterSelected();
    void fileModeChanged();
    void selectionAccepted();

protected:
    void updateModes();

protected:
    QPlatformFileDialogHelper *m_dlgHelper;
    QSharedPointer<QFileDialogOptions> m_options;
    bool m_selectExisting;
    bool m_selectMultiple;
    bool m_selectFolder;

    Q_DISABLE_COPY(QQuickAbstractFileDialog)
};

QT_END_NAMESPACE

#endif // QQUICKABSTRACTFILEDIALOG_P_H
