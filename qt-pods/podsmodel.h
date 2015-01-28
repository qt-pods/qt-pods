#ifndef PODSMODEL_H
#define PODSMODEL_H

#include <QStandardItemModel>

class PodsModel : public QStandardItemModel {
public:
    PodsModel();

    void reset();
};

#endif // PODSMODEL_H
