/**
 * Mupen64 - debugger.h
 * Copyright (C) 2002 DavFr - robind@esiee.fr
 *
 * Mupen64 homepage: http://mupen64.emulation64.com
 * email address: hacktarux@yahoo.fr
 * 
 * If you want to contribute to the project please contact
 * me first (maybe someone is already making what you are
 * planning to do).
 *
 *
 * This program is free software; you can redistribute it and/
 * or modify it under the terms of the GNU General Public Li-
 * cence as published by the Free Software Foundation; either
 * version 2 of the Licence, or any later version.
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

#ifndef DEBUGGER_H
#define DEBUGGER_H

#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "types.h"
#include "../r4300/r4300.h"
#include "../memory/memory.h"

extern int debugger_mode;  // Debugger option enabled.

extern int g_DebuggerEnabled;      // wether the debugger is enabled or not



// State of the Emulation Thread:
//  0 -> pause, 1 -> step, 2 -> run.
extern int run;

extern uint32 previousPC;

void init_debugger();
void update_debugger();

extern void init_debugger_frontend();
extern void update_debugger_frontend();

extern pthread_cond_t  debugger_done_cond;
extern pthread_mutex_t mutex;

#endif //DEBUGGER_H
