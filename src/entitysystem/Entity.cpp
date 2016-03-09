/* This file is part of the TikZKit project.
 *
 * Copyright (C) 2015 Dominik Haumann <dhaumann@kde.org>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as published
 * by the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "Entity.h"
#include "Document.h"

namespace es {

/**
 * Private data and helper functions of class Entity.
 */
class EntityPrivate
{
public:
    // unique id, or -1 (negative)
    Eid eid = Eid();

    // associated Document (if any)
    Document * document = nullptr;
};

Entity::Entity()
    : ConfigObject()
    , d(new EntityPrivate())
{
}

Entity::Entity(const Eid & eid, Document* doc)
    : ConfigObject(doc)
    , d(new EntityPrivate())
{
    Q_ASSERT(eid.isValid());
    Q_ASSERT(eid.document() == doc);
    Q_ASSERT(doc);

    d->document = doc;
    d->eid = eid;
}

Entity::~Entity()
{
}

Eid Entity::eid() const
{
    return d->eid;
}

void Entity::load(const QJsonObject & json)
{
    if (json.contains("eid")) {
        d->eid = Eid(json["eid"].toString(), d->document);
    }

    // load payload
    QJsonObject joData = json["data"].toObject();
    loadData(joData);
}

QJsonObject Entity::save() const
{
    QJsonObject json;

    json["eid"] = d->eid.toString();

    // save payload
    json["data"] = saveData();

    return json;
}

QJsonObject Entity::saveData() const
{
    return QJsonObject();
}

void Entity::loadData(const QJsonObject & json)
{
}

Document * Entity::document() const
{
    return d->document;
}

void Entity::notifyAboutToChange()
{
    emit entityAboutToChange(this);
}

void Entity::notifyChanged()
{
    emit entityChanged(this);
}

Entity::Transaction::Transaction(Entity * entity, const QString & name)
    : m_entity(entity)
{
    // Alghouth it works in release-mode, we usually want a valid document
    Q_ASSERT(entity != nullptr);

    // start the editing transaction
    if (m_entity) {
        if (m_entity->document()) {
            m_entity->document()->beginTransaction(name);
        }
        m_entity->beginConfig();
    }
}

void Entity::Transaction::cancel()
{
    if (m_entity && m_entity->document()) {
        m_entity->document()->cancelTransaction();
    }
}

void Entity::Transaction::finish()
{
    if (m_entity) {
        m_entity->endConfig();
        if (m_entity->document()) {
            m_entity->document()->finishTransaction();
        }
        m_entity = nullptr;
    }
}

bool Entity::Transaction::isRunning() const
{
    return m_entity != nullptr;
}

Entity::Transaction::~Transaction()
{
    // finish the editing transaction, if applicable
    finish();
}

}

// kate: indent-width 4; replace-tabs on;