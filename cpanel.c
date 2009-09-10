#include <gtk/gtk.h>
#include <tagReader.h>

/* TODO: there should be a system or POSIX definition of this that should be used */
#define MAX_PATH (1024)


typedef char	Action[MAX_PATH];

typedef struct {
   	uid		ID;
 	Action		INaction;
	Action		OUTaction;
	char		description[80];
	BOOL		enabled;
} panelEntry;

int
main(
     int 		argc, 
     char 		*argv[]
)
{
   GtkBuilder	*builder;
   GtkWidget  *window;
   GError     *error = NULL;

   /* Init GTK+ */
   gtk_init( &argc, &argv );

   /* Create new GtkBuilder object */
   builder = gtk_builder_new();

   /* Load UI from file. If error occurs, report it and quit application. */
   if( ! gtk_builder_add_from_file( builder, "cpanel.glade", &error ) )
   {
      g_warning( "%s", error->message );
      g_free( error );
      return( 1 );
   }

   /* Get main window pointer from UI */
   window = GTK_WIDGET( gtk_builder_get_object( builder, "window1" ) );

   /* Connect signals */
   gtk_builder_connect_signals( builder, NULL );

   /* Destroy builder, since we don't need it anymore */
   g_object_unref( G_OBJECT( builder ) );

/* TODO

load the config file of tags events etc
if none found then
   initialize a blank one and set error flag and message

Show daemon status in the status line, and the number of tags detected.

*/

   /* Show window. All other widgets are automatically shown by GtkBuilder */
   gtk_widget_show( window );

/* TODO
if error flag set then
   show an error dialog with message and let know what action was taken
*/
 
   /* Start main loop */
   gtk_main();

/* event processing
on any APPLY, signal daemon to reload the config file
on tag detection, refresh the tag list on screen, prefil the tag ID, and highlight the current tag rows in table 
*/

   return( 0 );

}


