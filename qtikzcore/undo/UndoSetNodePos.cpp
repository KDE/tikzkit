/* This file is part of the TikZKit project.
 *
 * Copyright (C) 2013 Dominik Haumann <dhaumann@kde.org>
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

UndoSetNodePos::UndoSetNodePos(qint64 id, const QPointF & newPos, Document * doc)
    : UndoItem(doc)
    , m_id(id)
{
    Node * node = doc->nodeFromId(id);
    Q_ASSERT(node);

    m_undoPos = node->pos();
    m_redoPos = newPos;
}

UndoSetNodePos::~UndoSetNodePos()
{
}

int UndoSetNodePos::id() const
{
    return m_id;
}

void UndoSetNodePos::undo()
{
    Node * node = document()->nodeFromId(m_id);
    Q_ASSERT(node);
    node->setPos(m_undoPos);
}

void UndoSetNodePos::redo()
{
    Node * node = document()->nodeFromId(m_id);
    Q_ASSERT(node);
    node->setPos(m_redoPos);
}

bool UndoSetNodePos::mergeWith(const QUndoCommand * command)
{
    Q_ASSERT(id() == command->id());

    m_redoPos = static_cast<const UndoSetNodePos*>(command)->m_redoPos;
    return true;
}

}

// kate: indent-width 4; replace-tabs on;