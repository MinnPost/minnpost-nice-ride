/***************************************
 $Header: /home/amb/CVS/routino/src/xml/xsd-to-xmlparser.c,v 1.10 2010-04-23 18:41:20 amb Exp $

 An XML parser for simplified XML Schema Definitions to create XML parser skeletons.

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2010 Andrew M. Bishop

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


#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "xmlparse.h"


/*+ A forward definition of the xmltagx +*/
typedef struct _xmltagx xmltagx;


/*+ A structure to hold the extended definition of a tag. +*/
struct _xmltagx
{
 char *name;                              /*+ The name of the tag. +*/
 char *type;                              /*+ The type of the tag. +*/

 int  nattributes;                        /*+ The number of valid attributes for the tag. +*/
 char *attributes[XMLPARSE_MAX_ATTRS];    /*+ The valid attributes for the tag. +*/

 int nsubtagsx;                           /*+ The number of valid attributes for the tag. +*/
 xmltagx *subtagsx[XMLPARSE_MAX_SUBTAGS]; /*+ The list of types for the subtags contained within this one. +*/
};


/* The local variables and functions */

int ntagsx=0;
xmltagx **tagsx=NULL;
char *currenttype=NULL;

static char *safe(const char *name);


/* The XML tag processing function prototypes */

static int xmlDeclaration_function(const char *_tag_,int _type_,const char *version,const char *encoding);
static int schemaType_function(const char *_tag_,int _type_,const char *elementFormDefault,const char *xmlns_xsd);
static int complexType_function(const char *_tag_,int _type_,const char *name);
static int attributeType_function(const char *_tag_,int _type_,const char *name,const char *type);
static int sequenceType_function(const char *_tag_,int _type_);
static int elementType_function(const char *_tag_,int _type_,const char *name,const char *type,const char *minOccurs,const char *maxOccurs);


/* The XML tag definitions */

/*+ The elementType type tag. +*/
static xmltag elementType_tag=
              {"xsd:element",
               4, {"name","type","minOccurs","maxOccurs"},
               elementType_function,
               {NULL}};

/*+ The sequenceType type tag. +*/
static xmltag sequenceType_tag=
              {"xsd:sequence",
               0, {NULL},
               sequenceType_function,
               {&elementType_tag,NULL}};

/*+ The attributeType type tag. +*/
static xmltag attributeType_tag=
              {"xsd:attribute",
               2, {"name","type"},
               attributeType_function,
               {NULL}};

/*+ The complexType type tag. +*/
static xmltag complexType_tag=
              {"xsd:complexType",
               1, {"name"},
               complexType_function,
               {&sequenceType_tag,&attributeType_tag,NULL}};

/*+ The schemaType type tag. +*/
static xmltag schemaType_tag=
              {"xsd:schema",
               2, {"elementFormDefault","xmlns:xsd"},
               schemaType_function,
               {&elementType_tag,&complexType_tag,NULL}};

/*+ The xmlDeclaration type tag. +*/
static xmltag xmlDeclaration_tag=
              {"xml",
               2, {"version","encoding"},
               xmlDeclaration_function,
               {NULL}};


/*+ The complete set of tags at the top level. +*/
static xmltag *xml_toplevel_tags[]={&xmlDeclaration_tag,&schemaType_tag,NULL};


/* The XML tag processing functions */


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the elementType XSD type is seen

  int elementType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *name The contents of the 'name' attribute (or NULL if not defined).

  const char *type The contents of the 'type' attribute (or NULL if not defined).

  const char *minOccurs The contents of the 'minOccurs' attribute (or NULL if not defined).

  const char *maxOccurs The contents of the 'maxOccurs' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int elementType_function(const char *_tag_,int _type_,const char *name,const char *type,const char *minOccurs,const char *maxOccurs)
{
 xmltagx *tagx=NULL;
 int i;

 if(_type_==XMLPARSE_TAG_END)
    return(0);

 for(i=0;i<ntagsx;i++)
    if(!strcmp(type,tagsx[i]->type) && !strcmp(name,tagsx[i]->name))
       tagx=tagsx[i];

 if(!tagx)
   {
    ntagsx++;
    tagsx=(xmltagx**)realloc((void*)tagsx,ntagsx*sizeof(xmltagx*));

    tagsx[ntagsx-1]=(xmltagx*)calloc(1,sizeof(xmltagx));
    tagsx[ntagsx-1]->name=strcpy(malloc(strlen(name)+1),name);
    tagsx[ntagsx-1]->type=strcpy(malloc(strlen(type)+1),type);

    tagx=tagsx[ntagsx-1];
   }

 if(!currenttype)
    return(0);

 for(i=0;i<ntagsx;i++)
    if(!strcmp(tagsx[i]->type,currenttype))
      {
       tagsx[i]->subtagsx[tagsx[i]->nsubtagsx]=tagx;
       tagsx[i]->nsubtagsx++;

       if(tagsx[i]->nsubtagsx==XMLPARSE_MAX_SUBTAGS)
         {fprintf(stderr,"Too many subtags seen for type '%s'.\n",currenttype); exit(1);}
      }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the sequenceType XSD type is seen

  int sequenceType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.
  ++++++++++++++++++++++++++++++++++++++*/

static int sequenceType_function(const char *_tag_,int _type_)
{
 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the attributeType XSD type is seen

  int attributeType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *name The contents of the 'name' attribute (or NULL if not defined).

  const char *type The contents of the 'type' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int attributeType_function(const char *_tag_,int _type_,const char *name,const char *type)
{
 int i;

 if(_type_==XMLPARSE_TAG_END)
    return(0);

 for(i=0;i<ntagsx;i++)
    if(!strcmp(tagsx[i]->type,currenttype))
      {
       tagsx[i]->attributes[tagsx[i]->nattributes]=strcpy(malloc(strlen(name)+1),name);
       tagsx[i]->nattributes++;

       if(tagsx[i]->nattributes==XMLPARSE_MAX_ATTRS)
         {fprintf(stderr,"Too many attributes seen for type '%s'.\n",currenttype); exit(1);}
      }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the complexType XSD type is seen

  int complexType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *name The contents of the 'name' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int complexType_function(const char *_tag_,int _type_,const char *name)
{
 if(_type_==XMLPARSE_TAG_END)
    return(0);

 currenttype=strcpy(realloc(currenttype,strlen(name)+1),name);

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the schemaType XSD type is seen

  int schemaType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *elementFormDefault The contents of the 'elementFormDefault' attribute (or NULL if not defined).

  const char *xmlns_xsd The contents of the 'xmlns:xsd' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int schemaType_function(const char *_tag_,int _type_,const char *elementFormDefault,const char *xmlns_xsd)
{
 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the XML declaration is seen

  int xmlDeclaration_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *version The contents of the 'version' attribute (or NULL if not defined).

  const char *encoding The contents of the 'encoding' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int xmlDeclaration_function(const char *_tag_,int _type_,const char *version,const char *encoding)
{
 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The XML Schema Definition XML parser and C program generator.
  ++++++++++++++++++++++++++++++++++++++*/

int main(int argc,char **argv)
{
 int i,j,k;

 if(ParseXML(stdin,xml_toplevel_tags,XMLPARSE_UNKNOWN_ATTR_IGNORE))
   {
    fprintf(stderr,"Cannot parse XML file - exiting.\n");
    exit(1);
   }

 /* Add the XML declaration as a tag. */

 currenttype=NULL;
 elementType_function("xsd:element",XMLPARSE_TAG_START|XMLPARSE_TAG_END,"xml","xmlDeclaration",NULL,NULL);
 complexType_function("xsd:complexType",XMLPARSE_TAG_START,"xmlDeclaration");
 attributeType_function("xsd:attribute",XMLPARSE_TAG_START|XMLPARSE_TAG_END,"version",NULL);
 attributeType_function("xsd:attribute",XMLPARSE_TAG_START|XMLPARSE_TAG_END,"encoding",NULL);
 complexType_function("xsd:complexType",XMLPARSE_TAG_END,NULL);

 /* Sort the tags */

 sorttags:

 for(i=0;i<ntagsx;i++)
   {
    for(j=0;j<tagsx[i]->nsubtagsx;j++)
      {
       for(k=0;k<ntagsx;k++)
          if(tagsx[i]->subtagsx[j]==tagsx[k])
             break;

       if(i<k)
         {
          xmltagx *temp=tagsx[i];

          tagsx[i]=tagsx[k];

          tagsx[k]=temp;

          goto sorttags;
         }
      }
   }

 /* Print the header */

 printf("/***************************************\n");
 printf(" An automatically generated skeleton XML parser.\n");
 printf("\n");
 printf(" Automatically generated by xsd-to-xmlparser.\n");
 printf(" ***************************************/\n");
 printf("\n");
 printf("\n");
 printf("#include <stdio.h>\n");
 printf("\n");
 printf("#include \"xmlparse.h\"\n");

 /* Print the function prototypes */

 printf("\n");
 printf("\n");
 printf("/* The XML tag processing function prototypes */\n");
 printf("\n");

 for(i=ntagsx-1;i>=0;i--)
   {
    printf("static int %s_function(const char *_tag_,int _type_",safe(tagsx[i]->type));

    for(j=0;j<tagsx[i]->nattributes;j++)
       printf(",const char *%s",safe(tagsx[i]->attributes[j]));

    printf(");\n");
   }

 /* Print the xmltag variables */

 printf("\n");
 printf("\n");
 printf("/* The XML tag definitions */\n");

 for(i=0;i<ntagsx;i++)
   {
    printf("\n");
    printf("/*+ The %s type tag. +*/\n",tagsx[i]->type);
    printf("static xmltag %s_tag=\n",safe(tagsx[i]->type));
    printf("              {\"%s\",\n",tagsx[i]->name);

    printf("               %d, {",tagsx[i]->nattributes);
    for(j=0;j<tagsx[i]->nattributes;j++)
       printf("%s\"%s\"",(j?",":""),tagsx[i]->attributes[j]);
    printf("%s},\n",(tagsx[i]->nattributes?"":"NULL"));

    printf("               %s_function,\n",safe(tagsx[i]->type));

    printf("               {");
    for(j=0;j<tagsx[i]->nsubtagsx;j++)
       printf("&%s_tag,",safe(tagsx[i]->subtagsx[j]->type));
    printf("NULL}};\n");
   }

 printf("\n");
 printf("\n");
 printf("/*+ The complete set of tags at the top level. +*/\n");
 printf("static xmltag *xml_toplevel_tags[]={");
 printf("&%s_tag,",safe(tagsx[ntagsx-1]->type));
 printf("&%s_tag,",safe(tagsx[ntagsx-2]->type));
 printf("NULL};\n");

 /* Print the functions */

 printf("\n");
 printf("\n");
 printf("/* The XML tag processing functions */\n");

 for(i=0;i<ntagsx;i++)
   {
    printf("\n");
    printf("\n");
    printf("/*++++++++++++++++++++++++++++++++++++++\n");
    if(i==(ntagsx-1))            /* XML tag */
       printf("  The function that is called when the XML declaration is seen\n");
    else
       printf("  The function that is called when the %s XSD type is seen\n",tagsx[i]->type);
    printf("\n");
    printf("  int %s_function Returns 0 if no error occured or something else otherwise.\n",safe(tagsx[i]->type));
    printf("\n");
    printf("  const char *_tag_ Set to the name of the element tag that triggered this function call.\n");
    printf("\n");
    printf("  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.\n");
    for(j=0;j<tagsx[i]->nattributes;j++)
      {
       printf("\n");
       printf("  const char *%s The contents of the '%s' attribute (or NULL if not defined).\n",safe(tagsx[i]->attributes[j]),tagsx[i]->attributes[j]);
      }
    printf("  ++++++++++++++++++++++++++++++++++++++*/\n");
    printf("\n");

    printf("static int %s_function(const char *_tag_,int _type_",safe(tagsx[i]->type));

    for(j=0;j<tagsx[i]->nattributes;j++)
       printf(",const char *%s",safe(tagsx[i]->attributes[j]));

    printf(")\n");

    printf("{\n");

    if(i==(ntagsx-1))            /* XML tag */
      {
       printf(" printf(\"<?%%s\",_tag_);\n");
       for(j=0;j<tagsx[i]->nattributes;j++)
         {
          char *safename=safe(tagsx[i]->attributes[j]);
          printf(" if(%s) printf(\" %s=\\\"%%s\\\"\",ParseXML_Encode_Safe_XML(%s));\n",safename,tagsx[i]->attributes[j],safename);
         }
       printf(" printf(\" ?>\\n\");\n");
      }
    else
      {
       printf(" printf(\"<%%s%%s\",(_type_==XMLPARSE_TAG_END)?\"/\":\"\",_tag_);\n");
       for(j=0;j<tagsx[i]->nattributes;j++)
         {
          char *safename=safe(tagsx[i]->attributes[j]);
          printf(" if(%s) printf(\" %s=\\\"%%s\\\"\",ParseXML_Encode_Safe_XML(%s));\n",safename,tagsx[i]->attributes[j],safename);
         }
       printf(" printf(\"%%s>\\n\",(_type_==(XMLPARSE_TAG_START|XMLPARSE_TAG_END))?\" /\":\"\");\n");
      }

    printf(" return(0);\n");
    printf("}\n");
   }

 /* Print the main function */

 printf("\n");
 printf("\n");
 printf("/*++++++++++++++++++++++++++++++++++++++\n");
 printf("  A skeleton XML parser.\n");
 printf("  ++++++++++++++++++++++++++++++++++++++*/\n");
 printf("\n");
 printf("int main(int argc,char **argv)\n");
 printf("{\n");
 printf(" if(ParseXML(stdin,xml_toplevel_tags,XMLPARSE_UNKNOWN_ATTR_WARN))\n");
 printf("    return(1);\n");
 printf(" else\n");
 printf("    return(0);\n");
 printf("}\n");

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  A function to return a safe C identifier from an XML tag or attribute name.

  char *safe Returns the safe name in a private string (only use once).

  const char *name The name to convert.
  ++++++++++++++++++++++++++++++++++++++*/

static char *safe(const char *name)
{
 static char *safe=NULL;
 int i;

 safe=realloc(safe,strlen(name)+1);

 for(i=0;name[i];i++)
    if(isalnum(name[i]))
       safe[i]=name[i];
    else
       safe[i]='_';

 safe[i]=0;

 return(safe);
}
