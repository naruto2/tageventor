/*
  controlPanelHelp.c - C source code for tagEventor Gtk/GNOME control panel

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

#ifdef BUILD_CONTROL_PANEL_HELP

#include "controlPanelHelp.h"

#include <gtk/gtk.h>

static void
controlPanelHelpClose(
                    GtkDialog   *dialog,
                    gpointer   user_data
                    )
{

    gtk_widget_destroy( (GtkWidget *)dialog );

}


static void
controlPanelHelpResponse(
                        GtkDialog *dialog,
                        gint       response_id,
                        gpointer   user_data
                        )
{
    /* will this be called when the CLOSE button is hit on the about dialog...? */
    gtk_widget_destroy( GTK_WIDGET( dialog ) );

}

void
controlPanelHelpShow( void )
{

    GtkWidget   *controlPanelHelpDialog;
    GError      *pError;

    controlPanelHelpDialog = gtk_dialog_new();

    /* set the icon for the window */
/* TODO fix all the icon path stuff */
    gtk_window_set_icon_from_file( (GtkWindow *)controlPanelHelpDialog, "icons/tageventor48x48.png", &pError );

    /* add a close button Response ID for it is 0, although we won't use it */
    gtk_dialog_add_button( (GtkDialog *)controlPanelHelpDialog, "Close", 0 );

    /* this will be triggered when escape key is used */
    g_signal_connect ( G_OBJECT (controlPanelHelpDialog), "close", G_CALLBACK (controlPanelHelpClose), NULL );
/* TODO set the default button that is pressed with Enter */

    g_signal_connect ( G_OBJECT (controlPanelHelpDialog), "response", G_CALLBACK (controlPanelHelpResponse), NULL );

    gtk_widget_show( controlPanelHelpDialog );

}

#endif
