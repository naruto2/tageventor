/*
  acr122UDriver.c - C source code for driver code for ACS ACR122 U
  NFC Tag Reader/Writer.

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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <PCSC/wintypes.h>
#include <PCSC/winscard.h>

#include "tagSpecs.h"
#include "readerDriver.h"

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
/* Command response bytes - array indexes */
#define SW1             (0)
#define SW2             (1)
#define NUM_TAGS_FOUND  (2)
#define TARGET_NUMBER   (3)
#define SENS_RES_1      (4)
#define SENS_RES_2      (5)
#define SEL_RES_BYTE    (6)
#define UID_LENGTH      (7)
#define UID_START       (8)

/* SW1 return codes */
#define SW1_SUCCESS 0x61
#define SW1_FAIL    0x63

/* Error codes from Get Status APDU */
#define GET_STATUS_NO_ERROR 0x00

#define MAX_FIRMWARE_STRING_LENGTH  (30)

#define ACR122U_MAX_NUM_TAGS        (2)

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


/**********************    CONSTANT ****************************/
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

#define ACS_GET_STATUS  { 0xd4, 0x04 }
#define ACS_POLL_MIFARE { 0xd4, 0x4a, 0x01, 0x00 }
#define ACS_POLL_TYPE_B { 0xd4, 0x4a, 0x01, 0x03, 0x00 }
#define ACS_POLL_FELICA { 0xd4, 0x4a, 0x01, 0x01, 0x00 , 0x00, 0xFF, 0xFF, 0x00, 0x00}

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


static const BYTE APDU_RATS_14443_4_OFF[] = ACS_RATS_14443_4_OFF;

static const BYTE APDU_POLL_MIFARE[] = ACS_POLL_MIFARE;

static const BYTE APDU_POLL_TYPE_B[] = ACS_POLL_TYPE_B;

static const BYTE APDU_POLL_FELICA[] = ACS_POLL_FELICA;

static const BYTE APDU_READ_MIFARE[] = ACS_READ_MIFARE;
static const BYTE APDU_GET_RESPONSE[] =  ACS_GET_RESPONSE;
static const BYTE APDU_GET_SAM_SERIAL[] = ACS_APDU_GET_SAM_SERIAL;
static const BYTE APDU_GET_SAM_ID[] = ACS_APDU_GET_SAM_ID;
static const BYTE APDU_SET_RETRY[] = ACS_SET_RETRY;

static const BYTE APDU_LED_GREEN[] = ACS_LED_GREEN;
static const BYTE APDU_LED_ORANGE[] = ACS_LED_ORANGE;
static const BYTE APDU_LED_RED[] = ACS_LED_RED;

static const BYTE APDU_GET_READER_FIRMWARE[] = ACS_GET_READER_FIRMWARE;


static char		messageString[MAX_LOG_MESSAGE];

/* for the PCSC subtype ACS ACR38U use the T0 protocol - from RFIDiot */
static const SCARD_IO_REQUEST 	*pioSendPci = SCARD_PCI_T0;

/* This is the array of strings of names of supported readers */
/* This constant is entered by hand to overcome an issue with Mac OS X */
static const int SUPPORTED_READER_NAME_ARRAY_COUNT = 1;
/* If you add a supported reader to the array below, then modify the   */
/* SUPPORTED_READER_NAME_COUNT to match                               */
static const char * const SUPPORTED_READER_NAME_ARRAY[] = { "ACS ACR 38U-CCID" };

/* This is the array of firmware strings of supported readers */
/* This constant is entered by hand to overcome an issue with Mac OS X */
static const int SUPPORTED_READER_FIRMWARE_ARRAY_COUNT = 1;
/* If you add a supported reader to the array below, then modify the   */
/* SUPPORTED_READER_ARRAY_COUNT to match                               */
static const char * const SUPPORTED_READER_FIRMWARE_ARRAY[] = { "ACR122U" };

/* Prototypes for each of the functions needed to constitute a driver */
LONG   acr122UReaderCheck(tReader   *pReader,
                          BOOL      *pReaderSupported );

LONG   acr122UGetContactlessStatus(const tReader	*pReader,
                                   BYTE			    *pRecvBuffer,
                                   DWORD		    *pRecvLength);
LONG   acr122UGetTagList(const tReader	*pReader,
                         tTagList	    *pTagList );

/******** A Text Descriptor for this driver ************/
static const char acr122UDescriptor[] = "Andrew's acr122U driver";

/******** Reader Driver Structure for this reader code */
tReaderDriver acr122UDriver = { &acr122UReaderCheck,
                                &acr122UGetContactlessStatus,
                                &acr122UGetTagList,
                                acr122UDescriptor
                              };

/**************************** APDU SEND ************************/
static LONG
apduSend (
        tCardHandle     hCard,
		const BYTE    	*apdu,
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
    readerLogMessage(LOG_INFO, 3, messageString);

    /* remember the size of the input buffer passed to us, so we don't exceed */
	rBufferMax = *dwRecvLength;

	rv = SCardTransmit((SCARDHANDLE) hCard,
                           pioSendPci, pbSendBuffer, dwSendLength,
                           &pioRecvPci, pbRecvBuffer, dwRecvLength);
    PCSC_ERROR(rv, "SCardTransmit");

    /* if it was a psuedo APDU then we need to get the response */
    if ( (rv == SCARD_S_SUCCESS) && psuedoAPDU )
    {
       sprintf(messageString, "Received: ");
       sPrintBufferHex((messageString + strlen("Received: ")), *dwRecvLength, pbRecvBuffer);
       readerLogMessage(LOG_INFO, 3, messageString);

       /* command went OK? */
       if (pbRecvBuffer[SW1] != SW1_SUCCESS)
       {
          sprintf(messageString, "APDU failed: SW1 = %02X", pbRecvBuffer[SW1]);
          readerLogMessage(LOG_ERR, 0, messageString);
          return ( SCARD_F_COMM_ERROR );
       }

       /* are their response bytes to get? */
       if (pbRecvBuffer[SW2] > 0)
       {
          sprintf(messageString, "Requesting Response Data (%d)",
                  pbRecvBuffer[SW2]);
          readerLogMessage(LOG_INFO, 3, messageString);

          /* copy the get_response APDU into the first bytes */
          memcpy(pbSendBuffer, APDU_GET_RESPONSE, sizeof(APDU_GET_RESPONSE));

          /* the second response byte tells us how many bytes are pending */
	      /* add that value at the end of the GET_RESPONSE APDU */
          pbSendBuffer[sizeof(APDU_GET_RESPONSE)] = pbRecvBuffer[SW2];
          dwSendLength = sizeof(APDU_GET_RESPONSE) + 1;

          /* specify the maximum size of the buffer */
	      *dwRecvLength = rBufferMax;

          rv = SCardTransmit((SCARDCONTEXT)hCard, pioSendPci, pbSendBuffer,
	                         dwSendLength, &pioRecvPci, pbRecvBuffer, &rBufferMax);
          PCSC_ERROR(rv, "SCardTransmit");
          sprintf(messageString, "Received: ");
          sPrintBufferHex(messageString, rBufferMax, pbRecvBuffer);
          readerLogMessage(LOG_INFO, 3, messageString);

	      *dwRecvLength = rBufferMax;
	   }
       else
          *dwRecvLength = 0;
    }

    return(rv);

}
/**************************** APDU SEND ************************/



/************************ GET CONTACTLESS STATUS *****************/
LONG   acr122UGetContactlessStatus(
                                       const tReader	*pReader,
                                        BYTE			*pRecvBuffer,
                                        DWORD		    *pRecvLength
                                        )
{
   LONG			rv;

   rv = apduSend(pReader->hCard, APDU_GET_STATUS, sizeof(APDU_GET_STATUS),
                 pRecvBuffer, pRecvLength);

   return (rv);
}
/************************ GET CONTACTLESS STATUS *****************/


/************************ GET TAG LIST ***************************/
LONG   acr122UGetTagList(
                         const tReader	*pReader,
                         tTagList	    *pTagList
                         )
{
   LONG			rv;
   BYTE			uid_length;
   DWORD		dwRecvLength;
   BYTE			pbRecvBuffer[20];
   int			i, other;
   BOOL			known;

   pTagList->numTags = 0;

   /* loop until we have read possible two unique tags on the reader */
   for (i = 0; i < ACR122U_MAX_NUM_TAGS; i++)
   {
      /* Poll for tag - actually seems to cause a block if no tag is present */
      dwRecvLength = sizeof(pbRecvBuffer);
      rv = apduSend(pReader->hCard, APDU_POLL_MIFARE, sizeof(APDU_POLL_MIFARE),
                     pbRecvBuffer, &dwRecvLength);
      if (rv == SCARD_S_SUCCESS)
      {
         /* ACS_TAG_FOUND = {0xD5, 0x4B}  */
         if ( (pbRecvBuffer[SW1] == 0xD5) &&
              (pbRecvBuffer[SW2] == 0x4B) &&
              (pbRecvBuffer[NUM_TAGS_FOUND] != 0x00) )
         {
             switch( pbRecvBuffer[SEL_RES_BYTE] )
             {
                case SEL_RES_MIFARE_ULTRA:
                    sprintf(messageString, "Tag Type: MIFARE_ULTRA");
                    break;

                case SEL_RES_MIFARE_1K:
                    sprintf(messageString, "Tag Type: MIFARE_1K");
                    break;

                case SEL_RES_MIFARE_MINI:
                    sprintf(messageString, "Tag Type: MIFARE_MINI");
                    break;

                case SEL_RES_MIFARE_4K:
                    sprintf(messageString, "Tag Type: MIFARE_4K");
                    break;

                case SEL_RES_MIFARE_DESFIRE:
                    sprintf(messageString, "Tag Type: MIFARE_DESFIRE");
                    break;

                case SEL_RES_JCOP30:
                    sprintf(messageString, "Tag Type: JCOP30");
                    break;

                case SEL_RES_GEMPLUS_MPCOS:
                    sprintf(messageString, "Tag Type: GEMPLUS_MPCOS");
                    break;

                default:
                    sprintf(messageString, "Tag Type: Unknown");
                    break;
             }

             readerLogMessage(LOG_INFO, 2, messageString);

             /* store tag type in tag struct */
             pTagList->tag[i].tagType = (tTagType)pbRecvBuffer[SEL_RES_BYTE];

             /* byte 7 is length of unique tag ID */
             uid_length = pbRecvBuffer[UID_LENGTH];

             /* how the rest is organized depends on tag Type */
             switch ( pbRecvBuffer[SEL_RES_BYTE] )
             {
                case SEL_RES_MIFARE_ULTRA:
                case SEL_RES_MIFARE_1K:
                case SEL_RES_MIFARE_MINI:      /* not sure but I think so */
                case SEL_RES_MIFARE_4K:
                case SEL_RES_MIFARE_DESFIRE:   /* not sure but I think so */
                    /* the uid is in the next 'uid_length' number bytes */
                    sPrintBufferHex(pTagList->tag[i].uid, uid_length, (pbRecvBuffer + UID_START) );
                    break;

                case SEL_RES_JCOP30:
                    break;

                case SEL_RES_GEMPLUS_MPCOS:
                    break;

                default:
                    sprintf( pTagList->tag[i].uid, "Unknown" );
                    break;
             }

             sprintf(messageString, "Tag ID:   %s", pTagList->tag[i].uid );
             readerLogMessage(LOG_INFO, 2, messageString);

             /* first one ? */
             if ( i == 0 )
                (pTagList->numTags)++; /* got first one */
             else
             {
                /* check if this ID is already in our list */
                known = FALSE;
                for (other = 0; ((other < i) && (!known)) ; other++)
                {
                   /* is this tag id already in our list? */
                   if (strcmp(pTagList->tag[i].uid,
                              pTagList->tag[other].uid) == 0)
                      known = TRUE;
                }
                if (!known)
                  (pTagList->numTags)++; /* got another one */
             }
         }
      }
      else
         return (rv);
   }

   return (rv);

}
/************************ GET TAG LIST ***************************/


/************************ READER CHECK ***************************/
 LONG   acr122UReaderCheck(
                           tReader   *pReader,
                           BOOL      *pReaderSupported
                           )
{
   static  char     pbReader[MAX_READERNAME] = "";

   DWORD  dwAtrLen, dwReaderLen, dwState, dwProt;
   DWORD  dwRecvLength;
   BYTE   pbAtr[MAX_ATR_SIZE] = "";
   int    i;
   LONG   rv;
   BYTE	  pbRecvBuffer[MAX_FIRMWARE_STRING_LENGTH];

   *pReaderSupported = FALSE;

   /* First check the name reported by pcscd to see if it's possible supported */
    for (i = 0; (i < SUPPORTED_READER_NAME_ARRAY_COUNT) & (*pReaderSupported == FALSE) ; i++)
    {
        if ( strncmp( pReader->name, SUPPORTED_READER_NAME_ARRAY[i],
                      sizeof(SUPPORTED_READER_NAME_ARRAY[i])-1) == 0 )
         *pReaderSupported = TRUE;
    }

    /* If even the name is not supported, then we're done */
    if ( *pReaderSupported == FALSE )
       return (SCARD_S_SUCCESS );

    /* just because the name was OK, doesn't mean it'll actually work! */
    *pReaderSupported = FALSE;

    /* Get firmware version and check it's really a ACR122* */
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

    for (i = 0; (i < SUPPORTED_READER_FIRMWARE_ARRAY_COUNT) & (*pReaderSupported == FALSE) ; i++)
    {
       if ( strncmp( ((char *)pbRecvBuffer), SUPPORTED_READER_FIRMWARE_ARRAY[i],
                    sizeof(SUPPORTED_READER_FIRMWARE_ARRAY[i])-1) == 0 )
          *pReaderSupported = TRUE;

        sprintf(messageString, "Reader: %s, with Firmware: '%s' supported by driver='%s'", pReader->name, pbRecvBuffer, acr122UDescriptor );
        readerLogMessage(LOG_INFO, 3, messageString);
    }

    if ( *pReaderSupported == FALSE )
    {
        sprintf(messageString, "Reader: %s, with Firmware: '%s' is not supported", pReader->name, pbRecvBuffer);
        readerLogMessage(LOG_INFO, 3, messageString);
    }

    /* If we got this far then the general name and specific firmware version is supported */

    /* Get ATR so we can tell if there is a SAM in the reader */
    dwAtrLen = sizeof(pbAtr);
    dwReaderLen = sizeof(pbReader);
    rv = SCardStatus( (SCARDHANDLE) (pReader->hCard), pbReader, &dwReaderLen, &dwState, &dwProt,
                     pbAtr, &dwAtrLen);
    sprintf(messageString, "ATR: ");
    sPrintBufferHex( (messageString + strlen("ATR: ")), dwAtrLen, pbAtr);
    readerLogMessage(LOG_INFO, 3, messageString);

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
          readerLogMessage(LOG_INFO, 1, messageString);
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
          readerLogMessage(LOG_INFO, 1, messageString);
        }
        else
            return( rv );

        pReader->SAM = TRUE;
    }

    /* Turning RATS off thus a JCOP tag will be detected as emulating a DESFIRE */
    rv = apduSend(pReader->hCard, APDU_RATS_14443_4_OFF, sizeof(APDU_RATS_14443_4_OFF),
                      pbRecvBuffer, &dwRecvLength);

    return( SCARD_S_SUCCESS );

}
/************************ READER CHECK ***************************/
