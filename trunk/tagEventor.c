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
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>  /* for umask() */
#include <limits.h>

#include "tagReader.h"
#include "tagEventor.h"
#include "systemTray.h"

/************************* CONSTANTS ************************/

#define SCRIPT_DOES_NOT_EXIST (127)

/* Tag event types */
#define TAG_IN  (0)
#define TAG_OUT (1)

#define NUM_DEFAULT_COMMANDS   (3)

/*************************** MACROS ************************/
#define SWAP(first, second)\
   {\
   temp = first;\
   first = second;\
   second = temp;\
   }

#ifdef BUILD_SYSTEM_TRAY
#define USAGE_STRING "Usage: %s <options>\n\t-n <reader number>   : default = 0\n\t-v <verbosity level> : default = 0 (silent), max = 3 \n\t-d start | stop | tray (daemon options: default = foreground)\n\t-p <msecs> : tag polling delay, min = %d, default = %d, max = %d\n\t-h : print this message\n"
#else
#define USAGE_STRING "Usage: %s <options>\n\t-n <reader number>   : default = 0\n\t-v <verbosity level> : default = 0 (silent), max = 3 \n\t-d start | stop (daemon options: default = foreground)\n\t-p <msecs> : tag polling delay, min = %d, default = %d, max = %d\n\t-h : print this message\n"
#endif
/*************    TYPEDEFS TO THIS FILE     **********************/
typedef enum { FOREGROUND, START_DAEMON, STOP_DAEMON, SYSTEM_TRAY } tRunOptions;


/************* VARIABLES STATIC TO THIS FILE  ********************/

/* this is the list of built-in commands, in the reverse order of which they will be tried */
static    tPanelEntry defaultCommands[NUM_DEFAULT_COMMANDS] = {
   { "*", DEFAULT_COMMAND_DIR, GENERIC_MATCH,    "Match any tag, then run script named 'generic' in command dir",    TRUE },
   { "*", DEFAULT_COMMAND_DIR, TAG_ID_MATCH,     "Match any tag, then run script with name $tagID in command dir",   TRUE },
   { "*", "./scripts",         TAG_ID_MATCH,     "Match any tag, then run script with name $tagID in './scripts/'",  TRUE },
        };

/* This is needed by handleSignal to clean-up, so can't be local :-( */
static  tReader     reader;
static  int         readerSetting; /* not used just yet */
static  int		    lockFile = -1;
static  char		lockFilename[PATH_MAX];
static  BOOL		runningAsDaemon = FALSE;
static  int         numTagEntries = 0;
static  tPanelEntry *tagEntryArray;


/* strings used for tag events, for text output and name of scipts */
static const char * const tagString[]  = { "IN", "OUT" };
static int		    pollDelayms;
static int			verbosityLevel;

static tTagList		tagList1, tagList2;

/* make each pointer point to an allocated tag list that can be filled */
static tTagList		*pnewTagList = &tagList1, *ppreviousTagList = &tagList2;
static BOOL         connected = FALSE; /* at the start, we are not connected */
static BOOL         tagListChanged = FALSE;


/* utility function for other modules that returns the current setting for the reader number bitmap */
int
readerSettingGet(
                void
                )
{
    return ( readerSetting );
}

/* utility function for settings modules that sets the state of current setting for the reader number */
int
readerSettingSet(
                int             readerSettingBitmap
                )
{

    readerSetting = readerSettingBitmap;

    return ( readerSetting );

}

int pollDelaySet(
                int     newPollDelay
                )
{
    /* make sure not exceed limit */
    if ( newPollDelay > POLL_DELAY_MILLI_SECONDS_MAX )
        newPollDelay = POLL_DELAY_MILLI_SECONDS_MAX;

    if ( newPollDelay < POLL_DELAY_MILLI_SECONDS_MIN )
        newPollDelay = POLL_DELAY_MILLI_SECONDS_MIN;

    /* set the new delay to the capped value */
    pollDelayms = newPollDelay;

#ifdef BUILD_SYSTEM_TRAY
    systemTraySetPollDelay( pollDelayms );
#endif

    /* return the value that was actually set */
    return ( pollDelayms );

}

int
pollDelayGet( void )
{
    return( pollDelayms );
}

int verbosityLevelSet(
                    int     newLevel
                    )
{

    /* make sure not exceed limit */
    if ( newLevel > VERBOSITY_MAX )
        newLevel = VERBOSITY_MAX;

    if ( newLevel < VERBOSITY_MIN )
        newLevel = VERBOSITY_MIN;

    verbosityLevel = newLevel;

    /* return the value that was actually set */
    return ( verbosityLevel );

}

int
verbosityLevelGet( void )
{
    return( verbosityLevel );
}

/************************ PARSE COMMAND LINE OPTIONS ********/
static void
parseCommandLine(
                int 		    argc,
                char 		    *argv[],
                int		        *pnumber,
                tRunOptions     *pRunOptions
                )
{

   BOOL		parseError = FALSE;
   int		option, newDelay, newVerbosity;

   /* Set default values */
   *pnumber = 0;
   verbosityLevel = VERBOSITY_DEFAULT;
   *pRunOptions = FOREGROUND;
   pollDelayms = POLL_DELAY_MILLI_SECONDS_DEFAULT;

   while ( ((option = getopt(argc, argv, "n:v:d:p:h")) != EOF) && (!parseError) )
      switch (option)
      {
         /* 'n' option is for reader number to connect to */
         case 'n':
            *pnumber = atoi(optarg);
            if (*pnumber  < 0)
            {
               *pnumber = 0;
               fprintf(stderr, "Reader number must be greater or equal to 0\n");
               fprintf(stderr, "Reader number has been forced to %d\n", *pnumber);
            }
            break;

         /* 'v' option is for verbosity level */
         case 'v':
            newVerbosity = atoi(optarg);
            /* check if the new verbosity was within range when set */
            if ( verbosityLevelSet( newVerbosity ) != newVerbosity )
            {
               fprintf(stderr, "Verbosity level must be greater or equal to 0\n");
               fprintf(stderr, "Verbosity level has been forced to %d\n", verbosityLevelGet() );
            }
            break;

         case 'd':
            if (strcmp(optarg, "start") == 0)
               *pRunOptions = START_DAEMON;
            else if (strcmp(optarg, "stop") == 0)
                  *pRunOptions = STOP_DAEMON;
#ifdef BUILD_SYSTEM_TRAY
                 else if (strcmp(optarg, "tray") == 0)
                  *pRunOptions = SYSTEM_TRAY;
#endif
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
            if ( pollDelaySet ( newDelay ) != newDelay )
            {
               fprintf(stderr, "Poll delay must be greater or equal to 0\n");
               fprintf(stderr, "Poll delay has been forced to %d\n", pollDelayGet() );
            }
            break;

         /* 'h' option is to request print out help */
         case 'h':
                fprintf(stderr, USAGE_STRING, argv[0], POLL_DELAY_MILLI_SECONDS_MIN, POLL_DELAY_MILLI_SECONDS_DEFAULT, POLL_DELAY_MILLI_SECONDS_MAX);
            exit ( 0 );
            break;

         default:
	    parseError = TRUE;
            break;
      }

   if (parseError)
   {
      fprintf(stderr, USAGE_STRING, argv[0], POLL_DELAY_MILLI_SECONDS_MIN, POLL_DELAY_MILLI_SECONDS_DEFAULT, POLL_DELAY_MILLI_SECONDS_MAX);
      exit( EXIT_FAILURE );
   }

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
         readerDisconnect(&reader);
         readerConnect(&reader);
         logMessage(LOG_INFO, 1, "Hangup signal received - disconnected and reconnected");
      break;

      case SIGINT: /* kill -2 or Control-C */
      case SIGTERM:/* "kill -15" or "kill" */
         readerDisconnect(&reader);
         logMessage( LOG_INFO, 1, "SIGTERM or SIGINT received, exiting gracefully");
         if ( runningAsDaemon )
         {
            logMessage( LOG_INFO, 1, "Closing and removing lockfile, closing log. Bye.");
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
stopDaemon(
	       int		readerNumber
	      )
{
   char		messageString[MAX_LOG_MESSAGE];
   char 	pidString[20];
   int		pid;

   /* open lock file to get PID */
   sprintf(lockFilename, "%s/%s_%d.lock", DEFAULT_LOCK_FILE_DIR, DAEMON_NAME, readerNumber);
   lockFile = open( lockFilename, O_RDONLY, 0 );
   if (lockFile == -1)
   {
      sprintf(messageString, "Could not open lock file %s, exiting", lockFilename);
      logMessage(LOG_ERR, 0, messageString);
      sprintf(messageString, "Check you have the necessary permission for it");
      logMessage(LOG_ERR, 0, messageString);
      exit( EXIT_FAILURE );
   }

   /* get the running PID in the lockfile */
   if (read(lockFile, pidString, 19) == -1)
   {
      sprintf(messageString, "Could not read PID from lock file %s, exiting", lockFilename);
      logMessage(LOG_ERR, 0, messageString);
      exit( EXIT_FAILURE );
   }

   close( lockFile );

   if (sscanf( pidString, "%d\n", &pid ) != 1)
   {
      sprintf(messageString, "Could not read PID from lock file %s, exiting", lockFilename);
      logMessage(LOG_ERR, 0, messageString);
      exit( EXIT_FAILURE );
   }

   sprintf(messageString, "Stopping daemon with PID = %d on readerNumber %d", pid, readerNumber);
   logMessage(LOG_INFO, 1, messageString);

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
getLockOrDie(
            int		readerNumber
            )
{
   char 	pidString[20];
   char		messageString[MAX_LOG_MESSAGE];
   struct flock lock;

   sprintf(lockFilename, "%s/%s_%d.lock", DEFAULT_LOCK_FILE_DIR, DAEMON_NAME, readerNumber);
   lockFile = open( lockFilename, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH );
   if ( lockFile == -1 )
   {
      sprintf(messageString,
              "Could not open lock file %s, check permissions or run as root, exiting", lockFilename);
      logMessage(LOG_ERR, 0, messageString);
      exit( EXIT_FAILURE );
   }

   /* Initialize the flock structure. */
   memset (&lock, 0, sizeof(lock));
   lock.l_type = F_WRLCK;

   /* try and get the lock file, non-blocking */
   if ( fcntl(lockFile, F_SETLK, &lock) == -1 )
   {
      sprintf(messageString, "Could not lock file %s", lockFilename);
      logMessage(LOG_ERR, 0, messageString);
      sprintf(messageString, "Probably indicates a previous copy is still running or crashed");
      logMessage(LOG_ERR, 0, messageString);
      sprintf(messageString, "Find PID using \"cat %s\" or \"ps -ef | grep %s\". Exiting.",
             lockFilename, DAEMON_NAME);
      logMessage(LOG_ERR, 0, messageString);
      exit( EXIT_FAILURE );
   }

   /* store the running PID in the lockfile */
   sprintf( pidString, "%d\n", getpid());
   if (write(lockFile, pidString, strlen(pidString)) != strlen(pidString) )
   {
      sprintf(messageString, "Could not write PID to lock file %s", lockFilename);
      logMessage(LOG_ERR, 0, messageString);
      exit( EXIT_FAILURE );
   }
}
/********************** GET LOCK OR DIE *********************/


/************************ DAEMONIZE *************************/
static void
daemonize (
		int		readerNumber
		)
{
   int		pid;
   char		messageString[MAX_LOG_MESSAGE];

   /* fork a copy of myself */
   pid = fork();
   if ( pid < 0 )
   { /* fork error */
      sprintf(messageString, "Error forking daemon %s, exiting", DAEMON_NAME);
      logMessage(LOG_ERR, 0, messageString);
      exit( EXIT_FAILURE );
   }

   if ( pid > 0 )
   { /* fork worked, this is the parent process so exit */
      sprintf(messageString, "Started daemon %s with PID=%d on reader %d, see /var/log/syslog",
              DAEMON_NAME, pid, readerNumber);
      logMessage(LOG_INFO, 1, messageString);
      exit( EXIT_FAILURE );
   }

   /* from here on I must be a successfully forked child */
   runningAsDaemon = TRUE;

   /* tell the library we are now going to be calling it from a daemon */
   readerSetOptions( IGNORE_OPTION, runningAsDaemon );

   /* set umask for creating files */
   umask(0);

   /* start logging --> /var/log/syslog on my system */
   openlog(DAEMON_NAME, LOG_PID, LOG_DAEMON);

   /* get a new process group for daemon */
   if ( setsid() < 0 )
   {
      sprintf(messageString, "Error creating new SID for daemon process %s with PID=%d on reader %d, see in /var/log/syslog", DAEMON_NAME, pid, readerNumber);
      logMessage(LOG_ERR, 0, messageString);
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
   getLockOrDie(readerNumber);

   /* ignore TTY related signales */
   signal( SIGTSTP, SIG_IGN );
   signal( SIGTTOU, SIG_IGN );
   signal( SIGTTIN, SIG_IGN );

   sprintf(messageString, "Daemon Started on reader number %d", readerNumber );
   logMessage(LOG_INFO, 1, messageString);
}
/*********************  DAEMONIZE ****************************/


int
tagTableAddEntry( void )
{

    int             i, newSize;
    tPanelEntry     *newTagEntryArray;

    /* increment counter of number of entries in array */
    numTagEntries++;

    /* allocate memory for the array of tag entries found */
    newSize = (numTagEntries * sizeof(tPanelEntry) );
    newTagEntryArray = malloc( newSize );

/* TODO figure out where to save this type of config stuff???? via GConf or something */

    /* copy over all the existing tag entries in the array */
    for (i = 0; i < (numTagEntries -1); i++)
    {
        newTagEntryArray[i].IDRegex         = tagEntryArray[i].IDRegex;
        newTagEntryArray[i].folder          = tagEntryArray[i].folder;
        newTagEntryArray[i].scriptMatchType = tagEntryArray[i].scriptMatchType;
        newTagEntryArray[i].description     = tagEntryArray[i].description;
        newTagEntryArray[i].enabled         = tagEntryArray[i].enabled;
    }

    /* now we can safely free the memory for the previous one */
    free( tagEntryArray );

    /* now make the global variable for the entry array point to the new one */
    tagEntryArray = newTagEntryArray;

    /* malloc each string for new entry added and add to tagEntryArray*/
    tagEntryArray[numTagEntries -1].IDRegex = malloc( sizeof( uid ) );
    /* paste in some text for now, although it's not yet editable by the user */
    sprintf( tagEntryArray[numTagEntries -1].IDRegex, "Tag ID Match Regexp" );
    tagEntryArray[numTagEntries -1].folder = malloc( PATH_MAX );
    /* initialize */
    sprintf( tagEntryArray[numTagEntries -1].folder, DEFAULT_COMMAND_DIR );
    tagEntryArray[numTagEntries -1].description = malloc( MAX_DESCRIPTION_LENGTH );
    /* NULL terminate the emptry string */
    sprintf( tagEntryArray[numTagEntries -1].description, "A generic tagID match command" );
    tagEntryArray[numTagEntries -1].enabled = FALSE;

    tagEntryArray[numTagEntries -1].scriptMatchType = TAG_ID_MATCH;

    return( numTagEntries );
}


void
tagTableSave( void )
{

/* TODO where to save this type of config stuff???? via GConf or something */

}

void
tagTableEntryEnable( int index, char enable )

{

    tagEntryArray[index].enabled = enable;

}

const tPanelEntry *
tagEntryGet(
            int index
            )
{
    /* first check we have an array of entries at all !*/
    if ( tagEntryArray == NULL )
        return (NULL);
    else /* then check the index is within bounds */
    {
        if ( ( index >= 0) && ( index < numTagEntries ) )
            return( &(tagEntryArray[index]) );
        else
            return (NULL);
    }
}

int
tagTableNumber( void )
{
    return ( numTagEntries );
}

static int
tagTableRead( void )
{
    int size;
    int i;


    /* TODO need to figure out how many tags we have before allocating memory! */
    numTagEntries = NUM_DEFAULT_COMMANDS;

/* TODO we should also check for duplicate tag entries, or we could allow that. Need to change code that
   executes scripts to allow that though ... */
/* TODO read the current config from the widgets into a temporary table, checking syntax for each
            and maybe checking execute permissions, etc */

    /* allocate memory for the array of tag entries found */
    size = (numTagEntries * sizeof(tPanelEntry) );
    tagEntryArray = malloc( size ); /* TODO check about alignment and need to pad out */

/* TODO figure out where to read/save this type of config stuff???? via GConf or something */
    for (i = 0; i < numTagEntries; i++)
    {
        /* malloc each string for new entry added and add to tagEntryArray*/
        tagEntryArray[i].IDRegex = malloc( sizeof( uid ) );
        tagEntryArray[i].folder = malloc( PATH_MAX );
        tagEntryArray[i].description = malloc( MAX_DESCRIPTION_LENGTH );

	/* load the array up with the default commands */
        strcpy( tagEntryArray[i].IDRegex,      defaultCommands[i].IDRegex );
        strcpy( tagEntryArray[i].folder,       defaultCommands[i].folder);
        strcpy( tagEntryArray[i].description , defaultCommands[i].description);
        tagEntryArray[i].enabled =             defaultCommands[i].enabled;
        tagEntryArray[i].scriptMatchType =     defaultCommands[i].scriptMatchType;
    }

    return( numTagEntries );

}

/*********************  EXEC SCRIPT **************************/
static int
execScript(
	const char * folderName,   /* without a trailing '/' */
	const char * fileName,
	const char * argv0,
	const char * SAM_serial,
	const char * tagUID,
	const char * eventTypeString,
	const char * ruleDescription
	)
{
   struct stat 	sts;
   int		pid;
   char		messageString[MAX_LOG_MESSAGE];
   int		ret;
   char		scriptPath[PATH_MAX];

   /* build a full path */
   sprintf( scriptPath, "%s/%s", folderName, fileName );

   /* check if the file exists */
   if ((stat (scriptPath, &sts)) == -1)
      return(SCRIPT_DOES_NOT_EXIST);

   /* fork a copy of myself for executing the script in the child process using exec() */
   pid = fork();
   if ( pid < 0 )
   { /* PARENT process - fork error */
      sprintf(messageString, "Error forking for script execution, fork() returned %d", pid);
      logMessage(LOG_ERR, 0, messageString);
      return ( pid ); /* TODO , not sure returning that is correct...check later */
   }

   if ( pid > 0 ) /* PARENT process - fork worked */
   {
      sprintf(messageString, "Fork of child process successful with child pid=%d", pid);
      logMessage(LOG_INFO, 3, messageString);
      return( 0 );  /* success = 0 */
   }

   /* If we got this far, then "pid" = 0 and we are in the child process */
   sprintf(messageString, "Attempting to execl() tag event script %s in child process with pid=%d",
           scriptPath, getpid());
   logMessage(LOG_INFO, 2, messageString);
   ret = execl( scriptPath, argv0, SAM_serial, tagUID, eventTypeString, NULL );
   /* If any of the exec() functions returns, an error will have occurred. The return value is -1,
      and the global variable errno will be set to indicate the error. */
   sprintf(messageString, "Return value from execl() of script was = %d, errno=%d", ret, errno);
   logMessage(LOG_INFO, 2, messageString);

   /* exit the child process and return the return value, parent keeps on going */
   exit( ret );
}
/*********************  EXEC SCRIPT **************************/

/*********************  TAG EVENT DISPATCH **********************/
static void
tagEventDispatch(
                int	  	        eventType,
                uid       	    tagUID,
                const tReader	*preader
                )
{
    char	        messageString[MAX_LOG_MESSAGE];
    int             ruleIndex;
    char            scriptName[PATH_MAX];

    sprintf(messageString, "Event: Tag %s - UID: %s", tagString[eventType], tagUID);
    logMessage(LOG_INFO, 1, messageString);


    /* run through the list of rules:
           - try to match tags detected tag with regex in rule for tagID
             IF matches
                 - try to find and execute a script
        done in reverse order, ending with most generic rules */
    for (ruleIndex = (numTagEntries-1); ruleIndex >= 0; ruleIndex--)
    {
        if ( tagEntryArray[ruleIndex].enabled )
        {
#if 0
/* TODO need to figure out how to do the regular expression matching */
            tagEntryArray[ruleIndex].IDRegex
#endif

            switch ( tagEntryArray[ruleIndex].scriptMatchType )
            {
                case TAG_ID_MATCH:
                    strcpy( scriptName, tagUID );
                break;
                case GENERIC_MATCH:
                    sprintf( scriptName, "generic" );
                break;
                case SAM_ID_MATCH:
                    strcpy( scriptName, preader->SAM_id );
                break;
                case SAM_SERIAL_MATCH:
                    strcpy( scriptName, preader->SAM_serial );
                break;
                case READER_NUM_MATCH:
                    sprintf( scriptName, "%d", preader->number );
                break;
                default:
                    logMessage(LOG_ERR, 0, "Invalid 'scriptMatchType' no script execution attempted" );
                    return;
                break;
            }

            if (execScript( tagEntryArray[ruleIndex].folder, scriptName, tagUID, preader->SAM_serial, tagUID, tagString[eventType],
                            tagEntryArray[ruleIndex].description) == 0)
                return;
        }
    }

   /* if we got this far, then nothing worked */
   logMessage(LOG_ERR, 0, "Failed to execute a script for tag event" );
}
/*********************  TAG EVENT DISPATCH ****************************/

static int
tagListCheck( void *updateSystemTray )
{
    tTagList        *temp;
    LONG 		    rv;
    BOOL			found;
    uid             ID;
    int			    i, j;
    char			messageString[MAX_LOG_MESSAGE];

    if ( connected == FALSE )
    {
        if ( readerConnect(&reader) == SCARD_S_SUCCESS )
        {
            connected = TRUE;

            /* get initial state to prime the pump for the later comparison */
            getTagList(&reader, ppreviousTagList);
        }
        else
        {
            sprintf( messageString, "Could not connect to tag reader." );
        }
    } /* if ( connected == FALSE ) */

    /* we may have just connected, or been connected from a previous call */
    if ( connected == TRUE )
    {
        /* get the list of tags on reader */
        rv = getTagList(&reader, pnewTagList);

        if (rv != SCARD_S_SUCCESS)
        {
            /* reading tags is not working */
            connected = FALSE;

            sprintf( messageString, "Connected to reader, problems reading.");

            /* disconnect and let it re-connect next time around */
            readerDisconnect( &reader );
        }
        else
        {
            sprintf( messageString, "Connected to reader %d, %d tags:", reader.number, (int)(pnewTagList->numTags) );
            for (i=0; i < pnewTagList->numTags; i++)
            {
                sprintf( ID, " %s", pnewTagList->tagUID[i]);
                strcat( messageString, ID );
            }

            /* for each tags that was here before */
            for (i = 0; i < ppreviousTagList->numTags; i++)
            {
                found = FALSE;
                /* Look for it in the new list */
                for (j = 0; (j < pnewTagList->numTags); j++)
                    if ( strcmp(ppreviousTagList->tagUID[i],
                          pnewTagList->tagUID[j]) == 0 )
                        found = TRUE;

                if (!found)
                {
                    tagListChanged = TRUE;
                    tagEventDispatch(TAG_OUT, ppreviousTagList->tagUID[i], &reader);
                }
            }

            /* check for tags that are here now */
            for (i = 0; i < pnewTagList->numTags; i++)
            {
                found = FALSE;
                /* Look for it in the old list */
                for (j = 0; ((j < ppreviousTagList->numTags) && (!found)); j++)
                    if ( strcmp(ppreviousTagList->tagUID[j],
                          pnewTagList->tagUID[i]) == 0 )
                        found = TRUE;
                if (!found)
                {
                    tagListChanged = TRUE;
                    tagEventDispatch(TAG_IN, pnewTagList->tagUID[i], &reader);
                }
            }

            /* the next time around the loop the previous state should be the current one */
            /* and make the new one point to a different list so it can be overwritten */
            SWAP(pnewTagList, ppreviousTagList);
            tagListChanged = FALSE;
        } /* if getTagList() was successful */
    } /* if ( connected == TRUE ) */

#ifdef BUILD_SYSTEM_TRAY
    if ( updateSystemTray )
        systemTraySetStatus( connected, messageString );
#endif

    if ( connected )
        logMessage(LOG_INFO, 2, messageString);
    else
        logMessage(LOG_WARNING, 0, messageString);

    /* keep calling me */
    return( TRUE );
}

/************************ MAIN ******************************/
int main(
        int 		argc,
        char 		*argv[]
        )
{
   tRunOptions	runOptions;

   /* some help to make sure we close whatś needed, not more */
   reader.hContext = ((SCARDCONTEXT)NULL);
   reader.hCard = ((SCARDHANDLE)NULL);
   reader.number = 0;

   parseCommandLine(argc, argv, &(reader.number), &runOptions );

   /* set-up signal handlers */
   signal(SIGTERM,  handleSignal); /* software termination signal from kill */
   signal(SIGHUP,   handleSignal); /* hangup signal - ´restart´ as best as we can */
   signal(SIGINT,   handleSignal); /* Interrupt or Control-C signal from terminal*/
   signal(SIGCHLD,  handleSignal); /* death of a child process */

   /* set reader library options other than defaults (i.e. verbosity) from command line */
   /* here we are always foreground, not a daemon, it maybe recalled from the daemon */
   readerSetOptions( verbosityLevel, FALSE );

	/* load the table of rules */
    tagTableRead();

   /* if requested to start as daemon, daemonize ourselves here */
   switch (runOptions)
   {
      case START_DAEMON:
         /* will fork a daemon and return if successful */
         daemonize(reader.number);
         break;
      case STOP_DAEMON:
         /* find the running daemon, kill it and exit */
         stopDaemon(reader.number);
         exit( 0 );
         break;
      case FOREGROUND:
         /* enter the loop to poll for tag and execute events, FALSE to not update system tray */
         /* Loop forever - doing our best - only way out is via a signal */
         while ( tagListCheck( FALSE  ) )
             usleep(pollDelayms * 1000);
         break;
      case SYSTEM_TRAY:
#ifdef BUILD_SYSTEM_TRAY
         /* build the status icon in the system tray area */
         startSystemTray( &argc, &argv, &tagListCheck, pollDelayms );
#endif
         break;
      default:
         break;
   }

   /* be a good citizen and clean-up on our way out System Tray version might get this far */
   /* the other versions will all exit via a signal handler elsewhere */
   readerDisconnect(&reader);

   return ( 0 );

} /* main */
