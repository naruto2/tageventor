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
#include <syslog.h>

#include "tagReader.h"


/* COMMAND : [Class, Ins, P1, P2, DATA, LEN] */
/* Direct to card commands start with 'd4', must prepend direct transmit to them*/
#define ACS_14443_A {0xd4,  0x40,  0x01}

#define ACS_IN_LIST_PASSIVE_TARGET  {0xd4, 0x4a}
#define ACS_MIFARE_LOGIN  {0xd4, 0x40, 0x01}
#define ACS_POWER_OFF  {0xd4, 0x32, 0x01, 0x00}
#define ACS_POWER_ON  {0xd4, 0x32, 0x01, 0x01}
#define ACS_RATS_14443_4_OFF  {0xd4, 0x12, 0x24}
#define ACS_RATS_14443_4_ON  {0xd4, 0x12, 0x34}
#define ACS_SET_PARAMETERS  {0xd4, 0x12}


/***************************** LED CONTROL *********************/
#define ACS_LED_GREEN     {0xff, 0x00, 0x40, 0x0e, 0x04, 0x00, 0x00, 0x00, 0x00}
#define ACS_LED_ORANGE    {0xff, 0x00, 0x40, 0x0f, 0x04, 0x00, 0x00, 0x00, 0x00}
#define ACS_LED_RED       {0xff, 0x00, 0x40, 0x0d, 0x04, 0x00, 0x00, 0x00, 0x00}

/****************************** APDU ***************************/
#define ACS_DIRECT_TRANSMIT {0xff, 0x00, 0x00, 0x00}
#define ACS_GET_READER_FIRMWARE  {0xff, 0x00, 0x48, 0x00, 0x00}

#define ACS_APDU_GET_SAM_SERIAL  {0x80, 0x14, 0x00, 0x00, 0x08}
#define ACS_APDU_GET_SAM_ID  {0x80, 0x14, 0x04, 0x00, 0x06}


/************************** PSUEDO APDU ************************/
#define ACS_GET_STATUS  { 0xd4, 0x04 }
#define ACS_POLL_MIFARE { 0xd4, 0x4a, 0x01, 0x00 }

/* This will avoid blocking on a Poll*/
#define ACS_SET_RETRY  {0xd4, 0x32, 0x05, 0x00, 0x00, 0x00}

/* TODO do a read block function */
/* READ Block 'n' = read (d4,40,1,30) + 'n=0x04' */
#define ACS_READ_MIFARE { 0xd4, 0x40, 0x01, 0x30, 0x04 }

/* GetResponse  =   0xFF 0xC0 0x00 0x00 NumbBytesRetrieve      */
#define ACS_GET_RESPONSE  {0xff, 0xc0, 0x00, 0x00}

#define ACS_TAG_FOUND {0xD5, 0x4B}
#define ACS_DATA_OK {0xD5, 0x41}
#define ACS_NO_SAM {0x3B, 0x00}

/* Command response bytes - array indexes */
#define SW1 0
#define SW2 1

/* SW1 return codes */
#define SW1_SUCCESS 0x61
#define SW1_FAIL    0x63

/* Error codes from Get Status APDU */
#define GET_STATUS_NO_ERROR 0x00

/**********************    CONSTANT ****************************/
/* See API_ACR122.pdf from ACS */
/*                  CLS  INS  P1   P2   Lc             Data In */
/* DirectTransmit = 0xFF 0x00 0x00 0x00 NumbBytesSend          */
static const BYTE APDU_DIRECT_TRANSMIT[] = ACS_DIRECT_TRANSMIT;

/***** GET STATUS  */
static const BYTE APDU_GET_STATUS[] = ACS_GET_STATUS;
/* Get the current setting of the contactless interface
   Step 1) Get Status Command
   << FF 00 00 00 02 D4 04
   >> 61 0C
   << FF C0 00 00 0C
   >> D5 05 [Err] [Field] [NbTg] [Tg] [BrRx] [BrTx] [Type] 80 90 00
*/

static const BYTE APDU_POLL_MIFARE[] = ACS_POLL_MIFARE;
static const BYTE APDU_READ_MIFARE[] = ACS_READ_MIFARE;
static const BYTE APDU_GET_RESPONSE[] =  ACS_GET_RESPONSE;
static const BYTE APDU_GET_SAM_SERIAL[] = ACS_APDU_GET_SAM_SERIAL;
static const BYTE APDU_GET_SAM_ID[] = ACS_APDU_GET_SAM_ID;
static const BYTE APDU_SET_RETRY[] = ACS_SET_RETRY;

static const BYTE APDU_LED_GREEN[] = ACS_LED_GREEN;
static const BYTE APDU_LED_ORANGE[] = ACS_LED_ORANGE;
static const BYTE APDU_LED_RED[] = ACS_LED_RED;

static const BYTE APDU_GET_READER_FIRMWARE[] = ACS_GET_READER_FIRMWARE;

/* This is the array of firmware string of supported readers */
static const char * const SUPPORTED_READER_ARRAY[] = { "ACR122U" };

/* for the PCSC subtype ACS ACR38U use the T0 protocol - from RFIDiot */
static const SCARD_IO_REQUEST 	*pioSendPci = SCARD_PCI_T0;

static         char		messageString[MAX_LOG_MESSAGE];

/*******************   MACROS ************************************/
#define sPrintBufferHex(string, num, array)\
        {\
        LONG j;\
	for (j=0; j<num; j++)\
	   sprintf((string + (j*2)), "%02X", array[j]);\
        string[num*2] = '\0';\
        }

#define PCSC_ERROR(rv, text) \
if (rv != SCARD_S_SUCCESS) \
{ \
   sprintf(messageString, "%s: %s (0x%lX)", text, pcsc_stringify_error(rv), rv); \
   logMessage( LOG_ERR, 0, messageString);\
}\


/**********    STATIC VARIABLES TO THIS FILE     ***************/
static  char 		pbReader[MAX_READERNAME] = "";
static  LPSTR 		mszReaders = NULL;
static  char 		**readers = NULL;
static  int		verbosityLevel = 0;
static  BOOL            runningInBackground = FALSE;

/**************************** LOG MESSAGE************************/
/* type = LOG_WARNING (something unexpected, not a big problem) */
/* type = LOG_ERR (something went wrong that was´t expected     */
/* type = LOG_INFO (just info */
void logMessage( int		type, 
                 int		messageLevel,
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
LONG readerSetOptions ( 
		int		verbosity,
		BOOL		background
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


/**************************** APDU SEND ************************/
static LONG apduSend (
                SCARDHANDLE   		hCard, 
		const BYTE    	        *apdu,
		DWORD			apduLength,
		BYTE			*pbRecvBuffer,
 		DWORD			*dwRecvLength
		)
{
	LONG 			rv;
	SCARD_IO_REQUEST 	pioRecvPci;
	BYTE 			pbSendBuffer[20];
	DWORD 			dwSendLength = 0;
	DWORD			rBufferMax;
        BOOL			psuedoAPDU = FALSE;

        /* The special psuedo APDU's to talk to a tag, need to have their */
        /* response read back in two chunks, using GET_RESPONSE for the second */
        if (apdu[0] == 0xd4)
        {
            psuedoAPDU = TRUE;
           /* prepend the DIRECT_TRANSMIT APDU  */
           memcpy(pbSendBuffer, APDU_DIRECT_TRANSMIT, sizeof(APDU_DIRECT_TRANSMIT));

           /* then a byte that tells it the length of the psuedo APDU to follow */
           pbSendBuffer[sizeof(APDU_DIRECT_TRANSMIT)] = (BYTE)apduLength;

           dwSendLength += (sizeof(APDU_DIRECT_TRANSMIT) + 1);
        }

        /* Add the APDU that was requested to be sent and increase length to send */
	memcpy((pbSendBuffer + dwSendLength), apdu, apduLength);
	dwSendLength += apduLength;

        sprintf(messageString, "APDU: ");
        sPrintBufferHex((messageString + strlen("APDU: ")), dwSendLength, pbSendBuffer);
        logMessage(LOG_INFO, 3, messageString);

        /* remember the size of the input buffer passed to us, so we don't exceed */
	rBufferMax = *dwRecvLength;

	rv = SCardTransmit(hCard, 
                           pioSendPci, pbSendBuffer, dwSendLength, 
                           &pioRecvPci, pbRecvBuffer, dwRecvLength);
        PCSC_ERROR(rv, "SCardTransmit");

        /* if it was a psuedo APDU then we need to get the response */
        if ( (rv == SCARD_S_SUCCESS) && psuedoAPDU )
        {
           sprintf(messageString, "Received: ");
           sPrintBufferHex((messageString + strlen("Received: ")), *dwRecvLength, pbRecvBuffer);
           logMessage(LOG_INFO, 3, messageString);

           /* command went OK? */
           if (pbRecvBuffer[SW1] != SW1_SUCCESS)
           {
              sprintf(messageString, "APDU failed: SW1 = %02X", pbRecvBuffer[SW1]);
              logMessage(LOG_ERR, 0, messageString);
              return ( SCARD_F_COMM_ERROR );
           }

           /* are their response bytes to get? */
           if (pbRecvBuffer[SW2] > 0)
           {
              sprintf(messageString, "Requesting Response Data (%d)", 
                      pbRecvBuffer[SW2]);
              logMessage(LOG_INFO, 3, messageString);

              /* copy the get_response APDU into the first bytes */
              memcpy(pbSendBuffer, APDU_GET_RESPONSE, sizeof(APDU_GET_RESPONSE));
              /* the second response byte tells us how many bytes are pending */
	      /* add that value at the end of the GET_RESPONSE APDU */
              pbSendBuffer[sizeof(APDU_GET_RESPONSE)] = pbRecvBuffer[SW2];
              dwSendLength = sizeof(APDU_GET_RESPONSE) + 1; 

              /* specify the maximum size of the buffer */
	      *dwRecvLength = rBufferMax;

              rv = SCardTransmit(hCard, pioSendPci, pbSendBuffer,
	                      dwSendLength, &pioRecvPci, pbRecvBuffer, &rBufferMax);
              PCSC_ERROR(rv, "SCardTransmit");
              sprintf(messageString, "Received: ");
              sPrintBufferHex(messageString, rBufferMax, pbRecvBuffer);
              logMessage(LOG_INFO, 3, messageString);

	      *dwRecvLength = rBufferMax;
	   }
           else
              *dwRecvLength = 0;
        }

   return(rv);

}
/**************************** APDU SEND ************************/


/************************* READER CONNECT **********************/
LONG readerConnect ( 
                  tReader	*pReader
                 )
{
   LONG 		rv;
   DWORD 		dwReaders,
			dwAtrLen,
			dwReaderLen,
			dwState,
			dwProt;    
   int                  nbReaders;
   char 		*ptr;
   DWORD 		dwActiveProtocol;
   BYTE 		pbAtr[MAX_ATR_SIZE] = "";
   DWORD		dwRecvLength;
   BYTE		        pbRecvBuffer[30];
   BOOL			readerSupported = FALSE;
   int			i;

   /* set strings to empty until we find out otherwise */
   strcpy(pReader->SAM_serial, "NoSAM");
   strcpy(pReader->SAM_id, "NoSAM");

   /* try and connect to the pcscd server if not already connected */
   if (!pReader->hContext)
   {
      rv = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, 
                                 &(pReader->hContext));
      PCSC_ERROR(rv, "SCardEstablishContext");
      if (rv != SCARD_S_SUCCESS)
         return (rv);
      logMessage(LOG_INFO, 2, "Successfully connected to pcscd server");
   }

   /* Retrieve the available readers list.
    * 1. Call with a null buffer to get the number of bytes to allocate
    * 2. malloc the necessary storage */
   rv = SCardListReaders(pReader->hContext, NULL, NULL, &dwReaders);
   PCSC_ERROR(rv, "SCardListReaders");
   if (rv != SCARD_S_SUCCESS)
      return (rv);

   /* malloc enough memory for dwReader string */
   mszReaders = malloc(sizeof(char)*dwReaders);
	
   /* now get the list into the mszReaders array */
   rv = SCardListReaders(pReader->hContext, NULL, mszReaders, &dwReaders);
   PCSC_ERROR(rv, "SCardListReaders");
   if (rv != SCARD_S_SUCCESS)
      return (rv);

   /* Extract readers from the null separated string and get the total
    * number of readers */
   nbReaders = 0;
   ptr = mszReaders;
   while (*ptr != '\0')
   {
      ptr += strlen(ptr)+1;
      nbReaders++;
   }

   sprintf(messageString, "Found %d Readers, ", nbReaders);
   logMessage(LOG_INFO, 2, messageString);

   if (nbReaders == 0)
      return(SCARD_E_UNKNOWN_READER);

   /* connect to the specified card reader */
   if (pReader->number < 0 || pReader->number >= nbReaders)
   {
     sprintf(messageString, "Wrong reader index: %d", pReader->number);
     logMessage(LOG_ERR, 0, messageString);
     return(SCARD_E_READER_UNAVAILABLE);
   }

   /* allocate the readers table, a set of pointers to the name of each */
   readers = calloc(nbReaders, sizeof(char *));
   if (NULL == readers)
   {
      logMessage(LOG_ERR, 0, "Not enough memory for readers[]");
      return(SCARD_F_INTERNAL_ERROR);
   }

   /* fill the readers table */
   nbReaders = 0;
   ptr = mszReaders;
   while (*ptr != '\0')
   {
      sprintf(messageString, "Reader [%d]: %s", nbReaders, ptr);
      logMessage(LOG_INFO, 3, messageString);
      readers[nbReaders] = ptr;
      ptr += strlen(ptr)+1;
      nbReaders++;
   }

   dwActiveProtocol = -1;
   rv = SCardConnect(pReader->hContext, readers[pReader->number],
                     SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0 , 
                     &(pReader->hCard), &dwActiveProtocol);
   PCSC_ERROR(rv, "SCardConnect");
   if (rv != SCARD_S_SUCCESS)
      return (rv);
   sprintf(messageString, "Connected to reader: %d", pReader->number);
   logMessage(LOG_INFO, 2, messageString);

   /* Get firmware version and check it's a ACR122* */
   dwRecvLength = sizeof(pbRecvBuffer);
   rv = apduSend(pReader->hCard, APDU_GET_READER_FIRMWARE, 
                 sizeof(APDU_GET_READER_FIRMWARE),
                 pbRecvBuffer, &dwRecvLength);
   if (rv != SCARD_S_SUCCESS)
      return (rv);

   /* Search the list of supported firmware versions (reader versions) */
   /* to check we are compatible with it - or have tested with it      */

   /* NULL terminate the BYTE array for subsequent string compares */
   pbRecvBuffer[dwRecvLength] = '\0';   
   sprintf(messageString, "Firmware: %s", pbRecvBuffer);
   logMessage(LOG_INFO, 3, messageString);

   for (i = 0; (i < sizeof(SUPPORTED_READER_ARRAY)) & !readerSupported ; i++)
   {
      if (strncmp(((char *)pbRecvBuffer),
          SUPPORTED_READER_ARRAY[i],
          sizeof(SUPPORTED_READER_ARRAY[i])) == 0)
         readerSupported = TRUE;
   }
   if (!readerSupported)
   {
      sprintf(messageString, "Reader (%s) not supported", pbRecvBuffer);
      logMessage(LOG_ERR, 0, messageString);
      return (SCARD_E_UNKNOWN_READER);
   }

   /* Get ATR so we can tell if there is a SAM in the reader */
   dwAtrLen = sizeof(pbAtr);
   dwReaderLen = sizeof(pbReader);
   rv = SCardStatus(pReader->hCard, pbReader, &dwReaderLen, &dwState, &dwProt,
                    pbAtr, &dwAtrLen);
   PCSC_ERROR(rv, "SCardStatus");
   sprintf(messageString, "ATR: ");
   sPrintBufferHex( (messageString + strlen("ATR: ")), dwAtrLen, pbAtr);
   logMessage(LOG_INFO, 3, messageString);

   dwRecvLength = sizeof(pbRecvBuffer);
   rv = apduSend(pReader->hCard, APDU_SET_RETRY, sizeof(APDU_SET_RETRY),
                 pbRecvBuffer, &dwRecvLength);
   if (rv != SCARD_S_SUCCESS)
      return (rv);

   /* get card status ATR first two bytes = ACS_NO_SAM= '3B00' */
   if ( (pbAtr[SW1] != 0x3B) || (pbAtr[SW2] != 0x00) )
   {
      dwRecvLength = sizeof(pbRecvBuffer);
      rv = apduSend(pReader->hCard, APDU_GET_SAM_SERIAL, 
                     sizeof(APDU_GET_SAM_SERIAL),
                     pbRecvBuffer, &dwRecvLength);
      if (rv == SCARD_S_SUCCESS)
      {
         sPrintBufferHex(pReader->SAM_serial, dwRecvLength, pbRecvBuffer);
         sprintf(messageString, "SAM Serial: %s", pReader->SAM_serial);
         logMessage(LOG_INFO, 1, messageString);
      }
      else
         return (rv);

      dwRecvLength = sizeof(pbRecvBuffer);
      rv = apduSend(pReader->hCard, APDU_GET_SAM_ID, sizeof(APDU_GET_SAM_ID),
                     pbRecvBuffer, &dwRecvLength);
      if (rv == SCARD_S_SUCCESS)
      {
         sPrintBufferHex(pReader->SAM_id, dwRecvLength, pbRecvBuffer);
         sprintf(messageString, "SAM ID: %s", pReader->SAM_id);
         logMessage(LOG_INFO, 1, messageString);
       }
   }

   return (rv);

}
/************************* READER CONNECT **********************/


/************************ READER DISCONNECT ********************/
void readerDisconnect(
                   tReader	*pReader
                   )
{
   LONG		rv;

   /* We try to leave things as clean as possible */
   sprintf(messageString, "Disconnecting from reader %d", pReader->number);
   logMessage(LOG_INFO, 2, messageString);

   if (pReader->hCard)
   {
      SCardDisconnect(pReader->hCard, SCARD_UNPOWER_CARD);
      pReader->hCard = ((SCARDHANDLE)NULL);
      PCSC_ERROR(rv, "SCardDisconnect");
   }

   logMessage(LOG_INFO, 2, "Disconnecting from pcscd server");

   if (pReader->hContext);
   {
      SCardReleaseContext(pReader->hContext);
      pReader->hContext = ((SCARDCONTEXT)NULL);
      PCSC_ERROR(rv, "SCardReleaseContext");
   }

   /* free allocated memory */
   if (mszReaders)
   {
      free(mszReaders);
      mszReaders = NULL;
   }
   if (readers)
   {
      free(readers);
      readers = NULL;
   }

}
/************************** READER DISCONNECT ********************/


/************************ GET CONTACTLESS STATUS *****************/
/* TODO : this function hasn't really been tested                */
LONG getContactlessStatus(
                          const tReader	*preader
                         )
{
   DWORD		dwRecvLength;
   BYTE			pbRecvBuffer[20];
   LONG			rv;

   sprintf(messageString, "Requesting status");
   logMessage(LOG_INFO, 2, messageString);

   dwRecvLength = sizeof(pbRecvBuffer);
   rv = apduSend(preader->hCard, APDU_GET_STATUS, sizeof(APDU_GET_STATUS),
                 pbRecvBuffer, &dwRecvLength);

   if ( rv == SCARD_S_SUCCESS )
   {
      if (verbosityLevel)
      {
         sprintf(messageString, "Status: ");
         sPrintBufferHex(messageString, dwRecvLength, pbRecvBuffer);
         logMessage(LOG_INFO, 2, messageString);

         sprintf(messageString, "Number of Tags = %d", pbRecvBuffer[4]);
         logMessage(LOG_INFO, 2, messageString);
      }
   }

   return (rv);
}
/************************ GET CONTACTLESS STATUS *****************/


/************************ GET TAG LIST ***************************/
LONG getTagList(
		const tReader	*preader,
		tTagList	*ptagList
               )
{
   LONG			rv;
   BYTE			uid_length;
   DWORD		dwRecvLength;
   BYTE			pbRecvBuffer[20];
   int			i, other;
   BOOL			known;

   ptagList->numTags = 0;

   /* loop until we have read possible two unique tags on the reader */
   for (i = 0; i < MAX_NUM_TAGS; i++)
   {
      /* Poll for tag - actually seems to cause a block if no tag is present */
      dwRecvLength = sizeof(pbRecvBuffer);
      rv = apduSend(preader->hCard, APDU_POLL_MIFARE, sizeof(APDU_POLL_MIFARE),
                     pbRecvBuffer, &dwRecvLength);
      if (rv == SCARD_S_SUCCESS)
      {
         /* ACS_TAG_FOUND = {0xD5, 0x4B}  */
         if ( (pbRecvBuffer[SW1] == 0xD5) && 
              (pbRecvBuffer[SW2] == 0x4B) &&
              (pbRecvBuffer[2] != 0x00) )
         {
            /* Check it's a MIFARE_ULTRA - byte 6 is tag type index */
            if (pbRecvBuffer[6] == 0x00)
            {
               sprintf(messageString, "Tag Type: MIFARE_ULTRA");
               logMessage(LOG_INFO, 2, messageString);

               /* byte 7 is length of unique tag ID */
               uid_length = pbRecvBuffer[7];
               /* the uid is in the next 'uid_length' number bytes */
               sPrintBufferHex(ptagList->tagUID[i], uid_length, (pbRecvBuffer+8));

               sprintf(messageString, "Tag ID:   %s", ptagList->tagUID[i]);
               logMessage(LOG_INFO, 2, messageString);
    
               /* first one ? */
               if ( i == 0 )
                  (ptagList->numTags)++; /* got first one */
               else
               {
                  /* check if this ID is already in our list */
                  known = FALSE;
                  for (other = 0; ((other < i) && (!known)) ; other++)
                  {
                     /* is this tag id already in our list? */
                     if (strcmp(ptagList->tagUID[i], 
                                ptagList->tagUID[other]) == 0)
                        known = TRUE;
                  }
                  if (!known) 
                    (ptagList->numTags)++; /* got another one */
               }
            }
         }
      }
      else
         return (rv);
   }

   sprintf(messageString, "Number of tags: %d", (int)ptagList->numTags);
   logMessage(LOG_INFO, 2, messageString);

   return (rv);

}
/************************ GET TAG LIST ***************************/