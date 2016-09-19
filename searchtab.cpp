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
#include "searchtab.h"

SearchTab::SearchTab(QWidget *parent) : QWidget(parent) {
    mainLayout = new QVBoxLayout(this);

    // Helpful functions
    auto checkBoxToWidget = [](QWidget* widget) -> QLayout* {
        widget->setEnabled(false);
        QCheckBox* checkBox = new QCheckBox(widget);
        connect(checkBox, &QCheckBox::toggled, [widget](bool state) -> void {
            widget->setEnabled(state);
        });

        QHBoxLayout* HBoxlayout = new QHBoxLayout();
        HBoxlayout->addWidget(widget);
        HBoxlayout->addWidget(checkBox);
        return HBoxlayout;
    };
    auto checkBoxToLayout = [this](QLayout* layout) -> QLayout* {
        for (int i = 0; i < layout->count(); i++) {
            QWidget* widget = layout->itemAt(i)->widget();
            if (widget != nullptr) { widget->setEnabled(false); }
        }
        QCheckBox* checkBox = new QCheckBox(this);
        connect(checkBox, &QCheckBox::toggled, [layout](bool state) -> void {
            for (int i = 0; i < layout->count(); i++) {
                QWidget* widget = layout->itemAt(i)->widget();
                if (widget != nullptr) { widget->setEnabled(state); }
            }
        });

        QHBoxLayout* HBoxlayout = new QHBoxLayout();
        HBoxlayout->addLayout(layout);
        HBoxlayout->addWidget(checkBox);
        return HBoxlayout;
    };

    // Common part for all missions
    topLayout = new QFormLayout();

    query = new QLineEdit(this);

    QHBoxLayout* sensingLayout = new QHBoxLayout();
    sensingFrom = new QDateEdit(QDate::currentDate(), this);
    sensingFrom->setDateRange(QDate(2014, 4, 3), QDate::currentDate());
    sensingFrom->setCalendarPopup(true);
    connect(sensingFrom, &QDateEdit::dateChanged, [this](QDate date) -> void {
        sensingFrom->setDate((date <= sensingTo->date() ? date : sensingTo->date()));
    });
    sensingTo = new QDateEdit(QDate::currentDate(), this);
    sensingTo->setDateRange(QDate(2014, 4, 3), QDate::currentDate());
    sensingTo->setCalendarPopup(true);
    connect(sensingTo, &QDateEdit::dateChanged, [this](QDate date) -> void {
        sensingTo->setDate((date >= sensingFrom->date() ? date : sensingFrom->date()));
    });
    sensingLayout->addWidget(sensingFrom);
    sensingLayout->addWidget(sensingTo);

    QHBoxLayout* ingestionLayout = new QHBoxLayout();
    ingestionFrom = new QDateEdit(QDate::currentDate(), this);
    ingestionFrom->setDateRange(QDate(2014, 4, 3), QDate::currentDate());
    ingestionFrom->setCalendarPopup(true);
    connect(ingestionFrom, &QDateEdit::dateChanged, [this](QDate date) -> void {
        ingestionFrom->setDate((date <= ingestionTo->date() ? date : ingestionTo->date()));
    });
    ingestionTo = new QDateEdit(QDate::currentDate(), this);
    ingestionTo->setDateRange(QDate(2014, 4, 3), QDate::currentDate());
    ingestionTo->setCalendarPopup(true);
    connect(ingestionTo, &QDateEdit::dateChanged, [this](QDate date) -> void {
        ingestionTo->setDate((date >= ingestionFrom->date() ? date : ingestionFrom->date()));
    });
    ingestionLayout->addWidget(ingestionFrom);
    ingestionLayout->addWidget(ingestionTo);

    QGridLayout* geoLayout = new QGridLayout();
    QIntValidator* bigValidator = new QIntValidator(-180, 180, this);
    QIntValidator* smallValidator = new QIntValidator(0, 60, this);

    for (uint  i = 0; i < 3; i++) {
        latitude[i] = new QLineEdit(this);
        latitude[i]->setFixedWidth(ingestionFrom->width());
        geoLayout->addWidget(latitude[i], 0, i);
    }
    latitude[0]->setValidator(bigValidator);
    latitude[1]->setValidator(smallValidator);
    latitude[2]->setValidator(smallValidator);

    for (uint  i = 0; i < 3; i++) {
        longitude[i] = new QLineEdit(this);
        longitude[i]->setFixedWidth(ingestionFrom->width());
        geoLayout->addWidget(longitude[i], 1, i);
    }
    longitude[0]->setValidator(bigValidator);
    longitude[1]->setValidator(smallValidator);
    longitude[2]->setValidator(smallValidator);

    relativeOrbitNumber = new QSpinBox(this);
    relativeOrbitNumber->setRange(0, 1000);

    orbitDirection = new QComboBox(this);
    orbitDirection->addItems({"Ascending", "Descending"});

    collection = new QComboBox(this);
    collection->addItems({"S1B_24AUG2016", "S3_OLCI_sample"});
    connect(collection, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [this](int index) -> void {
        missionLayout->setCurrentIndex((index == 0 ? 0 : 2));
    });

    topLayout->addRow("Query", query);
    topLayout->addRow("Sensing period", checkBoxToLayout(sensingLayout));
    topLayout->addRow("Ingestion period", checkBoxToLayout(ingestionLayout));
    topLayout->addRow("Geographical point", checkBoxToLayout(geoLayout));
    topLayout->addRow("Relative orbit number", checkBoxToWidget(relativeOrbitNumber));
    topLayout->addRow("Orbit direction", checkBoxToWidget(orbitDirection));
    topLayout->addRow("Collection", checkBoxToWidget(collection));


    // Data specific to mission
    missionLayout = new QTabWidget(this);

    // Sentinel-1
    QWidget* sentinel1 = new QWidget(this);
    QFormLayout* sentinel1Layout = new QFormLayout(sentinel1);
    sattelite = new QComboBox(sentinel1);
    sattelite->addItems({"Sentinel-1A", "Sentinel-1B"});

    productType = new QComboBox(sentinel1);
    productType->addItems({"SLC", "GRD", "OCN"});

    polarisationMode = new QComboBox(sentinel1);
    polarisationMode->addItems({"HH", "VV", "HV", "VH", "HH+HV", "VV+VH"});

    sensorMode = new QComboBox(sentinel1);
    sensorMode->addItems({"SM", "IW", "EW", "WV"});

    sentinel1Layout->addRow("Sattelite", checkBoxToWidget(sattelite));
    sentinel1Layout->addRow("Product type", checkBoxToWidget(productType));
    sentinel1Layout->addRow("Polarisation mode", checkBoxToWidget(polarisationMode));
    sentinel1Layout->addRow("Sensor mode", checkBoxToWidget(sensorMode));
    missionLayout->addTab(sentinel1, "Sentinel-1");

    // Sentinel-2
    QWidget* sentinel2 = new QWidget(this);
    QFormLayout* sentinel2Layout = new QFormLayout(sentinel2);

    QHBoxLayout* cloudCoverLayout = new QHBoxLayout();
    cloudCoverFrom = new QDoubleSpinBox(sentinel2);
    cloudCoverFrom->setRange(0, 100);
    connect(cloudCoverFrom, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [this](double value) -> void{
        cloudCoverFrom->setValue((value <= cloudCoverTo->value() ? value : cloudCoverTo->value()));
    });
    cloudCoverTo = new QDoubleSpinBox(sentinel2);
    cloudCoverTo->setRange(0, 100);
    connect(cloudCoverTo, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [this](double value) -> void{
        cloudCoverTo->setValue((value >= cloudCoverFrom->value() ? value : cloudCoverFrom->value()));
    });
    cloudCoverLayout->addWidget(cloudCoverFrom);
    cloudCoverLayout->addWidget(cloudCoverTo);

    sentinel2Layout->addRow("Cloud cover", checkBoxToLayout(cloudCoverLayout));
    missionLayout->addTab(sentinel2, "Sentinel-2");

    // Sentinel-3
    QWidget* sentinel3 = new QWidget(this);
    missionLayout->addTab(sentinel3, "Sentinel-3");


    search = new QPushButton("Search", this);
    connect(search, &QPushButton::clicked, this, &SearchTab::generateQuery);

    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(missionLayout);
    mainLayout->addWidget(search);
}

void SearchTab::generateQuery() {
    auto ifEnabled = [](QString string, QWidget* widget) -> QString {
        return (widget->isEnabled() ? string : "");
    };
    auto brackets = [](QString atribute, QString arg) -> QString {
        return "(" + atribute + ":" + arg + ")";
    };
    auto bracketsRange = [](QString atribute, QString arg1, QString arg2) -> QString {
        return "(" + atribute + ":[" + arg1 + " TO " + arg2 + "])";
    };
    auto merge = [](std::initializer_list<QString> list) -> QString {
        QString merged;
        for (QString string : list) { merged.append(string + (string.isEmpty() ? "" : " AND ")); }
        merged.chop(5);
        return merged;
    };

    QString generated = merge({
                                  query->text(),
                                  ifEnabled(bracketsRange("beginPosition", QDateTime(sensingFrom->date(), QTime(0,0,0)).toString(Qt::ISODate) + ".000Z",
                                                                           QDateTime(sensingTo->date(), QTime(23,59,59)).toString(Qt::ISODate) + ".999Z"), sensingFrom),
                                  ifEnabled(bracketsRange("endPosition", QDateTime(sensingFrom->date(), QTime(0,0,0)).toString(Qt::ISODate) + ".000Z",
                                                                         QDateTime(sensingTo->date(), QTime(23,59,59)).toString(Qt::ISODate) + ".999Z"), sensingTo),
                                  ifEnabled(bracketsRange("ingestionDate", QDateTime(ingestionFrom->date(), QTime(0,0,0)).toString(Qt::ISODate),
                                                                           QDateTime(ingestionTo->date(), QTime(23,59,59)).toString(Qt::ISODate)), ingestionFrom),
                                  ifEnabled(brackets("footprint", "\"Intersects(" + QString::number((latitude[0]->text().toInt() < 0 ? -1 : 1) * (fabs(latitude[0]->text().toDouble()) + latitude[1]->text().toDouble()/60 + latitude[2]->text().toDouble()/3600))
                                                                           + ", " + QString::number((longitude[0]->text().toInt() < 0 ? -1 : 1) * (fabs(longitude[0]->text().toDouble()) + longitude[1]->text().toDouble()/60 + longitude[2]->text().toDouble()/3600))
                                                                           + ")\""), latitude[0]),
                                  ifEnabled(brackets("relativeorbitnumber", QString::number(relativeOrbitNumber->value())), relativeOrbitNumber),
                                  ifEnabled(brackets("orbitdirection", orbitDirection->currentText()), orbitDirection),
                                  ifEnabled(brackets("collection", collection->currentText()), collection),
                                  ifEnabled(brackets("platformname", missionLayout->tabText(missionLayout->currentIndex())), missionLayout),
                              });

    if (missionLayout->isEnabled()) {
        switch (missionLayout->currentIndex()) {
        case 0: {
            generated = merge({
                                  generated,
                                  ifEnabled(brackets("filename", (sattelite->currentIndex() == 0 ? "S1A_*" : "S1B_*")), sattelite),
                                  ifEnabled(brackets("producttype", productType->currentText()), productType),
                                  ifEnabled(brackets("polarisationmode", polarisationMode->currentText()), polarisationMode),
                                  ifEnabled(brackets("sensormode", sensorMode->currentText()), sensorMode),
                              });
        } break;
        case 1: {
            generated = merge({
                                  generated,
                                  ifEnabled(bracketsRange("cloudcoverpercentage", QString::number(cloudCoverFrom->value()),
                                                                                   QString::number(cloudCoverTo->value())), cloudCoverFrom),
                              });
        } break;
        case 2: {
        } break;
        }
    }

    emit searchQuery(generated);

    qDebug() << generated;
}
