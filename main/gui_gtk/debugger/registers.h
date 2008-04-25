/*
 * debugger/registers.h
 * 
 * 
 * Debugger for Mupen64 - davFr
 * Copyright (C) 2002 davFr - robind@esiee.fr
 *
 * Mupen64 is copyrighted (C) 2002 Hacktarux
 * Mupen64 homepage: http://mupen64.emulation64.com
 *         email address: hacktarux@yahoo.fr
 * 
 * This program is free software; you can redistribute it and/
 * or modify it under the terms of the GNU General Public Li-
 * cence as published by the Free Software Foundation; either
 * version 2 of the Licence.
 *
 * This program is distributed in the hope that it will be use-
 * ful, but WITHOUT ANY WARRANTY; without even the implied war-
 * ranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public Licence for more details.
 *
 * You should have received a copy of the GNU General Public
 * Licence along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,
 * USA.
 *
**/

#ifndef GUIGTK_REGISTERS_H
#define GUIGTK_REGISTERS_H

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <gtk/gtk.h>
#include <glib.h>


#include "decoder.h"
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

extern pthread_t thread_n64;

void init_registers();
void update_registers();

#endif //GUIGTK_REGISTERS_H