/***************************************
 Load the tagging rules from a file and the functions for handling them.

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2010-2012 Andrew M. Bishop

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
#include <string.h>
#include <stdlib.h>

#include "files.h"
#include "tagging.h"
#include "xmlparse.h"
#include "logging.h"


/* Global variables */

TaggingRuleList NodeRules={NULL,0};
TaggingRuleList WayRules={NULL,0};
TaggingRuleList RelationRules={NULL,0};


/* Local variables */

TaggingRuleList *current_list=NULL;
TaggingRule     *current_rule=NULL;


/* Local functions */

static void apply_actions(TaggingRuleList *rules,TaggingRule *rule,int match,TagList *input,TagList *output,node_t id);


/* The XML tag processing function prototypes */

//static int xmlDeclaration_function(const char *_tag_,int _type_,const char *version,const char *encoding);
//static int RoutinoTaggingType_function(const char *_tag_,int _type_);
static int RelationType_function(const char *_tag_,int _type_);
static int WayType_function(const char *_tag_,int _type_);
static int NodeType_function(const char *_tag_,int _type_);
static int IfType_function(const char *_tag_,int _type_,const char *k,const char *v);
static int LogErrorType_function(const char *_tag_,int _type_,const char *k,const char *v);
static int OutputType_function(const char *_tag_,int _type_,const char *k,const char *v);
static int UnsetType_function(const char *_tag_,int _type_,const char *k);
static int SetType_function(const char *_tag_,int _type_,const char *k,const char *v);


/* The XML tag definitions */

/*+ The SetType type tag. +*/
static xmltag SetType_tag=
              {"set",
               2, {"k","v"},
               SetType_function,
               {NULL}};

/*+ The UnsetType type tag. +*/
static xmltag UnsetType_tag=
              {"unset",
               1, {"k"},
               UnsetType_function,
               {NULL}};

/*+ The OutputType type tag. +*/
static xmltag OutputType_tag=
              {"output",
               2, {"k","v"},
               OutputType_function,
               {NULL}};

/*+ The LogErrorType type tag. +*/
static xmltag LogErrorType_tag=
              {"logerror",
               2, {"k","v"},
               LogErrorType_function,
               {NULL}};

/*+ The IfType type tag. +*/
static xmltag IfType_tag=
              {"if",
               2, {"k","v"},
               IfType_function,
               {&SetType_tag,&UnsetType_tag,&OutputType_tag,&LogErrorType_tag,NULL}};

/*+ The NodeType type tag. +*/
static xmltag NodeType_tag=
              {"node",
               0, {NULL},
               NodeType_function,
               {&IfType_tag,NULL}};

/*+ The WayType type tag. +*/
static xmltag WayType_tag=
              {"way",
               0, {NULL},
               WayType_function,
               {&IfType_tag,NULL}};

/*+ The RelationType type tag. +*/
static xmltag RelationType_tag=
              {"relation",
               0, {NULL},
               RelationType_function,
               {&IfType_tag,NULL}};

/*+ The RoutinoTaggingType type tag. +*/
static xmltag RoutinoTaggingType_tag=
              {"routino-tagging",
               0, {NULL},
               NULL,
               {&NodeType_tag,&WayType_tag,&RelationType_tag,NULL}};

/*+ The xmlDeclaration type tag. +*/
static xmltag xmlDeclaration_tag=
              {"xml",
               2, {"version","encoding"},
               NULL,
               {NULL}};


/*+ The complete set of tags at the top level. +*/
static xmltag *xml_toplevel_tags[]={&xmlDeclaration_tag,&RoutinoTaggingType_tag,NULL};


/* The XML tag processing functions */


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the SetType XSD type is seen

  int SetType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *k The contents of the 'k' attribute (or NULL if not defined).

  const char *v The contents of the 'v' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int SetType_function(const char *_tag_,int _type_,const char *k,const char *v)
{
 if(_type_&XMLPARSE_TAG_START)
    AppendTaggingAction(current_rule,k,v,TAGACTION_SET);

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the UnsetType XSD type is seen

  int UnsetType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *k The contents of the 'k' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int UnsetType_function(const char *_tag_,int _type_,const char *k)
{
 if(_type_&XMLPARSE_TAG_START)
    AppendTaggingAction(current_rule,k,NULL,TAGACTION_UNSET);

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the OutputType XSD type is seen

  int OutputType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *k The contents of the 'k' attribute (or NULL if not defined).

  const char *v The contents of the 'v' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int OutputType_function(const char *_tag_,int _type_,const char *k,const char *v)
{
 if(_type_&XMLPARSE_TAG_START)
    AppendTaggingAction(current_rule,k,v,TAGACTION_OUTPUT);

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the LogErrorType XSD type is seen

  int LogErrorType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *k The contents of the 'k' attribute (or NULL if not defined).

  const char *v The contents of the 'v' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int LogErrorType_function(const char *_tag_,int _type_,const char *k,const char *v)
{
 if(_type_&XMLPARSE_TAG_START)
    AppendTaggingAction(current_rule,k,v,TAGACTION_LOGERROR);

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the IfType XSD type is seen

  int IfType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *k The contents of the 'k' attribute (or NULL if not defined).

  const char *v The contents of the 'v' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int IfType_function(const char *_tag_,int _type_,const char *k,const char *v)
{
 if(_type_&XMLPARSE_TAG_START)
   {
    current_rule=AppendTaggingRule(current_list,k,v);
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the NodeType XSD type is seen

  int NodeType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.
  ++++++++++++++++++++++++++++++++++++++*/

static int NodeType_function(const char *_tag_,int _type_)
{
 if(_type_&XMLPARSE_TAG_START)
    current_list=&NodeRules;

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the WayType XSD type is seen

  int WayType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.
  ++++++++++++++++++++++++++++++++++++++*/

static int WayType_function(const char *_tag_,int _type_)
{
 if(_type_&XMLPARSE_TAG_START)
    current_list=&WayRules;

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the RelationType XSD type is seen

  int RelationType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.
  ++++++++++++++++++++++++++++++++++++++*/

static int RelationType_function(const char *_tag_,int _type_)
{
 if(_type_&XMLPARSE_TAG_START)
    current_list=&RelationRules;

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the RoutinoTaggingType XSD type is seen

  int RoutinoTaggingType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.
  ++++++++++++++++++++++++++++++++++++++*/

//static int RoutinoTaggingType_function(const char *_tag_,int _type_)
//{
// return(0);
//}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the XML declaration is seen

  int xmlDeclaration_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *version The contents of the 'version' attribute (or NULL if not defined).

  const char *encoding The contents of the 'encoding' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

//static int xmlDeclaration_function(const char *_tag_,int _type_,const char *version,const char *encoding)
//{
// return(0);
//}


/*++++++++++++++++++++++++++++++++++++++
  The XML tagging rules parser.

  int ParseXMLTaggingRules Returns 0 if OK or something else in case of an error.

  const char *filename The name of the file to read.
  ++++++++++++++++++++++++++++++++++++++*/

int ParseXMLTaggingRules(const char *filename)
{
 FILE *file;
 int retval;

 if(!ExistsFile(filename))
   {
    fprintf(stderr,"Error: Specified tagging rules file '%s' does not exist.\n",filename);
    return(1);
   }

 file=fopen(filename,"r");

 if(!file)
   {
    fprintf(stderr,"Error: Cannot open tagging rules file '%s' for reading.\n",filename);
    return(1);
   }

 retval=ParseXML(file,xml_toplevel_tags,XMLPARSE_UNKNOWN_ATTR_ERRNONAME);

 fclose(file);

 if(retval)
    return(1);

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  Delete the tagging rules loaded from the XML file.
  ++++++++++++++++++++++++++++++++++++++*/

void DeleteXMLTaggingRules(void)
{
 DeleteTaggingRuleList(&NodeRules);
 DeleteTaggingRuleList(&WayRules);
 DeleteTaggingRuleList(&RelationRules);
}


/*++++++++++++++++++++++++++++++++++++++
  Append a tagging rule to the list of rules.

  TaggingRule *AppendTaggingRule Returns the latest rule (the just added one).

  TaggingRuleList *rules The list of rules to add to.

  const char *k The tag key.

  const char *v The tag value.
  ++++++++++++++++++++++++++++++++++++++*/

TaggingRule *AppendTaggingRule(TaggingRuleList *rules,const char *k,const char *v)
{
 if((rules->nrules%16)==0)
    rules->rules=(TaggingRule*)realloc((void*)rules->rules,(rules->nrules+16)*sizeof(TaggingRule));

 rules->nrules++;

 if(k)
    rules->rules[rules->nrules-1].k=strcpy(malloc(strlen(k)+1),k);
 else
    rules->rules[rules->nrules-1].k=NULL;

 if(v)
    rules->rules[rules->nrules-1].v=strcpy(malloc(strlen(v)+1),v);
 else
    rules->rules[rules->nrules-1].v=NULL;

 rules->rules[rules->nrules-1].nactions=0;
 rules->rules[rules->nrules-1].actions=NULL;

 return(&rules->rules[rules->nrules-1]);
}


/*++++++++++++++++++++++++++++++++++++++
  Append a tagging action to a tagging rule.

  TaggingRule *rule The rule to add the action to.

  const char *k The tag key.

  const char *v The tag value.

  int action Set to the type of action.
  ++++++++++++++++++++++++++++++++++++++*/

void AppendTaggingAction(TaggingRule *rule,const char *k,const char *v,int action)
{
 if((rule->nactions%16)==0)
    rule->actions=(TaggingAction*)realloc((void*)rule->actions,(rule->nactions+16)*sizeof(TaggingAction));

 rule->nactions++;

 rule->actions[rule->nactions-1].action=action;

 if(k)
    rule->actions[rule->nactions-1].k=strcpy(malloc(strlen(k)+1),k);
 else
    rule->actions[rule->nactions-1].k=NULL;

 if(v)
    rule->actions[rule->nactions-1].v=strcpy(malloc(strlen(v)+1),v);
 else
    rule->actions[rule->nactions-1].v=NULL;
}


/*++++++++++++++++++++++++++++++++++++++
  Delete a tagging rule.

  TaggingRuleList *rules The list of rules to be deleted.
  ++++++++++++++++++++++++++++++++++++++*/

void DeleteTaggingRuleList(TaggingRuleList *rules)
{
 int i,j;

 for(i=0;i<rules->nrules;i++)
   {
    if(rules->rules[i].k)
       free(rules->rules[i].k);
    if(rules->rules[i].v)
       free(rules->rules[i].v);

    for(j=0;j<rules->rules[i].nactions;j++)
      {
       if(rules->rules[i].actions[j].k)
          free(rules->rules[i].actions[j].k);
       if(rules->rules[i].actions[j].v)
          free(rules->rules[i].actions[j].v);
      }

    if(rules->rules[i].actions)
       free(rules->rules[i].actions);
   }

 if(rules->rules)
    free(rules->rules);
}


/*++++++++++++++++++++++++++++++++++++++
  Create a new TagList structure.

  TagList *NewTagList Returns the new allocated TagList.
  ++++++++++++++++++++++++++++++++++++++*/

TagList *NewTagList(void)
{
 return((TagList*)calloc(sizeof(TagList),1));
}


/*++++++++++++++++++++++++++++++++++++++
  Delete a tag list and the contents.

  TagList *tags The list of tags to delete.
  ++++++++++++++++++++++++++++++++++++++*/

void DeleteTagList(TagList *tags)
{
 int i;

 for(i=0;i<tags->ntags;i++)
   {
    if(tags->k[i]) free(tags->k[i]);
    if(tags->v[i]) free(tags->v[i]);
   }

 if(tags->ntags)
   {
    free(tags->k);
    free(tags->v);
   }

 free(tags);
}


/*++++++++++++++++++++++++++++++++++++++
  Append a tag to the list of tags.

  TagList *tags The list of tags to add to.

  const char *k The tag key.

  const char *v The tag value.
  ++++++++++++++++++++++++++++++++++++++*/

void AppendTag(TagList *tags,const char *k,const char *v)
{
 if((tags->ntags%16)==0)
   {
    int i;

    tags->k=(char**)realloc((void*)tags->k,(tags->ntags+16)*sizeof(char*));
    tags->v=(char**)realloc((void*)tags->v,(tags->ntags+16)*sizeof(char*));

    for(i=tags->ntags;i<(tags->ntags+16);i++)
       tags->k[i]=tags->v[i]=NULL;
   }

 tags->k[tags->ntags]=strcpy(realloc(tags->k[tags->ntags],strlen(k)+1),k);
 tags->v[tags->ntags]=strcpy(realloc(tags->v[tags->ntags],strlen(v)+1),v);

 tags->ntags++;
}


/*++++++++++++++++++++++++++++++++++++++
  Modify an existing tag or append a new tag to the list of tags.

  TagList *tags The list of tags to modify.

  const char *k The tag key.

  const char *v The tag value.
  ++++++++++++++++++++++++++++++++++++++*/

void ModifyTag(TagList *tags,const char *k,const char *v)
{
 int i;

 for(i=0;i<tags->ntags;i++)
    if(!strcmp(tags->k[i],k))
      {
       tags->v[i]=strcpy(realloc(tags->v[i],strlen(v)+1),v);
       return;
      }

 AppendTag(tags,k,v);
}


/*++++++++++++++++++++++++++++++++++++++
  Delete an existing tag from the list of tags.

  TagList *tags The list of tags to modify.

  const char *k The tag key.
  ++++++++++++++++++++++++++++++++++++++*/

void DeleteTag(TagList *tags,const char *k)
{
 int i,j;

 for(i=0;i<tags->ntags;i++)
    if(!strcmp(tags->k[i],k))
      {
       if(tags->k[i]) free(tags->k[i]);
       if(tags->v[i]) free(tags->v[i]);

       for(j=i+1;j<tags->ntags;j++)
         {
          tags->k[j-1]=tags->k[j];
          tags->v[j-1]=tags->v[j];
         }

       tags->ntags--;

       tags->k[tags->ntags]=NULL;
       tags->v[tags->ntags]=NULL;

       return;
      }
}


/*++++++++++++++++++++++++++++++++++++++
  Apply a set of tagging rules to a set of tags.

  TagList *ApplyTaggingRules Returns the list of output tags after modification.

  TaggingRuleList *rules The tagging rules to apply.

  TagList *tags The tags to be modified.

  node_t id The ID of the node, way or relation.
  ++++++++++++++++++++++++++++++++++++++*/

TagList *ApplyTaggingRules(TaggingRuleList *rules,TagList *tags,node_t id)
{
 TagList *result=NewTagList();
 int i,j;

 for(i=0;i<rules->nrules;i++)
   {
    if(rules->rules[i].k && rules->rules[i].v)
      {
       for(j=0;j<tags->ntags;j++)
          if(!strcmp(tags->k[j],rules->rules[i].k) && !strcmp(tags->v[j],rules->rules[i].v))
             apply_actions(rules,&rules->rules[i],j,tags,result,id);
      }
    else if(rules->rules[i].k && !rules->rules[i].v)
      {
       for(j=0;j<tags->ntags;j++)
          if(!strcmp(tags->k[j],rules->rules[i].k))
             apply_actions(rules,&rules->rules[i],j,tags,result,id);
      }
    else if(!rules->rules[i].k && rules->rules[i].v)
      {
       for(j=0;j<tags->ntags;j++)
          if(!strcmp(tags->v[j],rules->rules[i].v))
             apply_actions(rules,&rules->rules[i],j,tags,result,id);
      }
    else /* if(!rules->rules[i].k && !rules->rules[i].v) */
      {
       for(j=0;j<tags->ntags;j++)
          apply_actions(rules,&rules->rules[i],j,tags,result,id);
      }
   }

 return(result);
}


/*++++++++++++++++++++++++++++++++++++++
  Apply a set of actions to a matching tag.

  TaggingRuleList *rules The tagging rules to apply.

  TaggingRule *rule The rule that matched (containing the actions).

  int match The matching tag number.

  TagList *input The input tags.

  TagList *output The output tags.

  node_t id The ID of the node, way or relation.
  ++++++++++++++++++++++++++++++++++++++*/

static void apply_actions(TaggingRuleList *rules,TaggingRule *rule,int match,TagList *input,TagList *output,node_t id)
{
 int i;
 
 for(i=0;i<rule->nactions;i++)
   {
    char *k,*v;

    if(rule->actions[i].k)
       k=rule->actions[i].k;
    else
       k=input->k[match];

    if(rule->actions[i].v)
       v=rule->actions[i].v;
    else
       v=input->v[match];

    if(rule->actions[i].action==TAGACTION_SET)
       ModifyTag(input,k,v);
    if(rule->actions[i].action==TAGACTION_UNSET)
       DeleteTag(input,k);
    if(rule->actions[i].action==TAGACTION_OUTPUT)
       ModifyTag(output,k,v);
    if(rule->actions[i].action==TAGACTION_LOGERROR)
      {
       if(rules==&NodeRules)
          logerror("Node %"Pnode_t" has an unrecognised tag value '%s' = '%s' (in tagging rules).\n",id,k,v);
       if(rules==&WayRules)
          logerror("Way %"Pway_t" has an unrecognised tag value '%s' = '%s' (in tagging rules).\n",id,k,v);
       if(rules==&RelationRules)
          logerror("Relation %"Prelation_t" has an unrecognised tag value '%s' = '%s' (in tagging rules).\n",id,k,v);
      }
   }
}
