#ifdef WITH_OCR

#include <algorithm>
#include <xcb/xcb.h>

#include <QPointer>
#include <QMutex>
#include <QCursor>
#include <QPoint>
#include <QPainter>
#include <QScreen>
#include <QApplication>
#include <QDebug>

#include "xcbtools.h"

static const int minSize = 8;

ZXCBTools::ZXCBTools(QObject *parent)
    : QObject(parent)
{
    m_connection = xcb_connect(nullptr, nullptr);
    int res = xcb_connection_has_error(m_connection);
    if (res>0)
        qCritical() << "Error in XCB connection " << res;
}

ZXCBTools::~ZXCBTools()
{
    xcb_disconnect(m_connection);
}

xcb_connection_t *ZXCBTools::connection()
{
    static QPointer<ZXCBTools> inst;
    static QMutex instMutex;

    instMutex.lock();

    if (inst.isNull())
        inst = new ZXCBTools(QApplication::instance());

    instMutex.unlock();

    return inst->m_connection;
}

xcb_window_t ZXCBTools::appRootWindow()
{
    xcb_connection_t *c = connection();
    const xcb_setup_t *setup = xcb_get_setup(c);
    xcb_screen_iterator_t it = xcb_setup_roots_iterator(setup);

    while (it.rem>0) {
        xcb_window_t root = it.data->root;
        xcb_query_pointer_cookie_t pc = xcb_query_pointer(c, root);
        QScopedPointer<xcb_query_pointer_reply_t,QScopedPointerPodDeleter>
                pr(xcb_query_pointer_reply(c, pc, nullptr));
        if ((pr != nullptr) && (pr->same_screen > 0))
            return root;

        xcb_screen_next(&it);
    }

    return 0;
}

QPixmap ZXCBTools::convertFromNative(xcb_image_t *xcbImage)
{
    QImage::Format format = QImage::Format_Invalid;
    quint32 *pixels = nullptr;

    switch (xcbImage->depth) {
    case 1:
        format = QImage::Format_MonoLSB;
        break;
    case 16: // NOLINT
        format = QImage::Format_RGB16;
        break;
    case 24: // NOLINT
        format = QImage::Format_RGB32;
        break;
    case 30: // NOLINT
        // Qt doesn't have a matching image format. We need to convert manually
        pixels = reinterpret_cast<quint32 *>(xcbImage->data);
        for (uint i = 0; i < (xcbImage->size / 4); i++) {
            int r = (pixels[i] >> 22) & 0xff; // NOLINT
            int g = (pixels[i] >> 12) & 0xff; // NOLINT
            int b = (pixels[i] >>  2) & 0xff; // NOLINT

            pixels[i] = qRgba(r, g, b, 0xff); // NOLINT
        }
        // fall through, Qt format is still Format_ARGB32_Premultiplied
        [[clang::fallthrough]];
    case 32: // NOLINT
        format = QImage::Format_ARGB32_Premultiplied;
        break;
    default:
        return QPixmap(); // we don't know
    }

    QImage image(xcbImage->data, xcbImage->width, xcbImage->height, format);

    if (image.isNull()) {
        return QPixmap();
    }

    // work around an abort in QImage::color

    if (image.format() == QImage::Format_MonoLSB) {
        image.setColorCount(2);
        image.setColor(0, QColor(Qt::white).rgb());
        image.setColor(1, QColor(Qt::black).rgb());
    }

    // done

    return QPixmap::fromImage(image);
}

bool ZXCBTools::getWindowGeometry(xcb_window_t window, int &x, int &y, int &w, int &h)
{
    xcb_connection_t *xcbConn = connection();

    xcb_get_geometry_cookie_t geomCookie = xcb_get_geometry_unchecked(xcbConn, window);
    QScopedPointer<xcb_get_geometry_reply_t,QScopedPointerPodDeleter>
            geomReply(xcb_get_geometry_reply(xcbConn,geomCookie, nullptr));

    if (geomReply) {
        x = geomReply->x;
        y = geomReply->y;
        w = geomReply->width;
        h = geomReply->height;
        return true;
    }

    x = 0;
    y = 0;
    w = 0;
    h = 0;
    return false;
}

QPixmap ZXCBTools::getWindowPixmap(xcb_window_t window, bool blendPointer)
{
    xcb_connection_t *xcbConn = connection();

    // first get geometry information for our drawable

    xcb_get_geometry_cookie_t geomCookie = xcb_get_geometry_unchecked(xcbConn, window);
    QScopedPointer<xcb_get_geometry_reply_t,QScopedPointerPodDeleter>
            geomReply(xcb_get_geometry_reply(xcbConn, geomCookie, nullptr));

    // then proceed to get an image

    if (geomReply.isNull()) return QPixmap();

    QScopedPointer<xcb_image_t,QScopedPointerPodDeleter>
            xcbImage(xcb_image_get(xcbConn,
                                   window,
                                   geomReply->x,
                                   geomReply->y,
                                   geomReply->width,
                                   geomReply->height,
                                   ~0U,
                                   XCB_IMAGE_FORMAT_Z_PIXMAP
                                   ));


    // if the image is null, this means we need to get the root image window
    // and run a crop

    if (!xcbImage) {
        QRect geom(geomReply->x, geomReply->y, geomReply->width, geomReply->height);
        return getWindowPixmap(appRootWindow(), blendPointer).copy(geom);
    }

    // now process the image

    QPixmap nativePixmap = convertFromNative(xcbImage.data());
    if (!(blendPointer))
        return nativePixmap;

    // now we blend in a pointer image

    xcb_get_geometry_cookie_t geomRootCookie = xcb_get_geometry_unchecked(xcbConn, geomReply->root);
    QScopedPointer<xcb_get_geometry_reply_t,QScopedPointerPodDeleter>
            geomRootReply(xcb_get_geometry_reply(xcbConn, geomRootCookie, nullptr));

    xcb_translate_coordinates_cookie_t translateCookie = xcb_translate_coordinates_unchecked(
        xcbConn, window, geomReply->root, geomRootReply->x, geomRootReply->y);
    QScopedPointer<xcb_translate_coordinates_reply_t,QScopedPointerPodDeleter>
            translateReply(xcb_translate_coordinates_reply(xcbConn, translateCookie, nullptr));

    return blendCursorImage(nativePixmap, translateReply->dst_x,translateReply->dst_y,
                            geomReply->width, geomReply->height);
}

QPixmap ZXCBTools::blendCursorImage(const QPixmap &pixmap, int x, int y, int width, int height)
{
    // first we get the cursor position, compute the co-ordinates of the region
    // of the screen we're grabbing, and see if the cursor is actually visible in
    // the region

    QPoint cursorPos = QCursor::pos();
    QRect screenRect(x, y, width, height);

    if (!(screenRect.contains(cursorPos))) {
        return pixmap;
    }

    // now we can get the image and start processing

    xcb_connection_t *xcbConn = connection();

    xcb_xfixes_get_cursor_image_cookie_t  cursorCookie = xcb_xfixes_get_cursor_image_unchecked(xcbConn);
    QScopedPointer<xcb_xfixes_get_cursor_image_reply_t,QScopedPointerPodDeleter>
            cursorReply(xcb_xfixes_get_cursor_image_reply(xcbConn, cursorCookie, nullptr));

    if (!cursorReply)
        return pixmap;

    quint32 *pixelData = xcb_xfixes_get_cursor_image_cursor_image(cursorReply.data());
    if (!pixelData)
        return pixmap;

    // process the image into a QImage

    QImage cursorImage = QImage(reinterpret_cast<quint8 *>(pixelData),
                                cursorReply->width, cursorReply->height,
                                QImage::Format_ARGB32_Premultiplied);

    // a small fix for the cursor position for fancier cursors

    cursorPos -= QPoint(cursorReply->xhot, cursorReply->yhot);

    // now we translate the cursor point to our screen rectangle

    cursorPos -= QPoint(x, y);

    // and do the painting

    QPixmap blendedPixmap = pixmap;
    QPainter painter(&blendedPixmap);
    painter.drawImage(cursorPos, cursorImage);

    // and done

    return blendedPixmap;
}

// Recursively iterates over the window w and its children, thereby building
// a tree of window descriptors. Windows in non-viewable state or with height
// or width smaller than minSize will be ignored.
void ZXCBTools::getWindowsRecursive( QVector<QRect> &windows, xcb_window_t w, int rx, int ry, int depth )
{
    xcb_connection_t* c = connection();

    xcb_get_window_attributes_cookie_t ac = xcb_get_window_attributes_unchecked(c, w);
    QScopedPointer<xcb_get_window_attributes_reply_t,QScopedPointerPodDeleter>
            atts(xcb_get_window_attributes_reply(c, ac, nullptr));

    xcb_get_geometry_cookie_t gc = xcb_get_geometry_unchecked(c, w);
    QScopedPointer<xcb_get_geometry_reply_t,QScopedPointerPodDeleter>
            geom(xcb_get_geometry_reply(c, gc, nullptr));

    if ( atts && geom &&
            atts->map_state == XCB_MAP_STATE_VIEWABLE &&
            geom->width >= minSize && geom->height >= minSize ) {
        int x = 0;
        int y = 0;
        if ( depth != 0 ) {
            x = geom->x + rx;
            y = geom->y + ry;
        }

        QRect r( x, y, geom->width, geom->height );
        if (!windows.contains(r))
            windows.append(r);

        xcb_query_tree_cookie_t tc = xcb_query_tree_unchecked(c, w);
        QScopedPointer<xcb_query_tree_reply_t,QScopedPointerPodDeleter>
                tree(xcb_query_tree_reply(c, tc, nullptr));

        if (tree) {
            xcb_window_t* child = xcb_query_tree_children(tree.data());
            for (unsigned int i=0;i<tree->children_len;i++)
                getWindowsRecursive(windows, child[i], x, y, depth +1); // NOLINT
        }
    }

    if ( depth == 0 ) {
        std::sort(windows.begin(), windows.end(), []( const QRect& r1, const QRect& r2 )
        {
            return r1.width() * r1.height() < r2.width() * r2.height();
        });
    }
}

xcb_window_t ZXCBTools::findRealWindow( xcb_window_t w, int depth )
{
    const char *wm_state_s = "WM_STATE";

    xcb_connection_t* c = connection();

    if( depth > 5 )
        return 0;

    xcb_intern_atom_cookie_t ac = xcb_intern_atom(c, 0, strlen(wm_state_s), wm_state_s);
    QScopedPointer<xcb_intern_atom_reply_t,QScopedPointerPodDeleter>
            wm_state(xcb_intern_atom_reply(c, ac, nullptr));

    if (wm_state.isNull()) {
        qWarning() << "Unable to allocate xcb atom";
        return 0;
    }

    xcb_get_property_cookie_t pc = xcb_get_property(c, 0, w, wm_state->atom, XCB_GET_PROPERTY_TYPE_ANY, 0, 0 );
    QScopedPointer<xcb_get_property_reply_t,QScopedPointerPodDeleter>
            pr(xcb_get_property_reply(c, pc, nullptr));

    if (pr && pr->type != XCB_NONE)
        return w;

    xcb_window_t ret = XCB_NONE;

    xcb_query_tree_cookie_t tc = xcb_query_tree_unchecked(c, w);
    QScopedPointer<xcb_query_tree_reply_t,QScopedPointerPodDeleter>
            tree(xcb_query_tree_reply(c, tc, nullptr));

    if (tree) {
        xcb_window_t* child = xcb_query_tree_children(tree.data());
        for (unsigned int i=0;i<tree->children_len;i++)
            ret = findRealWindow(child[i], depth +1 ); // NOLINT
    }

    return ret;
}

xcb_window_t ZXCBTools::windowUnderCursor( bool includeDecorations )
{
    xcb_connection_t* c = connection();

    xcb_window_t child = appRootWindow();

    xcb_query_pointer_cookie_t pc = xcb_query_pointer(c,appRootWindow());
    QScopedPointer<xcb_query_pointer_reply_t,QScopedPointerPodDeleter>
            pr(xcb_query_pointer_reply(c, pc, nullptr));

    if (pr && pr->child!=XCB_NONE )
        child = pr->child;

    if( !includeDecorations ) {
        xcb_window_t real_child = findRealWindow( child );

        if( real_child != XCB_NONE ) { // test just in case
            child = real_child;
        }
    }

    return child;
}

#endif
