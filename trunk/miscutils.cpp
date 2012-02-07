#include "miscutils.h"

QColor middleColor(const QColor &c1, const QColor &c2, int mul, int div)
{
    QColor res(c1.red()+mul*(c2.red()-c1.red())/div,
               c1.green()+mul*(c2.green()-c1.green())/div,
               c1.blue()+mul*(c2.blue()-c1.blue())/div);
    return res;
}

