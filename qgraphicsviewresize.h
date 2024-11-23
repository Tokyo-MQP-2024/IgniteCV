#ifndef QGRAPHICSVIEWRESIZE_H
#define QGRAPHICSVIEWRESIZE_H

#include <QGraphicsView>

// I know the naming is inconsistent and stupid. My fault.
// Purpose of this class is to override resizeEvent to dynamically update area of the Graphics View
// as the size of the window changes
class qgraphicsviewresize : public QGraphicsView
{
public:
    qgraphicsviewresize(QWidget *parent = nullptr);
private:
    void resizeEvent(QResizeEvent *event) override;
};

#endif // QGRAPHICSVIEWRESIZE_H
