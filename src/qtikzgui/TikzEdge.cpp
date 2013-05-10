#include "TikzEdge.h"

#include "TikzNode.h"
#include "EdgeStyle.h"
#include "AnchorHandle.h"
#include "CurveHandle.h"

#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QTextLayout>
#include <QGraphicsTextItem>
#include <QDebug>
#include <PaintHelper.h>
#include <QGraphicsSceneMouseEvent>

#include <Coord.h>
#include <cmath>

class TikzEdgePrivate
{
    TikzEdge* q;

    public:
        enum DragMode {
            DM_Start = 0,
            DM_End,
        };
        
        TikzEdgePrivate(TikzEdge* edge) : q(edge) {}

    public:
        // edge and nodes
        tikz::Edge* edge;
        TikzNode* start;
        TikzNode* end;

        // draging state
        bool dragging;      // true: mouse is grabbed
        DragMode dragMode;

        // cached painter paths
        bool dirty;             // true: needs recalculation of paths
        QPainterPath linePath;
        QPainterPath arrowHead;
        QPainterPath arrowTail;

        // node handles on mouse over nodes
        QVector<AnchorHandle*> nodeHandles;

        // Bezier curve handles
        CurveHandle* startControlPoint;
        CurveHandle* endControlPoint;

    //
    // helper functions
    //
    public:
        void updateCache()
        {
            dirty = false;

            // reset old paths
            linePath = QPainterPath();
            arrowHead = QPainterPath();
            arrowTail = QPainterPath();

            // draw line
            const QPointF startAnchor = q->startPos();
            const QPointF endAnchor = q->endPos();
            const QPointF diff = endAnchor - startAnchor;
            const qreal len = sqrt(diff.x()*diff.x() + diff.y()*diff.y());

            if (q->style()->curveMode() == tikz::BendCurve) {
                // from tikz/libraries/tikzlibrarytopaths.code.tex:
                const qreal factor = 0.3915;
                const qreal vecLen = len * q->style()->looseness() * factor;
                const qreal startRad = startAngle();
                const qreal endRad = endAngle();
                QPointF cp1 = startAnchor + vecLen * QPointF(std::cos(startRad), std::sin(startRad));
                QPointF cp2 = endAnchor + vecLen * QPointF(std::cos(endRad), std::sin(endRad));
                linePath.moveTo(startAnchor);
                linePath.cubicTo(cp1, cp2, endAnchor);

                startControlPoint->setPos(cp1);
                endControlPoint->setPos(cp2);
            } else {
                linePath.lineTo(endAnchor);
            }

            createArrow(arrowHead, startAnchor, linePath.angleAtPercent(0.0) * M_PI / 180.0);
            createArrow(arrowTail, endAnchor, linePath.angleAtPercent(1.0) * M_PI / 180.0  - M_PI);
        }

        qreal startAngle()
        {
            const QPointF startAnchor = start ? start->pos() : edge->startPos();
            const QPointF endAnchor = end ? end->pos() : edge->endPos();
            const QPointF diff = endAnchor - startAnchor;
            qreal rad = std::atan2(diff.y(), diff.x());
            if (q->style()->curveMode() == tikz::BendCurve) {
                rad = rad + q->style()->bendAngle() * M_PI / 180.0;
            }
            return rad;
        }

        qreal endAngle()
        {
            const QPointF startAnchor = start ? start->pos() : edge->startPos();
            const QPointF endAnchor = end ? end->pos() : edge->endPos();
            const QPointF diff = startAnchor - endAnchor;
            qreal rad = std::atan2(diff.y(), diff.x());
            if (q->style()->curveMode() == tikz::BendCurve) {
                rad = rad - q->style()->bendAngle() * M_PI / 180.0;
            }
            return rad;
        }

        void createArrow(QPainterPath& path, const QPointF& arrowHead, qreal rad)
        {
            // TODO: fix style of arrow head
            const qreal arrowSize = 0.3; // TODO: fix size of arrow head
            QPointF sourceArrowP1 = arrowHead + QPointF(sin(rad + M_PI - M_PI / 3) * arrowSize,
                                                        cos(rad + M_PI - M_PI / 3) * arrowSize);
            QPointF sourceArrowP2 = arrowHead + QPointF(sin(rad + M_PI / 3) * arrowSize,
                                                        cos(rad + M_PI / 3) * arrowSize);
            path.addPolygon(QPolygonF() << arrowHead << sourceArrowP1 << sourceArrowP2);
            path.closeSubpath();
        }

        void drawArrow(QPainter* painter, const QPainterPath& path)
        {
            // TODO: fix style of arrow head
            painter->setBrush(Qt::black);
            painter->drawPath(path);
        }

        void drawHandle(QPainter* painter, const QPointF& pos, bool connected)
        {
            painter->save();
            painter->setPen(connected ? Qt::green : Qt::red);
            painter->setBrush(connected ? QColor(0, 255, 0, 125) : QColor(255, 0, 0, 125));
            painter->drawEllipse(pos, 0.2, 0.2);
            painter->restore();
        }
};

TikzEdge::TikzEdge(QGraphicsItem * parent)
    : TikzItem(parent)
    , d(new TikzEdgePrivate(this))
{
    d->edge = new tikz::Edge(this);
    d->start = 0;
    d->end = 0;

    d->dragging = false;
    d->dirty = true;

    d->startControlPoint = new CurveHandle(this);
    d->endControlPoint = new CurveHandle(this);

    connect(d->startControlPoint, SIGNAL(positionChanged(QPointF)), this, SLOT(startControlPointChanged(QPointF)));
    connect(d->endControlPoint, SIGNAL(positionChanged(QPointF)), this, SLOT(endControlPointChanged(QPointF)));

    connect(d->edge, SIGNAL(changed()), this, SLOT(slotUpdate()));
}

TikzEdge::~TikzEdge()
{
    delete d;
}

int TikzEdge::type() const
{
    return UserType + 3;
}

tikz::Edge& TikzEdge::edge()
{
    return *d->edge;
}

tikz::EdgeStyle* TikzEdge::style() const
{
    return d->edge->style();
}

void TikzEdge::setStartNode(TikzNode* start)
{
    d->start = start;
    d->edge->setStart(start ? &start->node() : 0);
}

void TikzEdge::setEndNode(TikzNode* end)
{
    d->end = end;
    d->edge->setEnd(end ? &end->node() : 0);
}

TikzNode* TikzEdge::startNode() const
{
    return d->start;
}

TikzNode* TikzEdge::endNode() const
{
    return d->end;
}

QPointF TikzEdge::startPos() const
{
    QPointF startPos;
    if (d->start) {
        startPos = mapFromItem(d->start, d->start->anchor(startAnchor(), d->startAngle()));
    } else {
        startPos = mapFromScene(d->edge->start().pos());
    }
    return startPos;
}

QPointF TikzEdge::endPos() const
{
    QPointF endPos;
    if (d->end) {
        endPos = mapFromItem(d->end, d->end->anchor(endAnchor(), d->endAngle()));
    } else {
        endPos = mapFromScene(d->edge->end().pos());
    }
    return endPos;
}

tikz::Anchor TikzEdge::startAnchor() const
{
    return d->edge->startAnchor();
}

tikz::Anchor TikzEdge::endAnchor() const
{
    return d->edge->endAnchor();
}

void TikzEdge::setStartAnchor(tikz::Anchor anchor)
{
    d->edge->setStartAnchor(anchor);
}

void TikzEdge::setEndAnchor(tikz::Anchor anchor)
{
    d->edge->setEndAnchor(anchor);
}

void TikzEdge::slotUpdate()
{
    prepareGeometryChange();

    // setPos() to the middle of start and end
    QPointF startScenePos = d->edge->start().pos();
    QPointF endScenePos = d->edge->end().pos();
    setPos(0.5 * (startScenePos + endScenePos));

    d->dirty = true;
}

void TikzEdge::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);
    Q_UNUSED(option);

    if (d->dirty) {
        d->updateCache();
    }

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing);

    PaintHelper sh(painter, d->edge->style());
    QPen p = sh.pen();
    painter->setPen(p);

    // draw line
    painter->drawPath(d->linePath);

    // draw arrows
    d->drawArrow(painter, d->arrowHead);
    d->drawArrow(painter, d->arrowTail);

    // TODO: create d->paths
    if (isHovered() && !d->dragging) {
        QPointF startAnchor = startPos();
        QPointF endAnchor = endPos();
        QPointF diff(endAnchor - startAnchor);
        const qreal radAngle = std::atan2(diff.y(), diff.x());
        d->drawHandle(painter, startAnchor, d->start != 0);
        d->drawHandle(painter, endAnchor, d->end != 0);
    }

    // debug: draw bounding rect:
    //painter->drawRect(boundingRect());

    // TODO: highlight selection
    //     if (option->state & QStyle::State_Selected)
    //         qt_graphicsItem_highlightSelected(this, painter, option);

    painter->restore();
}

QRectF TikzEdge::boundingRect() const
{
    // TODO: maybe use Style::lineWidth()


    // make sure the start and end nodes positions are up-to-date
    d->updateCache();

    QPainterPath joinedPath;
    joinedPath.addPath(d->linePath);

//     const QPointF src = startPos();
//     const QPointF dst = endPos();
// 
//     QRectF br(src, dst);
//     br = br.normalized();
//     br.adjust(-0.2, -0.2, 0.2, 0.2);

    QRectF br = joinedPath.boundingRect();
    br = br.normalized();
    br.adjust(-0.2, -0.2, 0.2, 0.2);    
    return br;
}

QPainterPath TikzEdge::shape() const
{
    QPainterPath path;
    path.addRect(boundingRect());
    return path;
}

bool TikzEdge::contains(const QPointF & point) const
{
    if (d->dirty) {
        return TikzItem::contains(point);
    } else {
        bool intersects = false;
        QPainterPath circle;
        circle.addEllipse(point, 0.2, 0.2);

        return d->linePath.intersects(circle)
            || d->arrowHead.intersects(circle)
            || d->arrowTail.intersects(circle);
    }
}

void TikzEdge::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
    if (!d->dragging) {
        event->ignore();
        TikzItem::mouseMoveEvent(event);
        return;
    }

    QList<QGraphicsItem *> items = scene()->items(event->scenePos(), Qt::ContainsItemShape, Qt::DescendingOrder);
    items.removeOne(this);

    bool connected = false;
    if (items.size()) {
        foreach (QGraphicsItem* item, items) {
            if (item->type() == UserType + 2) {
                TikzNode* node = dynamic_cast<TikzNode*>(item);
                Q_ASSERT(node);
                if (d->dragMode == TikzEdgePrivate::DM_Start) {
                    if (d->start != node) {
                        qDeleteAll(d->nodeHandles);
                        d->nodeHandles.clear();
                        setStartNode(node);

                        d->nodeHandles.append(new AnchorHandle(this, tikz::NoAnchor, true));
                        d->nodeHandles.append(new AnchorHandle(this, tikz::Center, true));
                        d->nodeHandles.append(new AnchorHandle(this, tikz::North, true));
                        d->nodeHandles.append(new AnchorHandle(this, tikz::NorthEast, true));
                        d->nodeHandles.append(new AnchorHandle(this, tikz::East, true));
                        d->nodeHandles.append(new AnchorHandle(this, tikz::SouthEast, true));
                        d->nodeHandles.append(new AnchorHandle(this, tikz::South, true));
                        d->nodeHandles.append(new AnchorHandle(this, tikz::SouthWest, true));
                        d->nodeHandles.append(new AnchorHandle(this, tikz::West, true));
                        d->nodeHandles.append(new AnchorHandle(this, tikz::NorthWest, true));
                    }
                } else {
                    if (d->end != node) {
                        qDeleteAll(d->nodeHandles);
                        d->nodeHandles.clear();
                        setEndNode(node);

                        d->nodeHandles.append(new AnchorHandle(this, tikz::NoAnchor, false));
                        d->nodeHandles.append(new AnchorHandle(this, tikz::Center, false));
                        d->nodeHandles.append(new AnchorHandle(this, tikz::North, false));
                        d->nodeHandles.append(new AnchorHandle(this, tikz::NorthEast, false));
                        d->nodeHandles.append(new AnchorHandle(this, tikz::East, false));
                        d->nodeHandles.append(new AnchorHandle(this, tikz::SouthEast, false));
                        d->nodeHandles.append(new AnchorHandle(this, tikz::South, false));
                        d->nodeHandles.append(new AnchorHandle(this, tikz::SouthWest, false));
                        d->nodeHandles.append(new AnchorHandle(this, tikz::West, false));
                        d->nodeHandles.append(new AnchorHandle(this, tikz::NorthWest, false));
                    }
                }
                connected = true;
                break;
            }
        }
    }

    if (!connected) {
        qDeleteAll(d->nodeHandles);
        d->nodeHandles.clear();

        if (d->dragMode == TikzEdgePrivate::DM_Start) {
            setStartNode(0);
            d->edge->start().setPos(event->scenePos());
        } else {
            setEndNode(0);
            d->edge->end().setPos(event->scenePos());
        }
    }

//     qDebug() << "move";
}

void TikzEdge::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    if (!contains(event->pos())) {
        TikzItem::mousePressEvent(event);
    } else {
//         grabMouse();
        d->dragging = true;
        const qreal distToStart = (event->scenePos() - d->edge->start().pos()).manhattanLength();
        const qreal distToEnd = (event->scenePos() - d->edge->end().pos()).manhattanLength();
        if (distToStart < distToEnd) {
            d->dragMode = TikzEdgePrivate::DM_Start;
        } else {
            d->dragMode = TikzEdgePrivate::DM_End;
        }
    }
}

void TikzEdge::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
    if (d->dragging) {
        d->dragging = false;
//         ungrabMouse();

        // clear node handles, if needed
        qDeleteAll(d->nodeHandles);
        d->nodeHandles.clear();
    }

    if (!contains(event->pos())) {
        TikzItem::mouseReleaseEvent(event);
    }
    update();
}

void TikzEdge::startControlPointChanged(const QPointF& pos)
{
    slotUpdate();
}

void TikzEdge::endControlPointChanged(const QPointF& pos)
{
    slotUpdate();
}

// kate: indent-width 4; replace-tabs on;
