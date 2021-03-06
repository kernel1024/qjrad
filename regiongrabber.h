/*
 *   Copyright (C) 2007 Luca Gugelmann <lucag@student.ethz.ch>
 *
 *   This program is free software; you can redistribute it and/or modify it
 *   under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef REGIONGRABBER_H
#define REGIONGRABBER_H

#ifdef WITH_OCR

#include <QWidget>
#include <QRegion>
#include <QPoint>
#include <QVector>
#include <QRect>

class QPaintEvent;
class QResizeEvent;
class QMouseEvent;

class ZRegionGrabber : public QWidget
{
    Q_OBJECT

    enum MaskType { StrokeMask, FillMask };

public:
    ZRegionGrabber(QWidget *parent, const QRect &startSelection);
    ~ZRegionGrabber() override;

protected Q_SLOTS:
    void init();

Q_SIGNALS:
    void regionGrabbed( const QPixmap & );
    void regionUpdated( const QRect & );

protected:
    void paintEvent( QPaintEvent* e ) override;
    void resizeEvent( QResizeEvent* e ) override;
    void mousePressEvent( QMouseEvent* e ) override;
    void mouseMoveEvent( QMouseEvent* e ) override;
    void mouseReleaseEvent( QMouseEvent* e ) override;
    void mouseDoubleClickEvent( QMouseEvent* ) override;
    void keyPressEvent( QKeyEvent* e ) override;
    void updateHandles();

private:
    QRegion handleMask( MaskType type ) const;
    QPoint limitPointToRect( const QPoint &p, const QRect &r ) const;
    QRect normalizeSelection( const QRect &s ) const;
    void grabRect();

    static bool blendPointer;
    QRect selection;
    bool mouseDown { false };
    bool newSelection { false };
    const int handleSize { 10 };
    QRect* mouseOverHandle { nullptr };
    QPoint dragStartPoint;
    QRect  selectionBeforeDrag;
    bool showHelp { true };
    bool grabbing { false };

    // naming convention for handles
    // T top, B bottom, R Right, L left
    // 2 letters: a corner
    // 1 letter: the handle on the middle of the corresponding side
    QRect TLHandle, TRHandle, BLHandle, BRHandle;
    QRect LHandle, THandle, RHandle, BHandle;
    QRect helpTextRect;

    QVector<QRect*> handles;
    QPixmap pixmap;
};

#endif // OCR

#endif
