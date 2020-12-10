#ifndef XCBTOOLS_H
#define XCBTOOLS_H

#ifdef WITH_OCR

#include <QObject>
#include <QPixmap>
#include <QVector>
#include <QRect>

#include <xcb/xcb.h>
#include <xcb/xcb_image.h>
#include <xcb/xfixes.h>

class ZXCBTools : public QObject
{
    Q_OBJECT

private:
    xcb_connection_t* m_connection;
    static xcb_connection_t* connection();

public:
    explicit ZXCBTools(QObject *parent = nullptr);
    ~ZXCBTools() override;

    static xcb_window_t appRootWindow();
    static QPixmap convertFromNative(xcb_image_t *xcbImage);
    static bool getWindowGeometry(xcb_window_t window, int &x, int &y, int &w, int &h);
    static QPixmap getWindowPixmap(xcb_window_t window, bool blendPointer);
    static QPixmap blendCursorImage(const QPixmap &pixmap, int x, int y, int width, int height);
    static void getWindowsRecursive( QVector<QRect> &windows, xcb_window_t w, int rx = 0, int ry = 0, int depth = 0 );
    static xcb_window_t findRealWindow( xcb_window_t w, int depth = 0 );
    static xcb_window_t windowUnderCursor( bool includeDecorations = true );
};

#endif // OCR

#endif // XCBTOOLS_H

