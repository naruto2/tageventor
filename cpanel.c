#include <stdlib.h>
#include <gtk/gtk.h>
#include "tagReader.h"
#include <string.h>

/* TODO: there should be a system or POSIX definition of this that should be used */
#define MAX_PATH (1024)

#define TOOL_TIP_TEXT "Tageventor: \nLeft-click for control panel.\nRight-click for actions."
#define PROGRAM_NAME    "Tageventor"
#define DIALOG_TITLE    "Tageventor Control Panel"
#define VERSION_STRING  "0.0.0.0"
/* TODO find a way to get this from svn, or Make or something */

typedef char	tScript[MAX_PATH];

/* where to save this type of config stuff???? via GConf or something */
typedef struct {
   	char		*ID;
 	char		*script;
	char		*description;
	BOOL		enabled;
} tPanelEntry;

const gchar *columnHeader[4] = { "Tag ID", "Description", "Script", "Enabled" };

const gchar  *authors[] = {"Andrew Mackenzie (andrew@autelic.org)", NULL };

const char      *iconNameList[] = { "tageventor16x16.png",
                                    "tageventor32x32.png",
                                    "tageventor48x48.png",
                                    "tageventor64x64.png",
                                    "tageventor128x128.png" };

BOOL            savePending = FALSE;
BOOL            cpanelVisible = FALSE;
int             numTagEntries = 0;
tPanelEntry     *tagEntryArray; /* TODO 10 for now for testing */
GtkWidget       *cpanelWindow = NULL, *statusBar = NULL;
GtkStatusIcon   *cpanelIcon = NULL;
guint           tagCheckTimeout;

static  tReader         reader;

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

static gboolean
delete_event(
            GtkWidget *widget,
            GdkEvent  *event,
            gpointer   data
            )
{

    if ( !savePending )
    {
        /* If you return FALSE in the "delete_event" signal handler,
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


static void destroy( GtkWidget *widget,
                     gpointer   data )
{

    /* hide the main cpanel window */
    hideCPanelWindow();

}

void
showHelp( void )
{

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

}


/* Callback for when each entry's toggle button to enable it is toggled */
static void
enableChange(
             GtkWidget *widget,
             gpointer   entryIndex
             )
{

    /* get the button state and put into tagEntryArray */
    tagEntryArray[(int)entryIndex].enabled = gtk_toggle_button_get_active( (GtkToggleButton *)widget);
    savePending = TRUE;

}


/* Non-interactive function that actually saves the data in the file */
static void
saveTagConfig( const tPanelEntry *pTagEntryArray )
{

/* where to save this type of config stuff???? via GConf or something */

    /* now the table used to process events matches what's in the UI so we are in sync */
    savePending = FALSE;
}

void
applyChanges( void )
{

    /* TODO read the current config from the widgets into a temporary table, checking syntax for each
            and maybe checking execute permissions, etc */

    /* if all is OK, then modify the table we use to process events */

    /* and save the changes to whereever, so they are loaded like this next time */
    saveTagConfig( tagEntryArray );

}


int
readTagConfig( tPanelEntry **pTagEntryArray )
{
    int     numTags, size, i;
#define NUM_FAKE_TAGS   (10)

    static    tPanelEntry fakeTags[NUM_FAKE_TAGS] = {
        { "12345678901234", "/home/andrew/test", "a fake tag for testing", TRUE },
        { "45678901234567", "/home/andrew/test2", "another fake tag for testing", FALSE },
        { "45678901234567", "/home/andrew/test2", "another fake tag for testing", FALSE },
        { "45678901234567", "/home/andrew/test2", "another fake tag for testing", FALSE },
        { "45678901234567", "/home/andrew/test2", "another fake tag for testing", FALSE },
        { "45678901234567", "/home/andrew/test2", "another fake tag for testing", FALSE },
        { "45678901234567", "/home/andrew/test2", "another fake tag for testing", FALSE },
        { "45678901234567", "/home/andrew/test2", "another fake tag for testing", FALSE },
        { "45678901234567", "/home/andrew/test2", "another fake tag for testing", FALSE },
        { "45678901234567", "/home/andrew/test2", "another fake tag for testing", FALSE }
        };

    /* TODO need to figure out how many tags we have before allocating memory! */
    numTags = NUM_FAKE_TAGS;

    /* allocate memory for the array of tag entries found */
    size = (numTags * sizeof(tPanelEntry) );
    *pTagEntryArray = malloc( size ); /* TODO check about alignment and need to pad out */

/* TODO figure out where to save this type of config stuff???? via GConf or something */

    for (i = 0; i < numTags; i++)
    {
/* TODO we will have to malloc each string when this is done for real */
        (*pTagEntryArray)[i].ID = fakeTags[i].ID;
        (*pTagEntryArray)[i].script = fakeTags[i].script;
        (*pTagEntryArray)[i].description = fakeTags[i].description;
        (*pTagEntryArray)[i].enabled = fakeTags[i].enabled;
    }

    return( numTags );

}

void
tableAddRow( GtkTable *pTable, int i, char * ID, char * description, char *script, BOOL enabled )
{
    GtkWidget    *label, *chooser, *enable, *entry;
    gchar       message[80];

    /* Tag ID of 14 char which does not need to expand with window size */
    label = gtk_label_new(ID);
    /* attach a new widget into the table */
    gtk_table_attach(pTable, label, 0, 1, i+1, i+2, GTK_FILL, GTK_FILL, 5, 0 );

    /* description entry of upto 80 characters which can benefit from expanding horizontally*/
    entry = gtk_entry_new();
    gtk_entry_set_max_length( (GtkEntry *)entry, 80);
    gtk_entry_set_text((GtkEntry *)entry, description);
    /* attach a new widget into the table */
    gtk_table_attach(pTable, (GtkWidget *)entry, 1, 2, i+1, i+2, GTK_EXPAND | GTK_FILL, GTK_FILL, 4, 0 );

    /* file chooser for script associated with this tag, which may have long name and so expand*/
    if ( ID == NULL )
        sprintf(message, "Choose the file to be executed for tag" );
    else
        sprintf(message, "Choose the file to be executed for tag with ID: %s", ID );
    chooser = gtk_file_chooser_button_new( message, GTK_FILE_CHOOSER_ACTION_OPEN );
    gtk_file_chooser_button_set_width_chars((GtkFileChooserButton *)chooser, strlen(script) );
/* TODO get the users home directory path */
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (chooser), "/home/andrew/.tageventor");
/* TODO set chooser text from file !!!!! */
    /* attach a new widget into the table */
    gtk_table_attach(pTable, chooser, 2, 3, i+1, i+2, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 2 );
    /* add a callback to the button which will be passed the index of this entry in the tagEntryArray */
/* TODO we need a callback for when the file is selected !!! */
g_signal_connect (G_OBJECT (chooser), "file-set", G_CALLBACK (scriptChosen), (gpointer)i);

    enable = gtk_check_button_new(); /* check button to enable the script for this tag */
    gtk_toggle_button_set_active( (GtkToggleButton *)enable, enabled);
    /* attach a new widget into the table */
    gtk_table_attach(pTable, enable, 3, 4, i+1, i+2, GTK_FILL, GTK_FILL, 0, 0 );
    /* add a callback to the button which will be passed the index of this entry in the tagEntryArray */
    g_signal_connect (G_OBJECT (enable), "toggled", G_CALLBACK (enableChange), (gpointer)i);

}


void
addTag(
        GtkWidget *widget,
        gpointer *pTable
        )
{
    int i;

    /* add an entry to the tabe */
    numTagEntries++;

    /* this entry's index */
    i = (numTagEntries -1);

    /* resize the table */
    gtk_table_resize( (GtkTable *)pTable, (numTagEntries + 1), 4 );

/* TODO URGENT need to resize the array and reset the new entry */

    tableAddRow((GtkTable *)pTable, i, tagEntryArray[i].ID, tagEntryArray[i].description, tagEntryArray[i].script, tagEntryArray[i].enabled );

/* TODO Get this to work, actually adding the new row visibly !!!!!!!!!!! */

    /* this should actually be set in each callback for each of the 4 widgets added so if you don't edit this new
    blank entry then nothing is considered changed */
    savePending = TRUE;
}

static void
closeAbout(
        GtkDialog   *dialog,
        gpointer   user_data
        )
{

    gtk_widget_destroy( (GtkWidget *)dialog );

}


static void
aboutResponse(
            GtkDialog *dialog,
            gint       response_id,
            gpointer   user_data
            )
{
}

static void
showAbout( void )
{

    GtkWidget   *aboutDialog;

    aboutDialog = gtk_about_dialog_new();

    gtk_about_dialog_set_program_name( (GtkAboutDialog *)aboutDialog, DIALOG_TITLE );

    gtk_about_dialog_set_version( (GtkAboutDialog *)aboutDialog, VERSION_STRING );

    gtk_about_dialog_set_copyright( (GtkAboutDialog *)aboutDialog, "Copyright Autelic Association" );

    gtk_about_dialog_set_license( (GtkAboutDialog *)aboutDialog, "Licensed under Apache 2.0 License" );
/* TODO show the full license text via including a file under version control into a string, at compile time
or maybe reading from a file at run time */

    gtk_about_dialog_set_website( (GtkAboutDialog *)aboutDialog, "http://tageventor.googlecode.com" );
    gtk_about_dialog_set_website_label( (GtkAboutDialog *)aboutDialog, "Development Site (Google Code)" );

    gtk_about_dialog_set_comments( (GtkAboutDialog *)aboutDialog, "Tageventor is a program that detects RFID/NFC tags, via an appropriate reader hardware connected to your computer, and performs actions when they are placed or removed from the reader. \nThis Control Panel allows the user to configure what actions are performed for each tag by assigning a script to it." );

/* find a way to read this at compile or run time from the COMMITTERS file */
    gtk_about_dialog_set_authors( (GtkAboutDialog *)aboutDialog, authors );

    /* hook-up the signal handler for the about box's close button */

/* TODO set the default button that is pressed with Enter */

/* TODO show the proper logo for this window also
gtk_about_dialog_set_logo_icon_name (GtkAboutDialog *about,
                                                         const gchar *icon_name);
*/
    /* this will be triggered when escape key is used */
    g_signal_connect ( G_OBJECT (aboutDialog), "close", G_CALLBACK (closeAbout), NULL );

    g_signal_connect ( G_OBJECT (aboutDialog), "response", G_CALLBACK (aboutResponse), NULL );

    gtk_widget_show( aboutDialog );

}

gboolean
tagPoll( gpointer    data)
{
    gchar               statusMessage[80];
    gchar               toolTipText[strlen(TOOL_TIP_TEXT) + 80];
    uid                 ID;
    tTagList	        TagList;
    LONG 		        rvalue;
    int                 i;
    static BOOL         connected = FALSE; /* we start, not being connected */

/* TODO
in this function at the moment we check if the window is visible and push the status message if it is,
but we don't do anything (yet) if it's not visible...
*/

    /* if not already connected, then try and connect */
    if ( connected == FALSE )
    {
        /* connect to reader */
        rvalue = readerConnect(&reader);

        if ( rvalue == SCARD_S_SUCCESS )
            connected = TRUE;
        else
            sprintf(statusMessage, "Could not connect to tag reader.");
    }

    /* we might be connected since before this function was called */
    /* or due to the attempt just above. Handle both the same */
    if ( connected == TRUE )
    {
        rvalue = getTagList(&reader, &TagList);

        if ( rvalue == SCARD_S_SUCCESS )
        {
            sprintf(statusMessage, "Connected to reader, %d tags:", (int)TagList.numTags);
            for (i=0; i<TagList.numTags; i++)
            {
                sprintf( ID, " %s", TagList.tagUID[i]);
                strcat( statusMessage, ID );
            }
        }
        else
            sprintf(statusMessage, "Connected to reader, problems reading.");
    }

    /* put the result onto the status bar */
    if ( cpanelWindow && cpanelVisible )
        gtk_statusbar_push((GtkStatusbar *)statusBar, 0, (gchar *)statusMessage);

    /* push the message into the tool tip for the status icon also */
    sprintf( toolTipText, "%s\n%s", TOOL_TIP_TEXT, statusMessage );
    gtk_status_icon_set_tooltip_text( cpanelIcon, toolTipText );

    /* return TRUE to indicate that this function should be called again */
    return ( TRUE );

}

void
createTable( GtkTable *pTable )
{
    int         i;
    GtkWidget    *label;

    if ( numTagEntries == 0 )
    {
        label = gtk_label_new( "No Tags configured. Use the 'Add' button below to configure a tag" );
        gtk_table_attach(pTable, label, 0, 1, 0, 1, GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 10, 10 );

        return;
    }

    /* resize the table to be big enough to hold all entries */
    /* Row 0  =  Column headers */
    /* So we need numConfigTags + 1 rows, and 4 columns */
    gtk_table_resize( (GtkTable *)pTable, (numTagEntries + 1), 4 );

    /* add the four column labels in the table*/
    for (i = 0; i < 4; i++)
    {
        /* create the text label */
        label = gtk_label_new( columnHeader[i] );

        /* attach a new widget into the table in column 'i', row 0 */
        gtk_table_attach(pTable, label, i, i+1, 0, 1, GTK_FILL, GTK_FILL, 5, 3 );
    }

    /* add a new row of widgets for each tag script configuration found */
    for (i = 0; i < numTagEntries; i++)
        tableAddRow(pTable, i, tagEntryArray[i].ID, tagEntryArray[i].description, tagEntryArray[i].script, tagEntryArray[i].enabled );

}

GtkWidget *
buildCPanel ( void  )
{
    GtkWidget   *mainWindow;
    GtkWidget   *vbox, *scroll, *buttonBox, *table;
    GtkWidget   *applyButton, *helpButton, *closeButton, *aboutButton, *addButton;
    GError      *pError;

    /******************************* Main Application Window ************************/
    /* create main window */
    mainWindow = gtk_window_new( GTK_WINDOW_TOPLEVEL );
    gtk_window_set_title( (GtkWindow *)mainWindow, PROGRAM_NAME );
    /* Smallest height possible then should expand to hold what's needed */
    gtk_window_set_default_size( (GtkWindow *)mainWindow, 540, 250 );

    /* set the icon for the window */
    gtk_window_set_icon_from_file( (GtkWindow *)mainWindow, iconNameList[0], &pError );
/* TODO pass in a full list when I figure out how to create the PixBufs from files!
    gtk_window_set_icon_list( window, iconList );
    */

    /* When the window is given the "delete_event" signal (this is given
     * by the window manager, usually by the "close" option, or on the
     * titlebar), we ask it to call the delete_event () function
     * as defined above. The data passed to the callback
     * function is NULL and is ignored in the callback function. */
    g_signal_connect (G_OBJECT (mainWindow), "delete_event",
		      G_CALLBACK (delete_event), NULL);

    /* Here we connect the "destroy" event to a signal handler.
     * This event occurs when we call gtk_widget_destroy() on the window,
     * or if we return FALSE in the "delete_event" callback. */
    g_signal_connect (G_OBJECT (mainWindow), "destroy",
		      G_CALLBACK (destroy), NULL);

    /* this will be triggered when escape key is used */
    /* TODO: this is invalid. is there a way to close it with escape key ?
    g_signal_connect ( G_OBJECT (mainWindow), "close", G_CALLBACK (closeMain), NULL );
    */

    /******************************* Vertical Box ***********************************/
    /* vertical box to hold things, Not homogeneous sizes and spaceing 0 */
    vbox = gtk_vbox_new(FALSE, 0);

    /* This packs the vbox into the window (a gtk container). */
    gtk_container_add (GTK_CONTAINER (mainWindow), vbox);

    /******************************* Tag Config ************************************/
    /* load the tag configuration into memory before creating the table to show it */
    numTagEntries = readTagConfig( &tagEntryArray );

    /******************************* Table ******************************************/
    /* create the scolled window that will hold the viewport and table */
    scroll = gtk_scrolled_window_new( NULL, NULL);

    /* This packs the scrolled window into the vbox (a gtk container). */
    gtk_box_pack_start( GTK_BOX(vbox), scroll, TRUE, TRUE, 3);

    /* create the table for tag IDs */
    table = gtk_table_new( 1, 1, FALSE);

    /* fill the table with rows and columns of controls for scripts */
    createTable( (GtkTable *)table );

    /* add the non-scrollable Table to the scrolled window via a viewport */
    gtk_scrolled_window_add_with_viewport( (GtkScrolledWindow *)scroll, table );
    /******************************* End of Table *************************************/

    /******************************** Add Button  *************************************/
    addButton = gtk_button_new_from_stock( "gtk-add" );

    /* This packs the button into the hbutton box  (a gtk container). */
    gtk_box_pack_start( GTK_BOX(vbox), addButton, FALSE, FALSE, 3);

    /* When the button receives the "clicked" signal, it will call the
     * function applyChanges() passing it NULL as its argument. */
    g_signal_connect (G_OBJECT (addButton), "released", G_CALLBACK (addTag), table);

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

    /********************************************* About Button ***********************/
    aboutButton = gtk_button_new_from_stock( "About" );

    /* This packs the button into the hbutton box  */
    gtk_box_pack_start( GTK_BOX(buttonBox), aboutButton, FALSE, TRUE, 3);

    /* When the button receives the "clicked" signal, it will call the
     * function applyChanges() passing it NULL as its argument. */
    g_signal_connect (G_OBJECT (aboutButton), "released", G_CALLBACK (showAbout), NULL);

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
iconActivate(
             GtkStatusIcon *status_icon,
             gpointer       user_data
             )
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
    /* build it from scratch */
    {
        /* build the widget tree for the entire UI */
        cpanelWindow = buildCPanel();

        /* Show window, and recursively all contained widgets */
        gtk_widget_show_all( cpanelWindow );
        cpanelVisible = TRUE;
    }

}

void
iconQuit( void )
{
/* TODO check stuff etc */
    exit( 0 );
}

void
iconPopupMenu(GtkStatusIcon *status_icon,
              guint          button,
              guint          activate_time,
              gpointer       popupMenu
              )
{

    gtk_menu_popup( (GtkMenu *)popupMenu, NULL, NULL, NULL, NULL, button, activate_time );

}



GtkStatusIcon *
buildCPanelIcon( void )
{
    GtkStatusIcon   *icon;
    GtkWidget       *popupMenu, *quitMenuItem, *aboutMenuItem;

    icon = gtk_status_icon_new_from_file( "tageventorStatusIcon22x22.png" );

    gtk_status_icon_set_tooltip_text( icon, TOOL_TIP_TEXT );
/* TODO add logic in polling to add minimal status info to the tool tip */

    /* connect up the handler for the left mouse-click */
    g_signal_connect (G_OBJECT (icon), "activate", G_CALLBACK (iconActivate), NULL);

    popupMenu = gtk_menu_new();

    /* Now add entries to the menu */
    aboutMenuItem = gtk_menu_item_new_with_label( "About" );
    gtk_menu_shell_append( GTK_MENU_SHELL( popupMenu ), aboutMenuItem );
    g_signal_connect (G_OBJECT (aboutMenuItem), "activate", G_CALLBACK (showAbout), NULL );
    gtk_widget_show( aboutMenuItem );

    quitMenuItem = gtk_menu_item_new_with_label( "Quit" );
    gtk_menu_shell_append( GTK_MENU_SHELL( popupMenu ), quitMenuItem );
    g_signal_connect (G_OBJECT (quitMenuItem), "activate", G_CALLBACK (iconQuit), NULL );
    gtk_widget_show( quitMenuItem );

    /* connect menu to the event that will be used (right click on status icon) and pass in pointer to menu */
    g_signal_connect (G_OBJECT (icon), "popup-menu", G_CALLBACK (iconPopupMenu), popupMenu );

    return( icon );

}

int
main (int argc, char *argv[])
{

    /* some help to make sure we close whatś needed, not more */
    reader.hContext = ((SCARDCONTEXT)NULL);
    reader.hCard = ((SCARDHANDLE)NULL);
    reader.number = 0;

/* TODO uncomment later when we merge code with tagEventor.c */
    /*parseCommandLine(argc, argv,
                    &(reader.number), &verbosityLevel, &daemonOptions, &retryDelaysec, &pollDelayus);
*/
    /* set reader library options other than defaults (i.e. verbosity) */
    /* this is only really needed if we are going to use logMessage() */
    readerSetOptions( 0, FALSE );

    /* Init GLib and GTK+ */
    g_type_init();
    gtk_init( &argc, &argv );

	/* add application specific icons to search path */
/* TODO we will probably need an icon at some point	gtk_icon_theme_append_search_path (gtk_icon_theme_get_default (),
                                           GPM_DATA G_DIR_SEPARATOR_S "icons");
*/

    /* build the status icon in the system tray area */
    cpanelIcon = buildCPanelIcon();

    /* make sure the icon is shown */
    gtk_status_icon_set_visible( cpanelIcon, TRUE );

    /* add a timeout to check for tags every second */
    tagCheckTimeout = g_timeout_add_seconds(1, tagPoll, NULL );

    /* Start main loop */
    gtk_main();

    /* Ignore errors from disconnect as we're going to try and reconnect anyway */
    readerDisconnect(&reader);

    return( 0 );

}
