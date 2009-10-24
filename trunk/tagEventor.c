/*
  tagEventor.c - C source code for tagEventor application / daemon

  Copyright 2008-2009 Autelic Association (http://www.autelic.org)

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

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <syslog.h>

#include <signal.h>
#include <sys/stat.h>  /* for umask() */
#include <limits.h>

#include <PCSC/winscard.h>

#include <tagReader.h>

#include "tagEventor.h"
#include "rulesTable.h"
#include "systemTray.h"
#include "stringConstants.h"

/*************************** MACROS ************************/
/* Tag event types */
#define TAG_IN  (0)
#define TAG_OUT (1)

/*************    TYPEDEFS TO THIS FILE     **********************/
typedef enum { FOREGROUND, START_DAEMON, STOP_DAEMON, SYSTEM_TRAY } tRunOptions;


/************* VARIABLES STATIC TO THIS FILE  ********************/
static  int		        lockFile = -1;
static  char		    lockFilename[PATH_MAX];
static  unsigned char	runningAsDaemon = FALSE;


/* strings used for tag events, for text output and name of scipts */
static int		    appPollDelayms;
static int			appVerbosityLevel;

static tTagList		currentTagList = { NULL, 0 }, previousTagList = { NULL, 0 };

/************** GLOBALS *******************************************/
/* these are only needed globally as a horrible workaround for
   readersTable */
tReaderManager  readerManager = { 0, NULL, NULL };
tReader         readers[MAX_NUM_READERS];

int
appPollDelaySet(
                int     newPollDelay
                )
{
    /* make sure not exceed limit */
    if ( newPollDelay > POLL_DELAY_MILLI_SECONDS_MAX )
        newPollDelay = POLL_DELAY_MILLI_SECONDS_MAX;

    if ( newPollDelay < POLL_DELAY_MILLI_SECONDS_MIN )
        newPollDelay = POLL_DELAY_MILLI_SECONDS_MIN;

    /* set the new delay to the capped value */
    appPollDelayms = newPollDelay;

#ifdef BUILD_SYSTEM_TRAY
    systemTraySetPollDelay( appPollDelayms );
#endif

    /* return the value that was actually set */
    return ( appPollDelayms );

}

int
appPollDelayGet( void )
{
    return( appPollDelayms );
}

int appVerbosityLevelSet(
                    int     newLevel
                    )
{

    /* make sure not exceed limit */
    if ( newLevel > VERBOSITY_MAX )
        newLevel = VERBOSITY_MAX;

    if ( newLevel < VERBOSITY_MIN )
        newLevel = VERBOSITY_MIN;

    appVerbosityLevel = newLevel;

    /* return the value that was actually set */
    return ( appVerbosityLevel );

}

int
appVerbosityLevelGet( void )
{
    return( appVerbosityLevel );
}

/************************ PARSE COMMAND LINE OPTIONS ********/
static void
parseCommandLine(
                int 		    argc,
                char 		    *argv[],
                tRunOptions     *pRunOptions
                )
{

   unsigned char parseError = FALSE;
   int		option, newDelay, newVerbosity;
   int      number;

   /* Set default values */
   appVerbosityLevel = VERBOSITY_DEFAULT;

#ifdef BUILD_SYSTEM_TRAY
   /* this is the gtagEventor GUI version built to install in system tray */
   *pRunOptions = SYSTEM_TRAY;
#else
   /* default mode is to run in foreground at the terminal */
   *pRunOptions = FOREGROUND;
#endif

   appPollDelayms = POLL_DELAY_MILLI_SECONDS_DEFAULT;

   while ( ((option = getopt(argc, argv, "n:v:d:p:h")) != EOF) && (!parseError) )
      switch (option)
      {
         /* 'n' option is for reader number(s) to connect to */
         /* this switch could be supplied repeatedly, once for each reader. */
         /* check for the AUTO term before converting to a number! */
        /* For each case add that reader to the list enabled, even if AUTO is also specified */
        /* the program logic will take notice of Auto, but GUI will also reflect the others */
         case 'n':
            if ( strcmp( optarg, "AUTO" ) == strlen( "AUTO" ) )
                readersSettingBitmapBitSet( READER_BIT_AUTO );
            else
            {
                number = atoi(optarg);
                if ( ( number  < 0) || ( number >= MAX_NUM_READERS ) )
                {
                    fprintf(stderr, TAGEVENTOR_STRING_COMMAND_LINE_READER_NUM_ERROR_1, MAX_NUM_READERS );
                    fprintf(stderr, TAGEVENTOR_STRING_COMMAND_LINE_READER_NUM_ERROR_2, optarg);
                }
                else
                    readersSettingBitmapNumberSet( number );
            }
            break;

         /* 'v' option is for verbosity level */
         case 'v':
            newVerbosity = atoi(optarg);
            /* check if the new verbosity was within range when set */
            if ( appVerbosityLevelSet( newVerbosity ) != newVerbosity )
            {
               fprintf(stderr, "Verbosity level must be greater or equal to 0\n");
               fprintf(stderr, "Verbosity level has been forced to %d\n", appVerbosityLevelGet() );
            }
            break;

         case 'd':
            if (strcmp(optarg, "start") == 0)
               *pRunOptions = START_DAEMON;
            else if (strcmp(optarg, "stop") == 0)
                  *pRunOptions = STOP_DAEMON;
                      else
                      {
                          parseError = TRUE;
                          fprintf(stderr, "Invalid parameter for '-d' daemon option\n");
                      }
            break;

         /* 'p' option is for the delay in polling in useconds */
         case 'p':
            newDelay = atoi(optarg);
            /* check if the new delay was within range when set */
            if ( appPollDelaySet ( newDelay ) != newDelay )
            {
               fprintf(stderr, "Poll delay must be greater or equal to 0\n");
               fprintf(stderr, "Poll delay has been forced to %d\n", appPollDelayGet() );
            }
            break;

         /* 'h' option is to request print out help */
         case 'h':
                fprintf(stderr, TAGEVENTOR_STRING_USAGE, argv[0],
                        POLL_DELAY_MILLI_SECONDS_MIN, POLL_DELAY_MILLI_SECONDS_DEFAULT,
                        POLL_DELAY_MILLI_SECONDS_MAX, VERSION_STRING );
            exit ( 0 );
            break;

         default:
	    parseError = TRUE;
            break;
      }

   if (parseError)
   {
      fprintf(stderr, TAGEVENTOR_STRING_USAGE, argv[0], POLL_DELAY_MILLI_SECONDS_MIN,
              POLL_DELAY_MILLI_SECONDS_DEFAULT, POLL_DELAY_MILLI_SECONDS_MAX,
              VERSION_STRING );
      exit( EXIT_FAILURE );
   }

    /* if no reader numbers were specified and AUTO neither, then set default */
    if ( readersSettingBitmapGet() == READER_BIT_NONE )
        readersSettingBitmapSet( READER_BIT_DEFAULT );

}
/************************ PARSE COMMAND LINE OPTIONS ********/



/***************** HANDLE SIGNALS ***************************/
static void
handleSignal(
	         int	sig
	        )
{

   switch( sig )
   {
      case SIGCHLD:
      break;

      case SIGHUP: /* restart the server using  "kill -1" at the command shell */
         readersDisconnect( &readerManager, readers );
         readersConnect( &readerManager, &(readers[0]) );
         readersLogMessage(LOG_INFO, 1, "Hangup signal received - disconnected and reconnected");
      break;

      case SIGINT: /* kill -2 or Control-C */
      case SIGTERM:/* "kill -15" or "kill" */
         readersDisconnect( &readerManager, readers );
         readersLogMessage( LOG_INFO, 1, "SIGTERM or SIGINT received, exiting gracefully");
         if ( runningAsDaemon )
         {
            readersLogMessage( LOG_INFO, 1, "Closing and removing lockfile, closing log. Bye.");
            close( lockFile );
            remove( lockFilename );
            closelog();
         }

         exit(0);
      break;
   }
}
/***************** HANDLE SIGNAL ***************************/


/************************ STOP DAEMON **************/
static void
stopDaemon( void )
{
   char		messageString[MAX_LOG_MESSAGE];
   char 	pidString[20];
   int		pid;

   /* open lock file to get PID */
   sprintf(lockFilename, "%s/%s.lock", DEFAULT_LOCK_FILE_DIR, DAEMON_NAME);
   lockFile = open( lockFilename, O_RDONLY, 0 );
   if (lockFile == -1)
   {
      sprintf(messageString, "Could not open lock file %s, exiting", lockFilename);
      readersLogMessage(LOG_ERR, 0, messageString);
      sprintf(messageString, "Check you have the necessary permission for it");
      readersLogMessage(LOG_ERR, 0, messageString);
      exit( EXIT_FAILURE );
   }

   /* get the running PID in the lockfile */
   if (read(lockFile, pidString, 19) == -1)
   {
      sprintf(messageString, "Could not read PID from lock file %s, exiting", lockFilename);
      readersLogMessage(LOG_ERR, 0, messageString);
      exit( EXIT_FAILURE );
   }

   close( lockFile );

   if (sscanf( pidString, "%d\n", &pid ) != 1)
   {
      sprintf(messageString, "Could not read PID from lock file %s, exiting", lockFilename);
      readersLogMessage(LOG_ERR, 0, messageString);
      exit( EXIT_FAILURE );
   }

   sprintf(messageString, "Stopping daemon with PID = %d", pid );
   readersLogMessage(LOG_INFO, 1, messageString);

   /* might need to be root for this to work ?  - try and kill nicely*/
   kill (pid, SIGTERM);

   /* TODO else, use a bit more brute force */
   /* check if running? */
   /* remove the lock file myself if I can ! */
   sleep(1);
   remove( lockFilename );
}
/************************ STOP DAEMON **************/

/********************** GET LOCK OR DIE *********************/
/* This runs in the forked daemon process */
/* make sure we are the only running copy for this reader number */
static void
getLockOrDie( void )
{
   char 	pidString[20];
   char		messageString[MAX_LOG_MESSAGE];
   struct flock lock;

   sprintf(lockFilename, "%s/%s.lock", DEFAULT_LOCK_FILE_DIR, DAEMON_NAME );
   lockFile = open( lockFilename, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH );
   if ( lockFile == -1 )
   {
      sprintf(messageString,
              "Could not open lock file %s, check permissions or run as root, exiting", lockFilename);
      readersLogMessage(LOG_ERR, 0, messageString);
      exit( EXIT_FAILURE );
   }

   /* Initialize the flock structure. */
   memset (&lock, 0, sizeof(lock));
   lock.l_type = F_WRLCK;

   /* try and get the lock file, non-blocking */
   if ( fcntl(lockFile, F_SETLK, &lock) == -1 )
   {
      sprintf(messageString, "Could not lock file %s", lockFilename);
      readersLogMessage(LOG_ERR, 0, messageString);
      sprintf(messageString, "Probably indicates a previous copy is still running or crashed");
      readersLogMessage(LOG_ERR, 0, messageString);
      sprintf(messageString, "Find PID using \"cat %s\" or \"ps -ef | grep %s\". Exiting.",
             lockFilename, DAEMON_NAME);
      readersLogMessage(LOG_ERR, 0, messageString);
      exit( EXIT_FAILURE );
   }

   /* store the running PID in the lockfile */
   sprintf( pidString, "%d\n", getpid());
   if (write(lockFile, pidString, strlen(pidString)) != strlen(pidString) )
   {
      sprintf(messageString, "Could not write PID to lock file %s", lockFilename);
      readersLogMessage(LOG_ERR, 0, messageString);
      exit( EXIT_FAILURE );
   }
}
/********************** GET LOCK OR DIE *********************/


/************************ DAEMONIZE *************************/
static void
daemonize (
		)
{
   int		pid;
   char		messageString[MAX_LOG_MESSAGE];

   /* fork a copy of myself */
   pid = fork();
   if ( pid < 0 )
   { /* fork error */
      sprintf(messageString, "Error forking daemon %s, exiting", DAEMON_NAME);
      readersLogMessage(LOG_ERR, 0, messageString);
      exit( EXIT_FAILURE );
   }

   if ( pid > 0 )
   { /* fork worked, this is the parent process so exit */
      sprintf(messageString, "Started daemon %s with PID=%d, see /var/log/syslog",
              DAEMON_NAME, pid );
      readersLogMessage(LOG_INFO, 1, messageString);
      exit( EXIT_FAILURE );
   }

   /* from here on I must be a successfully forked child */
   runningAsDaemon = TRUE;

   /* tell the library we are now going to be calling it from a daemon */
   readersSetOptions( IGNORE_OPTION, runningAsDaemon );

   /* set umask for creating files */
   umask(0);

   /* start logging --> /var/log/syslog on my system */
   openlog(DAEMON_NAME, LOG_PID, LOG_DAEMON);

   /* get a new process group for daemon */
   if ( setsid() < 0 )
   {
      sprintf(messageString, "Error creating new SID for daemon process %s with PID=%d, see in /var/log/syslog", DAEMON_NAME, pid );
      readersLogMessage(LOG_ERR, 0, messageString);
      exit( EXIT_FAILURE );
   }


   /* change working directory to / */
   if ( (chdir("/")) < 0 )
      exit( EXIT_FAILURE );

   /* These following lines to close open filedescriptors are recommended for daemons, but it      */
   /* seems to cause problems for executing some xternal scripts with system() so they are avoided */
#if 0
   /* close unneeded descriptions in the deamon child process */
   for (i = getdtablesize(); i >= 0; i--)
      close( i );

   /* close stdio */
   close ( STDIN_FILENO );
   close ( STDOUT_FILENO );
   close ( STDERR_FILENO );
#endif

   /* make sure Iḿ the only one reading from this reader */
   getLockOrDie();

   /* ignore TTY related signales */
   signal( SIGTSTP, SIG_IGN );
   signal( SIGTTOU, SIG_IGN );
   signal( SIGTTIN, SIG_IGN );

   sprintf(messageString, "Daemon Started" );
   readersLogMessage(LOG_INFO, 1, messageString);
}
/*********************  DAEMONIZE ****************************/



static int
tagListCheck(
            void *data /* pointer to a callback function */
            )
{
    int  		    rv;
    unsigned char	found;
    int			    i, j;
    char			messageString[MAX_LOG_MESSAGE],
                    tagLine[MAX_TAG_UID_SIZE + strlen(TAGEVENTOR_STRING_TAG_LINE)],
                    tagMessage[MAX_LOG_MESSAGE];
    void            (*callBack)( char, const char  *, const char * ) = data;

    /*** All this work about connecting to readers is done inside the poll */
    /* function to enable disconnect and reconnect of readers in operation */
    /* so that a "best effort" is always done, and hotplugging works       */
    /* Even late starting of the pcscd daemon, or killing it and restarting */
    /* should be recovered from by tagEventor, connecting to it when avail.*/

    /* make sure messages are always valid */
    messageString[0] = tagLine[0] = tagMessage[0] = '\0';

    /* If not connected to PCSCD, then try and connect */
    if ( readerManager.hContext == NULL )
    {
        if ( readersManagerConnect( &readerManager, readers ) != SCARD_S_SUCCESS )
        {
            sprintf( messageString, TAGEVENTOR_STRING_PCSCD_PROBLEM );

            if ( callBack )
                (*callBack)( FALSE, messageString, "" );

            /* nothing else to do for now, return TRUE so I get called again */
            return( TRUE );
        }
        else
        {
            /* try and connect to the specified readers on first connect */
            if ( readersConnect( &readerManager, readers ) != SCARD_S_SUCCESS )
                sprintf( messageString, TAGEVENTOR_STRING_PCSCD_OK_READER_NOT ,
                        readerManager.nbReaders );
        }
    }

    /*      - free the memory of the 'previous' tag array, i.e. 2 iterations ago
            - make 'previous' be 'current' from the previous iteration */
    if ( previousTagList.pTags != NULL )
        free ( previousTagList.pTags );
    previousTagList.pTags = currentTagList.pTags;  /* this will be freed next time around */

    previousTagList.numTags = currentTagList.numTags;

    /* get the new list of tags into currentTagList */
    rv = readersGetTagList( &readerManager, readers, &currentTagList);
    if (rv != SCARD_S_SUCCESS)
    {
        sprintf( messageString, TAGEVENTOR_STRING_PCSCD_OK_READER_NOT,
                 readerManager.nbReaders );
        if ( callBack )
            (*callBack)( FALSE, messageString, "" );

        currentTagList.pTags = NULL;
        currentTagList.numTags = 0;
    }
    else
    {
        /* create a string with some status text and a list of the UIDs of the tags found */
        sprintf( messageString, TAGEVENTOR_STRING_CONNECTED_READER, readerManager.nbReaders, (int)(currentTagList.numTags));
        for ( i = 0; i < currentTagList.numTags; i++ )
        {
            sprintf( tagLine, TAGEVENTOR_STRING_TAG_LINE, currentTagList.pTags[i].uid);
            strcat( tagMessage, tagLine );
        }

        if ( callBack )
            (*callBack)( TRUE, messageString, tagMessage );

        /* for each tags that was here before see if it is now missing */
        for (i = 0; i < previousTagList.numTags; i++)
        {
            found = FALSE;
            /* Look for it in the new list */
            for (j = 0; (j < currentTagList.numTags); j++)
                 if ( strcmp(previousTagList.pTags[i].uid,
                      currentTagList.pTags[j].uid) == 0 )
                    found = TRUE;

            if (!found)
                rulesTableEventDispatch( TAG_OUT, previousTagList.pTags[i].uid, readers );
        }

        /* check for tags that are here now */
        for (i = 0; i < currentTagList.numTags; i++)
        {
            found = FALSE;
            /* Look for it in the old list */
            for (j = 0; ((j < previousTagList.numTags) && (!found)); j++)
                if ( strcmp(previousTagList.pTags[j].uid,
                            currentTagList.pTags[i].uid) == 0 )
                    found = TRUE;
            if (!found)
                rulesTableEventDispatch(TAG_IN, currentTagList.pTags[i].uid, readers );
        }
    } /* if readerGetTagList() was successful */

    /* keep calling me */
    return( TRUE );

}

static void
pollCallback(
             char        connected,
             const char  *generalMessage,
             const char  *tagsMessage
             )
{
    readersLogMessage( LOG_INFO, 2, generalMessage );
    readersLogMessage( LOG_INFO, 2, tagsMessage );
}

/************************ MAIN ******************************/
int main(
        int 		argc,
        char 		*argv[]
        )
{
    tRunOptions	runOptions;
    int          i;

    /* some help to make sure we close whatś needed, not more */
    for ( i = 0; i < MAX_NUM_READERS; i++ )
    {
        readers[i].hCard = NULL;
        readers[i].name = NULL;
        readers[i].pDriver = NULL;
        readers[i].driverDescriptor = NULL;
        readers[i].SAM = FALSE;
        readers[i].SAM_serial[0] = '\0';
        readers[i].SAM_id[0] = '\0';
    }

    parseCommandLine(argc, argv, &runOptions );

    /* set-up signal handlers */
    signal(SIGTERM,  handleSignal); /* software termination signal from kill */
    signal(SIGHUP,   handleSignal); /* hangup signal - ´restart´ as best as we can */
    signal(SIGINT,   handleSignal); /* Interrupt or Control-C signal from terminal*/
    signal(SIGCHLD,  handleSignal); /* death of a child process */

    /* set reader library options other than defaults (i.e. verbosity) from command line */
    /* here we are always foreground, not a daemon, it maybe recalled from the daemon */
    readersSetOptions( appVerbosityLevel, FALSE );

	/* load the table of rules */
    rulesTableRead();

    /* if requested to start as daemon, daemonize ourselves here */
    switch (runOptions)
    {
        case START_DAEMON:
            /* will fork a daemon and return if successful */
            daemonize();
            break;
        case STOP_DAEMON:
            /* find the running daemon, kill it and exit */
            stopDaemon();
            exit( 0 );
            break;
        case FOREGROUND:
            /* enter the loop to poll for tag and execute events, FALSE to not update system tray */
            /* Loop forever - doing our best - only way out is via a signal */
            while ( tagListCheck( pollCallback ) )
                usleep(appPollDelayms * 1000);
            break;
        case SYSTEM_TRAY:
#ifdef BUILD_SYSTEM_TRAY
            /* build the status icon in the system tray area */
            startSystemTray( &argc, &argv, &tagListCheck, appPollDelayms, readers );
#endif
            break;
        default:
            break;
    }

    readersDisconnect( &readerManager, readers );

    /* clean up the connection to PCSCD */
    readersManagerDisconnect( &readerManager );

   return ( 0 );

} /* main */
