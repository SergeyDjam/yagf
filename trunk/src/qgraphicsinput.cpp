/*
    YAGF - cuneiform OCR graphical front-end
    Copyright (C) 2009-2011 Andrei Borovsky <anb@symmetrica.net>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "qgraphicsinput.h"
#include "qxtgraphicsview.h"
#include "qxtgraphicsproxywidget.h"
#include "ccbuilder.h"
#include "analysis.h"
#include "PageAnalysis.h"
#include "math.h"
#include "ycommon.h"
#include <QPixmap>
#include <QGraphicsPixmapItem>
#include <QMessageBox>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsPixmapItem>
#include <QKeyEvent>
#include <QToolBar>
#include <QLayout>


QGraphicsInput::QGraphicsInput(const QRectF &sceneRect, QGraphicsView *view) :
    QGraphicsScene(sceneRect)
{
    setView(view);
    m_image = 0;
    selecting  = NoSelect;
    hasImage = false;
    m_LastSelected = 0;
    buttonPressed = Qt::NoButton;
    near_res = 0;
    magnifierCursor = new QCursor(Qt::SizeAllCursor);
    toolbar = 0;
 }

QGraphicsInput::~QGraphicsInput()
{
    delete magnifierCursor;
}

void QGraphicsInput::addToolBar()
{
    toolbar = new QToolBar(m_view);
    toolbar->setMouseTracking(false);
    toolbar->setMovable(false);
    toolbar->setWindowOpacity(0.75);
    toolbar->move(0,0);
    toolbar->setIconSize(QSize(24,24));
    toolbar->setMinimumHeight(32);
    //toolbar->setCursor();
    actionList.at(0)->setText(QString::fromUtf8(">>"));
    setToolBarVisible();
    //QXtGraphicsProxyWidget * pw = new QXtGraphicsProxyWidget();
    //pw->setWidget(toolbar);

    //pw->setZValue(100);
    //this->addItem(pw);
    //pw->setView((QXtGraphicsView *) views().at(0));
    //toolbar->setParent(0);
    toolbar->show();
    foreach (QAction * action, actionList) {
        toolbar->addAction(action);
    }
    ((QXtGraphicsView *) views().at(0))->sendScrollSignal();
}

bool QGraphicsInput::loadImage(const QPixmap &pixmap)
{
    QApplication::processEvents();
    //old_pixmap = image;
    m_image = addPixmap(pixmap);
    setSceneRect(pixmap.rect());
    //m_realImage->setData(1, "image");
    //m_realImage->hide();
    this->setFocus();
    m_image->setFocus();
    m_image->setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton | Qt::MidButton);
    m_image->setAcceptHoverEvents(true);
    m_image->setData(1, "image");
    addToolBar();
    if (m_view) {
        m_view->centerOn(0, 0);
        m_view->show();
        update();
        hasImage = true;
        return true;
    } else
        return false;
}

void QGraphicsInput::setView(QGraphicsView *view)
{
    m_view = view;
    if (m_view)
        m_view->setScene(this);

}

void QGraphicsInput::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsScene::mousePressEvent(event);
    // QMessageBox::critical(0, "MOUS111", "MOUSE");
    if (!hasImage)
        return;

    if (event->buttons() == Qt::LeftButton) {
        buttonPressed = Qt::LeftButton;
        if (selecting == NoSelect) {
            if ((near_res = nearActiveBorder(event->scenePos().x(), event->scenePos().y())) != 0) {
                m_CurrentBlockRect = m_LastSelected;
                selecting = Selecting;
                blockRect = m_CurrentBlockRect->rect();
            }  else {
                selecting = StartSelect;
                blockRect.setLeft(event->lastScenePos().x());
                blockRect.setTop(event->lastScenePos().y());
                blockRect.setWidth(10);
                blockRect.setHeight(10);
            }
        } else {
            //TODO!!!
        }
    } else
        buttonPressed = Qt::RightButton;
}

void QGraphicsInput::deleteBlockRect(QGraphicsRectItem *item)
{
    if (item == 0)
        return;
    if (item == m_CurrentBlockRect)
        m_CurrentBlockRect = 0;
    if (item == m_LastSelected)
        m_LastSelected = 0;
    removeItem(item);
}

void QGraphicsInput::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    QGraphicsScene::mouseReleaseEvent(mouseEvent);
    if (buttonPressed == Qt::LeftButton) {
        if (selecting == Selecting) {
            selecting = NoSelect;
            if ((blockRect.width() < 12) || (blockRect.height() < 12)) {
                if (m_CurrentBlockRect == m_LastSelected)
                    m_LastSelected = 0;
                deleteBlockRect(m_CurrentBlockRect);
                //clik!!!
                leftMouseRelease(mouseEvent->scenePos().x(), mouseEvent->scenePos().y());
            }
            m_CurrentBlockRect = 0;
        }
        if (selecting == StartSelect) {
            selecting = NoSelect;
            m_CurrentBlockRect = 0;
            leftMouseRelease(mouseEvent->scenePos().x(), mouseEvent->scenePos().y());
        }
    }
    if (buttonPressed == Qt::RightButton) {
        this->rightMouseRelease(mouseEvent->scenePos().x(), mouseEvent->scenePos().y());
    }
    buttonPressed = Qt::NoButton;
}


QGraphicsRectItem *QGraphicsInput::newBlock(const QRectF &rect)
{
    QPen p(Qt::SolidLine);
    QBrush b(Qt::SolidPattern);
    b.setColor(QColor(0, 0, 127, 127));
    p.setWidth(2);
    p.setColor(QColor(0, 0, 255));
    QGraphicsRectItem *res;
    res = this->addRect(rect, p, b);
    res->setAcceptHoverEvents(true);
    res->setZValue(1);
    res->setData(1, "block");
    res->setData(2, "no");
    return res;
}

bool QGraphicsInput::addBlock(const QRectF &rect, bool removeObstacles)
{
    QGraphicsRectItem *block = newBlock(rect);
    if (!removeObstacles) {
        if (block->collidingItems().size() > 0) {
            deleteBlockRect(block);
            return false;
        }
    } else {
        for (int i = block->collidingItems().size() - 1; i >= 0; i--) {
            if (block->collidingItems().at(i)->data(1) == "block")
                deleteBlockRect((QGraphicsRectItem *) block->collidingItems().at(i));
        }
    }
    m_CurrentBlockRect = block;
    return true;
}

void QGraphicsInput::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    QGraphicsScene::mouseMoveEvent(mouseEvent);
    if (selecting == StartSelect) {
        selecting = Selecting;
        m_CurrentBlockRect = newBlock(blockRect);
    }

    if ((mouseEvent->modifiers() & Qt::ControlModifier) == 0)
        if (mouseEvent->buttons() == Qt::NoButton) {
            near_res = nearActiveBorder(mouseEvent->scenePos().x(), mouseEvent->scenePos().y());
            switch (near_res) {
                case 0:
                    m_view->viewport()->setCursor(Qt::ArrowCursor);
                    break;
                case 1:
                    m_view->viewport()->setCursor(Qt::SplitHCursor);
                    break;
                case 2:
                    m_view->viewport()->setCursor(Qt::SplitVCursor);
                    break;
                case 3:
                    m_view->viewport()->setCursor(Qt::SplitHCursor);
                    break;
                case 4:
                    m_view->viewport()->setCursor(Qt::SplitVCursor);
                    break;
                default:
                    break;
            }

        }
    QRectF newRect;
    if (near_res && (mouseEvent->buttons()&Qt::LeftButton)) {
        QRectF newRect = m_LastSelected->mapRectToScene(m_LastSelected->rect());
        switch (near_res) {
            case 1:
                newRect.setLeft(mouseEvent->lastScenePos().x());
                break;
            case 2:
                newRect.setTop(mouseEvent->lastScenePos().y());
                break;
            case 3:
                newRect.setRight(mouseEvent->lastScenePos().x());
                break;
            case 4:
                newRect.setBottom(mouseEvent->lastScenePos().y());
                break;
            default:
                break;
        }
        m_CurrentBlockRect = m_LastSelected;
        m_CurrentBlockRect->setRect(m_LastSelected->mapRectFromScene(newRect));
        for (int i = 0; i < m_CurrentBlockRect->collidingItems().size(); ++i)
            if (m_CurrentBlockRect->collidingItems().at(i)->data(1) == "block") {
                m_CurrentBlockRect->setRect(m_LastSelected->mapRectFromScene(selBlockRect));
                return;
            }
        selBlockRect = newRect;
        return;
    }
    if (selecting == Selecting) {
        newRect = blockRect;
        if (newRect.left() < mouseEvent->lastScenePos().x())
            newRect.setRight(mouseEvent->lastScenePos().x());
        else
            newRect.setLeft(mouseEvent->lastScenePos().x());
        if (newRect.top() < mouseEvent->lastScenePos().y())
            newRect.setBottom(mouseEvent->lastScenePos().y());
        else
            newRect.setTop(mouseEvent->lastScenePos().y());
        m_CurrentBlockRect->setRect(newRect);
        for (int i = 0; i < m_CurrentBlockRect->collidingItems().size(); ++i)
            if (m_CurrentBlockRect->collidingItems().at(i)->data(1) == "block") {
                m_CurrentBlockRect->setRect(blockRect);
                return;
            }
        blockRect = newRect;
        return;
    }
    if (!toolbar.isNull()) {
       // if (mouseEvent->pos().y() < toolbar->height())
       //     toolbar->setFocus();
    }

}

void QGraphicsInput::leftMouseRelease(qreal x, qreal y)
{
    QGraphicsItem *it = this->itemAt(x, y);
    if (it) {
        if (it->data(1).toString() == "block") {
            QGraphicsRectItem *r = (QGraphicsRectItem *) it;
            QPen p(Qt::SolidLine);
            QBrush b(Qt::SolidPattern);
            b.setColor(QColor(0, 0, 127, 127));
            p.setColor(QColor(0, 0, 255));
            p.setWidth(2);
            if (r->data(2).toString() == "no") {
                //select block!!!!
                if (m_LastSelected) {
                    m_LastSelected->setPen(p);
                    m_LastSelected->setBrush(b);
                    m_LastSelected->setData(2, "no");
                }
                b.setColor(QColor(127, 0, 0, 127));
                p.setColor(QColor(255, 0, 0));
                r->setData(2, "yes");
                m_LastSelected = r;
                selBlockRect = m_LastSelected->rect();
            } else {
                m_LastSelected = 0;
                r->setData(2, "no");
            }
            r->setPen(p);
            r->setBrush(b);
            //         m_CurrentBlockRect = r;
        }
    } else
        m_CurrentBlockRect = 0;
    emit leftMouseClicked(m_view->mapFromScene(x, y).x(), m_view->mapFromScene(x, y).y(), m_CurrentBlockRect != 0);
}


void QGraphicsInput::rightMouseRelease(qreal x, qreal y)
{
    QGraphicsItem *it = this->itemAt(x, y);
    if (it) {
        if (it->data(1).toString() == "block") {
            m_CurrentBlockRect = (QGraphicsRectItem *) it;
        }
    } else
        m_CurrentBlockRect = 0;
    emit rightMouseClicked(m_view->mapFromScene(x, y).x(), m_view->mapFromScene(x, y).y(), m_CurrentBlockRect != 0);
}


int QGraphicsInput::nearActiveBorder(qreal x, qreal y)
{
    if (m_LastSelected == 0)
        return 0;
    x = m_LastSelected->mapFromScene(x, y).x();
    y = m_LastSelected->mapFromScene(x, y).y();
    qreal xcenter = m_LastSelected->rect().center().x();
    qreal ycenter = m_LastSelected->rect().center().y();
    qreal xcd = abs(m_LastSelected->rect().right() - xcenter) + 8;
    qreal ycd = abs(m_LastSelected->rect().bottom() - ycenter) + 8;
    if ((abs(x - m_LastSelected->rect().left()) <= 4)) {
        if (abs(y - ycenter) < ycd)
            return 1;
        else
            return 0;
    }
    if ((abs(m_LastSelected->rect().top() - y) <= 4)) {
        if (abs(x - xcenter) < xcd)
            return 2;
        else
            return 0;
    }
    if ((abs(x - m_LastSelected->rect().right()) <= 4)) {
        if (abs(y - ycenter) < ycd)
            return 3;
        else
            return 0;
    }
    if ((abs(m_LastSelected->rect().bottom() - y) <= 4)) {
        if (abs(x - xcenter) < xcd)
            return 4;
        else
            return 0;
    }
    return 0;
}

QRect QGraphicsInput::getActiveBlock()
{
    return QRectF2Rect(m_LastSelected->rect());
}

QRect QGraphicsInput::getCurrentBlock()
{
    return QRectF2Rect(m_CurrentBlockRect->rect());
}

void QGraphicsInput::deleteCurrentBlock()
{
    if (m_CurrentBlockRect != 0)
        deleteBlockRect(m_CurrentBlockRect);
}

void QGraphicsInput::deleteBlock(int index)
{
    int count = 0;
    for (int i = 0; i < items().count(); i++) {
        if (items().at(i)->data(1) == "block") {
            if (index == count) {
                deleteBlockRect((QGraphicsRectItem *)items().at(i));
                return;
            }
            count++;
        }
    }
}

void QGraphicsInput::clearBlocks() // KEEP
{
    for (int i = items().count() - 1; i >= 0; i--) {
        if (items().at(i)->data(1) == "block") {
            deleteBlockRect((QGraphicsRectItem *)items().at(i));
        }
    }
}

void QGraphicsInput::setMagnifierCursor(QCursor *cursor)
{
    delete magnifierCursor;
    magnifierCursor = new QCursor(cursor->pixmap());
}

void QGraphicsInput::addToolBarAction(QAction *action)
{
    actionList.append(action);
}

void QGraphicsInput::addToolBarSeparator()
{
    QAction * action = new QAction(" | ", 0);
    action->setEnabled(false);
    actionList.append(action);
}

void QGraphicsInput::setToolBarVisible()
{
    if (toolbar.isNull())
        return;
    if (actionList.at(0)->text() == QString::fromUtf8("<<")) {
        for (int i = 1; i < actionList.count(); i++)
            actionList.at(i)->setVisible(false);
            toolbar->setMaximumWidth(32);
            toolbar->setMinimumWidth(32);
            actionList.at(0)->setText(QString::fromUtf8(">>"));
    } else {
        for (int i = 1; i < actionList.count(); i++)
            actionList.at(i)->setVisible(true);
            toolbar->setMaximumWidth(400);
            toolbar->setMinimumWidth(400);
            actionList.at(0)->setText(QString::fromUtf8("<<"));
    }
}

void QGraphicsInput::wheelEvent(QGraphicsSceneWheelEvent *wheelEvent)
{
    if (wheelEvent->modifiers() == Qt::ControlModifier) {
        int delta = wheelEvent->delta();
        qreal coeff = delta < 0 ? 1 / (1 - delta / (360.)) : 1 + delta / (240.);
        if (coeff >= 1)
            emit increaseMe();
        else
            emit decreaseMe();
        wheelEvent->accept();
        m_view->viewport()->setCursor(*magnifierCursor);
    } else
        QGraphicsScene::wheelEvent(wheelEvent);
}

void QGraphicsInput::keyReleaseEvent(QKeyEvent *keyEvent)
{
    if (keyEvent->key() == Qt::Key_Control)
        m_view->viewport()->setCursor(Qt::ArrowCursor);
    if (keyEvent->modifiers() & Qt::ControlModifier) {
        if ((keyEvent->key() == Qt::Key_Plus) || (keyEvent->key() == Qt::Key_Equal)) {
            emit increaseMe();
            return;
        }
        if ((keyEvent->key() == Qt::Key_Minus) || (keyEvent->key() == Qt::Key_Underscore)) {
            emit decreaseMe();
            return;
        }

    }
    if (keyEvent->key() > Qt::Key_F1) {
        emit keyPressed((int)keyEvent->key());
    }
}

void QGraphicsInput::clearTransform()
{
    if (m_view) {
        QTransform tr = m_view->transform();
        tr.reset();
        m_view->setTransform(tr);
    }
}

void QGraphicsInput::keyPressEvent(QKeyEvent *keyEvent)
{
    if (keyEvent->key() == Qt::Key_Control) {
        m_view->viewport()->setCursor(*magnifierCursor);
        //QApplication::
    }

}

void QGraphicsInput::drawLine(int x1, int y1, int x2, int y2)
{

    QPen pen(QColor("red"));
    pen.setWidth(2);
    this->addLine(x1, y1, x2, y2, pen);
}

void QGraphicsInput::imageOrigin(QPoint &p)
{
    p.setX(m_image->mapToScene(0,0).x());
    p.setY(m_image->mapToScene(0,0).y());
}

QPixmap QGraphicsInput::getCurrentImage()
{
    return (m_image->pixmap());
}

void QGraphicsInput::addBlockColliding(const QRectF &rect)
{
    QGraphicsRectItem *block = newBlock(rect);
    m_CurrentBlockRect = block;
}


