/*
  tagSPECS.h - definitions of logical tag structures for tagReader library

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

#ifndef TAG_SPECS_INCLUDED
#define TAG_SPECS_INCLUDED

/**************************** CONSTANTS ******************************/
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#define SEL_RES_MIFARE_ULTRA                    (0x00)
#define SEL_RES_MIFARE_1K                       (0x08)
#define SEL_RES_MIFARE_MINI                     (0x09)
#define SEL_RES_MIFARE_4K                       (0x18)
#define SEL_RES_MIFARE_DESFIRE                  (0x20)
#define SEL_RES_JCOP30                          (0x28)
#define SEL_RES_GEMPLUS_MPCOS                   (0x98)


/*********************************************************************/
/**************************** MiFARE Ultra ***************************/
/*********************************************************************/

/**************************** CONSTANTS ******************************/
#define	NUM_BYTES_UID_MIFARE_ULTRA		        (7)
#define NUM_USER_BYTES_PER_PAGE_MIFARE_ULTRA	(4)
#define NUM_OTP_BYTES_MIFARE_ULTRA		        (4)
#define NUM_PAGES_MIFARE_ULTRA			        (12)



/**************************    TYPEDEFS    **************************/
typedef const char	t_Logical_UID_MIFARE_ULTRA[NUM_BYTES_UID_MIFARE_ULTRA];

typedef struct		{
			char		data[NUM_USER_BYTES_PER_PAGE_MIFARE_ULTRA];
			unsigned char	locked;/* is it locked or can it be written to */
			unsigned char	valid; /* has this data been read from tag?    */
			unsigned char	writePending;
			/* data has been changed but not yet written to the actual tag */
			} t_Logical_Page_MIFARE_ULTRA;

typedef	char		t_Logical_OTP[NUM_OTP_BYTES_MIFARE_ULTRA];

typedef struct		{
			    t_Logical_UID_MIFARE_ULTRA	uid;
			    t_Logical_OTP		OTP;
			    t_Logical_Page_MIFARE_ULTRA	pages[NUM_PAGES_MIFARE_ULTRA];
			} t_Logical_Tag_Contents_MIFARE_ULTRA;

typedef enum	{ NXP=0x04 } t_Logical_Tag_Manufacturer_MIFARE_ULTRA;

#if 0
const char NXPString[] = "NXP Semiconductor";
const char UnknownString[] = "Unknown Semiconductor";
#endif

typedef union		{
			    const char *NXPString;
			    const char *Unknown;
			} t_Logical_Tag_Manufacturer_String_MIFARE_ULTRA;


/*********************************************************************/
/******************************* Global ******************************/
/*********************************************************************/
typedef union 		{
			    t_Logical_Tag_Manufacturer_MIFARE_ULTRA mifareManufacturer;
			} t_Logical_Tag_Manufacturer;

typedef union 		{
			    t_Logical_Tag_Manufacturer_String_MIFARE_ULTRA mifareManufacturerString;
			} t_Logical_Tag_Manufacturer_String;

typedef union		{
			    t_Logical_UID_MIFARE_ULTRA mifareCOntents;
			} t_Logical_Tag_Contents;

typedef struct		{
			    t_Logical_Tag_Manufacturer		    manufacturer;
			    t_Logical_Tag_Manufacturer_String	manufacturerString;
			    t_Logical_Tag_Contents		        logicalTagContents;
			} t_Logical_Tag;

#endif /* TAG_SPECS_INCLUDED */
