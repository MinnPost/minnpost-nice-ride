/***************************************
 A simple XML parser

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2010-2011 Andrew M. Bishop

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU Affero General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Affero General Public License for more details.

 You should have received a copy of the GNU Affero General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ***************************************/


#ifndef XMLPARSE_H
#define XMLPARSE_H    /*+ To stop multiple inclusions. +*/

#include <stdio.h>


/*+ The maximum number of attributes per tag. +*/
#define XMLPARSE_MAX_ATTRS   16

/*+ The maximum number of subtags per tag. +*/
#define XMLPARSE_MAX_SUBTAGS 16

/*+ A flag to indicate the start and/or end of a tag. +*/
#define XMLPARSE_TAG_START    1
#define XMLPARSE_TAG_END      2


/*+ A forward definition of the xmltag +*/
typedef struct _xmltag xmltag;


/*+ A structure to hold the definition of a tag. +*/
struct _xmltag
{
 char *name;                            /*+ The name of the tag. +*/

 int  nattributes;                      /*+ The number of valid attributes for the tag. +*/
 char *attributes[XMLPARSE_MAX_ATTRS];  /*+ The valid attributes for the tag. +*/

 int  (*callback)();                    /*+ The callback function when the tag is seen. +*/

 xmltag *subtags[XMLPARSE_MAX_SUBTAGS]; /*+ The list of valid tags contained within this one (null terminated). +*/
};


/* XML Parser options */

#define XMLPARSE_UNKNOWN_ATTRIBUTES     0x0003
#define XMLPARSE_UNKNOWN_ATTR_ERROR     0x0000 /* Flag an error and exit. */
#define XMLPARSE_UNKNOWN_ATTR_ERRNONAME 0x0001 /* Flag an error and exit unless a namespace is specified. */
#define XMLPARSE_UNKNOWN_ATTR_WARN      0x0002 /* Warn about the problem and continue. */
#define XMLPARSE_UNKNOWN_ATTR_IGNORE    0x0003 /* Ignore the potential problem. */

#define XMLPARSE_RETURN_ATTR_ENCODED    0x0004 /* Return the XML attribute strings without decoding them. */


/* XML parser functions */

int ParseXML(FILE *file,xmltag **tags,int options);

unsigned long long ParseXML_LineNumber(void);

char *ParseXML_Decode_Entity_Ref(const char *string);
char *ParseXML_Decode_Char_Ref(const char *string);
char *ParseXML_Encode_Safe_XML(const char *string);

int ParseXML_IsInteger(const char *string);
int ParseXML_IsFloating(const char *string);

/* Macros to simplify the callback functions */

#define XMLPARSE_MESSAGE(tag,message) \
 do \
   { \
    fprintf(stderr,"XML Parser: Error on line %llu: " message " in <%s> tag.\n",ParseXML_LineNumber(),tag); \
    return(1); \
   } \
    while(0)

#define XMLPARSE_INVALID(tag,attribute) \
 do \
   { \
    fprintf(stderr,"XML Parser: Error on line %llu: Invalid value for '" #attribute "' attribute in <%s> tag.\n",ParseXML_LineNumber(),tag); \
    return(1); \
   } \
    while(0)

#define XMLPARSE_ASSERT_STRING(tag,attribute) \
 do \
   { \
    if(!attribute) \
      { \
       fprintf(stderr,"XML Parser: Error on line %llu: '" #attribute "' attribute must be specified in <%s> tag.\n",ParseXML_LineNumber(),tag); \
       return(1); \
      } \
   } \
    while(0)

#define XMLPARSE_ASSERT_INTEGER(tag,attribute)  \
 do \
   { \
    if(!attribute || !*attribute || !ParseXML_IsInteger(attribute)) \
      { \
       fprintf(stderr,"XML Parser: Error on line %llu: '" #attribute "' attribute must be a integer in <%s> tag.\n",ParseXML_LineNumber(),tag); \
       return(1); \
      } \
   } \
    while(0)

#define XMLPARSE_ASSERT_FLOATING(tag,attribute)  \
 do \
   { \
    if(!attribute || !*attribute || !ParseXML_IsFloating(attribute)) \
      { \
       fprintf(stderr,"XML Parser: Error on line %llu: '" #attribute "' attribute must be a number in <%s> tag.\n",ParseXML_LineNumber(),tag); \
       return(1); \
      } \
   } \
    while(0)


#endif /* XMLPARSE_H */
