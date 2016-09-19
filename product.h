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
#ifndef PRODUCT_H
#define PRODUCT_H

#include <QMap>
#include <functional>
#include <vector>

struct Product {
    struct Node {
        QMap<QString, Node*> nodes;
        QString name;
        QString href;
        ulong size = 0;
        QString checksum;
        bool directory = true;
    };
    ~Product() {
        std::function<void (Node*)> clear = [&](Node* node) -> void { for (Node* n : node->nodes) { clear(n); } delete node; node = nullptr; };
        clear(root);
    }

    QMap<QString, std::pair<QString, QString>> attributes;
    QByteArray quicklook;
    QByteArray manifest;
    Node* root = nullptr;
};

#endif // PRODUCT_H
