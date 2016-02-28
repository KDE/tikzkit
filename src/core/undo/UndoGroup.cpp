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

#include "UndoGroup.h"
#include "UndoItem.h"
#include "UndoManager.h"
#include "UndoFactory.h"

#include <QDebug>
#include <QJsonArray>

namespace tikz {
namespace core {

class UndoGroupPrivate
{
public:
    //! Pointer to the undo manager.
    UndoManager * manager = nullptr;

    //! Name of the undo group.
    QString text;

    //! list of undo items
    QVector<UndoItem *> undoItems;

    //! list of redo items
    QVector<UndoItem *> redoItems;
};

UndoGroup::UndoGroup(const QString & text, UndoManager * manager)
    : d(new UndoGroupPrivate())
{
    d->manager = manager;
    d->text = text;
}

UndoGroup::~UndoGroup()
{
    qDeleteAll(d->undoItems);
    qDeleteAll(d->redoItems);

    d->undoItems.clear();
    d->redoItems.clear();

    delete d;
}

Document * UndoGroup::document() const
{
    return d->manager->document();
}

QString UndoGroup::text() const
{
    return d->text;
}

bool UndoGroup::isEmpty() const
{
    return d->undoItems.isEmpty();
}

void UndoGroup::undo()
{
    for (int i = d->undoItems.size() - 1; i >= 0; --i) {
        d->undoItems[i]->apply();
    }
}

void UndoGroup::redo()
{
    for (int i = 0; i < d->redoItems.size(); ++i) {
        d->redoItems[i]->apply();
    }
}

void UndoGroup::addUndoItem(UndoItem *item)
{
    // only try merge, if undo item id's match
    const int lastUndoId = d->undoItems.isEmpty() ? -1 : d->undoItems.last()->id();
    const int newUndoId = item->id();
    if (lastUndoId >= 0 && lastUndoId == newUndoId && d->undoItems.last()->mergeWith(item)) {
        delete item;
    } else {
        // add to this undo group
        d->undoItems.append(item);
    }
}

void UndoGroup::addRedoItem(UndoItem *item)
{
    // only try merge, if redo item id's match
    const int lastRedoId = d->redoItems.isEmpty() ? -1 : d->redoItems.last()->id();
    const int newRedoId = item->id();
    if (lastRedoId >= 0 && lastRedoId == newRedoId && d->redoItems.last()->mergeWith(item)) {
        delete d->redoItems.back();
        d->redoItems.pop_back();
    }
    // add to this undo group
    d->redoItems.append(item);
}

QVector<UndoItem *> UndoGroup::undoItems() const
{
    return d->undoItems;
}

int UndoGroup::count() const
{
    return d->undoItems.count();
}

void UndoGroup::load(const QJsonObject & json)
{
    d->text = json["text"].toString();

    QJsonArray array = json["items"].toArray();
    foreach (QJsonValue item, array) {
        const QString type = item.toObject()["type"].toString();
        UndoFactory factory(document());
        auto undoItem = factory.createItem(type);

        d->undoItems.push_back(undoItem);
    }
}

QJsonObject UndoGroup::save() const
{
    QJsonArray array;
    foreach (auto item, undoItems()) {
        array.append(item->save());
    }

    QJsonObject json;
    json["text"] = text();
    json["items"] = array;
    return json;
}

void UndoGroup::printTree()
{
    QString str = "group: " + text();
    for (auto item : d->undoItems) {
        str += " -->" + item->text();
    }
    qDebug() << str;
}

}
}

// kate: indent-width 4; replace-tabs on;
