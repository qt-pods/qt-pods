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
#include "poddialog.h"
#include "ui_poddialog.h"

// Qt includes
#include <QDesktopServices>
#include <QUrl>

PodDialog::PodDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PodDialog) {
    ui->setupUi(this);
}

void PodDialog::setEditable(bool editable) {
    if(!editable) {
        ui->lineEditAuthor->setPlaceholderText("<Not provided>");
        ui->lineEditDescription->setPlaceholderText("<Not provided>");
        ui->lineEditWebsite->setPlaceholderText("<Not provided>");
        ui->lineEditPodUrl->setPlaceholderText("<Not provided>");
        ui->lineEditPodName->setPlaceholderText("<Not provided>");
    }

    ui->lineEditAuthor->setReadOnly(!editable);
    ui->lineEditDescription->setReadOnly(!editable);
    ui->lineEditWebsite->setReadOnly(!editable);
    ui->lineEditPodUrl->setReadOnly(!editable);
    ui->lineEditPodName->setReadOnly(!editable);
    ui->comboBoxLicense->setEnabled(editable);
}

void PodDialog::setPod(Pod pod) {
    ui->lineEditPodName->setText        (pod.name);
    ui->lineEditPodUrl->setText         (pod.url);
    ui->lineEditAuthor->setText         (pod.author);
    ui->lineEditWebsite->setText        (pod.website);
    ui->lineEditDescription->setText    (pod.description);
    ui->comboBoxLicense->setCurrentText (pod.license);

    ui->pushButtonVisit->setEnabled(!ui->lineEditWebsite->text().isEmpty());
}

Pod PodDialog::pod() {
    Pod pod;
    pod.name        = ui->lineEditPodName->text();
    pod.url         = ui->lineEditPodUrl->text();
    pod.author      = ui->lineEditAuthor->text();
    pod.website     = ui->lineEditWebsite->text();
    pod.description = ui->lineEditDescription->text();
    pod.license     = ui->comboBoxLicense->currentText();
    return pod;
}

void PodDialog::on_pushButtonVisit_clicked() {
    QDesktopServices::openUrl(QUrl(ui->lineEditWebsite->text()));
}

PodDialog::~PodDialog()
{
    delete ui;
}
