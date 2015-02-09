///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//    This file is part of qt-pods.                                          //
//    Copyright (C) 2015 Jacob Dawid, jacob@omg-it.works                     //
//                                                                           //
//    qt-pods is free software: you can redistribute it and/or modify        //
//    it under the terms of the GNU General Public License as published by   //
//    the Free Software Foundation, either version 3 of the License, or      //
//    (at your option) any later version.                                    //
//                                                                           //
//    qt-pods is distributed in the hope that it will be useful,             //
//    but WITHOUT ANY WARRANTY; without even the implied warranty of         //
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          //
//    GNU General Public License for more details.                           //
//                                                                           //
//    You should have received a copy of the GNU General Public License      //
//    along with qt-pods. If not, see <http://www.gnu.org/licenses/>.        //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

// Own includes
#include "podsmodel.h"

PodsModel::PodsModel() {
}

void PodsModel::setModelData(QList<Pod> modelData) {
    beginResetModel();
    _modelData = modelData;
    endResetModel();
}

QList<Pod> PodsModel::modelData() {
    return _modelData;
}

Pod PodsModel::pod(QModelIndex modelIndex) {
    return _modelData.at(modelIndex.row());
}

QList<Pod> PodsModel::pods(QModelIndexList modelIndexList) {
    QList<Pod> pods;
    foreach (QModelIndex index, modelIndexList) {
        pods.append(_modelData.at(index.row()));
    }
    return pods;
}

QVariant PodsModel::data(const QModelIndex &index, int role) const {
    Q_UNUSED(role);
    if(Qt::DisplayRole == role) {
        Pod pod = _modelData.at(index.row());

        switch (index.column()) {
        case PodsModel::PodFieldName:
            return pod.name;
            break;
        case PodsModel::PodFieldAuthor:
            return pod.author;
            break;
        case PodsModel::PodFieldLicense:
            return pod.license;
            break;
        case PodsModel::PodFieldDescription:
            return pod.description;
            break;
        default:
            return "";
        }
    } else {
        return QVariant();
    }
}

Qt::ItemFlags PodsModel::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return QAbstractItemModel::flags(index) | Qt::ItemIsEnabled;
}

QVariant PodsModel::headerData(int section, Qt::Orientation orientation, int role) const {
    Q_UNUSED(role);
    Q_UNUSED(orientation);

    if(Qt::DisplayRole == role) {
        switch (section) {
        case PodsModel::PodFieldName:
            return tr("Name");
            break;
        case PodsModel::PodFieldAuthor:
            return tr("Author");
            break;
        case PodsModel::PodFieldLicense:
            return tr("License");
            break;
        case PodsModel::PodFieldDescription:
            return tr("Description");
            break;
        default:
            return "";
        }
    } else {
        return QVariant();
    }
}

QModelIndex PodsModel::index(int row, int column, const QModelIndex &parent) const {
    Q_UNUSED(parent);
    if(hasIndex(row, column)) {
        return createIndex(row, column);
    }
    return QModelIndex();
}

QModelIndex PodsModel::parent(const QModelIndex &child) const {
    Q_UNUSED(child);
    return QModelIndex();
}

int PodsModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return _modelData.count();
}

int PodsModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return 4;
}
