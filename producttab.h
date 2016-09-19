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
#ifndef PRODUCTTAB_H
#define PRODUCTTAB_H

#include "product.h"
#include <QtWidgets>
#include <memory>

class ProductTab : public QWidget {
    Q_OBJECT
public:
    explicit ProductTab(std::shared_ptr<Product*> product, QWidget *parent = 0);

private:
    std::shared_ptr<Product*> product;

    QTextBrowser* manifest = nullptr;
    QGraphicsView* quicklook = nullptr;
    QTreeWidget* structure = nullptr;

public slots:
    void updateManifest();
    void updateQuicklook();

signals:
    void downloadNode(Product::Node*);
};

#endif // PRODUCTTAB_H
