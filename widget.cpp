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
#include "widget.h"

Widget::Widget(QWidget* parent) : QWidget(parent) {
    resize(800, 500);
    QBoxLayout* mainLayout = new QBoxLayout(QBoxLayout::Down, this);
    mainTabWidget = new QTabWidget(this);
    mainTabWidget->setTabsClosable(true);
    connect(mainTabWidget, &QTabWidget::tabCloseRequested, [this](int index) -> void {
        if (index != 0) { mainTabWidget->removeTab(index); }
    });

    searchTab = new SearchTab(mainTabWidget);
    connect(searchTab, &SearchTab::searchQuery, [this](QString query) -> void { search(query, 0, std::make_shared<ResultsTab*>(nullptr)); });

    networkAccess = new QNetworkAccessManager(this);
    networkAccess->connectToHostEncrypted("https://scihub.copernicus.eu");
    connect(networkAccess, &QNetworkAccessManager::authenticationRequired, [this](QNetworkReply*, QAuthenticator* authenticator) -> void {
        AuthDialog* dialog = new AuthDialog(this);
        if (dialog->exec() == QDialog::Accepted) {
            setAuth(dialog->getAuth());
            authenticator->setUser(username);
            authenticator->setPassword(getPassword());
        }
        delete dialog; dialog = nullptr;
    });

    mainTabWidget->addTab(searchTab, "Search");
    mainLayout->addWidget(mainTabWidget);

}

Widget::~Widget() {
    username.clear();
    delete password.first; password.first = nullptr;
    delete password.second; password.second = nullptr;
}

void Widget::setAuth(std::pair<QString, std::pair<QByteArray*, QByteArray*> > authData) {
    username = authData.first;
    password = authData.second;
}
QString Widget::getPassword() {
    QByteArray tmp;

    for (int i = 0; i < password.first->length(); i++) {
        tmp[i] = (*password.first)[i] xor (*password.second)[i];
    }

    return tmp;
}
void Widget::waitForRequest() {
    QEventLoop loop;
    QThread* thread = new QThread(this);
    connect(thread, &QThread::started, [=]() mutable -> void {
        requestsSem->acquire();
        thread->exit();
    });
    connect(thread, &QThread::finished, &loop, &QEventLoop::quit);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    thread->start();
    loop.exec();
}

void Widget::downloadUrl(QUrl url, QString md5, QString path) {
    std::shared_ptr<QCryptographicHash> md5sum = std::make_shared<QCryptographicHash>(QCryptographicHash::Md5);
    std::shared_ptr<QFile> file = std::make_shared<QFile>(path);

    if (!(*file).open(QFile::ReadWrite)) {
        QMessageBox::critical(this, "Opening file", "Cannot open file.");
        return;
    }

    QNetworkRequest request(url);
    request.setRawHeader("Authorization", "Basic " + QByteArray(QString("%1:%2").arg(username).arg(getPassword()).toUtf8()).toBase64());

    if ((*file).size() != 0 &&
            QMessageBox::question(this, "Resume", "Would you like to resume download?\nResuming could be time consuming due to checksum calculation.") == QMessageBox::Yes) {
        request.setRawHeader("Range","bytes=" + QByteArray::number((*file).size()) + "-");

        // Calculating MD5 checksum
        QProgressDialog* md5Progress = new QProgressDialog("", "", 0, 100, this);
        md5Progress->setWindowTitle("Checksum calculating");
        md5Progress->setCancelButton(nullptr);
        md5Progress->show();

        QThread* thread = new QThread(this);
        QEventLoop loop;
        connect(thread, &QThread::started, [=]() mutable -> void {
            qint64 blocks = ((*file).size() / 4194304 + 1);
            for (qint64 i = 0; i < blocks; i++) {
                (*md5sum).addData((*file).read(4194304));
                md5Progress->setValue((i*100)/blocks);
                md5Progress->setLabelText(QString::number(i) + " / " + QString::number(blocks));
            }
            thread->exit();
        });
        connect(thread, &QThread::finished, &loop, &QEventLoop::quit);
        connect(thread, &QThread::finished, md5Progress, &QProgressDialog::close);
        connect(thread, &QThread::finished, md5Progress, &QProgressDialog::deleteLater);
        connect(thread, &QThread::finished, thread, &QThread::deleteLater);

        thread->start();
        loop.exec();

        (*file).close();
        (*file).open(QFile::ReadWrite | QFile::Append);
    }

    waitForRequest();

    QNetworkReply* reply = networkAccess->get(request);
    QProgressDialog* progress = new QProgressDialog("", "Cancel", 0, 100, this);
    progress->show();

    connect(progress, &QProgressDialog::canceled, reply, &QNetworkReply::abort);
    connect(reply, &QNetworkReply::readyRead, [=]() ->  void {
        QByteArray data = reply->readAll();
        (*md5sum).addData(data);
        (*file).write(data);
    });
    connect(reply, &QNetworkReply::downloadProgress, [=](qint64 bytesReceived, qint64 bytesTotal) {
        if (bytesTotal > 0) {
            progress->setValue((bytesReceived*100)/bytesTotal);
            progress->setLabelText(QString::number(bytesReceived) + " / " + QString::number(bytesTotal));
        } else {
            reply->abort();
        }
    });
    connect(reply, &QNetworkReply::finished, [=]() mutable -> void {
        requestsSem->release();
        QByteArray data = reply->readAll();

        if (reply->error() == QNetworkReply::NoError) {
            (*md5sum).addData(data);
            (*file).write(data);

            (*file).close();
            progress->close();
            delete progress; progress = nullptr;

            if ((*md5sum).result().toHex().toUpper() != md5.toUpper()) {
                QMessageBox::critical(this, "MD5 checksum", "MD5 checksum is incorrect.\nFile could be damaged.");
                if (QMessageBox::question(this, "MD5 checksum", "Download again corrupted file?") == QMessageBox::Yes) {
                    downloadUrl(url, md5, path);
                }
            }
        } else {

            if (reply->hasRawHeader("cause-message") && reply->rawHeader("cause-message").contains("null")) { return; }

            QMessageBox::warning(this, "Conenction error", reply->errorString());
            if (QMessageBox::question(this, "Conenction error", "Download corrupted file again?") == QMessageBox::Yes) {
                downloadUrl(url, md5, path);
            }
        }

    });
}

void Widget::search(QString query, uint page, std::shared_ptr<ResultsTab*> tab) {
    QNetworkRequest request(QUrl("https://scihub.copernicus.eu/dhus/search?q=" + query + "&start=" + QString::number(page * 10)));
    if (!username.isEmpty()) { request.setRawHeader("Authorization", "Basic " + QByteArray(QString("%1:%2").arg(username).arg(getPassword()).toUtf8()).toBase64()); }

    QNetworkReply* reply = networkAccess->get(request);
    connect(reply, &QNetworkReply::finished, [=]() mutable -> void {
        if (reply->error() == QNetworkReply::NoError) {
            if ((*tab) == nullptr) {
                tab = std::make_shared<ResultsTab*>(new ResultsTab(query, this));
                connect((*tab), &ResultsTab::changePage, [=](QString query, uint page) -> void {
                    search(query, page, tab);
                });
                connect((*tab), &ResultsTab::downloadProduct, this, &Widget::download);
                connect((*tab), &ResultsTab::openProduct, this, &Widget::openProduct);

                int newTabIndex = mainTabWidget->addTab((*tab), "Reply");
                mainTabWidget->setCurrentIndex(newTabIndex);
            }

            (*tab)->updateResults(reply->readAll(), page);
        } else {
            QMessageBox::warning(this, "Conenction error", reply->errorString());
        }
    });
}
void Widget::download(std::shared_ptr<Product*> product) {
    QString filename = QFileDialog::getSaveFileName(this, "Save file",
                                                    QDir::homePath() + "/" + (*product)->attributes["title"].second + ".zip", "Zip (*.zip)");

    if (filename.isEmpty()) { return; }
    QUrl url = QUrl("https://scihub.copernicus.eu/dhus/odata/v1/Products('" + (*product)->attributes["id"].second + "')/$value");

    QNetworkRequest md5request(QUrl("https://scihub.copernicus.eu/dhus/odata/v1/Products('" + (*product)->attributes["id"].second + "')/Checksum/Value/$value"));
    md5request.setRawHeader("Authorization", "Basic " + QByteArray(QString("%1:%2").arg(username).arg(getPassword()).toUtf8()).toBase64());

    waitForRequest();

    QNetworkReply* md5Reply = networkAccess->get(md5request);
    connect(md5Reply, &QNetworkReply::finished, [=]() -> void {
        requestsSem->release();
        if (md5Reply->error() == QNetworkReply::NoError) {
            downloadUrl(url, QString(md5Reply->readAll()), filename);
        } else {
            QMessageBox::warning(this, "Conenction error", md5Reply->errorString());
        }
    });
}
void Widget::openProduct(std::shared_ptr<Product*> product) {

    std::shared_ptr<ProductTab*> tab = std::make_shared<ProductTab*>(new ProductTab(product, this));
    connect((*tab), &ProductTab::downloadNode, this, &Widget::downloadNode);

    QString platformSpecific;
    if ((*product)->attributes["platformname"].second == "Sentinel-1") {
        platformSpecific = "Nodes('preview')/Nodes('quick-look.png')";
    } else  if ((*product)->attributes["platformname"].second == "Sentinel-2") {
        platformSpecific = "Nodes('" + (*product)->attributes["title"].second.replace("PRD", "BWI") + ".png')";
    } else {
        return; // Sentinel-3 has no internal structure avaliable
    }

    if ((*product)->quicklook.isEmpty()) {
        waitForRequest();

            QNetworkRequest quicklookRequest(QUrl("https://scihub.copernicus.eu/dhus/odata/v1/Products('" + (*product)->attributes["id"].second
                                             + "')/Nodes('" + (*product)->attributes["filename"].second + "')/" + platformSpecific + "/$value"));
            quicklookRequest.setRawHeader("Authorization", "Basic " + QByteArray(QString("%1:%2").arg(username).arg(getPassword()).toUtf8()).toBase64());

            QNetworkReply* quicklookReply = networkAccess->get(quicklookRequest);
            connect(quicklookReply, &QNetworkReply::finished, [=]() -> void {
                requestsSem->release();
                if (quicklookReply->error() == QNetworkReply::NoError) {
                    (*product)->quicklook = quicklookReply->readAll();
                    (*tab)->updateQuicklook();
                } else {
                    QMessageBox::warning(this, "Conenction error", quicklookReply->errorString());
                }
            });
    } else {
        (*tab)->updateQuicklook();
    }

    if ((*product)->manifest.isEmpty()) {
        waitForRequest();

            QNetworkRequest manifestRequest(QUrl("https://scihub.copernicus.eu/dhus/odata/v1/Products('" + (*product)->attributes["id"].second
                                            + "')/Nodes('" + (*product)->attributes["filename"].second + "')/Nodes('manifest.safe')/$value"));
            manifestRequest.setRawHeader("Authorization", "Basic " + QByteArray(QString("%1:%2").arg(username).arg(getPassword()).toUtf8()).toBase64());

            QNetworkReply* manifestReply = networkAccess->get(manifestRequest);
            connect(manifestReply, &QNetworkReply::finished, [=]() -> void {
                requestsSem->release();
                if (manifestReply->error() == QNetworkReply::NoError) {
                    (*product)->manifest = manifestReply->readAll();
                    (*tab)->updateManifest();
                } else {
                    QMessageBox::warning(this, "Conenction error", manifestReply->errorString());
                }
            });
    } else {
        (*tab)->updateManifest();
    }

    int newTabIndex = mainTabWidget->addTab((*tab), (*product)->attributes["title"].second);
    mainTabWidget->setCurrentIndex(newTabIndex);
}
void Widget::downloadNode(Product::Node* node) {
    QString filename = QFileDialog::getExistingDirectory(this, "Save", QDir::homePath());
    if (filename.isEmpty()) { return; }

    std::function<void (Product::Node*, QString)> build = [&](Product::Node* node, QString path) -> void {
        if (node->directory) {
            QDir currentDir(path);
            currentDir.mkdir(node->name);
            path += ("/" + node->name);

            for (Product::Node* n : node->nodes.values()) {
                build(n, path);
            }
        } else {
            path += ("/" + node->name);
            downloadUrl(QUrl(node->href + "/$value"), node->checksum, path);
        }
    };

    build(node, filename);
}

