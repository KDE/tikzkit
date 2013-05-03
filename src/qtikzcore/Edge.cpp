#include "Edge.h"
#include "Coord.h"
#include "MetaNode.h"
#include "Style.h"

namespace tikz {

class EdgePrivate
{
    public:
        MetaNode start;
        MetaNode end;
        Style style;
};

Edge::Edge(QObject * parent)
    : QObject(parent)
    , d(new EdgePrivate())
{
    connect(&d->start, SIGNAL(changed()), this, SIGNAL(changed()));
    connect(&d->end, SIGNAL(changed()), this, SIGNAL(changed()));
}

Edge::~Edge()
{
    delete d;
}

void Edge::setStart(Node* node)
{
    // update node
    if (d->start.setNode(node)) {
        // notify about change
        emit changed();
    }
}

Coord& Edge::start()
{
    return d->start.coord();
}

void Edge::setEnd(Node* node)
{
    // update node
    if (d->end.setNode(node)) {
        // notify about change
        emit changed();
    }
}

Coord& Edge::end()
{
    return d->end.coord();
}

void Edge::setStartPos(const QPointF& pos)
{
    d->start.setPos(pos);
    emit changed();
}

void Edge::setEndPos(const QPointF& pos)
{
    d->end.setPos(pos);
    emit changed();
}

Style& Edge::style()
{
    return d->style;
}
}

// kate: indent-width 4; replace-tabs on;
