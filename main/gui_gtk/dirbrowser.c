/***************************************************************************
 dirbrowser.c - Handles directory browsing
----------------------------------------------------------------------------
Began            : Sat Nov 9 2002
Copyright        : (C) 1998-2002 Peter Alm, Mikael Alm, Olle Hallnas,
                                 Thomas Nilsson and 4Front Technologies
Email            : <Occluded for size reasons>
---------------------------------------------------------------------------
Notice: This file was derived from the XMMS source code.
****************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#if defined(HAVE_CONFIG_H)
#include "../config.h"
#endif

#include <sys/types.h>
#include <sys/stat.h>

#include <dirent.h>
#include <unistd.h>

#include <string.h>

#include <gtk/gtk.h>
#include <stdio.h>

#include "../translate.h"

/* XPM */
static char *folder[] =
{
    "16 16 16 1",
    "   c None",
    ".  c #f4f7e4",
    "X  c #dee4b5",
    "o  c #e1e7b9",
    "O  c #c6cba4",
    "+  c #dce2b8",
    "@  c #e9e9ec",
    "#  c #d3d8ae",
    "$  c #d8daca",
    "%  c #b2b2b5",
    "&  c #767862",
    "*  c #e3e6c3",
    "=  c #1b1b1a",
    "-  c #939684",
    ";  c #555555",
    ":  c #000000",
    "                ",
    "                ",
    "  ::::          ",
    " :.@@O:         ",
    ":-&&&&&:::::    ",
    ":.@@@@@*$O#O=   ",
    ":@*+XXXX+##O:   ",
    ":.*#oooXXXXX:   ",
    ":@+XoXXXXXX#:   ",
    ":@*ooXXXXXX#:   ",
    ":@**XXXXXXX#:   ",
    ":@*XXXXXXXX%:   ",
    ":$.*OOOOOO%-:   ",
    " ;:::::::::::   ",
    "                ",
    "                "};

/* Icon by Jakub Steiner <jimmac@ximian.com> */

/* XPM */
static char *ofolder[] =
{
    "16 16 16 1",
    "   c None",
    ".  c #a9ad93",
    "X  c #60634d",
    "o  c #dee4b5",
    "O  c #9ca085",
    "+  c #0c0d04",
    "@  c #2f2f31",
    "#  c #3b3d2c",
    "$  c #c8cda2",
    "%  c #e6e6e9",
    "&  c #b3b5a5",
    "*  c #80826d",
    "=  c #292a1c",
    "-  c #fefef6",
    ";  c #8f937b",
    ":  c #000000",
    "                ",
    "                ",
    "  ::::          ",
    " :-%%&:         ",
    ":-;;;OX:::::    ",
    ":-;;;;O;O;&.:   ",
    ":-*X##@@@@@=#:  ",
    ":%*+-%%ooooooO: ",
    ":%X;%ooooooo.*: ",
    ":.+-%oooooooO:  ",
    ":*O-oooooooo*:  ",
    ":O-oooooooo.:   ",
    ":*-%$$$$$$OX:   ",
    " :::::::::::    ",
    "                ",
    "                "};

static GdkPixmap *folder_pixmap = NULL, *ofolder_pixmap;
static GdkBitmap *folder_mask, *ofolder_mask;

typedef struct
{
    gboolean scanned;
    gchar *path;
}
DirNode;

static gboolean check_for_subdir(gchar * path)
{
    DIR *dir;
    struct dirent *dirent;
    struct stat statbuf;
    gchar *npath;

    if ((dir = opendir(path)) != NULL)
    {
        while ((dirent = readdir(dir)) != NULL)
        {
            if (dirent->d_name[0] != '.')
            {
                npath = g_strconcat(path, dirent->d_name, NULL);
                if (stat(npath, &statbuf) != -1 && S_ISDIR(statbuf.st_mode))
                {
                    g_free(npath);
                    closedir(dir);
                    return TRUE;
                }
                g_free(npath);
            }
        }
        closedir(dir);
    }
    return FALSE;
}

static void destroy_cb(gpointer data)
{
    DirNode *node = data;

    g_free(node->path);
    g_free(node);
}

static void expand_cb(GtkWidget * widget, GtkCTreeNode * parent_node)
{
    DIR *dir;
    struct dirent *dirent;
    gchar *path, *text, *dummy = "dummy";
    struct stat statbuf;
    GtkCTreeNode *node, *sub_node;
    DirNode *parent_dirnode, *dirnode;
    gboolean has_subdir = FALSE;

    parent_dirnode = gtk_ctree_node_get_row_data(GTK_CTREE(widget), parent_node);
    if (!parent_dirnode->scanned)
    {
        gtk_clist_freeze(GTK_CLIST(widget));
        node = gtk_ctree_find_by_row_data(GTK_CTREE(widget), parent_node, NULL);
        gtk_ctree_remove_node(GTK_CTREE(widget), node);
        if ((dir = opendir(parent_dirnode->path)) != NULL)
        {
            while ((dirent = readdir(dir)) != NULL)
            {
                path = g_strconcat(parent_dirnode->path, dirent->d_name, NULL);
                if (stat(path, &statbuf) != -1 && S_ISDIR(statbuf.st_mode) && dirent->d_name[0] != '.')
                {
                    dirnode = g_malloc0(sizeof (DirNode));
                    dirnode->path = g_strconcat(path, "/", NULL);
                    text = dirent->d_name;
                    if (check_for_subdir(dirnode->path))
                        has_subdir = TRUE;
                    else
                        has_subdir = FALSE;
                    node = gtk_ctree_insert_node(GTK_CTREE(widget), parent_node, NULL, &text, 4, folder_pixmap, folder_mask, ofolder_pixmap, ofolder_mask, !has_subdir, FALSE);
                    gtk_ctree_node_set_row_data_full(GTK_CTREE(widget), node, dirnode, destroy_cb);
                    if (has_subdir)
                        sub_node = gtk_ctree_insert_node(GTK_CTREE(widget), node, NULL, &dummy, 4, NULL, NULL, NULL, NULL, FALSE, FALSE);
                }
                g_free(path);
            }
            closedir(dir);
            gtk_ctree_sort_node(GTK_CTREE(widget), parent_node);
        }
        gtk_clist_thaw(GTK_CLIST(widget));
        parent_dirnode->scanned = TRUE;
    }
}

static void select_row_cb(GtkWidget * widget, gint row, gint column, GdkEventButton * bevent, gpointer data)
{
    DirNode *dirnode;
    GtkCTreeNode *node;
    void (*handler) (gchar *);

    if (bevent)
    {
        if (bevent->type == GDK_2BUTTON_PRESS)
        {
            node = gtk_ctree_node_nth(GTK_CTREE(widget), row);
            dirnode = gtk_ctree_node_get_row_data(GTK_CTREE(widget), node);
            handler = (void (*)(gchar *)) gtk_object_get_user_data(GTK_OBJECT(widget));
            if (handler)
                handler(dirnode->path);
        }
    }

}

static void ok_clicked(GtkWidget * widget, GtkWidget * tree)
{
    GtkCTreeNode *node;
    DirNode *dirnode;
    GList *list_node;
    GtkWidget *window;
    void (*handler) (gchar *);

    window = gtk_object_get_user_data(GTK_OBJECT(widget));
    gtk_widget_hide(window);
    list_node = GTK_CLIST(tree)->selection;
    while (list_node)
    {
        node = list_node->data;
        dirnode = gtk_ctree_node_get_row_data(GTK_CTREE(tree), node);
        handler = (void (*)(gchar *)) gtk_object_get_user_data(GTK_OBJECT(tree));
        if (handler)
            handler(dirnode->path);
        list_node = g_list_next(list_node);
    }
    gtk_widget_destroy(window);

}

static int filetreeent_compare_func(const void *a, const void *b)
{
    if (!a || !b || !((DirNode *) a)->path)
        return -1;
    return strcmp(((DirNode *) a)->path, (gchar *) b);
}

GtkWidget *create_dir_browser(gchar * title, gchar * current_path, GtkSelectionMode mode, void (*handler) (gchar *))
{
    GtkWidget *window, *scroll_win, *tree, *vbox, *bbox, *ok, *cancel,
             *sep;
    gchar *root_text = "/", *node_text = "dummy";
    gchar *currentdir, *pos, *tpath, *tpathnew;
    GtkCTreeNode *root_node, *node, *nextnode;
    DirNode *dirnode;
    gboolean leaf;

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_modal(GTK_WINDOW(window), TRUE);
//  gtk_window_set_decorated(GTK_WINDOW(window), TRUE);

    gtk_window_set_title(GTK_WINDOW(window), title);
    gtk_container_border_width(GTK_CONTAINER(window), 10);

    vbox = gtk_vbox_new(FALSE, 10);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    scroll_win = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_win), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_usize(scroll_win, 250, 200);
    gtk_box_pack_start(GTK_BOX(vbox), scroll_win, TRUE, TRUE, 0);
    gtk_widget_show(scroll_win);

    gtk_widget_realize(window);
    if (!folder_pixmap)
    {
        folder_pixmap = gdk_pixmap_create_from_xpm_d(window->window, &folder_mask, NULL, folder);
        ofolder_pixmap = gdk_pixmap_create_from_xpm_d(window->window, &ofolder_mask, NULL, ofolder);
    }

    tree = gtk_ctree_new(1, 0);
    gtk_clist_set_column_auto_resize(GTK_CLIST(tree), 0, TRUE);
    gtk_clist_set_selection_mode(GTK_CLIST(tree), mode);
    gtk_ctree_set_line_style(GTK_CTREE(tree), GTK_CTREE_LINES_DOTTED);
    gtk_signal_connect(GTK_OBJECT(tree), "tree_expand", GTK_SIGNAL_FUNC(expand_cb), NULL);
    gtk_signal_connect(GTK_OBJECT(tree), "select_row", GTK_SIGNAL_FUNC(select_row_cb), NULL);
    gtk_container_add(GTK_CONTAINER(scroll_win), tree);
    gtk_object_set_user_data(GTK_OBJECT(tree), (gpointer) handler);

    root_node = gtk_ctree_insert_node(GTK_CTREE(tree), NULL, NULL, &root_text, 4, folder_pixmap, folder_mask, ofolder_pixmap, ofolder_mask, FALSE, FALSE);
    dirnode = g_malloc0(sizeof (DirNode));
    dirnode->path = g_strdup("/");
    gtk_ctree_node_set_row_data_full(GTK_CTREE(tree), root_node, dirnode, destroy_cb);
    node = gtk_ctree_insert_node(GTK_CTREE(tree), root_node, NULL, &node_text, 4, NULL, NULL, NULL, NULL, TRUE, TRUE);
    gtk_ctree_expand(GTK_CTREE(tree), root_node);
    gtk_widget_show(tree);

    sep = gtk_hseparator_new();
    gtk_box_pack_start(GTK_BOX(vbox), sep, FALSE, FALSE, 0);
    gtk_widget_show(sep);

    bbox = gtk_hbutton_box_new();
    gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_END);
    gtk_button_box_set_spacing(GTK_BUTTON_BOX(bbox), 5);

    cancel = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
    GTK_WIDGET_SET_FLAGS(cancel, GTK_CAN_DEFAULT);
    gtk_box_pack_start(GTK_BOX(bbox), cancel, TRUE, TRUE, 0);
    gtk_signal_connect_object(GTK_OBJECT(cancel), "clicked", GTK_SIGNAL_FUNC(gtk_widget_destroy), GTK_OBJECT(window));
    gtk_widget_show(cancel);

    ok = gtk_button_new_from_stock(GTK_STOCK_OK);
    gtk_object_set_user_data(GTK_OBJECT(ok), window);
    GTK_WIDGET_SET_FLAGS(ok, GTK_CAN_DEFAULT);
    gtk_window_set_default(GTK_WINDOW(window), ok);
    gtk_box_pack_start(GTK_BOX(bbox), ok, TRUE, TRUE, 0);
    gtk_signal_connect(GTK_OBJECT(ok), "clicked", GTK_SIGNAL_FUNC(ok_clicked), tree);
    gtk_widget_show(ok);

    gtk_box_pack_start(GTK_BOX(vbox), bbox, FALSE, FALSE, 0);
    gtk_widget_show(bbox);
    gtk_widget_show(vbox);

    if (current_path && *current_path)
    {
        currentdir = g_strdup(current_path);
        tpath = g_strdup("/");
        pos = strtok(currentdir, "/");
        node = gtk_ctree_find_by_row_data_custom(GTK_CTREE(tree), NULL, "/", filetreeent_compare_func);
        do
        {
            tpathnew = g_strconcat(tpath, pos, "/", NULL);
            g_free(tpath);
            tpath = tpathnew;
            nextnode = gtk_ctree_find_by_row_data_custom(GTK_CTREE(tree), node, tpath, filetreeent_compare_func);
            if (!nextnode)
                break;
            node = nextnode;
            pos = strtok(NULL, "/");
            gtk_ctree_get_node_info(GTK_CTREE(tree), node, NULL, NULL, NULL, NULL, NULL, NULL, &leaf, NULL);
            if (!leaf && pos)
                gtk_ctree_expand(GTK_CTREE(tree), node);
            else
            {
                gtk_ctree_select(GTK_CTREE(tree), node);
                break;
            }
        }
        while (pos);
        g_free(tpath);
        g_free(currentdir);
    }
    else
        gtk_ctree_select(GTK_CTREE(tree), root_node);

    return window;
}
