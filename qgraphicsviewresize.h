#ifndef QGRAPHICSVIEWRESIZE_H
#define QGRAPHICSVIEWRESIZE_H

#include <QGraphicsView>

class qgraphicsviewresize : public QGraphicsView
{
public:
    qgraphicsviewresize(QWidget *parent = nullptr);
private:
    void resizeEvent(QResizeEvent *event) override;
};

#endif // QGRAPHICSVIEWRESIZE_H
