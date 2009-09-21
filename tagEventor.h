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


#define MAX_DESCRIPTION_LENGTH  (80)

#define PROGRAM_NAME    "Tageventor"

/* where to save this type of config stuff???? via GConf or something */
typedef struct {
   	char		*ID;            /* Max size = sizeof(uid) */
 	char		*script;        /* Max size = PATH_MAX */
	char		*description;   /* Max size = MAX_DESCRIPTION_LENGTH */
	char		enabled;
} tPanelEntry;

extern int  tagTableAddEntry( void );
extern void tagTableSave( void );
extern int  tagTableRead( void );
extern void tagTableEntryEnable( int index, char enable );
extern const tPanelEntry *tagEntryGet( int index );
