/* This file is part of the TikZKit project.
 *
 * Copyright (C) 2013-2015 Dominik Haumann <dhaumann@kde.org>
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

#include "UndoSetNodePos.h"
#include "Document.h"
#include "Node.h"

namespace tikz {
namespace core {

UndoSetNodePos::UndoSetNodePos(Node * node,
                               const MetaPos & newPos,
                               Document * doc)
    : UndoItem("Move Node", doc)
    , m_id(node->id())
    , m_undoPos(node->metaPos())
    , m_redoPos(newPos)
{
}

UndoSetNodePos::~UndoSetNodePos()
{
}

UndoSetNodePos::UndoSetNodePos(const QJsonObject & json, Document * doc)
    : UndoSetNodePos(doc->nodeFromId(json["node-id"].toString().toLongLong()),
                     MetaPos(json["redo-pos"].toString(), doc),
                     doc)
{
    Q_ASSERT(m_id >= 0);
}

void UndoSetNodePos::undo()
{
    const bool wasActive = document()->setUndoActive(true);

    Node * node = document()->nodeFromId(m_id);
    Q_ASSERT(node);

    node->setMetaPos(m_undoPos);

    document()->setUndoActive(wasActive);
}

void UndoSetNodePos::redo()
{
    const bool wasActive = document()->setUndoActive(true);

    Node * node = document()->nodeFromId(m_id);
    Q_ASSERT(node);
    node->setMetaPos(m_redoPos);

    document()->setUndoActive(wasActive);
}

bool UndoSetNodePos::mergeWith(const UndoItem * command)
{
    Q_ASSERT(id() == command->id());

    // only merge when command is of correct type
    const UndoSetNodePos* other = dynamic_cast<const UndoSetNodePos*>(command);
    if (other) {
        m_redoPos = other->m_redoPos;
    }

    return other != nullptr;
}

QJsonObject UndoSetNodePos::toJsonObject() const
{
    QJsonObject json;
    json["type"] = "node-set-pos";
    json["node-id"] = QString::number(m_id);
    json["redo-pos"] = m_redoPos.toString();
    return json;
}

}
}

// kate: indent-width 4; replace-tabs on;
