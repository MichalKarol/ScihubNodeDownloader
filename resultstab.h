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
#ifndef RESULTSTAB_H
#define RESULTSTAB_H

#include "product.h"
#include <QtWidgets>
#include <memory>

class ResultsTab : public QWidget {
    Q_OBJECT
public:
    explicit ResultsTab(QString query, QWidget *parent = 0);
    ~ResultsTab();

private:
    QString query;
    uint page = 0;
    uint results = 0;
    QXmlStreamReader reader;
    std::vector<std::shared_ptr<Product*>> products;

    QTreeWidget* view = nullptr;

    QPushButton* previousPage = nullptr;
    QPushButton* nextPage = nullptr;

public slots:
    void updateResults(QByteArray res, uint page);

signals:
    void changePage(QString, uint);
    void downloadProduct(std::shared_ptr<Product*>);
    void openProduct(std::shared_ptr<Product*>);
};

#endif // RESULTSTAB_H
