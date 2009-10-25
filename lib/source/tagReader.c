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
   readersLogMessage( LOG_ERR, 0, messageString);\
}
#else
#define PCSC_ERROR(rv, text)
#endif


#define RESET_READER( read )    { \
                                (read).hCard = NULL; \
                                (read).name = NULL; \
                                (read).pDriver = NULL; \
                                (read).driverDescriptor = NULL; \
                                (read).SAM = FALSE; \
                                (read).SAM_serial[0] = '\0'; \
                                (read).SAM_id[0] = '\0'; \
}

/**********    STATIC VARIABLES TO THIS FILE     ***************/
static  int     readerSetting = 0; /* no readers, not even AUTO to start with */
static  int     libVerbosityLevel = 0;
static  BOOL    runningInBackground = FALSE;
static char		messageString[MAX_LOG_MESSAGE];

extern tReaderDriver    acr122UDriver;
/* This is a NULL terminated list of pointers to driver structures */
/* one for each driver we know about                               */
static tReaderDriver   *readerDriverTable[] = { &acr122UDriver, NULL };

/* utility function for other modules that returns the current setting for the reader number bitmap */
unsigned int
readersSettingBitmapGet(
                void
                )
{
    return ( readerSetting );
}

/* utility function for settings modules that sets the state of current setting for the reader number bitmap */
unsigned int
readersSettingBitmapSet(
                unsigned int             readerSettingBitmap
                )
{

    readerSetting = readerSettingBitmap;

    return ( readerSetting );

}

/* Utility function to set one specific reader number in the bitmap */
void
readersSettingBitmapBitSet( unsigned int bitmap )
{
    /* bit 0 of the bitmap is reserved for AUTO */
    /* so OR in the bit shifted an extra 1   number 0 = bit 1 etc */
    readerSetting |= bitmap;

}

/* Utility function to unset one specific reader number in the bitmap */
void
readersSettingBitmapBitUnset( unsigned int bitmap )
{
    /* bit 0 of the bitmap is reserved for AUTO */
    /* so OR in the bit shifted an extra 1   number 0 = bit 1 etc */
    readerSetting &= (~bitmap);

}

unsigned int
readersSettingBitmapNumberSet(
                            unsigned int     numberOfTheBitToSet
                            )
{
    /* bit 0 of the bitmap is reserved for AUTO */
    /* so test  number 0 = bit 1 etc */
    readerSetting |= (1 << ( numberOfTheBitToSet +1 ) );

    return (readerSetting);
}

/* Utility function to add one specific reader number to the bitmap */
unsigned int
readersSettingBitmapBitTest( unsigned int bitmap )
{
    /* bit 0 of the bitmap is reserved for AUTO */
    /* so test  number 0 = bit 0 etc */
    return( readerSetting & bitmap );

}

/* Utility function to add one specific reader number to the bitmap */
unsigned int
readersSettingBitmapNumberTest( unsigned int bitNumber )
{
    /* bit 0 of the bitmap is reserved for AUTO */
    /* so test  number 0 = bit 1 etc */
    return( readerSetting & ( 1 << ( bitNumber +1 )) );

}

/**************************** LOG MESSAGE************************/
/* type = LOG_WARNING (something unexpected, not a big problem) */
/* type = LOG_ERR (something went wrong that was´t expected     */
/* type = LOG_INFO (just info */
void
readersLogMessage(
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
         if ( libVerbosityLevel >= messageLevel )
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
int readersSetOptions (
		int		        verbosity,
		unsigned char   background
		 )

{

   /* set the verbosity level for all our output, if requested to */
   if ( verbosity != IGNORE_OPTION )
      libVerbosityLevel = verbosity;

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
    int     i, previousNumReaders;

    /* remember how many readers there used to be */
    previousNumReaders = pManager->nbReaders;

    /* Call with a null buffer to get the number of bytes to allocate */
    rv = SCardListReaders( (SCARDCONTEXT)(pManager->hContext), NULL, NULL, &dwReaders);
    if ( rv != SCARD_S_SUCCESS )
    {
        /* if there are no readers, then zero everything out but don't report an error */
        if ( rv == SCARD_E_NO_READERS_AVAILABLE )
        {
            pManager->nbReaders = 0;

            if ( pManager->mszReaders )
                free( pManager->mszReaders );
            pManager->mszReaders = NULL;

            readersLogMessage(LOG_INFO, 2, "Found 0 Readers, ");
        }
        else
            PCSC_ERROR(rv, "SCardListReaders");

        return ( rv );
    }

    /* if array already exists, then liberate it and alloc a new one for the */
    /* number of readers reported from SCardListReader */
    if ( pManager->mszReaders )
        free( pManager->mszReaders );
    /* malloc enough memory for dwReader string */
    pManager->mszReaders = malloc(sizeof(char)*dwReaders);

    /* now get the list into the mszReaders array */
    rv = SCardListReaders( (SCARDCONTEXT)(pManager->hContext), NULL, pManager->mszReaders, &dwReaders);
    if (rv != SCARD_S_SUCCESS)
    {
        /* Avoid reporting an error just because no reader is connected */
        if ( rv != SCARD_E_NO_READERS_AVAILABLE )
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
    readersLogMessage(LOG_INFO, 2, messageString);

    /* fill the array of readers with pointers to the appropriate point */
    /* in the long mszReaders multi-string */
    pManager->nbReaders = 0;
    ptr = pManager->mszReaders;
    while (*ptr != '\0')
    {
        sprintf(messageString, "Reader [%d]: %s", pManager->nbReaders, ptr);
        readersLogMessage(LOG_INFO, 3, messageString);
        pManager->readers[pManager->nbReaders].name = ptr;
        ptr += strlen(ptr)+1;
        (pManager->nbReaders)++;
    }

    /* if we have fewer readers than we used to then zero out the "lost ones" */
    if ( pManager->nbReaders < previousNumReaders )
       for ( i = pManager->nbReaders; i < previousNumReaders; i++ )
          RESET_READER( pManager->readers[i] );

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
readersManagerConnect(
                    tReaderManager *pManager
                    )
{
    LONG 		    rv;

    if ( pManager == NULL )
        return( SCARD_E_INVALID_PARAMETER );

    rv = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, (LPSCARDCONTEXT)&(pManager->hContext) );
    if (rv != SCARD_S_SUCCESS)
    {
        PCSC_ERROR(rv, "SCardEstablishContext");
        readersLogMessage( LOG_ERR, 1, LIBTAGREADER_STRING_PCSCD_NO );
        pManager->hContext = NULL;
        return( rv );
    }

    readersLogMessage( LOG_INFO, 2, LIBTAGREADER_STRING_PCSCD_OK );

    /* Find all the readers and fill the readerManager data structure */
    rv = readersEnumerate( pManager );

    return (rv);

}
/************************* READER MANAGER CONNECT **************/

/************************* READER MANAGER DISCONNECT ***********/
int
readersManagerDisconnect( tReaderManager *pManager )
{
    LONG 		    rv;

    if ( pManager == NULL )
        return( SCARD_E_INVALID_PARAMETER );

    if ( pManager->hContext )
    {
        readersLogMessage(LOG_INFO, 2, "Disconnecting from pcscd server");

        rv = SCardReleaseContext( (SCARDCONTEXT) (pManager->hContext) );
        if ( rv != SCARD_S_SUCCESS )
            PCSC_ERROR(rv, "SCardReleaseContext");

        /* libreate memory that was allocated when we connected to manager */
        if (pManager->mszReaders)
        {
            free(pManager->mszReaders);
            pManager->mszReaders = NULL;
        }
    }
    else
        rv = SCARD_E_INVALID_PARAMETER;

    /* now it should be NULL either way */
    pManager->hContext = NULL;

    return( rv );
}
/************************* READER MANAGER DISCONNECT ***********/


/************************* READER CONNECT **********************/
int readersConnect (
                    tReaderManager  *pManager
                 )
{
    LONG 		    rv;
    BOOL			    readerSupported;
    DWORD 		    dwActiveProtocol;
    int              i, num;
    BOOL             automatic;

    /* Duh. If you pass me a NULL pointer then I'm out of here */
    if ( pManager == NULL )
        return( SCARD_E_INVALID_PARAMETER );

    automatic = ( readersSettingBitmapBitTest( READER_BIT_AUTO ) != 0 );

    if ( automatic )
    {
        /* re-enumerate the readers in the system as it may have changed */
        rv = readersEnumerate( pManager );
        if ( rv != SCARD_S_SUCCESS )
            return( rv );
    }

    if ( pManager->nbReaders == 0 )
        return( SCARD_E_NO_READERS_AVAILABLE );

    /* try and connect to all readers that are present according to readerSetting */
    for ( num = 0; num < pManager->nbReaders; num++ )
    {
        /* if we are not already connected and should be trying then do so */
        if ( ( pManager->readers[num].hCard == NULL) &&
             ( automatic || readersSettingBitmapNumberTest( num ) ) )
        {
            dwActiveProtocol = -1;
            rv = SCardConnect( (SCARDCONTEXT)(pManager->hContext),
                                pManager->readers[num].name,
                                SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0 ,
                                (LPSCARDHANDLE) &(pManager->readers[num].hCard),
                                &dwActiveProtocol);
            if (rv == SCARD_S_SUCCESS)
            {
                sprintf(messageString, "Connected to reader: %d, %s", num, pManager->readers[num].name );
                readersLogMessage(LOG_INFO, 2, messageString);

                /* Query the drivers in our list until one of them can handle the reader */
                i = 0;
                readerSupported = FALSE;
                /* call the function to check if this driver works with this reader */
                while ( (readerDriverTable[i] != NULL) && (readerSupported == FALSE) )
                    rv = readerDriverTable[i++]->readerCheck( &(pManager->readers[num]), &readerSupported );

                /* we couldn't find a driver that knows how to handle this reader... */
                if ( ( readerSupported == FALSE) )
                {
                    pManager->readers[num].pDriver = NULL;
                    sprintf(messageString, "Reader (%s) not supported by any known driver", pManager->readers[num].name );
                    readersLogMessage(LOG_ERR, 0, messageString);
                    return (SCARD_E_UNKNOWN_READER);
                }
                else
                {
                    /* if we got this far then a driver was successfully found, remember it! */
                    pManager->readers[num].pDriver = readerDriverTable[i -1];
                    pManager->readers[num].driverDescriptor = readerDriverTable[i -1]->driverDescriptor;
                }
            }
            else
                RESET_READER( pManager->readers[num] );
        }
    }

    return ( SCARD_S_SUCCESS );

}
/************************* READER CONNECT **********************/

/************************ READER DISCONNECT ********************/
void
readersDisconnect(
                tReaderManager  *pManager
                   )
{
    LONG    rv;
    int     i;

    /* Duh. If you pass me a NULL pointer then I'm out of here */
    if ( ( pManager == NULL ) || ( pManager->hContext == NULL ) )
       return;

    if ( readersSettingBitmapBitTest( READER_BIT_AUTO ) )
    {
        /* re-enumerate the readers in the system as it may have changed */
        rv = readersEnumerate( pManager );
        if ( rv != SCARD_S_SUCCESS )
            return;
    }

    /* then check all readers we were ABLE to connect to */
    for ( i = 0; i < pManager->nbReaders; i++ )
    {
        if ( pManager->readers[i].hCard != NULL )
        {
            sprintf(messageString, "Disconnecting from reader %d", i );
            readersLogMessage(LOG_INFO, 2, messageString);

            rv = SCardDisconnect( (SCARDHANDLE) (pManager->readers[i].hCard), SCARD_UNPOWER_CARD);
            RESET_READER( pManager->readers[i] );

            if ( rv != SCARD_S_SUCCESS )
                PCSC_ERROR(rv, "SCardDisconnect");
        }
    }
}
/************************** READER DISCONNECT ********************/

/************************ GET CONTACTLESS STATUS *****************/
/* TODO : this function hasn't really been tested                */
int
readerGetContactlessStatus(
                            tReaderManager  *pManager,
                            tReader	        *pReader
                          )
{
    DWORD		dwRecvLength;
    BYTE		pbRecvBuffer[20];
    LONG		rv;
    int         i;
    BOOL        automatic;

    /* Duh. If you pass me a NULL pointer then I'm out of here */
    if ( ( pManager == NULL ) || ( pManager->hContext == NULL ) )
       return( SCARD_E_INVALID_PARAMETER );

    automatic = readersSettingBitmapBitTest( READER_BIT_AUTO );

    if ( automatic )
    {
        /* re-enumerate the readers in the system as it may have changed */
        rv = readersConnect( pManager );
        if ( rv != SCARD_S_SUCCESS )
            return( rv );
    }

    sprintf(messageString, "Requesting contactles status");
    readersLogMessage(LOG_INFO, 2, messageString);

    /* then check all readers we were ABLE to connect to */
    for ( i = 0; i < pManager->nbReaders; i++ )
    {
        /* check we have connected and have a driver for this reader */
        if ( ( pReader[i].hCard != NULL ) && ( pReader[i].pDriver != NULL ) &&
             ( automatic || readersSettingBitmapNumberTest( i ) ) )
        {
            dwRecvLength = sizeof(pbRecvBuffer);
            rv = ((tReaderDriver *)(pReader[i].pDriver))->getContactlessStatus(pReader, pbRecvBuffer, &dwRecvLength);

            if ( rv == SCARD_S_SUCCESS )
            {
                if (libVerbosityLevel)
                {
                    sprintf(messageString, "Reader %d Status: ", i);
                    sPrintBufferHex( (messageString + strlen("Reader %d Status: ") ), dwRecvLength, pbRecvBuffer);
                    readersLogMessage(LOG_INFO, 2, messageString);

                    sprintf(messageString, "Number of Tags = %d", pbRecvBuffer[4]);
                    readersLogMessage(LOG_INFO, 2, messageString);
                }
            }
        }
    }

   return (rv);
}
/************************ GET CONTACTLESS STATUS *****************/



/************************ GET TAG LIST ***************************/
/*** as the list of tags is of unknown and varying length this   */
/* function allocates the memory for the list you must handle it!*/
int
readersGetTagList(
                tReaderManager  *pManager
                )
{
    LONG		rv = SCARD_S_SUCCESS;
    tTag        *pTags[MAX_NUM_READERS];
    int         i, j;
    int         uniqueListIndex = 0;
    int         numTags[MAX_NUM_READERS];
    BOOL        automatic;

    /* reset everthing before we attempt to get the new list */
    pManager->tagList.pTags = NULL;
    pManager->tagList.numTags = 0;

    /* Duh. If you pass me a NULL pointer then I'm out of here */
    if ( ( pManager == NULL ) || ( pManager->hContext == NULL ) )
       return( SCARD_E_INVALID_PARAMETER );

    automatic = readersSettingBitmapBitTest( READER_BIT_AUTO );

    if ( automatic )
    {
        /* re-connect the readers in the system as it may have changed */
        rv = readersConnect( pManager );
        if ( rv != SCARD_S_SUCCESS )
            return( rv );
    }

    /* before we start, reset the count to 0 */
    pManager->tagList.numTags = 0;

    /* for all readers we were connected to PREVIOUSLY or are now after AUTMATIC reconnect s*/
    for ( i = 0; i < pManager->nbReaders; i++ )
    {
        /* make sure it's initialized, as depending on reader settings we may skip over */
        /* one of these pointers in the array and later try to free an invalid pointer */
        pTags[i] = NULL;
        numTags[i] = 0;

        /* check we are connected, have a driver and should be reading it */
        if ( ( pManager->readers[i].hCard != NULL ) && ( pManager->readers[i].pDriver != NULL ) &&
             ( automatic || readersSettingBitmapNumberTest( i ) ) )
        {
            /* allocate the structure for this reader to read tag list into upto max size */
            pTags[i] = (tTag *)malloc( ( ((tReaderDriver *)(pManager->readers[i].pDriver))->maxTags ) * sizeof(tTag) );

            /* call the reader's associated driver function to read the tag list */
            rv = ((tReaderDriver *)(pManager->readers[i].pDriver))->getTagList( &(pManager->readers[i]), &(numTags[i]), pTags[i] );
            if ( rv != SCARD_S_SUCCESS )
            {
                RESET_READER( pManager->readers[i] );
                numTags[i] = 0;
                if ( pTags[i] != NULL )
                    free( pTags[i] );
                pTags[i] = NULL;
            }
            /* accumulate the total number of tags found */
            pManager->tagList.numTags += numTags[i];
        }
    }

    /* mash them all up into one list */
    if ( pManager->tagList.numTags > 0 )
    {
        pManager->tagList.pTags = malloc( (pManager->tagList.numTags) * sizeof( tTag ) );

        /* copy all the individual lists across into the unique list */
        for( i = 0; i < pManager->nbReaders; i++)
        {
            for ( j = 0; j < numTags[i]; j++ )
                pManager->tagList.pTags[uniqueListIndex++] = (pTags[i])[j];

            /* free the individual list */
            if ( pTags[i] )
                free( pTags[i] );
        }
    }
    else
        pManager->tagList.pTags = NULL;

    sprintf( messageString, "Number of tags: %d", (int)pManager->tagList.numTags );
    readersLogMessage( LOG_INFO, 2, messageString );
    readersLogMessage( LOG_INFO, 2, messageString );

   return (rv);
}
/************************ GET TAG LIST ***************************/
