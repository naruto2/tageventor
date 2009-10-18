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


#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#define DEFAULT_LOCK_FILE_DIR "/var/run/tagEventor"
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



extern int  readerSettingGet( void );
extern int  readerSettingSet( int bitmap );

extern int  pollDelaySet( int     newPollDelay );
extern int  pollDelayGet( void );

extern int  verbosityLevelSet( int     newLevel );
extern int  verbosityLevelGet( void );
