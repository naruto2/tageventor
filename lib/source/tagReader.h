/*
  tagReader.h - public header files for tagReader library

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

#ifndef TAG_READER_INCLUDED
#define TAG_READER_INCLUDED

/* this is needed for definitions of different types of log messages etc */
#include <syslog.h>

/**************************** CONSTANTS ******************************/
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

/* for MiFare tags the UID should be 7 bytes or 14 hex digits plus a null */
#define MAX_TAG_UID_SIZE (20)

#define	MAX_LOG_MESSAGE	(200)

/* use this to have readerSetOptions() not touch a given option value */
#define	IGNORE_OPTION	(-1)

#define MAX_SAM_SERIAL_SIZE (30)    /* TODO what is max size? */
#define MAX_SAM_ID_SIZE     (30)    /* TODO what is max size? */


#define MAX_NUM_READERS     (6)
/* NOTE that the first bit is used for AUTO, so we use MAX_NUM_READERS +1 bits */
/* reader setting is a bitmap so that multiple can be set and remember at same time */
#define READER_BIT_NONE     (0)
#define READER_BIT_AUTO     (1<<0)
#define READER_BIT_0        (1<<1)
#define READER_BIT_1        (1<<2)
#define READER_BIT_2        (1<<3)
#define READER_BIT_3        (1<<4)
#define READER_BIT_4        (1<<5)
#define READER_BIT_5        (1<<6)
#define READER_BIT_DEFAULT  READER_BIT_AUTO

/**********************    STRINGS ****************************/
#define LIBTAGREADER_STRING_PCSCD_OK              "libTagReader: Successfully connected to pcscd server"
#define LIBTAGREADER_STRING_PCSCD_NO              "libTagReader: Failed to connect to pcscd server"


/**************************    TYPEDEFS    **************************/

typedef char	tUID[MAX_TAG_UID_SIZE];

typedef enum {  MIFARE_ULTRALIGHT   =0x00,
                MIFARE_1K           =0x08,
                MIFARE_MINI         =0x09,
                MIFARE_4K           =0x18,
                MIFARE_DESFIRE      =0x20,
                JCOP                =0x28,
                GEMPLUS_MPCOS       =0x98 } tTagType;

typedef struct {
                tUID        uid;
                tTagType    tagType;
                } tTag;

typedef struct {
		tTag        *pTags;    /* really a pointer to an array of tags */
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
    int         nbReaders;
    tReader     readers[MAX_NUM_READERS];
    char 	    *mszReaders;
    void        *hContext;
    tTagList    tagList;  /* overall list of tags in all readers */
} tReaderManager;

/************************ EXTERNAL FUNCTIONS **********************/
extern unsigned int     readersSettingBitmapGet( void );
extern unsigned int     readersSettingBitmapSet( unsigned int bitmap );
extern void             readersSettingBitmapBitSet( unsigned int bitmap );
extern void             readersSettingBitmapBitUnset( unsigned int bitmap );
extern unsigned int     readersSettingBitmapBitTest( unsigned int bitmap );
extern unsigned int     readersSettingBitmapNumberSet( unsigned int number );
extern unsigned int     readersSettingBitmapNumberTest( unsigned int bitNumber );

extern int              readersSetOptions (  int	            verbosity,
                                            unsigned char	background );
extern int              readersManagerConnect( tReaderManager *pManager );
extern int              readersManagerDisconnect( tReaderManager *pManager );

extern int              readersConnect(  tReaderManager  *pManager );
extern void             readersDisconnect( tReaderManager *pManager );
extern int              readersGetTagList( tReaderManager *pManager );
extern int              readersGetContactlessStatus( tReaderManager *pManager );
extern void             readersLogMessage( int		messageType,
                                    	  int	    	messageLevel,
                                    	const char 	*message);

#endif
