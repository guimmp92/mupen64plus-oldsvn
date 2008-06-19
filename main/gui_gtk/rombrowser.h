/***************************************************************************
rombrowser.h - Handles VCR mode GUI elements
----------------------------------------------------------------------------
Began                : Sat Nov 9 2002
Copyright            : (C) 2002 by blight
Email                : blight@Ashitaka
****************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __ROMBROWSER_H__
#define __ROMBROWSER_H__

#include <gtk/gtk.h>

int create_romBrowser( void );
void apply_filter( void );
void fillrombrowser();

void rombrowser_refresh( unsigned int roms, unsigned short clear );

gboolean filter_function( GtkTreeModel *model, GtkTreeIter *iter, gpointer data);
void countrycodestring(unsigned short int countrycode, char *string);
void countrycodeflag(unsigned short int countrycode, GdkPixbuf **flag);
char* filefrompath(const char *string);

#endif // __ROMBROWSER_H__
