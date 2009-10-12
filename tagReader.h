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

/**************************** CONSTANTS ******************************/
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

/* ACR122U can read two tags simulataneously, code can do more */
#define	MAX_NUM_TAGS	(2)

/* for MiFare tags the UID should be 14 hex digits I think */
#define MAX_TAG_UID_SIZE (20)

#define	MAX_LOG_MESSAGE	(200)

/* use this to have readerSetOptions() not touch a given option value */
#define	IGNORE_OPTION	(-1)

#define MAX_SAM_SERIAL_SIZE (30)    /* TODO what is max size? */
#define MAX_SAM_ID_SIZE     (30)    /* TODO what is max size? */

/**************************    TYPEDEFS    **************************/
typedef void    *tReaderManager;

typedef void    *tCardHandle;

typedef struct {
   int      		number;
   tCardHandle		hCard;
   char     		SAM;
   char     		SAM_serial[MAX_SAM_SERIAL_SIZE];
   char     		SAM_id[MAX_SAM_ID_SIZE];
} tReader;

typedef char	uid[MAX_TAG_UID_SIZE];

typedef struct {
	uid	        tagUID[MAX_NUM_TAGS];
    int	        numTags;
} tTagList;


/************************ EXTERNAL FUNCTIONS **********************/
extern int              readerSetOptions (  int	            verbosity,
                                            unsigned char	background );
extern int              readerManagerConnect( tReaderManager *pManager );
extern int              readerManagerDisconnect( tReaderManager *pManager );

extern int              readerConnect(  tReaderManager   *pmanager,
                                        tReader	*pReader );
extern void             readerDisconnect( tReader	*pReader );
extern int              getTagList( const tReader	*preader,
                                    tTagList	    *ptagList);
extern int              getContactlessStatus( const tReader	*preader );
extern void             logMessage( int		    type,
                                    int		    messageLevel,
                                    const char 	*message);

#endif
