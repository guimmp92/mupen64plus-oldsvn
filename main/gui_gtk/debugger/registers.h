/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - registers.h                                             *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2002 davFr                                              *
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

#ifndef __GUIGTK_REGISTERS_H__
#define __GUIGTK_REGISTERS_H__

#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>
#include <glib.h>

#include "regGPR.h"
#include "regSpecial.h"
#include "regCop0.h"
#include "regCop1.h"
#include "regAI.h"
#include "regPI.h"
#include "regRI.h"
#include "regSI.h"
#include "regVI.h"

int registers_opened;

void init_registers();
void update_registers();
GtkCList* init_hwreg_clist(int n, char** regnames);

#endif /* __GUIGTK_REGISTERS_H__ */

