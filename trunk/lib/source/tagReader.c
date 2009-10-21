/*
  tagReader.c - C source code for tagReader library

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
#include <string.h>
#include <unistd.h>
#include <stdarg.h>

#include <PCSC/wintypes.h>
#include <PCSC/winscard.h>

#include "tagReader.h"
#include "tagSpecs.h"
#include "readerDriver.h"


/*******************   MACROS ************************************/
#define sPrintBufferHex(string, num, array)\
        {\
        LONG j;\
	for (j=0; j<num; j++)\
	   sprintf((string + (j*2)), "%02X", array[j]);\
        string[num*2] = '\0';\
        }

#ifdef DEBUG
#define PCSC_ERROR(rv, text) \
if (rv != SCARD_S_SUCCESS) \
{ \
   sprintf(messageString, "%s: %s (0x%lX)", text, pcsc_stringify_error(rv), rv); \
   readerLogMessage( LOG_ERR, 0, messageString);\
}
#else
#define PCSC_ERROR(rv, text)
#endif

/**********    STATIC VARIABLES TO THIS FILE     ***************/
static  int     verbosityLevel = 0;
static  BOOL    runningInBackground = FALSE;
static char		messageString[MAX_LOG_MESSAGE];

extern tReaderDriver    acr122UDriver;

/* This is a NULL terminated list of pointers to driver structures */
/* one for each driver we know about                               */
tReaderDriver   *readerDriverTable[] = { &acr122UDriver, NULL };

/**************************** LOG MESSAGE************************/
/* type = LOG_WARNING (something unexpected, not a big problem) */
/* type = LOG_ERR (something went wrong that was´t expected     */
/* type = LOG_INFO (just info */
void
readerLogMessage(
                 int		    type,
                 int		    messageLevel,
                 const char 	*message
                    )
{

   switch (type)
   {
      case LOG_WARNING:
         if (runningInBackground)
            syslog( type , "WARNING: %s", message );
         else
            fprintf(stderr, "WARNING: %s\n", message);
      break;

      case LOG_ERR:
         if (runningInBackground)
            syslog( type , "ERROR: %s", message );
         else
            fprintf(stderr, "ERROR: %s\n", message);
      break;

      case LOG_INFO:
         if ( verbosityLevel >= messageLevel )
         {
            if (runningInBackground)
               syslog( type, "INFO [%d]: %s", messageLevel, message);
            else
               fprintf( stdout,  "INFO [%d]: %s\n", messageLevel, message);
         }
         break;
   }

}
/**************************** LOG ************************/


/**************************** READER SET OPTIONS *********/
int readerSetOptions (
		int		        verbosity,
		unsigned char   background
		 )

{

   /* set the verbosity level for all our output, if requested to */
   if ( verbosity != IGNORE_OPTION )
      verbosityLevel = verbosity;

   /* set flag for running in foreground, or background as daemon */
   runningInBackground = background;

   return ( SCARD_S_SUCCESS );

}
/**************************** READER SET OPTIONS *********/

/*************************** READERS ENUMERATE *****************/
static int
readersEnumerate(
                tReaderManager  *pManager
                )
{

    LONG    rv;
    DWORD   dwReaders;
    char 	*ptr;

    /* Call with a null buffer to get the number of bytes to allocate */
    rv = SCardListReaders( (SCARDCONTEXT)(pManager->hContext), NULL, NULL, &dwReaders);
    if (rv != SCARD_S_SUCCESS)
    {
        PCSC_ERROR(rv, "SCardListReaders");
        return ( rv );
    }

    /* if array already exists, then free it and alloc a new one for the */
    /* number of readers reported from SCardListReader */
    if ( pManager->mszReaders )
        free( pManager->mszReaders );
    /* malloc enough memory for dwReader string */
    pManager->mszReaders = malloc(sizeof(char)*dwReaders);

    /* now get the list into the mszReaders array */
    rv = SCardListReaders( (SCARDCONTEXT)(pManager->hContext), NULL, pManager->mszReaders, &dwReaders);
    if (rv != SCARD_S_SUCCESS)
    {
        PCSC_ERROR(rv, "SCardListReaders");
        return (rv);
    }

    /* Extract readers from the null separated string and get the total
        * number of readers */
    pManager->nbReaders = 0;
    ptr = pManager->mszReaders;
    while (*ptr != '\0')
    {
        ptr += strlen(ptr)+1;
        (pManager->nbReaders)++;
    }

    sprintf(messageString, "Found %d Readers, ", pManager->nbReaders);
    readerLogMessage(LOG_INFO, 2, messageString);

    /* if there was already a readers table, free it */
    if ( pManager->readers )
        free( pManager->readers );

    /* allocate the readers table, a set of pointers to the name of each */
    pManager->readers = calloc( pManager->nbReaders, sizeof(char *) );
    if ( pManager->readers == NULL )
    {
        readerLogMessage(LOG_ERR, 0, "Not enough memory for readers[]");
        return(SCARD_F_INTERNAL_ERROR);
    }

    /* fill the readers table */
    pManager->nbReaders = 0;
    ptr = pManager->mszReaders;
    while (*ptr != '\0')
    {
        sprintf(messageString, "Reader [%d]: %s", pManager->nbReaders, ptr);
        readerLogMessage(LOG_INFO, 3, messageString);
        pManager->readers[pManager->nbReaders] = ptr;
        ptr += strlen(ptr)+1;
        (pManager->nbReaders)++;
    }

    return( SCARD_S_SUCCESS );

}
/*************************** READERS ENUMERATE *****************/


/************************* READER MANAGER CONNECT **************/
/* This is added to make things a bit more efficient when you  */
/* are using multiple readers connected to one reader manager  */
/* I have left the readerConnect() function as is for back-    */
/* -wards compatibility. If you use this function then set the */
/* hCOntext element of each tReader structure to this value and*/
/* then they will all use the same context and avoid overhead  */
int
readerManagerConnect( tReaderManager *pManager )
{
    LONG 		    rv;

    rv = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, (LPSCARDCONTEXT)&(pManager->hContext) );
    if (rv != SCARD_S_SUCCESS)
    {
        PCSC_ERROR(rv, "SCardEstablishContext");
        readerLogMessage( LOG_ERR, 1, TAGREADER_STRING_PCSCD_NO );
        pManager->hContext = NULL;
        return( rv );
    }

    readerLogMessage( LOG_INFO, 2, TAGREADER_STRING_PCSCD_OK );

    rv = readersEnumerate( pManager );

    return (rv);

}
/************************* READER MANAGER CONNECT **************/

/************************* READER MANAGER DISCONNECT ***********/
int
readerManagerDisconnect( tReaderManager *pManager )
{
    LONG 		    rv;

    if ( pManager->hContext );
    {
        readerLogMessage(LOG_INFO, 2, "Disconnecting from pcscd server");

        rv = SCardReleaseContext( (SCARDCONTEXT) (pManager->hContext) );
        if ( rv != SCARD_S_SUCCESS )
            PCSC_ERROR(rv, "SCardReleaseContext");

/* TODO, make these memory pointers part of the hCOntext structure associated with manager */

        /* Free up memory that was allocated when we connected to manager */
        if (pManager->mszReaders)
        {
            free(pManager->mszReaders);
            pManager->mszReaders = NULL;
        }

        /* free allocated memory */
        if (pManager->readers)
        {
            free(pManager->readers);
            pManager->readers = NULL;
        }
    }

    /* now it should be NULL either way */
    pManager->hContext = NULL;

    return( rv );
}
/************************* READER MANAGER DISCONNECT ***********/


/************************* READER CONNECT **********************/
int readerConnect (
                    tReaderManager  *pManager,
                    tReader	        *pReader
                 )
{
   LONG 		    rv;
   BOOL			    readerSupported = FALSE;
   DWORD 		    dwActiveProtocol;
   int              i;

   /* if not already connected to daemon that manager readers */
   if ( pManager->hContext == NULL )
   { /* then try and connect to the pcscd server */
       rv = readerManagerConnect( pManager );

       if ( rv != SCARD_S_SUCCESS )
        return ( rv );
   }
   else
   {
       /* re-enumerate the readers in the system as it may have changed */
       rv = readersEnumerate( pManager );
       if ( rv != SCARD_S_SUCCESS )
          return( rv );
   }

   if ( pManager->nbReaders == 0 )
   {
        readerLogMessage(LOG_ERR, 0, "No readers were found" );
        return( SCARD_E_UNKNOWN_READER );
   }

   /* connect to the specified card reader */
   if (pReader->number < 0 || pReader->number >= pManager->nbReaders)
   {
     sprintf(messageString, "Reader number '%d' out of range, only %d readers detected",
             pReader->number, pManager->nbReaders);
     readerLogMessage(LOG_ERR, 0, messageString);
     return(SCARD_E_READER_UNAVAILABLE);
   }

   /* set strings to empty until we find out otherwise */
   strcpy(pReader->SAM_serial, "NoSAM");
   strcpy(pReader->SAM_id, "NoSAM");

   dwActiveProtocol = -1;
   rv = SCardConnect( (SCARDCONTEXT)(pManager->hContext),
                      pManager->readers[pReader->number],
                      SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0 ,
                      (LPSCARDHANDLE) &(pReader->hCard),
                      &dwActiveProtocol);
   if (rv != SCARD_S_SUCCESS)
      return (rv);

   /* make the tReader structure also have an entry of it's own name */
   pReader->name = pManager->readers[pReader->number];

   sprintf(messageString, "Connected to reader: %d, %s", pReader->number, pReader->name );
   readerLogMessage(LOG_INFO, 2, messageString);

   /* Query the drivers in our list until one of them can handle the reader */
   rv = SCARD_S_SUCCESS;
   i = 0;
   while ( (readerDriverTable[i] != NULL) && (readerSupported == FALSE) )
   {
      /* call the function to check if this driver works with this reader */
      rv = readerDriverTable[i]->readerCheck( pReader, &readerSupported );
      i++;
   }

   /* we couldn't find a driver that knows how to handle this reader... */
   if ( !readerSupported )
   {
        pReader->pDriver = NULL;
        sprintf(messageString, "Reader (%s) not supported by any known driver", pReader->name );
        readerLogMessage(LOG_ERR, 0, messageString);
        return (SCARD_E_UNKNOWN_READER);
   }
   else
   {
       /* if we got this far then a driver was successfully found, remember it! */
       pReader->pDriver = readerDriverTable[i -1];
   }

   return (rv);

}
/************************* READER CONNECT **********************/


/************************ READER DISCONNECT ********************/
void readerDisconnect(
                   tReader	*pReader
                   )
{
    LONG rv;

    if (pReader->hCard)
    {
        sprintf(messageString, "Disconnecting from reader %d", pReader->number);
        readerLogMessage(LOG_INFO, 2, messageString);

        rv = SCardDisconnect( (SCARDHANDLE) (pReader->hCard), SCARD_UNPOWER_CARD);
        pReader->hCard = NULL;
        PCSC_ERROR(rv, "SCardDisconnect");
    }
    else
    {
        sprintf(messageString, "Not currently connected to reader %d, cannot disconnect", pReader->number);
        readerLogMessage(LOG_INFO, 2, messageString);
    }
}
/************************** READER DISCONNECT ********************/


/************************ GET CONTACTLESS STATUS *****************/
/* TODO : this function hasn't really been tested                */
int readerGetContactlessStatus(
                                const tReader	*pReader
                                )
{
   DWORD		dwRecvLength;
   BYTE			pbRecvBuffer[20];
   LONG			rv;

   /* check we have connected and have a driver for this reader */
   if ( pReader->pDriver == NULL )
      return ( SCARD_E_UNKNOWN_READER );

   sprintf(messageString, "Requesting status");
   readerLogMessage(LOG_INFO, 2, messageString);

   dwRecvLength = sizeof(pbRecvBuffer);
   rv = ((tReaderDriver *)(pReader->pDriver))->getContactlessStatus(pReader, pbRecvBuffer, &dwRecvLength);

   if ( rv == SCARD_S_SUCCESS )
   {
      if (verbosityLevel)
      {
         sprintf(messageString, "Status: ");
         sPrintBufferHex(messageString, dwRecvLength, pbRecvBuffer);
         readerLogMessage(LOG_INFO, 2, messageString);

         sprintf(messageString, "Number of Tags = %d", pbRecvBuffer[4]);
         readerLogMessage(LOG_INFO, 2, messageString);
      }
   }

   return (rv);
}
/************************ GET CONTACTLESS STATUS *****************/


/************************ GET TAG LIST ***************************/
int readerGetTagList(
                    const tReader	*pReader,
                    tTagList	    *pTagList
               )
{
   LONG			rv;

   /* check we have connected and have a driver for this reader */
   if ( pReader->pDriver == NULL )
      return ( SCARD_E_UNKNOWN_READER );

   rv = ((tReaderDriver *)(pReader->pDriver))->getTagList( pReader, pTagList );

   sprintf(messageString, "Number of tags: %d", (int)pTagList->numTags);
   readerLogMessage(LOG_INFO, 2, messageString);

   return (rv);

}
/************************ GET TAG LIST ***************************/
