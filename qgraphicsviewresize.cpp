#include "qgraphicsviewresize.h"

qgraphicsviewresize::qgraphicsviewresize(QWidget *parent) : QGraphicsView(parent) {}


void qgraphicsviewresize::resizeEvent(QResizeEvent *event) {
    QGraphicsView::resizeEvent(event);
    if(scene()) {
        fitInView(scene()->sceneRect(), Qt::KeepAspectRatio);
    }
}

