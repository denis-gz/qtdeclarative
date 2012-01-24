/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the QtDeclarative module of the Qt Toolkit.
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

#include "QtQuick1/private/qdeclarativelistmodelworkeragent_p.h"
#include "QtQuick1/private/qdeclarativelistmodel_p_p.h"
#include <QtDeclarative/private/qdeclarativedata_p.h>
#include <QtDeclarative/private/qdeclarativeengine_p.h>
#include <QtDeclarative/qdeclarativeinfo.h>

#include <QtCore/qcoreevent.h>
#include <QtCore/qcoreapplication.h>
#include <QtCore/qdebug.h>


QT_BEGIN_NAMESPACE




void QDeclarative1ListModelWorkerAgent::Data::clearChange() 
{ 
    changes.clear(); 
}

void QDeclarative1ListModelWorkerAgent::Data::insertChange(int index, int count) 
{
    Change c = { Change::Inserted, index, count, 0, QList<int>() };
    changes << c;
}

void QDeclarative1ListModelWorkerAgent::Data::removeChange(int index, int count) 
{
    Change c = { Change::Removed, index, count, 0, QList<int>() };
    changes << c;
}

void QDeclarative1ListModelWorkerAgent::Data::moveChange(int index, int count, int to)
{
    Change c = { Change::Moved, index, count, to, QList<int>() };
    changes << c;
}

void QDeclarative1ListModelWorkerAgent::Data::changedChange(int index, int count, const QList<int> &roles)
{
    Change c = { Change::Changed, index, count, 0, roles };
    changes << c;
}

QDeclarative1ListModelWorkerAgent::QDeclarative1ListModelWorkerAgent(QDeclarative1ListModel *model)
    : m_engine(0), 
      m_ref(1), 
      m_orig(model), 
      m_copy(new QDeclarative1ListModel(model, this))
{
}

QDeclarative1ListModelWorkerAgent::~QDeclarative1ListModelWorkerAgent()
{
}

void QDeclarative1ListModelWorkerAgent::setScriptEngine(QScriptEngine *eng)
{
    m_engine = eng;
    if (m_copy->m_flat)
        m_copy->m_flat->m_scriptEngine = eng;
}

QScriptEngine *QDeclarative1ListModelWorkerAgent::scriptEngine() const
{
    return m_engine;
}

void QDeclarative1ListModelWorkerAgent::addref()
{
    m_ref.ref();
}

void QDeclarative1ListModelWorkerAgent::release()
{
    bool del = !m_ref.deref();

    if (del)
        delete this;
}

int QDeclarative1ListModelWorkerAgent::count() const
{
    return m_copy->count();
}

void QDeclarative1ListModelWorkerAgent::clear()
{
    data.clearChange();
    data.removeChange(0, m_copy->count());
    m_copy->clear();
}

void QDeclarative1ListModelWorkerAgent::remove(int index)
{
    int count = m_copy->count();
    m_copy->remove(index);

    if (m_copy->count() != count)
        data.removeChange(index, 1);
}

void QDeclarative1ListModelWorkerAgent::append(const QScriptValue &value)
{
    int count = m_copy->count();
    m_copy->append(value);

    if (m_copy->count() != count)
        data.insertChange(m_copy->count() - 1, 1);
}

void QDeclarative1ListModelWorkerAgent::insert(int index, const QScriptValue &value)
{
    int count = m_copy->count();
    m_copy->insert(index, value);

    if (m_copy->count() != count)
        data.insertChange(index, 1);
}

QScriptValue QDeclarative1ListModelWorkerAgent::get(int index) const
{
    return m_copy->get(index);
}

void QDeclarative1ListModelWorkerAgent::set(int index, const QScriptValue &value)
{
    QList<int> roles;
    m_copy->set(index, value, &roles);
    if (!roles.isEmpty())
        data.changedChange(index, 1, roles);
}

void QDeclarative1ListModelWorkerAgent::setProperty(int index, const QString& property, const QVariant& value)
{
    QList<int> roles;
    m_copy->setProperty(index, property, value, &roles);
    if (!roles.isEmpty())
        data.changedChange(index, 1, roles);
}

void QDeclarative1ListModelWorkerAgent::move(int from, int to, int count)
{
    m_copy->move(from, to, count);
    data.moveChange(from, to, count);
}

void QDeclarative1ListModelWorkerAgent::sync()
{
    Sync *s = new Sync;
    s->data = data;
    s->list = m_copy;
    data.changes.clear();

    mutex.lock();
    QCoreApplication::postEvent(this, s);
    syncDone.wait(&mutex);
    mutex.unlock();
}

void QDeclarative1ListModelWorkerAgent::changedData(int index, int count, const QList<int> &roles)
{
    data.changedChange(index, count, roles);
}

bool QDeclarative1ListModelWorkerAgent::event(QEvent *e)
{
    if (e->type() == QEvent::User) {
        QMutexLocker locker(&mutex);
        Sync *s = static_cast<Sync *>(e);

        const QList<Change> &changes = s->data.changes;

        if (m_copy) {
            bool cc = m_orig->count() != s->list->count();

            FlatListModel_1 *orig = m_orig->m_flat;
            FlatListModel_1 *copy = s->list->m_flat;
            if (!orig || !copy) {
                syncDone.wakeAll();
                return QObject::event(e);
            }

            orig->m_roles = copy->m_roles;
            orig->m_strings = copy->m_strings;
            orig->m_values = copy->m_values;

            // update the orig->m_nodeData list
            for (int ii = 0; ii < changes.count(); ++ii) {
                const Change &change = changes.at(ii);
                switch (change.type) {
                case Change::Inserted:
                    orig->insertedNode(change.index);
                    break;
                case Change::Removed:
                    orig->removedNode(change.index);
                    break;
                case Change::Moved:
                    orig->moveNodes(change.index, change.to, change.count);
                    break;
                case Change::Changed:
                    break;
                }
            }

            syncDone.wakeAll();
            locker.unlock();

            for (int ii = 0; ii < changes.count(); ++ii) {
                const Change &change = changes.at(ii);
                switch (change.type) {
                case Change::Inserted:
                    emit m_orig->itemsInserted(change.index, change.count);
                    break;
                case Change::Removed:
                    emit m_orig->itemsRemoved(change.index, change.count);
                    break;
                case Change::Moved:
                    emit m_orig->itemsMoved(change.index, change.to, change.count);
                    break;
                case Change::Changed:
                    emit m_orig->itemsChanged(change.index, change.count, change.roles);
                    break;
                }
            }

            if (cc)
                emit m_orig->countChanged();
        } else {
            syncDone.wakeAll();
        }
    }

    return QObject::event(e);
}



QT_END_NAMESPACE

