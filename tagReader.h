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

#include <PCSC/wintypes.h>
#include <PCSC/winscard.h>

/**************************** CONSTANTS ******************************/
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

/* ACR122U can read two tags simulataneously, code can do more */
#define	MAX_NUM_TAGS	(2)

#define	MAX_LOG_MESSAGE	200

#define	IGNORE_OPTION	(-1)


/**************************    TYPEDEFS    **************************/
typedef struct {
   SCARDCONTEXT 	hContext;
   int      		number;
   SCARDHANDLE		hCard;
   char     		SAM;
   char     		SAM_serial[30]; /* TODO what is max size? */
   char     		SAM_id[30];     /* TODO what is max size? */
} tReader;

typedef char	uid[20]; /* for MiFare tags the UID should be 14 hex digits I think */

typedef struct {
	uid	tagUID[MAX_NUM_TAGS];
        DWORD	numTags;
} tTagList;


/************************ EXTERNAL FUNCTIONS **********************/
extern LONG readerSetOptions ( int	verbosity,
                               BOOL	background );
extern LONG readerConnect ( tReader	*pReader );
extern void readerDisconnect( tReader	*pReader );
extern LONG getTagList( const tReader	*preader,
			tTagList	*ptagList);
extern LONG getContactlessStatus( const tReader	*preader );
extern void logMessage( int		type, 
                        int		messageLevel,
	                const char 	*message);
