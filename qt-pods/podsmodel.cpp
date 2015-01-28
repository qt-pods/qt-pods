#include "podsmodel.h"

PodsModel::PodsModel() {
    reset();
}

void PodsModel::reset() {
    clear();

    setColumnCount(2);
    QStringList headerLabels;
    headerLabels << "Name" << "Source";

    setHorizontalHeaderLabels(headerLabels);
}
