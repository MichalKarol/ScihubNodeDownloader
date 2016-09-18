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
#include "authdialog.h"

AuthDialog::AuthDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Authentication requierd");
    username = new QLineEdit(this);
    password = new QLineEdit(this);
    password->setEchoMode(QLineEdit::Password);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    cancel = new QPushButton(style()->standardIcon(QStyle::SP_DialogCancelButton), "", this);
    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
    apply = new QPushButton(style()->standardIcon(QStyle::SP_DialogApplyButton), "", this);
    apply->setDefault(true);
    connect(apply, &QPushButton::clicked, this, &QDialog::accept);
    buttonLayout->addWidget(cancel);
    buttonLayout->addWidget(apply);

    QFormLayout* mainLayout = new QFormLayout(this);
    mainLayout->addRow("Username", username);
    mainLayout->addRow("Password", password);
    mainLayout->addRow(buttonLayout);
}

std::pair<QString, std::pair<QByteArray *, QByteArray *> > AuthDialog::getAuth() {
    QByteArray* key = new QByteArray();
    QByteArray* pass = new QByteArray(password->text().toUtf8());

    for (int i = 0; i < (pass->length()/64 + 1); i++) {
        key->append(QCryptographicHash::hash(QUuid::createUuid().toByteArray(), QCryptographicHash::Sha3_512));
    }

    for (int i = 0; i < pass->length(); i++) {
        (*pass)[i] = (*pass)[i] xor (*key)[i];
    }

    password->clear();

    return std::pair<QString, std::pair<QByteArray*, QByteArray*>>(username->text(), std::pair<QByteArray*, QByteArray*>(pass, key));
}
