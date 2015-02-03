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
#include "sourcesdialog.h"
#include "ui_sourcesdialog.h"

SourcesDialog::SourcesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SourcesDialog) {
    ui->setupUi(this);
}

SourcesDialog::~SourcesDialog() {
    delete ui;
}

void SourcesDialog::setSources(QStringList sources) {
    foreach(QString source, sources) {
        ui->listWidgetSources->addItem(source);
    }
}

QStringList SourcesDialog::sources() {
    int count = ui->listWidgetSources->count();
    QStringList sources;
    for(int i = 0; i < count; i++) {
        sources.append(ui->listWidgetSources->item(i)->text());
    }
    return sources;
}

void SourcesDialog::on_pushButtonAddSource_clicked() {
    QString source = ui->lineEditSource->text();
    if(!source.isEmpty()) {
        ui->listWidgetSources->addItem(source);
        ui->lineEditSource->setText("");
    }
}

void SourcesDialog::on_pushButtonRemoveSources_clicked() {
    QItemSelectionModel *selectionModel = ui->listWidgetSources->selectionModel();
    QModelIndexList modelIndices = selectionModel->selectedRows();

    int removedItems = 0;
    foreach(QModelIndex modelIndex, modelIndices) {
        if(ui->listWidgetSources->model()->removeRow(modelIndex.row() - removedItems)) {
            removedItems++;
        }
    }
}
