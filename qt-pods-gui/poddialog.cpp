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

PodDialog::PodDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PodDialog) {
    ui->setupUi(this);
}

void PodDialog::setPod(Pod pod) {
    ui->lineEditPodName->setText(pod.name);
    ui->lineEditPodUrl->setText(pod.url);
}

Pod PodDialog::pod() {
    Pod pod;
    pod.name = ui->lineEditPodName->text();
    pod.url = ui->lineEditPodUrl->text();
    return pod;
}

PodDialog::~PodDialog()
{
    delete ui;
}
