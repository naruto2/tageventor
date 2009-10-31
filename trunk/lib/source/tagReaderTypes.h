/*
  tagReaderTypes.h - public header files for tagReader library

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

#ifndef TAG_READER_TYPES_INCLUDED
#define TAG_READER_TYPES_INCLUDED

#include "tagMifareUltra.h"

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#define MAX_SAM_SERIAL_SIZE (30)    /* TODO what is max size? */
#define MAX_SAM_ID_SIZE     (30)    /* TODO what is max size? */

/* for MiFare tags the UID should be 7 bytes or 14 hex digits plus a null */
#define MAX_TAG_UID_SIZE (20)

#define MAX_NUM_READERS     (6)

#define SEL_RES_MIFARE_ULTRA                    (0x00)
#define SEL_RES_MIFARE_1K                       (0x08)
#define SEL_RES_MIFARE_MINI                     (0x09)
#define SEL_RES_MIFARE_4K                       (0x18)
#define SEL_RES_MIFARE_DESFIRE                  (0x20)
#define SEL_RES_JCOP30                          (0x28)  /* TYPE A */
#define SEL_RES_GEMPLUS_MPCOS                   (0x98)


/**************************    TYPEDEFS    **************************/
typedef union		{
			    tTagContents_MIFARE_ULTRA   mifareUltra;
			} tTagContents;

typedef char	tUID[MAX_TAG_UID_SIZE];

typedef enum {  MIFARE_ULTRALIGHT   = SEL_RES_MIFARE_ULTRA,
                MIFARE_1K           = SEL_RES_MIFARE_1K,
                MIFARE_MINI         = SEL_RES_MIFARE_MINI,
                MIFARE_4K           = SEL_RES_MIFARE_4K,
                MIFARE_DESFIRE      = SEL_RES_MIFARE_DESFIRE,
                JCOP                = SEL_RES_JCOP30,
                GEMPLUS_MPCOS       = SEL_RES_GEMPLUS_MPCOS,
                UNKNOWN_TYPE        =0xFF } tTagType;

typedef struct {
                tTagContents    *pContents;
                tTagType        tagType;
                tUID            uid;
                } tTag;

typedef struct {
		tTag        *pTags;
    	int	        numTags;
		} tTagList;

typedef void    *tCardHandle;

typedef struct {
   char             *name;
   tCardHandle		hCard;
   void             *pDriver;  /* hide the driver details to the outside world */
   const char       *driverDescriptor;
   char     		SAM;
   char     		SAM_serial[MAX_SAM_SERIAL_SIZE];
   char     		SAM_id[MAX_SAM_ID_SIZE];
   unsigned char    scanning; /* BOOL if being scanned currently or not */
   tTagList         tagList;     /* the tags in this reader */
} tReader;

typedef struct {
    int             nbReaders;
    tReader         readers[MAX_NUM_READERS];
    char 	        *mszReaders;
    void            *hContext;
    tTagList        tagList;  /* overall list of tags in all readers */
    int             readerSetting;
    int             libVerbosityLevel;
    unsigned char   runningInBackground;
} tReaderManager;

#endif /* TAG_READER_TYPES_INCLUDED */
