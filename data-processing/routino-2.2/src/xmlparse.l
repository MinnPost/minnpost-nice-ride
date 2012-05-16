%{
/***************************************
 A simple generic XML parser where the structure comes from the function parameters.
 Not intended to be fully conforming to XML staandard or a validating parser but
 sufficient to parse OSM XML and simple program configuration files.

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


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <strings.h>

#include "xmlparse.h"


/* Parser outputs */

#define LEX_EOF                    0

#define LEX_TAG_BEGIN              1
#define LEX_XML_DECL_BEGIN         2
#define LEX_TAG_POP                3
#define LEX_TAG_PUSH               4
#define LEX_XML_DECL_FINISH        6
#define LEX_TAG_FINISH             7
#define LEX_ATTR_KEY               8
#define LEX_ATTR_VAL               9

#define LEX_ERROR                100

#define LEX_ERROR_TAG_START      101
#define LEX_ERROR_XML_DECL_START 102
#define LEX_ERROR_TAG            103
#define LEX_ERROR_XML_DECL       104
#define LEX_ERROR_ATTR           105
#define LEX_ERROR_END_TAG        106
#define LEX_ERROR_COMMENT        107
#define LEX_ERROR_CLOSE          108
#define LEX_ERROR_ATTR_VAL       109
#define LEX_ERROR_ENTITY_REF     110
#define LEX_ERROR_CHAR_REF       111

#define LEX_ERROR_UNEXP_TAG      201
#define LEX_ERROR_UNBALANCED     202
#define LEX_ERROR_NO_START       203
#define LEX_ERROR_UNEXP_ATT      204
#define LEX_ERROR_UNEXP_EOF      205
#define LEX_ERROR_XML_NOT_FIRST  206

#define LEX_ERROR_CALLBACK       255


/* Lexer definitions */

/*+ Reset the current string. +*/
#define reset_string \
 if(!string) string=(char*)malloc(16); \
 *string=0; \
 stringused=0;

/*+ append information to the current string. +*/
#define append_string(xx) \
 newlen=strlen(xx); \
 if((stringused+newlen)>=stringlen) \
    string=(char*)realloc((void*)string,stringlen=(stringused+newlen+16)); \
 strcpy(string+stringused,xx); \
 stringused+=newlen;


/* Lexer functions and variables */

extern int yylex(void);

static char *yylval=NULL;

static int xmlparse_options;

static unsigned long long lineno;

%}

%option 8bit
%option pointer
%option batch
%option never-interactive

%option nodefault
%option perf-report
%option fast
%option noread

%option nounput
%option noinput
%option noyywrap


 /* Grammar based on http://www.w3.org/TR/2004/REC-xml-20040204/ but for ASCII tags not Unicode. */

S               [ \t]

U1              [\x09\x0A\x0D\x20-\x7F]
U2              [\xC2-\xDF][\x80-\xBF]
U3a             \xE0[\xA0-\xBF][\x80-\xBF]
U3b             [\xE1-\xEC][\x80-\xBF][\x80-\xBF]
U3c             \xED[\x80-\x9F][\x80-\xBF]
U3d             [\xEE-\xEF][\x80-\xBF][\x80-\xBF]
U3              {U3a}|{U3b}|{U3c}|{U3d}
U4a             \xF0[\x90-\xBF][\x80-\xBF][\x80-\xBF]
U4b             [\xF1-\xF3][\x80-\xBF][\x80-\xBF][\x80-\xBF]
U4c             \xF4[\x80-\x8F][\x80-\xBF][\x80-\xBF]
U4              {U4a}|{U4b}|{U4c}

U               ({U1}|{U2}|{U3}|{U4})
UquotedS        ([\x09\x0A\x0D\x20-\x25\x28-\x3B\x3D\x3F-\x7F]|{U2}|{U3}|{U4})
UquotedD        ([\x09\x0A\x0D\x20-\x21\x23-\x25\x27-\x3B\x3D\x3F-\x7F]|{U2}|{U3}|{U4})

N               (\n|\r\n)

letter          [a-zA-Z]
digit           [0-9]
xdigit          [a-fA-F0-9]

namechar        ({letter}|{digit}|[-._:])
name            ({letter}|[_:]){namechar}*

entityref       &{name};
charref         &#({digit}+|x{xdigit}+);


%x COMMENT
%x CDATA
%x DOCTYPE
%x XML_DECL_START XML_DECL
%x TAG_START TAG
%x ATTR_KEY ATTR_VAL
%x END_TAG1 END_TAG2
%x DQUOTED SQUOTED

%%
 /* Must use static variables since the parser returns often. */
 static char *string=NULL;
 static int stringlen=0,stringused=0;
 static int after_attr=0;
 int newlen;
 int doctype_depth=0;

 /* Handle top level entities */

"<!--"                      { BEGIN(COMMENT); }
"<![CDATA["                 { BEGIN(CDATA); }
"<!DOCTYPE"                 { BEGIN(DOCTYPE); doctype_depth=0; }
"</"                        { BEGIN(END_TAG1); }
"<?"                        { BEGIN(XML_DECL_START); }
"<"                         { BEGIN(TAG_START); }
">"                         { return(LEX_ERROR_CLOSE); }
{N}                         { lineno++; }
[^<>]                       { }

 /* Comments */

<COMMENT>"-->"              { BEGIN(INITIAL); }
<COMMENT>"--"[^>]           { return(LEX_ERROR_COMMENT); }
<COMMENT>{N}                { lineno++; }
<COMMENT>[^-]               { }
<COMMENT>"-"                { }

 /* CDATA */

<CDATA>"]]>"                { BEGIN(INITIAL); }
<CDATA>"]"                  { }
<CDATA>{N}                  { lineno++; }
<CDATA>[^]]                 { }

 /* CDATA */

<DOCTYPE>"<"                { doctype_depth++; }
<DOCTYPE>">"                { if(doctype_depth==0) BEGIN(INITIAL); else doctype_depth--; }
<DOCTYPE>{N}                { lineno++; }
<DOCTYPE>[^<>]              { }

 /* XML Declaration start */

<XML_DECL_START>xml         { BEGIN(XML_DECL); yylval=yytext; return(LEX_XML_DECL_BEGIN); }
<XML_DECL_START>{N}         { /* lineno++; */ return(LEX_ERROR_XML_DECL_START); }
<XML_DECL_START>.           { return(LEX_ERROR_XML_DECL_START); }

 /* Tag middle */

<XML_DECL>"?>"              { BEGIN(INITIAL); return(LEX_XML_DECL_FINISH); }
<XML_DECL>{S}+              { }
<XML_DECL>{N}               { lineno++; }
<XML_DECL>{name}            { after_attr=XML_DECL; BEGIN(ATTR_KEY); yylval=yytext; return(LEX_ATTR_KEY); }
<XML_DECL>.                 { return(LEX_ERROR_XML_DECL); }

 /* Any tag start */

<TAG_START>{name}           { BEGIN(TAG); yylval=yytext; return(LEX_TAG_BEGIN); }
<TAG_START>{N}              { /* lineno++; */ return(LEX_ERROR_TAG_START); }
<TAG_START>.                { return(LEX_ERROR_TAG_START); }

 /* End-tag start */

<END_TAG1>{name}            { BEGIN(END_TAG2); yylval=yytext; return(LEX_TAG_POP); }
<END_TAG1>{N}               { /* lineno++; */ return(LEX_ERROR_END_TAG); }
<END_TAG1>.                 { return(LEX_ERROR_END_TAG); }

<END_TAG2>">"               { BEGIN(INITIAL); }
<END_TAG2>{N}               { /* lineno++; */ return(LEX_ERROR_END_TAG); }
<END_TAG2>.                 { return(LEX_ERROR_END_TAG); }

 /* Any tag middle */

<TAG>"/>"                   { BEGIN(INITIAL); return(LEX_TAG_FINISH); }
<TAG>">"                    { BEGIN(INITIAL); return(LEX_TAG_PUSH); }
<TAG>{S}+                   { }
<TAG>{N}                    { lineno++; }
<TAG>{name}                 { after_attr=TAG; BEGIN(ATTR_KEY); yylval=yytext; return(LEX_ATTR_KEY); }
<TAG>.                      { return(LEX_ERROR_TAG); }

 /* Attributes */

<ATTR_KEY>=                 { BEGIN(ATTR_VAL); }
<ATTR_KEY>{N}               { /* lineno++; */ return(LEX_ERROR_ATTR); }
<ATTR_KEY>.                 { return(LEX_ERROR_ATTR); }

<ATTR_VAL>\"                { BEGIN(DQUOTED); reset_string; }
<ATTR_VAL>\'                { BEGIN(SQUOTED); reset_string; }
<ATTR_VAL>{N}               { /* lineno++; */ return(LEX_ERROR_ATTR); }
<ATTR_VAL>.                 { return(LEX_ERROR_ATTR); }

 /* Quoted strings */

<DQUOTED>\"                 { BEGIN(after_attr); yylval=string; return(LEX_ATTR_VAL); }
<DQUOTED>{entityref}        { if(xmlparse_options&XMLPARSE_RETURN_ATTR_ENCODED) {append_string(yytext);}
                              else { const char *str=ParseXML_Decode_Entity_Ref(yytext); if(str) {append_string(str);} else {yylval=yytext; return(LEX_ERROR_ENTITY_REF);} } }
<DQUOTED>{charref}          { if(xmlparse_options&XMLPARSE_RETURN_ATTR_ENCODED) {append_string(yytext);}
                              else { const char *str=ParseXML_Decode_Char_Ref(yytext);   if(str) {append_string(str);} else {yylval=yytext; return(LEX_ERROR_CHAR_REF);} } }
<DQUOTED>[<>&\"]            { yylval=yytext; return(LEX_ERROR_ATTR_VAL); }
<DQUOTED>{UquotedD}+        { append_string(yytext); }
<DQUOTED>.                  { yylval=yytext; return(LEX_ERROR_ATTR_VAL); }

<SQUOTED>\'                 { BEGIN(after_attr); yylval=string; return(LEX_ATTR_VAL); }
<SQUOTED>{entityref}        { if(xmlparse_options&XMLPARSE_RETURN_ATTR_ENCODED) {append_string(yytext);}
                              else { const char *str=ParseXML_Decode_Entity_Ref(yytext); if(str) {append_string(str);} else {yylval=yytext; return(LEX_ERROR_ENTITY_REF);} } }
<SQUOTED>{charref}          { if(xmlparse_options&XMLPARSE_RETURN_ATTR_ENCODED) {append_string(yytext);}
                              else { const char *str=ParseXML_Decode_Char_Ref(yytext);   if(str) {append_string(str);} else {yylval=yytext; return(LEX_ERROR_CHAR_REF);} } }
<SQUOTED>[<>&]              { yylval=yytext; return(LEX_ERROR_ATTR_VAL); }
<SQUOTED>{UquotedS}+        { append_string(yytext); }
<SQUOTED>.                  { yylval=yytext; return(LEX_ERROR_ATTR_VAL); }

 /* End of file */

<<EOF>>                     { free(string); string=NULL; stringlen=stringused=0; BEGIN(INITIAL); return(LEX_EOF); }

%%


/*++++++++++++++++++++++++++++++++++++++
  A function to call the callback function with the parameters needed.

  int call_callback Returns 1 if the callback returned with an error.

  const char *name The name of the tag.

  int (*callback)() The callback function.

  int type The type of tag (start and/or end).

  int nattributes The number of attributes collected.

  char *attributes[XMLPARSE_MAX_ATTRS] The list of attributes.
  ++++++++++++++++++++++++++++++++++++++*/

static inline int call_callback(const char *name,int (*callback)(),int type,int nattributes,char *attributes[XMLPARSE_MAX_ATTRS])
{
 switch(nattributes)
   {
   case  0: return (*callback)(name,type);
   case  1: return (*callback)(name,type,attributes[0]);
   case  2: return (*callback)(name,type,attributes[0],attributes[1]);
   case  3: return (*callback)(name,type,attributes[0],attributes[1],attributes[2]);
   case  4: return (*callback)(name,type,attributes[0],attributes[1],attributes[2],attributes[3]);
   case  5: return (*callback)(name,type,attributes[0],attributes[1],attributes[2],attributes[3],attributes[4]);
   case  6: return (*callback)(name,type,attributes[0],attributes[1],attributes[2],attributes[3],attributes[4],attributes[5]);
   case  7: return (*callback)(name,type,attributes[0],attributes[1],attributes[2],attributes[3],attributes[4],attributes[5],attributes[6]);
   case  8: return (*callback)(name,type,attributes[0],attributes[1],attributes[2],attributes[3],attributes[4],attributes[5],attributes[6],attributes[7]);
   case  9: return (*callback)(name,type,attributes[0],attributes[1],attributes[2],attributes[3],attributes[4],attributes[5],attributes[6],attributes[7],attributes[8]);
   case 10: return (*callback)(name,type,attributes[0],attributes[1],attributes[2],attributes[3],attributes[4],attributes[5],attributes[6],attributes[7],attributes[8],attributes[9]);
   case 11: return (*callback)(name,type,attributes[0],attributes[1],attributes[2],attributes[3],attributes[4],attributes[5],attributes[6],attributes[7],attributes[8],attributes[9],attributes[10]);
   case 12: return (*callback)(name,type,attributes[0],attributes[1],attributes[2],attributes[3],attributes[4],attributes[5],attributes[6],attributes[7],attributes[8],attributes[9],attributes[10],attributes[11]);
   case 13: return (*callback)(name,type,attributes[0],attributes[1],attributes[2],attributes[3],attributes[4],attributes[5],attributes[6],attributes[7],attributes[8],attributes[9],attributes[10],attributes[11],attributes[12]);
   case 14: return (*callback)(name,type,attributes[0],attributes[1],attributes[2],attributes[3],attributes[4],attributes[5],attributes[6],attributes[7],attributes[8],attributes[9],attributes[10],attributes[11],attributes[12],attributes[13]);
   case 15: return (*callback)(name,type,attributes[0],attributes[1],attributes[2],attributes[3],attributes[4],attributes[5],attributes[6],attributes[7],attributes[8],attributes[9],attributes[10],attributes[11],attributes[12],attributes[13],attributes[14]);
   case 16: return (*callback)(name,type,attributes[0],attributes[1],attributes[2],attributes[3],attributes[4],attributes[5],attributes[6],attributes[7],attributes[8],attributes[9],attributes[10],attributes[11],attributes[12],attributes[13],attributes[14],attributes[15]);

   default:
    fprintf(stderr,"XML Parser: Error on line %llu: too many attributes for tag '%s' source code needs changing.\n",lineno,name);
    exit(1);
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Parse the XML and call the functions for each tag as seen.

  int ParseXML Returns 0 if OK or something else in case of an error.

  FILE *file The file to parse.

  xmltag **tags The array of pointers to tags for the top level.

  int options A list of XML Parser options OR-ed together.
  ++++++++++++++++++++++++++++++++++++++*/

int ParseXML(FILE *file,xmltag **tags,int options)
{
 int yychar,i;

 char *attributes[XMLPARSE_MAX_ATTRS]={NULL};
 int attribute=0;

 int stackdepth=0,stackused=0;
 xmltag ***tags_stack=NULL;
 xmltag **tag_stack=NULL;
 xmltag *tag=NULL;

 /* The actual parser. */

 xmlparse_options=options;

 yyin=file;

 yyrestart(yyin);

 lineno=1;

 BEGIN(INITIAL);

 do
   {
    yychar=yylex();

    switch(yychar)
      {
       /* The start of a tag for an XML declaration */

      case LEX_XML_DECL_BEGIN:

       if(tag_stack)
         {
          fprintf(stderr,"XML Parser: Error on line %llu: XML declaration not before all other tags.\n",lineno);
          yychar=LEX_ERROR_XML_NOT_FIRST;
          break;
         }

       /* The start of a tag for an element */

      case LEX_TAG_BEGIN:

       tag=NULL;

       for(i=0;tags[i];i++)
          if(!strcasecmp(yylval,tags[i]->name))
            {
             tag=tags[i];

             for(i=0;i<tag->nattributes;i++)
                if(attributes[i])
                  {
                   free(attributes[i]);
                   attributes[i]=NULL;
                  }

             break;
            }

       if(tag==NULL)
         {
          fprintf(stderr,"XML Parser: Error on line %llu: unexpected tag '%s'.\n",lineno,yylval);
          yychar=LEX_ERROR_UNEXP_TAG;
         }

       break;

       /* The end of the start-tag for an element */

      case LEX_TAG_PUSH:

       if(stackused==stackdepth)
         {
          tag_stack =(xmltag**) realloc((void*)tag_stack ,(stackdepth+=8)*sizeof(xmltag*));
          tags_stack=(xmltag***)realloc((void*)tags_stack,(stackdepth+=8)*sizeof(xmltag**));
         }

       tag_stack [stackused]=tag;
       tags_stack[stackused]=tags;
       stackused++;

       if(tag->callback)
          if(call_callback(tag->name,tag->callback,XMLPARSE_TAG_START,tag->nattributes,attributes))
             yychar=LEX_ERROR_CALLBACK;

       tags=tag->subtags;

       break;

       /* The end of the empty-element-tag for an XML declaration */

      case LEX_XML_DECL_FINISH:

       /* The end of the empty-element-tag for an element */

      case LEX_TAG_FINISH:

       if(tag->callback)
          if(call_callback(tag->name,tag->callback,XMLPARSE_TAG_START|XMLPARSE_TAG_END,tag->nattributes,attributes))
             yychar=LEX_ERROR_CALLBACK;

       if(stackused>0)
          tag=tag_stack[stackused-1];
       else
          tag=NULL;

       break;

       /* The end of the end-tag for an element */

      case LEX_TAG_POP:

       stackused--;
       tags=tags_stack[stackused];
       tag =tag_stack [stackused];

       if(strcmp(tag->name,yylval))
         {
          fprintf(stderr,"XML Parser: Error on line %llu: end tag '</%s>' doesn't match start tag '<%s ...>'.\n",lineno,yylval,tag->name);
          yychar=LEX_ERROR_UNBALANCED;
         }

       if(stackused<0)
         {
          fprintf(stderr,"XML Parser: Error on line %llu: end tag '</%s>' seen but there was no start tag '<%s ...>'.\n",lineno,yylval,yylval);
          yychar=LEX_ERROR_NO_START;
         }

       for(i=0;i<tag->nattributes;i++)
          if(attributes[i])
            {
             free(attributes[i]);
             attributes[i]=NULL;
            }

       if(tag->callback)
          if(call_callback(tag->name,tag->callback,XMLPARSE_TAG_END,tag->nattributes,attributes))
             yychar=LEX_ERROR_CALLBACK;

       if(stackused>0)
          tag=tag_stack[stackused-1];
       else
          tag=NULL;

       break;

       /* An attribute key */

      case LEX_ATTR_KEY:

       attribute=-1;

       for(i=0;i<tag->nattributes;i++)
          if(!strcasecmp(yylval,tag->attributes[i]))
            {
             attribute=i;

             break;
            }

       if(attribute==-1)
         {
          if((options&XMLPARSE_UNKNOWN_ATTRIBUTES)==XMLPARSE_UNKNOWN_ATTR_ERROR ||
             ((options&XMLPARSE_UNKNOWN_ATTRIBUTES)==XMLPARSE_UNKNOWN_ATTR_ERRNONAME && !strchr(yylval,':')))
            {
             fprintf(stderr,"XML Parser: Error on line %llu: unexpected attribute '%s' for tag '%s'.\n",lineno,yylval,tag->name);
             yychar=LEX_ERROR_UNEXP_ATT;
            }
          else if((options&XMLPARSE_UNKNOWN_ATTRIBUTES)==XMLPARSE_UNKNOWN_ATTR_WARN)
             fprintf(stderr,"XML Parser: Warning on line %llu: unexpected attribute '%s' for tag '%s'.\n",lineno,yylval,tag->name);
         }

       break;

       /* An attribute value */

      case LEX_ATTR_VAL:

       if(tag->callback && attribute!=-1 && yylval)
          attributes[attribute]=strcpy(malloc(strlen(yylval)+1),yylval);

       break;

       /* End of file */

      case LEX_EOF:

       if(tag)
         {
          fprintf(stderr,"XML Parser: Error on line %llu: end of file seen without end tag '</%s>'.\n",lineno,tag->name);
          yychar=LEX_ERROR_UNEXP_EOF;
         }

       break;

      case LEX_ERROR_TAG_START:
       fprintf(stderr,"XML Parser: Error on line %llu: character '<' seen not at start of tag.\n",lineno);
       break;

      case LEX_ERROR_XML_DECL_START:
       fprintf(stderr,"XML Parser: Error on line %llu: characters '<?' seen not at start of XML declaration.\n",lineno);
       break;

      case LEX_ERROR_TAG:
       fprintf(stderr,"XML Parser: Error on line %llu: invalid character seen inside tag '<%s...>'.\n",lineno,tag->name);
       break;

      case LEX_ERROR_XML_DECL:
       fprintf(stderr,"XML Parser: Error on line %llu: invalid character seen inside XML declaration '<?%s...>'.\n",lineno,tag->name);
       break;

      case LEX_ERROR_ATTR:
       fprintf(stderr,"XML Parser: Error on line %llu: invalid attribute definition seen in tag.\n",lineno);
       break;

      case LEX_ERROR_END_TAG:
       fprintf(stderr,"XML Parser: Error on line %llu: invalid character seen in end-tag.\n",lineno);
       break;

      case LEX_ERROR_COMMENT:
       fprintf(stderr,"XML Parser: Error on line %llu: invalid comment seen.\n",lineno);
       break;

      case LEX_ERROR_CLOSE:
       fprintf(stderr,"XML Parser: Error on line %llu: character '>' seen not at end of tag.\n",lineno);
       break;

      case LEX_ERROR_ATTR_VAL:
       fprintf(stderr,"XML Parser: Error on line %llu: invalid character '%s' seen in attribute value.\n",lineno,yylval);
       break;

      case LEX_ERROR_ENTITY_REF:
       fprintf(stderr,"XML Parser: Error on line %llu: invalid entity reference '%s' seen in attribute value.\n",lineno,yylval);
       break;

      case LEX_ERROR_CHAR_REF:
       fprintf(stderr,"XML Parser: Error on line %llu: invalid character reference '%s' seen in attribute value.\n",lineno,yylval);
       break;
      }
   }
 while(yychar>LEX_EOF && yychar<LEX_ERROR);

 /* Delete the tagdata */

 for(i=0;i<XMLPARSE_MAX_ATTRS;i++)
    if(attributes[i])
       free(attributes[i]);

 if(stackdepth)
   {
    free(tag_stack);
    free(tags_stack);
   }

 return(yychar);
}


/*++++++++++++++++++++++++++++++++++++++
  Return the current parser line number.

  unsigned long long ParseXML_LineNumber Returns the line number.
  ++++++++++++++++++++++++++++++++++++++*/

unsigned long long ParseXML_LineNumber(void)
{
 return(lineno);
}


/*++++++++++++++++++++++++++++++++++++++
  Convert an XML entity reference into an ASCII string.

  char *ParseXML_Decode_Entity_Ref Returns a pointer to the replacement decoded string.

  const char *string The entity reference string.
  ++++++++++++++++++++++++++++++++++++++*/

char *ParseXML_Decode_Entity_Ref(const char *string)
{
 if(!strcmp(string,"&amp;"))  return("&");
 if(!strcmp(string,"&lt;"))   return("<");
 if(!strcmp(string,"&gt;"))   return(">");
 if(!strcmp(string,"&apos;")) return("'");
 if(!strcmp(string,"&quot;")) return("\"");
 return(NULL);
}


/*++++++++++++++++++++++++++++++++++++++
  Convert an XML character reference into an ASCII string.

  char *ParseXML_Decode_Char_Ref Returns a pointer to the replacement decoded string.

  const char *string The character reference string.
  ++++++++++++++++++++++++++++++++++++++*/

char *ParseXML_Decode_Char_Ref(const char *string)
{
 static char result[5]="";
 long int unicode;

 if(string[2]=='x') unicode=strtol(string+3,NULL,16);
 else               unicode=strtol(string+2,NULL,10);

 if(unicode<0x80)
   {
    /* 0000 0000-0000 007F  =>  0xxxxxxx */
    result[0]=unicode;
    result[1]=0;
   }
 else if(unicode<0x07FF)
   {
    /* 0000 0080-0000 07FF  =>  110xxxxx 10xxxxxx */
    result[0]=0xC0+((unicode&0x07C0)>>6);
    result[1]=0x80+ (unicode&0x003F);
    result[2]=0;
   }
 else if(unicode<0xFFFF)
   {
    /* 0000 0800-0000 FFFF  =>  1110xxxx 10xxxxxx 10xxxxxx */
    result[0]=0xE0+((unicode&0xF000)>>12);
    result[1]=0x80+((unicode&0x0FC0)>>6);
    result[2]=0x80+ (unicode&0x003F);
    result[3]=0;
   }
 else if(unicode<0x1FFFFF)
   {
    /* 0001 0000-001F FFFF  =>  11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
    result[0]=0xF0+((unicode&0x1C0000)>>18);
    result[1]=0x80+((unicode&0x03F000)>>12);
    result[2]=0x80+((unicode&0x000FC0)>>6);
    result[3]=0x80+ (unicode&0x00003F);
    result[4]=0;
   }
 else
   {
    result[0]=0xFF;
    result[1]=0xFD;
    result[2]=0;
   }

 return(result);
}


/*++++++++++++++++++++++++++++++++++++++
  Convert a string into something that is safe to output in an XML file.

  char *ParseXML_Encode_Safe_XML Returns a pointer to the replacement encoded string (or the original if no change needed).

  const char *string The string to convert.
  ++++++++++++++++++++++++++++++++++++++*/

char *ParseXML_Encode_Safe_XML(const char *string)
{
 static const char hexstring[17]="0123456789ABCDEF";
 int i=0,j=0,len;
 char *result;

 for(i=0;string[i];i++)
    if(string[i]=='<' || string[i]=='>' || string[i]=='&' || string[i]=='\'' || string[i]=='"' || string[i]<32 || (unsigned char)string[i]>127)
       break;

 if(!string[i])
    return((char*)string);

 len=i+256-6;

 result=(char*)malloc(len+7);
 strncpy(result,string,j=i);

 do
   {
    for(;j<len && string[i];i++)
       if(string[i]=='<')
         {
          result[j++]='&';
          result[j++]='l';
          result[j++]='t';
          result[j++]=';';
         }
       else if(string[i]=='>')
         {
          result[j++]='&';
          result[j++]='g';
          result[j++]='t';
          result[j++]=';';
         }
       else if(string[i]=='&')
         {
          result[j++]='&';
          result[j++]='a';
          result[j++]='m';
          result[j++]='p';
          result[j++]=';';
         }
       else if(string[i]=='\'')
         {
          result[j++]='&';
          result[j++]='a';
          result[j++]='p';
          result[j++]='o';
          result[j++]='s';
          result[j++]=';';
         }
       else if(string[i]=='"')
         {
          result[j++]='&';
          result[j++]='q';
          result[j++]='u';
          result[j++]='o';
          result[j++]='t';
          result[j++]=';';
         }
       else if(string[i]>=32 && (unsigned char)string[i]<=127)
          result[j++]=string[i];
       else
         {
          unsigned int unicode;

          /* Decode the UTF-8 */

          if((string[i]&0x80)==0)
            {
             /* 0000 0000-0000 007F  =>  0xxxxxxx */
             unicode=string[i];
            }
          else if((string[i]&0xE0)==0xC0 && (string[i]&0x1F)>=2 && (string[i+1]&0xC0)==0x80)
            {
             /* 0000 0080-0000 07FF  =>  110xxxxx 10xxxxxx */
             unicode =(string[i++]&0x1F)<<6;
             unicode|= string[i  ]&0x3F;
            }
          else if((string[i]&0xF0)==0xE0 && (string[i+1]&0xC0)==0x80 && (string[i+2]&0xC0)==0x80)
            {
             /* 0000 0800-0000 FFFF  =>  1110xxxx 10xxxxxx 10xxxxxx */
             unicode =(string[i++]&0x0F)<<12;
             unicode|=(string[i++]&0x3F)<<6;
             unicode|= string[i  ]&0x3F;
            }
          else if((string[i]&0xF8)==0xF0 && (string[i+1]&0xC0)==0x80 && (string[i+2]&0xC0)==0x80 && (string[i+3]&0xC0)==0x80)
            {
             /* 0001 0000-001F FFFF  =>  11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
             unicode =(string[i++]&0x07)<<18;
             unicode|=(string[i++]&0x3F)<<12;
             unicode|=(string[i++]&0x3F)<<6;
             unicode|= string[i  ]&0x3F;
            }
          else
             unicode=0xFFFD;

          /* Output the character entity */

          result[j++]='&';
          result[j++]='#';
          result[j++]='x';

          if(unicode&0x00FF0000)
            {
             result[j++]=hexstring[((unicode>>16)&0xf0)>>4];
             result[j++]=hexstring[((unicode>>16)&0x0f)   ];
            }
          if(unicode&0x00FFFF00)
            {
             result[j++]=hexstring[((unicode>>8)&0xf0)>>4];
             result[j++]=hexstring[((unicode>>8)&0x0f)   ];
            }
          result[j++]=hexstring[(unicode&0xf0)>>4];
          result[j++]=hexstring[(unicode&0x0f)   ];

          result[j++]=';';
         }

    if(string[i])                  /* Not finished */
      {
       len+=256;
       result=(char*)realloc((void*)result,len+7);
      }
   }
 while(string[i]);

 result[j]=0;

 return(result);
}


/*++++++++++++++++++++++++++++++++++++++
  Check that a string really is an integer.

  int ParseXML_IsInteger Returns 1 if an integer could be found or 0 otherwise.

  const char *string The string to be parsed.
  ++++++++++++++++++++++++++++++++++++++*/

int ParseXML_IsInteger(const char *string)
{
 const char *p=string;

 if(*p=='-' || *p=='+')
    p++;

 while(isdigit(*p))
    p++;

 if(*p)
    return(0);
 else
    return(1);
}


/*++++++++++++++++++++++++++++++++++++++
  Check that a string really is a floating point number.

  int ParseXML_IsFloating Returns 1 if a floating point number could be found or 0 otherwise.

  const char *string The string to be parsed.
  ++++++++++++++++++++++++++++++++++++++*/

int ParseXML_IsFloating(const char *string)
{
 const char *p=string;

 if(*p=='-' || *p=='+')
    p++;

 while(isdigit(*p) || *p=='.')
    p++;

 if(*p=='e' || *p=='E')
   {
    p++;

    if(*p=='-' || *p=='+')
       p++;

    while(isdigit(*p))
       p++;
   }

 if(*p)
    return(0);
 else
    return(1);
}
