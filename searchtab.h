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
#ifndef SEARCHTAB_H
#define SEARCHTAB_H

#include <QtWidgets>

class SearchTab : public QWidget {
    Q_OBJECT
public:
    explicit SearchTab(QWidget *parent = 0);

private:
    QVBoxLayout* mainLayout = nullptr;


    // Common part for all missions
    QFormLayout* topLayout = nullptr;

    QLineEdit* query = nullptr;

    QDateEdit* sensingFrom = nullptr;
    QDateEdit* sensingTo = nullptr;

    QDateEdit* ingestionFrom = nullptr;
    QDateEdit* ingestionTo = nullptr;

    QLineEdit* latitude[3];
    QLineEdit* longitude[3];

    QSpinBox* relativeOrbitNumber = nullptr;
    QComboBox* orbitDirection = nullptr;

    QComboBox* collection = nullptr;


    // Data specific to mission
    QTabWidget* missionLayout = nullptr;

    // Sentinel-1
    QComboBox* sattelite = nullptr;
    QComboBox* productType = nullptr;
    QComboBox* polarisationMode = nullptr;
    QComboBox* sensorMode = nullptr;

    // Sentinel-2
    QDoubleSpinBox* cloudCoverFrom = nullptr;
    QDoubleSpinBox* cloudCoverTo = nullptr;

    // Sentinel-3


    QPushButton* search = nullptr;

private slots:
    void generateQuery();

signals:
    void searchQuery(QString);
};

#endif // SEARCHTAB_H
