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

#include "controlPanel.h"

#include "aboutDialog.h"

#include "systemTray.h"

#define TOOL_TIP_TEXT   "Tageventor: \nLeft-click for control panel.\nRight-click for actions."

static GtkStatusIcon    *systemTrayIcon = NULL;

void
systemTraySetStatus(
                    char        connected,
                    const char  *message
                    )
{
    gchar   toolTipText[strlen(TOOL_TIP_TEXT) + SYSTEM_TRAY_ICON_MESSAGE_MAX];

    if ( connected )
        gtk_status_icon_set_from_file( systemTrayIcon, "icons/tageventor48x48.png" );
    else
        gtk_status_icon_set_from_file( systemTrayIcon, "icons/tageventor48x48NoReader.png" );

    /* push the message into the tool tip for the status icon */
    sprintf( toolTipText, "%s\n%s", TOOL_TIP_TEXT, message );
    gtk_status_icon_set_tooltip_text( systemTrayIcon, toolTipText );

#ifdef BUILD_CONTROL_PANEL
    /* let the control panel know of status updates too */
    controlPanelSetStatus( connected, message );
#endif

}


static void
iconQuit( void )
{
    /* this is a request to exit tageventor altogether */

#ifdef BUILD_CONTROL_PANEL
    /* pass the request onto the control panel, and only exit if it says so */
    /* as it may want to pop-up a dialog to save etc */
    if (controlPanelQuit())
        exit( 0 );
#else
    exit( 0 );
#endif

}

static void
controlPanelDialogShow( void )
{
    controlPanelActivate();
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
                int    (*pollFunction)( void *data )
                )
{
    GtkWidget       *popupMenu, *quitMenuItem;
#ifdef BUILD_ABOUT_DIALOG
    GtkWidget       *aboutMenuItem;
#endif

#ifdef BUILD_CONTROL_PANEL
    GtkWidget       *controlPanelMenuItem;
#endif

    /* Init GTK+ it might modify significantly the command line options */
    gtk_init( argc, argv );

	/* add application specific icons to search path - I'm not sure this is needed the way we work....*/
/* gtk_icon_theme_append_search_path (gtk_icon_theme_get_default (), " $PWD " );
*/
    /* until we actually connect to a reader set the icon to show not connected to one */
    systemTrayIcon = gtk_status_icon_new_from_file( "icons/tageventor48x48NoReader.png" );

    /* Set the basic tooltip info. Tag polling routine may update it with more info */
    gtk_status_icon_set_tooltip_text( systemTrayIcon, TOOL_TIP_TEXT );

#ifdef BUILD_CONTROL_PANEL
    /* if we have built the control panel then connect up the handler for the left mouse-click */
    g_signal_connect (G_OBJECT (systemTrayIcon), "activate", G_CALLBACK (controlPanelActivate), NULL);
#endif

    /* now create the pop-up menu that is shown by right-clicking the systemTray icon */
    /* create the menu item */
    popupMenu = gtk_menu_new();

    /* Now add entries to the menu */
#ifdef BUILD_CONTROL_PANEL
    controlPanelMenuItem = gtk_menu_item_new_with_label( "Control Panel" );
    gtk_menu_shell_append( GTK_MENU_SHELL( popupMenu ), controlPanelMenuItem );
    g_signal_connect (G_OBJECT (controlPanelMenuItem), "activate", G_CALLBACK (controlPanelDialogShow), NULL );
    gtk_widget_show( controlPanelMenuItem );
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

    /* add a timeout to check for tags, TRUE to ask it to update system tray */
    g_timeout_add( 1000, pollFunction, (gpointer)TRUE );

    /* Start main loop processing UI events */
    gtk_main();

}

#endif /* BUILD_SYSTEM_TRAY */
