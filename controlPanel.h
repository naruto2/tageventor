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

#ifdef BUILD_CONTROL_PANEL

/* this is a function that systemTray.c expects to exist to pop-up the control */
/* panel dialog                                                                */
extern void controlPanelActivate( void );

extern void controlPanelSetStatus( char connected, const char *message );

extern char controlPanelQuit( void );

#endif
