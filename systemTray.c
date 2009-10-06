/*
  systemTray.c - C source code for tagEventor Gtk/GNOME system tray icon

  Copyright 2009 Autelic Association (http://www.autelic.org)

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#ifdef BUILD_SYSTEM_TRAY

#include <stdlib.h>
#include <gtk/gtk.h>
#include <string.h>
#include <unistd.h>
#include "rulesEditor.h"

#include "aboutDialog.h"

#include "systemTray.h"

#include "settingsDialog.h"

#define TOOL_TIP_TEXT               "Tageventor: \nLeft-click for Rules Editor.\nRight-click for menu."

static GtkStatusIcon    *systemTrayIcon = NULL;
static int    (*readerPollFunction)( void *data );
static guint    timeoutID = 0;


void systemTraySetPollDelay(
                            int     validPollDelay  /* already been validated as within range */
                            )
{

    /* delete the old timeout */
    if ( timeoutID )
        g_source_remove( timeoutID );

    /* add a new timeout to check for tags, TRUE to ask it to update system tray */
    if ( readerPollFunction )
        timeoutID = g_timeout_add( validPollDelay, readerPollFunction, (gpointer)TRUE );

}


void
systemTraySetStatus(
                    char        connected,
                    const char  *message
                    )
{
    gchar   toolTipText[strlen(TOOL_TIP_TEXT) + SYSTEM_TRAY_ICON_MESSAGE_MAX];

    if ( connected )
        gtk_status_icon_set_from_icon_name( systemTrayIcon, ICON_NAME_CONNECTED );
    else
        gtk_status_icon_set_from_icon_name( systemTrayIcon, ICON_NAME_NOT_CONNECTED );

    /* push the message into the tool tip for the status icon */
    sprintf( toolTipText, "%s\n%s", TOOL_TIP_TEXT, message );
    gtk_status_icon_set_tooltip_text( systemTrayIcon, toolTipText );

#ifdef BUILD_RULES_EDITOR
    /* let the ruls editor know of status updates too */
    rulesEditorSetStatus( connected, message );
#endif

}


static void
iconQuit( void )
{
    /* this is a request to exit tageventor altogether */

#ifdef BUILD_RULES_EDITOR
    /* pass the request onto the rulse editor, and only exit if it says so */
    /* as it may want to pop-up a dialog to save etc */
    if ( rulesEditorQuit())
    {
#ifdef BUILD_SETTINGS_DIALOG
        if ( settingsDialogQuit() )
            exit( 0 );
#else
        exit( 0 );
#endif
    }
#else
#ifdef BUILD_SETTINGS_DIALOG
        if ( settingsDialogQuit() )
            exit( 0 );
#else
    exit( 0 );
#endif

#endif

}

static void
iconPopupMenu(GtkStatusIcon *status_icon,
              guint          button,
              guint          activate_time,
              gpointer       popupMenu
              )
{

    gtk_menu_popup( (GtkMenu *)popupMenu, NULL, NULL, NULL, NULL, button, activate_time );

}

void
startSystemTray(
                int     *argc,
                char    ***argv,
                int    (*pollFunction)( void *data ),
                int     pollDelay
                )
{
    GtkWidget       *popupMenu, *quitMenuItem;
#ifdef DEBUG
    char		    currentDir[ PATH_MAX + 10 ];
#endif

#ifdef BUILD_ABOUT_DIALOG
    GtkWidget       *aboutMenuItem;
#endif

#ifdef BUILD_RULES_EDITOR
    GtkWidget       *rulesEditorMenuItem;
#endif

#ifdef BUILD_SETTINGS_DIALOG
    GtkWidget       *settingsDialogMenuItem;
#endif

    /* Init GTK+ it might modify significantly the command line options */
    gtk_init( argc, argv );

#ifdef DEBUG
    /* this might be useful for use during development */
    if ( ( getcwd( currentDir, PATH_MAX ) ) != NULL )
    {
        /* add 'icons' on the end of it */
        strcat( currentDir, "/icons" );
        gtk_icon_theme_prepend_search_path (gtk_icon_theme_get_default (), currentDir );
    }
#endif

	/* make sure the directory where we install icons is in the icon search path */
/* TODO I can't get this to find them in the subdirectories correctly,
        currently have copied them into root icon directory as a klude till I figure it out */
    gtk_icon_theme_append_search_path (gtk_icon_theme_get_default(), ICON_INSTALL_DIR );

    /* until we actually connect to a reader set the icon to show not connected to one */
    systemTrayIcon = gtk_status_icon_new_from_icon_name( ICON_NAME_NOT_CONNECTED );

    /* Set the basic tooltip info. Tag polling routine may update it with more info */
    gtk_status_icon_set_tooltip_text( systemTrayIcon, TOOL_TIP_TEXT );

#ifdef BUILD_RULES_EDITOR
    /* if we have built the rules editor then connect up the handler for the left mouse-click */
    g_signal_connect (G_OBJECT (systemTrayIcon), "activate", G_CALLBACK (rulesEditorActivate), NULL);
#endif

    /* now create the pop-up menu that is shown by right-clicking the systemTray icon */
    /* create the menu item */
    popupMenu = gtk_menu_new();

    /* Now add entries to the menu */
#ifdef BUILD_RULES_EDITOR
    rulesEditorMenuItem = gtk_menu_item_new_with_label( "Rules Editor" );
    gtk_menu_shell_append( GTK_MENU_SHELL( popupMenu ), rulesEditorMenuItem );
    g_signal_connect (G_OBJECT (rulesEditorMenuItem), "activate", G_CALLBACK (rulesEditorActivate), NULL );
    gtk_widget_show( rulesEditorMenuItem );
#endif

#ifdef BUILD_SETTINGS_DIALOG
    settingsDialogMenuItem = gtk_menu_item_new_with_label( "Settings" );
    gtk_menu_shell_append( GTK_MENU_SHELL( popupMenu ), settingsDialogMenuItem );
    g_signal_connect (G_OBJECT (settingsDialogMenuItem), "activate", G_CALLBACK (settingsDialogActivate), NULL );
    gtk_widget_show( settingsDialogMenuItem );
#endif

#ifdef BUILD_ABOUT_DIALOG
    aboutMenuItem = gtk_menu_item_new_with_label( "About" );
    gtk_menu_shell_append( GTK_MENU_SHELL( popupMenu ), aboutMenuItem );
    g_signal_connect (G_OBJECT (aboutMenuItem), "activate", G_CALLBACK (aboutDialogShow), NULL );
    gtk_widget_show( aboutMenuItem );
#endif

    quitMenuItem = gtk_menu_item_new_with_label( "Quit" );
    gtk_menu_shell_append( GTK_MENU_SHELL( popupMenu ), quitMenuItem );
    g_signal_connect (G_OBJECT (quitMenuItem), "activate", G_CALLBACK (iconQuit), NULL );
    gtk_widget_show( quitMenuItem );

    /* connect menu to the event that will be used (right click on status icon) and pass in pointer to menu */
    g_signal_connect (G_OBJECT (systemTrayIcon), "popup-menu", G_CALLBACK (iconPopupMenu), popupMenu );

    /* make sure the icon is shown */
    gtk_status_icon_set_visible( systemTrayIcon, TRUE );

    /* set the global variable for the polling function as we'll need it later */
    readerPollFunction = pollFunction;

    /* add a timeout to check for tags, TRUE to ask it to update system tray */
    /* pass in the value for the delay in milliseconds */
    timeoutID = g_timeout_add( pollDelay, readerPollFunction, (gpointer)TRUE );

    /* Start main loop processing UI events */
    gtk_main();

}

#endif /* BUILD_SYSTEM_TRAY */
