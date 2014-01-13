/* This file is part of the TikZKit project.
 *
 * Copyright (C) 2013-2014 Dominik Haumann <dhaumann@kde.org>
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

#ifndef TIKZ_UI_PATH_ITEM_H
#define TIKZ_UI_PATH_ITEM_H

#include "TikzItem.h"

#include <tikz/core/tikz.h>


namespace tikz {
namespace core {
    class Path;
    class EdgeStyle;
}

namespace ui {

class TikzDocument;
class PathPrivate;

class TIKZGUI_EXPORT Path :  public TikzItem
{
    Q_OBJECT

    public:
        /**
         * Constructor for @p path and parent @p parent.
         */
        Path(tikz::core::Path * path, QGraphicsItem * parent = 0);

        /**
         * Destructor
         */
        virtual ~Path();

        /**
         * Returns the associated document, if available.
         */
        TikzDocument * document() const;

        /**
         * Reimplment to return a proper UserType + 3.
         */
        virtual int type() const;

        /**
         * Returns the pointer to the associated Path.
         * @warning Use setStartNode() and setEndNode() to change nodes
         */
        tikz::core::Path * path() const;

        /**
         * Returns path().id();
         */
        qint64 id() const;

        /**
         * Get the EdgeStyle of this path.
         * The returned pointer is always valid.
         */
        tikz::core::EdgeStyle* style() const;

    Q_SIGNALS:
        /**
         * This signal is emitted whenever this path changes in any way.
         * This is the same as the signal path()->changed().
         */
        void changed();

    //
    // internal
    //
    private:
        /**
         * Private default constructor, not implemented
         */
        Path();

    private:
        PathPrivate * const d;
};

}
}

#endif // TIKZ_UI_PATH_ITEM_H

// kate: indent-width 4; replace-tabs on;
