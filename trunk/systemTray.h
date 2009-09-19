/*
  systemTray.h - headerfile that defines the entry point for code
                 that will create and start and run a system tray icon

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

#ifdef BUILD_SYSTEM_TRAY

#define SYSTEM_TRAY_ICON_MESSAGE_MAX    (80)

/* define the function we must implement for tagEventor.c to start system tray icon */
extern void startSystemTray( int     *argc, char    ***argv, char (*pollFunction)( void  *data ) );

extern void systemTraySetStatus( char connected, const char *message );
#endif
