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

#ifndef TIKZ_UNDO_ITEM_H
#define TIKZ_UNDO_ITEM_H

#include <QString>

namespace tikz {
namespace core {

class Document;
class UndoItemPrivate;

/**
 * Base class for undo/redo items.
 */
class UndoItem
{
public:
    /**
     * Constructor, setting the undo item's description to @p text,
     * and the associated tikz::core::document to @p doc.
     */
    UndoItem(const QString & text, Document* doc);

    /**
     * Virtual destructor.
     */
    virtual ~UndoItem();

    /**
     * Accessor to the tikz Document.
     */
    Document* document();

    /**
     * Set the undo item description to @p text.
     */
    void setText(const QString & text);

    /**
     * Returns the description of the undo item.
     */
    QString text() const;

    /**
     * Apply the redo operation.
     */
    virtual void redo() = 0;

    /**
     * Apply the undo operation.
     */
    virtual void undo() = 0;

    /**
     * Returns the uniq undo identifier of this undo item.
     * Whenever two successive undo items have the same id, the function
     * mergeWith() is executed to fold the two undo items into a single
     * undo item.
     *
     * By default, -1 is returned. In this case (or any other negative id),
     * mergeWith() is not called.
     */
    virtual int id() const;

    /**
     * Merge @p item into this item. Afterwards, @p item is deleted.
     * Typically, the undo() operation is unchanged, and you only have to
     * copy the redo data of @p item into this undo item.
     */
    virtual bool mergeWith(const UndoItem * item);

private:
    /**
     * Pimpl pointer to the held data.
     */
    UndoItemPrivate * const d;
};

}
}

#endif // TIKZ_UNDO_ITEM_H

// kate: indent-width 4; replace-tabs on;
