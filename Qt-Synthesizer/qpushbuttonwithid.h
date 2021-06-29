#ifndef QPUSHBUTTONWITHID_H
#define QPUSHBUTTONWITHID_H

#include <QPushButton>

class QPushButtonWithId : public QPushButton
{
    Q_OBJECT
public:
    explicit QPushButtonWithId(int id);
protected:
    int id;
signals:
    void clicked(int);

};

#endif // QPUSHBUTTONWITHID_H
