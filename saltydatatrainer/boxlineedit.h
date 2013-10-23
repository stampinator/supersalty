#ifndef BOXLINEEDIT_H
#define BOXLINEEDIT_H

#include <QLineEdit>
#include <QTextStream>



class BoxLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit BoxLineEdit(const QStringList &vals);
    int left;
    int bottom;
    int right;
    int top;
    int page;


private:

signals:

public slots:

};

QTextStream &operator<<(QTextStream &, const BoxLineEdit &);


#endif // BOXLINEEDIT_H
