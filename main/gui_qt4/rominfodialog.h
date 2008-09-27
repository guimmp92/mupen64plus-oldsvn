/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*   Mupen64plus - rominfodialog.h                                         *
*   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
*   Copyright (C) 2008 slougi                                             *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef MUPEN64_ROM_INFO_DIALOG_H
#define MUPEN64_ROM_INFO_DIALOG_H

#include <QDialog>
#include <QPersistentModelIndex>
#include "ui_rominfodialog.h"

class RomInfoDialog : public QDialog, public Ui_RomInfoDialog
{
    Q_OBJECT
    public:
        RomInfoDialog(QWidget* parent = 0);

        QPersistentModelIndex index;

    public slots:
        virtual void accept();
};

#endif // MUPEN64_ROM_INFO_DIALOG_H

