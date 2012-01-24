/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <qtest.h>
#include <QDebug>
#include <QtDeclarative/qdeclarativeengine.h>
#include <QtDeclarative/qdeclarativecomponent.h>
#include <QtQuick1/private/qdeclarativesystempalette_p.h>
#include <qpalette.h>

class tst_qdeclarativesystempalette : public QObject
{
    Q_OBJECT
public:
    tst_qdeclarativesystempalette();

private slots:
    void activePalette();
    void inactivePalette();
    void disabledPalette();
    void paletteChanged();

private:
    QDeclarativeEngine engine;
};

tst_qdeclarativesystempalette::tst_qdeclarativesystempalette()
{
}

void tst_qdeclarativesystempalette::activePalette()
{
    QString componentStr = "import QtQuick 1.0\nSystemPalette { }";
    QDeclarativeComponent component(&engine);
    component.setData(componentStr.toLatin1(), QUrl::fromLocalFile(""));
    QDeclarative1SystemPalette *object = qobject_cast<QDeclarative1SystemPalette*>(component.create());

    QVERIFY(object != 0);

    QPalette palette;
    palette.setCurrentColorGroup(QPalette::Active);
    QCOMPARE(palette.window().color(), object->window());
    QCOMPARE(palette.windowText().color(), object->windowText());
    QCOMPARE(palette.base().color(), object->base());
    QCOMPARE(palette.text().color(), object->text());
    QCOMPARE(palette.alternateBase().color(), object->alternateBase());
    QCOMPARE(palette.button().color(), object->button());
    QCOMPARE(palette.buttonText().color(), object->buttonText());
    QCOMPARE(palette.light().color(), object->light());
    QCOMPARE(palette.midlight().color(), object->midlight());
    QCOMPARE(palette.dark().color(), object->dark());
    QCOMPARE(palette.mid().color(), object->mid());
    QCOMPARE(palette.shadow().color(), object->shadow());
    QCOMPARE(palette.highlight().color(), object->highlight());
    QCOMPARE(palette.highlightedText().color(), object->highlightedText());

    delete object;
}

void tst_qdeclarativesystempalette::inactivePalette()
{
    QString componentStr = "import QtQuick 1.0\nSystemPalette { colorGroup: SystemPalette.Inactive }";
    QDeclarativeComponent component(&engine);
    component.setData(componentStr.toLatin1(), QUrl::fromLocalFile(""));
    QDeclarative1SystemPalette *object = qobject_cast<QDeclarative1SystemPalette*>(component.create());

    QVERIFY(object != 0);
    QVERIFY(object->colorGroup() == QDeclarative1SystemPalette::Inactive);

    QPalette palette;
    palette.setCurrentColorGroup(QPalette::Inactive);
    QCOMPARE(palette.window().color(), object->window());
    QCOMPARE(palette.windowText().color(), object->windowText());
    QCOMPARE(palette.base().color(), object->base());
    QCOMPARE(palette.text().color(), object->text());
    QCOMPARE(palette.alternateBase().color(), object->alternateBase());
    QCOMPARE(palette.button().color(), object->button());
    QCOMPARE(palette.buttonText().color(), object->buttonText());
    QCOMPARE(palette.light().color(), object->light());
    QCOMPARE(palette.midlight().color(), object->midlight());
    QCOMPARE(palette.dark().color(), object->dark());
    QCOMPARE(palette.mid().color(), object->mid());
    QCOMPARE(palette.shadow().color(), object->shadow());
    QCOMPARE(palette.highlight().color(), object->highlight());
    QCOMPARE(palette.highlightedText().color(), object->highlightedText());

    delete object;
}

void tst_qdeclarativesystempalette::disabledPalette()
{
    QString componentStr = "import QtQuick 1.0\nSystemPalette { colorGroup: SystemPalette.Disabled }";
    QDeclarativeComponent component(&engine);
    component.setData(componentStr.toLatin1(), QUrl::fromLocalFile(""));
    QDeclarative1SystemPalette *object = qobject_cast<QDeclarative1SystemPalette*>(component.create());

    QVERIFY(object != 0);
    QVERIFY(object->colorGroup() == QDeclarative1SystemPalette::Disabled);

    QPalette palette;
    palette.setCurrentColorGroup(QPalette::Disabled);
    QCOMPARE(palette.window().color(), object->window());
    QCOMPARE(palette.windowText().color(), object->windowText());
    QCOMPARE(palette.base().color(), object->base());
    QCOMPARE(palette.text().color(), object->text());
    QCOMPARE(palette.alternateBase().color(), object->alternateBase());
    QCOMPARE(palette.button().color(), object->button());
    QCOMPARE(palette.buttonText().color(), object->buttonText());
    QCOMPARE(palette.light().color(), object->light());
    QCOMPARE(palette.midlight().color(), object->midlight());
    QCOMPARE(palette.dark().color(), object->dark());
    QCOMPARE(palette.mid().color(), object->mid());
    QCOMPARE(palette.shadow().color(), object->shadow());
    QCOMPARE(palette.highlight().color(), object->highlight());
    QCOMPARE(palette.highlightedText().color(), object->highlightedText());

    delete object;
}

void tst_qdeclarativesystempalette::paletteChanged()
{
    QString componentStr = "import QtQuick 1.0\nSystemPalette { }";
    QDeclarativeComponent component(&engine);
    component.setData(componentStr.toLatin1(), QUrl::fromLocalFile(""));
    QDeclarative1SystemPalette *object = qobject_cast<QDeclarative1SystemPalette*>(component.create());

    QVERIFY(object != 0);

    QPalette p;
    p.setCurrentColorGroup(QPalette::Active);
    p.setColor(QPalette::Active, QPalette::Text, QColor("red"));
    p.setColor(QPalette::Active, QPalette::ButtonText, QColor("green"));
    p.setColor(QPalette::Active, QPalette::WindowText, QColor("blue"));

    qApp->setPalette(p);

    object->setColorGroup(QDeclarative1SystemPalette::Active);
    QTRY_COMPARE(QColor("red"), object->text());
    QTRY_COMPARE(QColor("green"), object->buttonText());
    QTRY_COMPARE(QColor("blue"), object->windowText());

    delete object;
}

QTEST_MAIN(tst_qdeclarativesystempalette)

#include "tst_qdeclarativesystempalette.moc"
