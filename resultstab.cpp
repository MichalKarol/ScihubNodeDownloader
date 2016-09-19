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
#include "resultstab.h"

ResultsTab::ResultsTab(QString query, QWidget* parent) : QWidget(parent), query(query) {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    view = new QTreeWidget(this);
    view->setColumnCount(2);
    view->header()->hide();
    view->setRootIsDecorated(false);
    connect(view, &QTreeWidget::doubleClicked, [=](QModelIndex index) -> void {
       emit openProduct(products[index.row()]);
    });

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    previousPage = new QPushButton("<", this);
    connect(previousPage, &QPushButton::clicked, [=]() mutable -> void {
        if (page >= 1) { emit changePage(query, --page); }
    });
    nextPage = new QPushButton(">", this);
    connect(nextPage, &QPushButton::clicked, [=]() mutable -> void {
        if (page + 1 < (results/10 + (results%10 == 0 ? 0 : 1))) { emit changePage(query, ++page); }
    });
    buttonLayout->addWidget(previousPage);
    buttonLayout->addWidget(nextPage);

    mainLayout->addWidget(view);
    mainLayout->addLayout(buttonLayout);
}
ResultsTab::~ResultsTab() {
}

void ResultsTab::updateResults(QByteArray res, uint page) {
    this->page = page;
    products.clear();
    view->clear();

    // Parsing XML search resluts
    reader.clear();
    reader.addData(res);
    std::shared_ptr<Product*> product = nullptr;

    while(!reader.atEnd()) {
        reader.readNext();

        if (reader.isStartElement()) {
            if (reader.name() == "entry") {
                product = std::make_shared<Product*>(new Product());
            } else if (reader.name() == "totalResults") {
                results = reader.readElementText().toUInt();
            }

            if (product != nullptr) {
                if (reader.name() == "title"
                        || reader.name() == "id"
                        || reader.name() == "summary") {
                    (*product)->attributes.insert(reader.name().toString(), {"str", reader.readElementText()});
                } else if (reader.name() == "str"
                           || reader.name() == "int"
                           || reader.name() == "long"
                           || reader.name() == "double"
                           || reader.name() == "float"
                           || reader.name() == "date"
                           || reader.name() == "bool") {

                    for(QXmlStreamAttribute attr : reader.attributes()) {

                        if (attr.name().toString() == "name") {
                            QString value = attr.value().toString();
                            (*product)->attributes.insert(value, {reader.name().toString(), reader.readElementText()});
                            break;
                        }
                    }
                }
            }
        } else if (reader.isEndElement() && reader.name() == "entry") {
            products.push_back(product); product = nullptr;
        }
    }


    for (uint  i = 0; i < products.size(); i++) {
        QTreeWidgetItem* item = new QTreeWidgetItem(view);
        item->setText(0, (*products[i])->attributes["title"].second);

        QPushButton* saveProduct = new QPushButton(style()->standardIcon(QStyle::SP_DialogSaveButton), "", this);
        saveProduct->setToolTip((*products[i])->attributes["size"].second);
        connect(saveProduct, &QPushButton::clicked, [this, i]() -> void {
            emit downloadProduct(products[i]);
        });

        view->setItemWidget(item, 1, saveProduct);
        view->addTopLevelItem(item);
    }

    view->resizeColumnToContents(1);
    view->resizeColumnToContents(0);
}
