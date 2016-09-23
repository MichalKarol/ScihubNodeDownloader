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
#ifndef WIDGET_H
#define WIDGET_H

#include "authdialog.h"
#include "product.h"
#include "searchtab.h"
#include "resultstab.h"
#include "producttab.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QAuthenticator>
#include <QtWidgets>
#include <queue>

class Widget : public QWidget {
    Q_OBJECT

public:
    Widget(QWidget *parent = 0);
    void setAuth(std::pair<QString, std::pair<QByteArray *, QByteArray *>> authData);
    ~Widget();

private:
    QString username;
    std::pair<QByteArray*, QByteArray*> password;

    QTabWidget* mainTabWidget = nullptr;
    SearchTab* searchTab = nullptr;
    QLabel* sciHubStatus = nullptr;

    QNetworkAccessManager* networkAccess = nullptr;
    QSemaphore* requestsSem = new QSemaphore(2);

    QString getPassword();
    void downloadUrl(QUrl url, QString md5, QString path);
    void waitForRequest();

private slots:
    void search(QString, uint, std::shared_ptr<ResultsTab *>);
    void download(std::shared_ptr<Product*>);
    void openProduct(std::shared_ptr<Product*>);
    void downloadNode(Product::Node*);
};

#endif // WIDGET_H
