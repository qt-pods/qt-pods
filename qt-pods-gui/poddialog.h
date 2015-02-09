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

#pragma once

// Qt includes
#include <QDialog>

// Own includes
#include "pod.h"

namespace Ui {
class PodDialog;
}

class PodDialog : public QDialog {
    Q_OBJECT

public:
    explicit PodDialog(QWidget *parent = 0);
    ~PodDialog();

    void setEditable(bool editable = true);

    void setPod(Pod pod);
    Pod pod();

protected slots:
    void on_pushButtonVisit_clicked();

private:
    Ui::PodDialog *ui;
};
