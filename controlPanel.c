/*
  cpanel.c - C source code for tagEventor Gtk/GNOME control panel

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

#ifdef BUILD_CONTROL_PANEL

#include <stdlib.h>
#include <gtk/gtk.h>
#include <string.h>

#include "systemTray.h"

#include "controlPanel.h"

#include "tagEventor.h"

#include "aboutDialog.h"

#define DEFAULT_WIDTH_PIX       (620)
#define DEFAULT_HEIGHT_PIX      (250)


static const gchar *columnHeader[4] = { "Tag ID", "Description", "Script", "Enabled" };

static const char      *iconNameList[] = {
				"icons/tageventor16x16.png",
				"icons/tageventor32x32.png",
				"icons/tageventor48x48.png",
				"icons/tageventor64x64.png",
				"icons/tageventor128x128.png"
				};

static char            savePending = FALSE;
static char            cpanelVisible = FALSE;
static GtkWidget       *cpanelWindow = NULL, *statusBar = NULL, *applyButton = NULL;

char
controlPanelQuit( void )
{
    /* request from the systemtrayIcon to exit */
    /* if we can, then close our own stuff and tell the systemtrayIcon it can also exit */
    if (savePending == FALSE )
        return (TRUE ); /* can exit */
    else
    {
        /* TODO call cpanel exit routines to handle the saving etc */
        return( FALSE );
    }
}

void
controlPanelSetStatus(
                        char        connected,
                        const char  *message
                        )
{

    /* put the result onto the status bar */
    if ( cpanelWindow && cpanelVisible )
        gtk_statusbar_push((GtkStatusbar *)statusBar, 0, (gchar *)message);

}

static void
hideCPanelWindow( void )
{

    /* hide the main window if it exists ADN is visible */
    if ( cpanelWindow && cpanelVisible )
    {
        gtk_widget_hide(cpanelWindow);
        cpanelVisible = FALSE;
    }

}


/* Call back for the close button */
static void
closeWindow( void )
{
    /* if there's nothing to be applied then it's OK to close window ASAP */
    if ( !savePending )
        hideCPanelWindow();
    else
    {
/* TODO Pop-up the window here */
    }

}

static char
deleteSignalHandler(
            GtkWidget *widget,
            GdkEvent  *event,
            gpointer   data
            )
{

    if ( !savePending )
    {
        /* If you return FALSE in the "deleteSignalHandler"
           GTK will emit the "destroy" signal. */
        return( FALSE );
    }
    else
    {
      /* Returning TRUE means you don't want the window to be destroyed. */
/* TODO Pop-up the window here */
        return( TRUE );
    }
}

/* this handles the escape key */
static void
closeMain(
        GtkDialog   *dialog,
        gpointer   user_data
        )
{

    /* re use the handler for the CLOSE button */
    closeWindow();

}


static void
destroy(
        GtkWidget *widget,
        gpointer   data
        )
{

    /* hide the main cpanel window */
    hideCPanelWindow();

}

void
showHelp( void )
{

/* TODO write the help dialog ! */
/* put it into it's own source file also */
/* compile in a text file ??? */

}

/* Callback for when each entry's file selection is changed */
static void
scriptChosen(
             GtkWidget *widget,
             gpointer   entryIndex
             )
{

    /* get the button state and put into tagEntryArray */
/* TODO figure this out how to get the entry !!!!     tagEntryArray[(int)entryIndex].script = gtk_toggle_button_get_active( (GtkToggleButton *)widget); */
    savePending = TRUE;
    gtk_widget_set_sensitive( applyButton, TRUE );

}


/* Callback for when each entry's toggle button to enable it is toggled */
static void
enableChange(
             GtkWidget *widget,
             gpointer   entryIndex
             )
{

    /* get the button state and put into tagEntryArray */
/* TODO check that casting this is correct, and it's not the destination of the pointer */
    tagTableEntryEnable( (int)entryIndex, gtk_toggle_button_get_active( (GtkToggleButton *)widget) );
    savePending = TRUE;
    gtk_widget_set_sensitive( applyButton, TRUE );

}

static void
applyChanges( void )
{

    /* if all is OK, then modify the table we use to process events */
    /* save the changes */
    tagTableSave();

    /* now the table used to process events matches what's in the UI so we are in sync */
    savePending = FALSE;
    gtk_widget_set_sensitive( applyButton, FALSE );

}

static void
tableAddRow( GtkTable *pTable, int i )
{
    GtkWidget           *label, *chooser, *enable, *entry;
    gchar               message[80];
    const tPanelEntry   *pTagEntry;

    /* get a pointer to this entry in the table */
    pTagEntry = tagEntryGet( i );

    /* Tag ID of upto 20 char (usually 14) which does not need to expand with window size */
    label = gtk_label_new(pTagEntry->ID);
    /* attach a new widget into the table */
    gtk_table_attach(pTable, label, 0, 1, i+1, i+2, GTK_FILL, GTK_FILL, 5, 0 );

    /* description entry of upto 80 characters which can benefit from expanding horizontally*/
    entry = gtk_entry_new();
    gtk_entry_set_max_length( (GtkEntry *)entry, MAX_DESCRIPTION_LENGTH);
    gtk_entry_set_text((GtkEntry *)entry, pTagEntry->description);
    /* attach a new widget into the table */
    gtk_table_attach(pTable, (GtkWidget *)entry, 1, 2, i+1, i+2, GTK_EXPAND | GTK_FILL, GTK_FILL, 4, 0 );

    /* file chooser for script associated with this tag, which may have long name and so expand*/
    if ( pTagEntry->ID == NULL )
        sprintf(message, "Choose the file to be executed for tag" );
    else
        sprintf(message, "Choose the file to be executed for tag with ID: %s", pTagEntry->ID );
    chooser = gtk_file_chooser_button_new( message, GTK_FILE_CHOOSER_ACTION_OPEN );

    /* if the script is defined, try and give the filename the width required to hold it */
    if (pTagEntry->script)
        gtk_file_chooser_button_set_width_chars((GtkFileChooserButton *)chooser, strlen(pTagEntry->script) );
    else /* otherwise it's a new blank name, so default to width of 10 characters */
        gtk_file_chooser_button_set_width_chars((GtkFileChooserButton *)chooser, 10 );

/* TODO get the users home directory path */
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (chooser), "/home/andrew/.tageventor");
/* TODO set file chooser text from the tagEntryArray ... */
    /* attach a new widget into the table */
    gtk_table_attach(pTable, chooser, 2, 3, i+1, i+2, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 2 );
    /* add a callback to the button which will be passed the index of this entry in the tagEntryArray */
    g_signal_connect (G_OBJECT (chooser), "file-set", G_CALLBACK (scriptChosen), (gpointer)i);

    enable = gtk_check_button_new(); /* check button to enable the script for this tag */
    gtk_toggle_button_set_active( (GtkToggleButton *)enable, pTagEntry->enabled);
    /* attach a new widget into the table */
    gtk_table_attach(pTable, enable, 3, 4, i+1, i+2, GTK_FILL, GTK_FILL, 0, 0 );
    /* add a callback to the button which will be passed the index of this entry in the tagEntryArray */
    g_signal_connect (G_OBJECT (enable), "toggled", G_CALLBACK (enableChange), (gpointer)i);

    /* make the new widgets visible */
    gtk_widget_show( label );
    gtk_widget_show( chooser );
    gtk_widget_show( enable );
    gtk_widget_show( entry );
}


static void
addTagEntry(
            GtkWidget *widget,
            gpointer *pTable
            )
{
    int i, numEntries;

    numEntries = tagTableAddEntry();

    /* resize the table - with an extra row for the header */
    gtk_table_resize( (GtkTable *)pTable, (numEntries + 1), 4 );

    /* this entry's index */
    i = (numEntries -1);
    tableAddRow((GtkTable *)pTable, i);

/* TODO try and force scroll or focus on the new row so that it becomes visible */
}

static void
fillTable( GtkTable *pTable, int numTags )
{
    int         i;
    GtkWidget    *label;

    if ( numTags == 0 )
    {
        label = gtk_label_new( "No Tags configured. Use the 'Add' button below to configure a tag" );
        gtk_table_attach(pTable, label, 0, 1, 0, 1, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 10, 10 );

        return;
    }

    /* resize the table to be big enough to hold all entries */
    /* Row 0  =  Column headers */
    /* So we need numConfigTags + 1 rows, and 4 columns */
    gtk_table_resize( (GtkTable *)pTable, (numTags + 1), 4 );

    /* add the four column labels in the table*/
    for (i = 0; i < 4; i++)
    {
        /* create the text label */
        label = gtk_label_new( columnHeader[i] );

        /* attach a new widget into the table in column 'i', row 0 */
        gtk_table_attach(pTable, label, i, i+1, 0, 1, GTK_FILL, GTK_FILL, 5, 3 );
    }

    /* add a new row of widgets for each tag script configuration found */
    for (i = 0; i < numTags; i++)
        tableAddRow(pTable, i );

}


static GtkWidget *
buildCPanel ( void  )
{
    GtkWidget   *mainWindow;
    GtkWidget   *vbox, *scroll, *buttonBox, *table;
    GtkWidget   *helpButton, *closeButton, *aboutButton, *addButton;
    GError      *pError;
    int         numEntries;

    /******************************* Main Application Window ************************/
    mainWindow = gtk_window_new( GTK_WINDOW_TOPLEVEL );
    gtk_window_set_title( (GtkWindow *)mainWindow, PROGRAM_NAME );
    /* Smallest height possible then should expand to hold what's needed */
    gtk_window_set_default_size( (GtkWindow *)mainWindow, DEFAULT_WIDTH_PIX, DEFAULT_HEIGHT_PIX );

    /* set the icon for the window */
    gtk_window_set_icon_from_file( (GtkWindow *)mainWindow, iconNameList[0], &pError );
/* TODO pass in a full list when I figure out how to create the PixBufs from files!
    gtk_window_set_icon_list( window, iconList );
    */

    /* When the window is given the "delete" signal (this is given
     * by the window manager, usually by the "close" option, or on the
     * titlebar), we ask it to call the delete_event () function
     * as defined above. The data passed to the callback
     * function is NULL and is ignored in the callback function. */
    g_signal_connect (G_OBJECT (mainWindow), "delete_event",
		      G_CALLBACK (deleteSignalHandler), NULL);

    /* Here we connect the "destroy" event to a signal handler.
     * This event occurs when we call gtk_widget_destroy() on the window,
     * or if we return FALSE in the "delete_event" callback. */
    g_signal_connect (G_OBJECT (mainWindow), "destroy",
		      G_CALLBACK (destroy), NULL);

    /* this will be triggered when escape key is used */
    g_signal_connect ( G_OBJECT (mainWindow), "close", G_CALLBACK (closeMain), NULL );

    /******************************* Vertical Box ***********************************/
    /* vertical box to hold things, Not homogeneous sizes and spaceing 0 */
    vbox = gtk_vbox_new(FALSE, 0);

    /* This packs the vbox into the window (a gtk container). */
    gtk_container_add (GTK_CONTAINER (mainWindow), vbox);

    /******************************* Tag Config ************************************/
    /* load the tag configuration into memory before creating the table to show it */
    numEntries = tagTableRead( );

    /******************************* Table ******************************************/
    /* create the scolled window that will hold the viewport and table */
    scroll = gtk_scrolled_window_new( NULL, NULL);

    /* This packs the scrolled window into the vbox (a gtk container). */
    gtk_box_pack_start( GTK_BOX(vbox), scroll, TRUE, TRUE, 3);

    /* create the table for tag IDs */
    table = gtk_table_new( 1, 1, FALSE);

    /* fill the table with rows and columns of controls for scripts */
    fillTable( (GtkTable *)table, numEntries );

    /* add the non-scrollable Table to the scrolled window via a viewport */
    gtk_scrolled_window_add_with_viewport( (GtkScrolledWindow *)scroll, table );
    /******************************* End of Table *************************************/

    /******************************** Add Button  *************************************/
    addButton = gtk_button_new_from_stock( "gtk-add" );

    /* This packs the button into the hbutton box  (a gtk container). */
    gtk_box_pack_start( GTK_BOX(vbox), addButton, FALSE, FALSE, 3);

    /* When the button receives the "clicked" signal, it will call the
     * function applyChanges() passing it NULL as its argument. */
    g_signal_connect (G_OBJECT (addButton), "released", G_CALLBACK (addTagEntry), table);

    /******************************* Button Box ***************************************/
    /* create the box for the buttons */
    buttonBox = gtk_hbox_new( FALSE, 0);

    /********************************************* Help Button ***********************/
    helpButton = gtk_button_new_from_stock( "gtk-help" );

    /* This packs the button into the hbutton box */
    gtk_box_pack_start( GTK_BOX(buttonBox), helpButton, FALSE, TRUE, 3);

    /* When the button receives the "clicked" signal, it will call the
     * function applyChanges() passing it NULL as its argument. */
    g_signal_connect (G_OBJECT (helpButton), "released", G_CALLBACK (showHelp), NULL);

#ifdef BUILD_ABOUT_DIALOG
    /********************************************* About Button ***********************/
    aboutButton = gtk_button_new_from_stock( "About" );

    /* This packs the button into the hbutton box  */
    gtk_box_pack_start( GTK_BOX(buttonBox), aboutButton, FALSE, TRUE, 3);

    /* When the button receives the "clicked" signal, it will call the
     * function applyChanges() passing it NULL as its argument. */
    g_signal_connect (G_OBJECT (aboutButton), "released", G_CALLBACK (aboutDialogShow), NULL);
#endif

    /********************************************* Close Button ***********************/
    closeButton = gtk_button_new_from_stock( "gtk-close" );

    /* This packs the button into the hbutton box  (a gtk container). */
    gtk_box_pack_end( GTK_BOX(buttonBox), closeButton, FALSE, TRUE, 3);

    /* When the button receives the "clicked" signal, it will call the
     * function applyChanges() passing it NULL as its argument. */
     /* TODO can't get this signal to work and close things! */
    g_signal_connect (G_OBJECT (closeButton), "released", G_CALLBACK (closeWindow), NULL);

    /********************************************* Apply Button ***********************/
    applyButton = gtk_button_new_from_stock( "gtk-apply" );

    /* This packs the button into the hbutton box  (a gtk container). */
    gtk_box_pack_end( GTK_BOX(buttonBox), applyButton, FALSE, TRUE, 3);

    /* When the button receives the "clicked" signal, it will call the
     * function applyChanges() passing it NULL as its argument. */
    g_signal_connect (G_OBJECT (applyButton), "released", G_CALLBACK (applyChanges), NULL);

    /* make this button only clickable when there is a pending change to apply */
    gtk_widget_set_sensitive( applyButton, FALSE);

    /* This packs the hbutton box into the vbox (a gtk container). */
    gtk_box_pack_start(GTK_BOX(vbox), buttonBox, FALSE, TRUE, 3);
    /**************************** End of buttons **************************************/

    /**************************** Status Bar ******************************************/
    /* create the status bar */
    statusBar = gtk_statusbar_new();

    /* This packs the status bar into the vbox (a gtk container)  */
    gtk_box_pack_start(GTK_BOX(vbox), statusBar, FALSE, TRUE, 0);

    return ( mainWindow );

}

void
controlPanelActivate( void )
{

    /* if it exists i.e. has been built */
    if ( cpanelWindow )
    {
        /* if it's already visible */
        if ( cpanelVisible )
        {
            /* check to see if it's on top level for user, if so, then hide it */
            /* thus clicking on the status icon toggles it, like skype */
            if (gtk_window_is_active( (GtkWindow *)cpanelWindow ) )
                closeWindow();
        }
        else
        /* it's build, but not visible, so pop it up to the top! */
        {
            /* make sure it always become visible for the user as could be hidden
               on another workspace, iconized etc */
            gtk_window_present( (GtkWindow *)cpanelWindow );
            cpanelVisible = TRUE;
        }
    }
    else
    /* build it from scratch the first time, so we're not using that memory etc if not needed */
    {
        /* build the widget tree for the entire UI */
        cpanelWindow = buildCPanel();

        /* Show window, and recursively all contained widgets */
        gtk_widget_show_all( cpanelWindow );
        cpanelVisible = TRUE;
    }

}

#endif