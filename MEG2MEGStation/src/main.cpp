/*
 * main.cpp
 *
 * Author: Andrey Zhdanov
 * Copyright (C) 2015 Department of Neuroscience and Biomedical Engineering,
 * Aalto University School of Science
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "maindialog.h"

#include <iostream>
#include <QtGui>
#include <QApplication>


using namespace std;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainDialog w;

    w.show();
    return a.exec();
}