/*
  readersDialog.c - C source code for gtagEventor Gtk/GNOME readers
                    dialog window

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

#ifdef BUILD_READERS_DIALOG
#include <stdlib.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <string.h>

#include <tagReader.h>

#include "stringConstants.h"
#include "tagEventor.h"
#include "systemTray.h"
#include "readersDialog.h"


#define NUM_READER_TABLE_COLUMNS  (5)
static const gchar      *columnHeader[NUM_READER_TABLE_COLUMNS] = { "Number", "Name", "Driver Descriptor", "SAM ID", "SAM Serial" };

static GtkWidget   *readersDialog = NULL;

static void
hideReadersDialog( void )
{

    gtk_widget_hide( readersDialog );

}


/* Call back for the close button */
static void
closeSignalHandler(
                   GtkDialog    *dialog,
                   gpointer     user_data
                   )
{
    hideReadersDialog();
}

/* This get's called when the user closes the window using the Window Manager 'X' box */
static char
deleteSignalHandler(
            GtkWidget *widget,
            GdkEvent  *event,
            gpointer   data
            )
{
    /* If you return FALSE GTK will propograte event and we'll get a "destroy" signal. */
    return( FALSE );
}

static void
destroy(
        GtkWidget *widget,
        gpointer   data
        )
{

    /* hide the main cpanel window */
    hideReadersDialog();

    /* it seems that when this get's called the widget actually get's destroyed */
    /* so, show it's not existant and on next activate it will get re-built */
    readersDialog = NULL;

}

static void
tableAddReader( GtkTable *pTable, const tReaderManager *pManager, int i )
{
    GtkWidget           *number, *name, *driverDescriptor, *SAM_id, *SAM_serial;
    char                numberString[5];

    sprintf( numberString, "%d", i );
    number = gtk_label_new( numberString );
    gtk_table_attach(pTable, number,  0, 1, i+1, i+2, GTK_FILL, GTK_FILL, 10, 3 );
    gtk_widget_show( number );

    /* if the reader has been connected to we can fill in mode info */
    if ( pManager->readers[i].hCard != NULL )
    {
        name = gtk_label_new( pManager->readers[i].name );
        gtk_table_attach(pTable, name,  1, 2, i+1, i+2, GTK_FILL, GTK_FILL, 10, 3 );
        gtk_widget_show( name );

        driverDescriptor = gtk_label_new( pManager->readers[i].driverDescriptor );
        gtk_table_attach(pTable, driverDescriptor,  2, 3, i+1, i+2, GTK_FILL, GTK_FILL, 10, 3 );
        gtk_widget_show( driverDescriptor );

        /* if there is a SAM present then show it's details */
        if ( pManager->readers[i].SAM )
        {
            SAM_id = gtk_label_new( pManager->readers[i].SAM_id );
            gtk_table_attach(pTable, SAM_id,  3, 4, i+1, i+2, GTK_FILL, GTK_FILL, 10, 3 );
            gtk_widget_show( SAM_id );

            SAM_serial = gtk_label_new( pManager->readers[i].SAM_serial );
            gtk_table_attach(pTable, SAM_serial,  4, 5, i+1, i+2, GTK_FILL, GTK_FILL, 10, 3 );
            gtk_widget_show( SAM_serial );
        }
    }
    else
    {   /* we have not connected to this reader, see if it exists */
        if ( i >= pManager->nbReaders )
        {
            /* No such reader, put that in as text */
            name = gtk_label_new( "No reader detected" );
            gtk_table_attach(pTable, name,  1, 5, i+1, i+2, GTK_FILL, GTK_FILL, 10, 3 );
            gtk_widget_show( name );
        }
        else
        {   /* there is such a reader in the system, although we're not connect */
            if ( pManager->readers[i].name != NULL )
            {
                name = gtk_label_new( pManager->readers[i].name );
                gtk_table_attach(pTable, name,  1, 2, i+1, i+2, GTK_FILL, GTK_FILL, 10, 3 );
                gtk_widget_show( name );

                driverDescriptor = gtk_label_new( "Information not availabe, Not connected to this reader" );
                gtk_table_attach(pTable, driverDescriptor,  2, 5, i+1, i+2, GTK_FILL, GTK_FILL, 10, 3 );
                gtk_widget_show( driverDescriptor );
            }
        }
    }
}

static GtkWidget *
buildReadersDialog ( void *pData )
{
    GtkWidget   *dialog;
    GtkWidget   *vbox, *buttonBox, *pTable, *label, *closeButton, *statusBar;
    int         i;
    char        windowTitle[strlen(PROGRAM_NAME) + strlen(READERS_DIALOG_WINDOW_TITLE) + 10];

    dialog = gtk_window_new( GTK_WINDOW_TOPLEVEL );
    sprintf(windowTitle, "%s%s", PROGRAM_NAME, READERS_DIALOG_WINDOW_TITLE);
    gtk_window_set_title( (GtkWindow *)dialog, windowTitle );

    /* set the icon for the window */
    gtk_window_set_icon_name( (GtkWindow *)dialog, ICON_NAME_CONNECTED );

    /* When the window is given the "delete" signal (this is given
     * by the window manager, usually by the "close" option, or on the
     * titlebar) */
    g_signal_connect (G_OBJECT (dialog), "delete_event",
		      G_CALLBACK (deleteSignalHandler), NULL);

    /* Here we connect the "destroy" event to a signal handler.
     * This event occurs when we call gtk_widget_destroy() on the window,
     * or if we return FALSE in the "delete_event" callback. */
    g_signal_connect (G_OBJECT (dialog), "destroy",
		      G_CALLBACK (destroy), NULL);

    /******************************* Vertical Box ***********************************/
    /* vertical box to hold things, Not homogeneous sizes and spaceing 0 */
    vbox = gtk_vbox_new(FALSE, 0);

    /* This packs the vbox into the window (a gtk container). */
    gtk_container_add (GTK_CONTAINER (dialog), vbox);

    /******************************* Table ******************************************/
    /* create the table for tag IDs, 7 rows, 5 columns */
    pTable = gtk_table_new( ( MAX_NUM_READERS +1 ), 5, FALSE);

    /* This packs the scrolled window into the vbox (a gtk container). */
    gtk_box_pack_start( GTK_BOX(vbox), pTable, TRUE, TRUE, 3);

    /* add the column labels in the table*/
    for (i = 0; i < NUM_READER_TABLE_COLUMNS; i++)
    {
        label = gtk_label_new( columnHeader[i] );

        /* attach a new widget into the table in column 'i', row 0 */
        gtk_table_attach((GtkTable *)pTable, label, i, i+1, 0, 1, GTK_FILL, GTK_FILL, 5, 3 );

        gtk_widget_show( label );
    }

    /* the data passed in via the callback is a pointer to a reader Array */
/* ADM TODO I can't figure out why passing the pointer via the callback doesn't work,
   so a hack for now
   pManager = pData; */

    /* add rows for each reader */
    for ( i = 0; i < MAX_NUM_READERS; i++ )
        tableAddReader( (GtkTable *)pTable, &readerManager, i );

    /******************************* Button Box ***************************************/
    /* create the box for the buttons */
    buttonBox = gtk_hbox_new( FALSE, 0);

    /********************************************* Close Button ***********************/
    closeButton = gtk_button_new_from_stock( "gtk-close" );

    /* This packs the button into the hbutton box  (a gtk container). */
    gtk_box_pack_end( GTK_BOX(buttonBox), closeButton, FALSE, TRUE, 3);

    /* When the button receives the "clicked" signal, it will call the
     * function applyChanges() passing it NULL as its argument. */
     /* TODO can't get this signal to work and close things! */
    g_signal_connect (G_OBJECT (closeButton), "released", G_CALLBACK (closeSignalHandler), NULL);

    /**************************** End of buttons **************************************/
    /* This packs the hbutton box into the vbox (a gtk container). */
    gtk_box_pack_start(GTK_BOX(vbox), buttonBox, FALSE, TRUE, 3);

   /**************************** Status Bar ******************************************/
    /* create the status bar */
    statusBar = gtk_statusbar_new();

    /* This packs the status bar into the vbox (a gtk container)  */
    gtk_box_pack_start(GTK_BOX(vbox), statusBar, FALSE, TRUE, 0);

    return ( dialog );

}

void
readersDialogActivate( void *readersArray )
{

    /* if it exists delete it as the situation might have changed */
    if ( readersDialog )
        gtk_widget_destroy( readersDialog );

    /* build the widget tree for the entire UI */
    readersDialog = buildReadersDialog( readersArray );

    /* Show window, and recursively all contained widgets */
    gtk_widget_show_all( readersDialog );

}

#endif
