/*
  tagEventor.h - C source code for tagEventor

  Copyright 2009 Autelic Association (http://www.autelic.org)

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

#include "tagReader.h"

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#define MAX_DESCRIPTION_LENGTH  (80)

#define DEFAULT_LOCK_FILE_DIR "/var/run/tagEventor"
#define DEFAULT_COMMAND_DIR "/etc/tagEventor"
#define DAEMON_NAME "tagEventord"

#define POLL_DELAY_MILLI_SECONDS_MIN        (500)
#define POLL_DELAY_MILLI_SECONDS_DEFAULT    (1000)
#define POLL_DELAY_MILLI_SECONDS_MAX        (5000)

#define VERBOSITY_MIN       (0)
#define VERBOSITY_DEFAULT   (0)
#define VERBOSITY_MAX       (3)

#define MAX_NUM_READERS     (6)
/* NOTE that the first bit is used for AUTO, so we use MAX_NUM_READERS +1 bits */
/* reader setting is a bitmap so that multiple can be set and remember at same time */
#define READER_NUM_AUTO     (1<<0)
#define READER_NUM_0        (1<<1)
#define READER_NUM_1        (1<<2)
#define READER_NUM_2        (1<<3)
#define READER_NUM_3        (1<<4)
#define READER_NUM_4        (1<<5)
#define READER_NUM_5        (1<<6)
#define READER_NUM_DEFAULT  READER_NUM_AUTO

/* different options for matching script names */
#define TAG_ID_MATCH		(1)
#define GENERIC_MATCH		(2)
#define SAM_ID_MATCH	    (3)
#define SAM_SERIAL_MATCH    (4)
#define READER_NUM_MATCH	(5)

/* where to save this type of config stuff???? via GConf or something */
typedef struct {
   	char		*IDRegex;         /* specific ID or a regular expression - Max size = sizeof(uid) */
	char		*folder;          /* folder where to look for script     - Max size = PATH_MAX */
	int		    scriptMatchType;  /* type of match to use for script name */
	char		*description;     /* Max size = MAX_DESCRIPTION_LENGTH */
	char		enabled;
} tPanelEntry;

extern int  readerSettingGet( void );
extern int  readerSettingSet( int bitmap );

extern int  pollDelaySet( int     newPollDelay );
extern int  pollDelayGet( void );

extern int  verbosityLevelSet( int     newLevel );
extern int  verbosityLevelGet( void );


/*************** RULES TABLE *************/
extern int  rulesTableAddEntry( void );
extern void rulesTableSave( void );
extern int  rulesTableNumber( void );
extern int  rulesTableRead( void );
extern void rulesTableEntryEnable( int index, char enable );
extern const tPanelEntry *rulesTableEntryGet( int index );
extern void  rulesTableEventDispatch(
                int	  	        eventType,
                uid       	    tagUID,
                const tReader	*preader
                );

