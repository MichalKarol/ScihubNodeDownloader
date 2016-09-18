//    Copyright (C) 2016 Michał Karol <michal.p.karol@gmail.com>

//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.

//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.

//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#ifndef AUTHDIALOG_H
#define AUTHDIALOG_H

#include <QtWidgets>

class AuthDialog : public QDialog {
    Q_OBJECT

public:
    AuthDialog(QWidget* parent = 0);
    std::pair<QString, std::pair<QByteArray*, QByteArray*>> getAuth();

private:
    QLineEdit* username = nullptr;
    QLineEdit* password = nullptr;

    QPushButton* cancel = nullptr;
    QPushButton* apply = nullptr;
};

#endif // AUTHDIALOG_H
