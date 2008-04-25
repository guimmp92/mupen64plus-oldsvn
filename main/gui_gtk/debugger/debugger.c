/*
 * debugger/debugger.c
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

#include <glib.h>
#include "debugger.h"

GdkColor    color_modif,    // Color of modified register.
                   color_ident;    // Unchanged register.

GtkWidget   *winRegisters;

void init_debugger_frontend()
{
    color_modif.red = 0x0000;
    color_modif.green = 0xA000;
    color_modif.blue = 0xFFFF;

    color_ident.red = 0xFFFF;
    color_ident.green = 0xFFFF;
    color_ident.blue = 0xFFFF;

    gdk_threads_enter();
    init_registers();
    gdk_threads_leave();

    gdk_threads_enter();
    init_desasm();
    gdk_threads_leave();

    gdk_threads_enter();
    init_breakpoints();
    gdk_threads_leave();

    //init_TLBwindow();
}


void update_debugger_frontend()
// Update debugger state and display.
{
    if(registers_opened) {
        gdk_threads_enter();
        update_registers();
        gdk_threads_leave();
    }
    if(desasm_opened) {
        gdk_threads_enter();
        update_desasm( PC->addr );
        gdk_threads_leave();
    }
    /*if(regTLB_opened) {
        gdk_threads_enter();
        update_TLBwindow();
        gdk_threads_leave();
    }*/
}