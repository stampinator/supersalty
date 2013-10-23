#include "boxlineedit.h"
#include <QDebug>

BoxLineEdit::BoxLineEdit(const QStringList &vals) :
    QLineEdit(vals[0])
{
    Q_ASSERT(vals.size() == 6);
    Q_ASSERT(vals[5].toInt() == 0);
    left = vals[1].toInt();
    bottom = vals[2].toInt();
    right = vals[3].toInt();
    top = vals[4].toInt();
}

QTextStream &operator<<(QTextStream &d, const BoxLineEdit &c){
    d << c.text().toLatin1() << " "
      << c.left << " "
      << c.bottom << " "
      << c.right << " "
      << c.top << " "
      << 0 << '\n';
    return d;
}

