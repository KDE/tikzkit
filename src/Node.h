#ifndef TIKZ_NODE_H
#define TIKZ_NODE_H

#include <QPointF>

#include "tikz.h"
#include "Coord.h"

namespace tikz
{

class NodePrivate;

class Node : public Coord
{
    public:
        /**
         * Default constructor.
         */
        Node();

        /**
         * Destructor
         */
        virtual ~Node();

        virtual const QPointF& anchor(Anchor anchor) const;

    private:
        NodePrivate * const d;
};

}

#endif // TIKZ_NODE_H

// kate: indent-width 4; replace-tabs on;
