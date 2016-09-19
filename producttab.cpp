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
#include "producttab.h"
class ZoomQGraphicsView : public QGraphicsView {
    Q_OBJECT
public:
    ZoomQGraphicsView(QWidget* parent = nullptr) : QGraphicsView(parent) {}

private:
    virtual void wheelEvent (QWheelEvent* event) {
        if (event->modifiers() & Qt::ControlModifier) {
            setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
            double scaleFactor = (event->delta() > 0 ? 1.15 : (1.0 / 1.15));
            scale(scaleFactor, scaleFactor);
        } else {
            QGraphicsView::wheelEvent(event);
        }
    }
};


ProductTab::ProductTab(std::shared_ptr<Product*> product, QWidget *parent) : QWidget(parent), product(product) {
    manifest = new QTextBrowser(this);
    QString metadata;

    for (auto i = (*product)->attributes.begin(); i != (*product)->attributes.end(); i++) {
        metadata.append((*i).first + " " + i.key() + ": " + (*i).second + "\n");
    }
    manifest->setText(metadata);

    quicklook = new ZoomQGraphicsView(this);
    quicklook->setDragMode(QGraphicsView::ScrollHandDrag);

    structure = new QTreeWidget(this);
    structure->header()->hide();
    structure->setColumnCount(2);

    QGridLayout* mainLayout = new QGridLayout(this);
    mainLayout->addWidget(manifest, 0, 0);
    mainLayout->addWidget(quicklook, 1, 0);
    mainLayout->addWidget(structure, 0, 1, 2, 1);
}

void ProductTab::updateManifest() {
    Product::Node*& root = (*product)->root;

    if ((*product)->root == nullptr) {
        QString url = "https://scihub.copernicus.eu/dhus/odata/v1/Products('" + (*product)->attributes["id"].second
                + "')/Nodes('" + (*product)->attributes["filename"].second + "')";
        root = new Product::Node;
        root->name = (*product)->attributes["filename"].second;
        root->href = url;

        Product::Node* parentNode  = nullptr;
        Product::Node* node = nullptr;


        QXmlStreamReader reader((*product)->manifest);
        while(!reader.atEnd()) {
            reader.readNext();

            if (reader.isStartElement()) {

                if (reader.name() == "byteStream") {
                    node = new Product::Node;
                    node->directory = false;
                    node->size = reader.attributes()[1].value().toULong();
                } else if (reader.name() == "fileLocation") {
                    QStringList path = reader.attributes()[1].value().toString().split("/");
                    path.removeFirst();

                    QString localUrl = url;
                    parentNode = root;

                    for (int i = 0; i < path.size() - 1; i++) {
                        localUrl += ("/Nodes('" + path[i] + "')");

                        if (!parentNode->nodes.contains(path[i])) {
                            Product::Node* tmpNode = new Product::Node;
                            tmpNode->name = path[i];
                            tmpNode->href = localUrl;
                            parentNode->nodes.insert(path[i].toUpper(), tmpNode);
                            parentNode = tmpNode;
                        } else {
                            parentNode = parentNode->nodes[path[i]];
                        }
                    }


                    node->name = path.last();
                    node->href = localUrl + ("/Nodes('" + path.last() + "')");
                    parentNode->nodes.insert(path.last(), node);

                } else if (reader.name() == "checksum") {

                    if (reader.attributes()[0].value().toString() != "MD5") {
                        qDebug() << "Checksum algorithm different then MD5";
                    }
                    node->checksum = reader.readElementText();
                }
            } else if (reader.isEndElement() && reader.name() == "byteStream") {
                node = nullptr;
            }
        }
    }

    auto setItem = [this](Product::Node* node, QTreeWidgetItem* item) -> void {
        item->setText(0, node->name);
        QPushButton* saveButton = new QPushButton(style()->standardIcon(QStyle::SP_DialogSaveButton), "", this);
        connect(saveButton, &QPushButton::clicked, [this, node]() -> void { emit downloadNode(node); });
        structure->setItemWidget(item, 1, saveButton);
    };
    std::function<QTreeWidgetItem* (Product::Node*, QTreeWidgetItem*)> build = [&setItem, &build](Product::Node* node, QTreeWidgetItem* parent) -> QTreeWidgetItem* {
        QTreeWidgetItem* item = new QTreeWidgetItem(parent);
        setItem(node, item);
        for (Product::Node* n : node->nodes.values()) { build(n, item); }
        return item;
    };

    structure->addTopLevelItem(build(root, nullptr));
    structure->resizeColumnToContents(1);
    structure->resizeColumnToContents(0);
}

void ProductTab::updateQuicklook() {
    QGraphicsScene* scene = new QGraphicsScene(quicklook);
    QPixmap tmpPixmap;
    tmpPixmap.loadFromData((*product)->quicklook);
    QGraphicsItem* item = scene->addPixmap(tmpPixmap);
    quicklook->setScene(scene);
    quicklook->fitInView(item, Qt::KeepAspectRatio);
}

#include "producttab.moc"
