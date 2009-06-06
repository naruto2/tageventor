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

#include <tagReader.h>

/************************* CONSTANTS ************************/
#define RETRY_DELAY_SECONDS (10)
#define POLL_DELAY_MICRO_SECONDS (1000000)
#define MICROSLEEP_MAX (1000000)
#define DEFAULT_LOCK_FILE_DIR "/var/run/tagEventor"
#define DEFAULT_COMMAND_DIR "/etc/tagEventor"
#define DAEMON_NAME "tagEventord"
#define SCRIPT_DOES_NOT_EXIST (127)

/* Tag event types */
#define TAG_IN  (0)
#define TAG_OUT (1)

#define MAX_PATH (1024)

/*************************** MACROS ************************/
#define SWAP(first, second)\
   {\
   temp = first;\
   first = second;\
   second = temp;\
   }

#define PRINT_TAG_STATE(pTagList) \
        {\
        char	string[MAX_LOG_MESSAGE];\
        int tagIndex;\
        sprintf(string, "State: %d Tag(s)   ", (int)pTagList->numTags);\
        for (tagIndex = 0; tagIndex < pTagList->numTags; tagIndex++)\
           sprintf(string, " - %s", pTagList->tagUID[tagIndex]);\
        logMessage(LOG_INFO, 2, string);\
        }


/*************    TYPEDEFS TO THIS FILE     **********************/
typedef enum { FOREGROUND, START_DAEMON, STOP_DAEMON } tDaemonOptions;


/************* VARIABLES STATIC TO THIS FILE  ********************/

/* This is needed by handleSignal to clean-up, so can't be local :-( */
static  tReader         reader;
static  int		lockFile = -1;
static  char		lockFilename[MAX_PATH];
static  BOOL		runningAsDaemon = FALSE;
/* strings used for tag events, for text output and name of scipts */
static const char * const tagString[]  = { "IN", "OUT" };


/************************ PRINT USAGE ***********************/
static void printUsage( 
	const char 	*name
 			)
{

   fprintf(stderr, "Usage: %s <options>\n\t-n <reader number>   : default = 0\n\t-v <verbosity level> : default = 0 (silent), max = 3 \n\t-d start | stop : start or stop daemon, default = foreground\n\t-r <secs> : retry delay to connect to reader (seconds), default = %d\n\t-p <usecs> : tag polling delay (micro seconds), default = %d\n\t-h : print this message\n",
           name, RETRY_DELAY_SECONDS, POLL_DELAY_MICRO_SECONDS);

}

/************************ PRINT USAGE ***********************/



/************************ PARSE COMMAND LINE OPTIONS ********/
static void   parseCommandLine(
        int 		argc, 
	char 		*argv[],
	int		*pnumber,
	int		*pverbosityLevel, 
        tDaemonOptions	*pDaemonOptions,
	int		*pretryDelay,
        int             *ppollDelay
 			)
{

   BOOL		parseError = FALSE;
   int		option;

   /* Set default values */
   *pnumber = 0;
   *pverbosityLevel = 0;
   *pDaemonOptions = FOREGROUND;
   *pretryDelay = RETRY_DELAY_SECONDS;
   *ppollDelay = POLL_DELAY_MICRO_SECONDS;

   while ( ((option = getopt(argc, argv, "n:v:d:r:p:h")) != EOF) && (!parseError) )
      switch (option) 
      {
         case 'n':
            *pnumber = atoi(optarg);
            if (*pnumber  < 0)
            {
               *pnumber = 0;
               fprintf(stderr, "Reader number must be greater or equal to 0\n");
               fprintf(stderr, "Reader number has been forced to %d\n", *pnumber);
            }
            break;
         case 'v':
            *pverbosityLevel = atoi(optarg);
            if (*pverbosityLevel  < 0)
            {
               *pverbosityLevel = 0;
               fprintf(stderr, "Verbosity level must be greater or equal to 0\n");
               fprintf(stderr, "Verbosity level has been forced to %d\n", *pverbosityLevel);
            }
            break;
         case 'd':
            if (strcmp(optarg, "start") == 0)
               *pDaemonOptions = START_DAEMON;
            else if (strcmp(optarg, "stop") == 0)
                  *pDaemonOptions = STOP_DAEMON;
               else
               {
                  parseError = TRUE;
                  fprintf(stderr, "Invalid parameter for -d daemon option\n");
               }
            break;
         case 'r':
            *pretryDelay = atoi(optarg);
            if (*pretryDelay < 0)
            {
               *pretryDelay = RETRY_DELAY_SECONDS;
               fprintf(stderr, "Retry delay must be greater or equal to 0\n");
               fprintf(stderr, "Retry delay has been forced to %d\n", *pretryDelay);
            }
            break;
         case 'p':
            *ppollDelay = atoi(optarg);
            /* make sure not exceed limit of usleep */
            if (*ppollDelay > MICROSLEEP_MAX)
            {
               *ppollDelay = MICROSLEEP_MAX;
               fprintf(stderr, "Poll delay must be greater or equal to 0\n");
               fprintf(stderr, "Poll delay has been forced to %d\n", *ppollDelay);
            }

            if (*ppollDelay < 0)
            {
               *ppollDelay = POLL_DELAY_MICRO_SECONDS;
               fprintf(stderr, "Poll delay must be greater or equal to 0\n");
               fprintf(stderr, "Poll delay has been forced to %d\n", *ppollDelay);
            }
            break;

         case 'h':
            printUsage(argv[0]);
            exit ( 0 );
            break;

         default:
	    parseError = TRUE;
            break;
      }
 
   if (parseError)
   {
      printUsage(argv[0]);
      exit( EXIT_FAILURE );
   }

}
/************************ PARSE COMMAND LINE OPTIONS ********/



/***************** HANDLE SIGNALS ***************************/
static void handleSignal(
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
static void stopDaemon( 
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
static void getLockOrDie(
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
static void daemonize (
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

/*********************  EXEC SCRIPT **************************/
static int execScript(
			const char * scriptPath,
			const char * argv0,
			const char * SAM_serial,
			const char * tagUID,
			const char * eventTypeString
			)
{
   struct stat 	sts;
   int		pid;
   char		messageString[MAX_LOG_MESSAGE];
   int		ret;

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


/*********************  TAG EVENT ****************************/
static void tagEvent (
		int	  	eventType,
                uid       	tagUID,
                const tReader	*preader
              )
{
   char		filePath[MAX_PATH];
   char		currentDir[MAX_PATH];
   char		messageString[MAX_LOG_MESSAGE];

   sprintf(messageString, "Event: Tag %s - UID: %s", tagString[eventType], tagUID); 
   logMessage(LOG_INFO, 1, messageString);

   /* if we are running in foreground then check current directory - for testing/development */
   if (!runningAsDaemon)
   {
      /* first steps to build a full path */
      getcwd(currentDir, 100);
      sprintf(filePath, "%s/%s", currentDir, tagUID);

      /* if returned 0 then it worked (as far as we know, so exit here */
      if ( execScript(filePath, tagUID, preader->SAM_serial, tagUID, tagString[eventType]) == 0 )
         return;
   }

   /* This part tries a couple of options in the system directory DEFAULT_COMMAND_DIR */

   /* try a script of with the name of the tagUID in system directory */
   sprintf(filePath, "%s/%s", DEFAULT_COMMAND_DIR, tagUID);

   /* if that worked, then we´re done */
   if (execScript(filePath, tagUID, preader->SAM_serial, tagUID, tagString[eventType]) == 0 )
      return;

   /* then try a generic one before giving up */
   sprintf(filePath, "%s/generic", DEFAULT_COMMAND_DIR);

   /* if that worked, then we´re done */
   if (execScript(filePath, tagUID, preader->SAM_serial, tagUID, tagString[eventType]) == 0 )
      return;

   logMessage(LOG_ERR, 0, "Failed to execute a script for tag event" );
}
/*********************  TAG EVENT ****************************/


/************************ MAIN ******************************/
int main(
        int 		argc, 
	char 		*argv[],
 	char		*envp[]
        )
{
   tTagList		tagList1, tagList2;
   tTagList		*pnewTagList, *ppreviousTagList, *temp;
   LONG 		rv;
   int			i, j, retryDelaysec, pollDelayus;
   BOOL			change, found;
   tDaemonOptions	daemonOptions;
   int			verbosityLevel;
   char			messageString[MAX_LOG_MESSAGE];

   /* some help to make sure we close whatś needed, not more */
   reader.hContext = ((SCARDCONTEXT)NULL);
   reader.hCard = ((SCARDHANDLE)NULL);

   parseCommandLine(argc, argv, 
                    &(reader.number), &verbosityLevel, &daemonOptions, &retryDelaysec, &pollDelayus);

   /* set-up signal handlers */
   signal(SIGTERM,  handleSignal); /* software termination signal from kill */
   signal(SIGHUP,   handleSignal); /* hangup signal - ´restart´ as best as we can */
   signal(SIGINT,   handleSignal); /* Interrupt or Control-C signal from terminal*/
   signal(SIGCHLD,  handleSignal); /* death of a child process */

   /* set reader library options other than defaults (i.e. verbosity) from command line */
   /* here we are always foreground, not a daemon, it maybe recalled from the daemon */
   readerSetOptions( verbosityLevel, FALSE );

   /* if requested to start as daemon, daemonize ourselves here */
   switch (daemonOptions)
   {
      case START_DAEMON:
         /* will convert oursevles into a daemon and return if successful */
         daemonize(reader.number);
         break;
      case STOP_DAEMON:
         /* find the running daemon, kill it and exit */
         stopDaemon(reader.number);
         exit( 0 );
         break;
      default:
         break;
   }

i=0;
while (envp[i] != NULL)
{
snprintf(messageString,  (MAX_LOG_MESSAGE -1), "Env: %s", envp[i]);
logMessage(LOG_INFO, 3, messageString);
i++;
}

   while (TRUE)
   {
      pnewTagList = &tagList1;
      ppreviousTagList = &tagList2;

      /* connect to reader */
      rv = readerConnect(&reader);

      /* Loop trying to connect to the pcscd server and find the reader */
      while ( rv != SCARD_S_SUCCESS )
      {
         sprintf(messageString, "Will wait %d seconds and retry connection", retryDelaysec);
         logMessage(LOG_WARNING, 0, messageString);
         sleep(retryDelaysec);
         rv = readerConnect(&reader);
      }

      /* report initial state */
      rv = getTagList(&reader, ppreviousTagList);
      PRINT_TAG_STATE(ppreviousTagList);

      while ( rv == SCARD_S_SUCCESS )
      {
         /* get the list of tags on reader */
         rv = getTagList(&reader, pnewTagList);
         if (rv == SCARD_S_SUCCESS)
         {
            change = FALSE;

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
                  tagEvent(TAG_OUT, ppreviousTagList->tagUID[i], &reader);
                  change = TRUE;
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
                  tagEvent(TAG_IN, pnewTagList->tagUID[i], &reader);
                  change = TRUE;
               }
            }

            if (change)
            {
               PRINT_TAG_STATE(pnewTagList);
               SWAP(pnewTagList, ppreviousTagList);            
            }

            /* Wait between polls */
            usleep(pollDelayus);
         }
      }   
      /* if we got this far, we got an error. Disconnect and try reconnecting */
      /* and start from scratch */

      /* Ignore errors from disconnect as we're going to try and reconnect anyway */
      readerDisconnect(&reader);
   } /* Loop forever - doing our best - only way out is via a signal */
} /* main */
