/***************************************
 The data types for the tagging rules.

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

#ifndef TAGGING_H
#define TAGGING_H    /*+ To stop multiple inclusions. +*/

#include "typesx.h"


/* Constants */

#define TAGACTION_SET      0
#define TAGACTION_UNSET    1
#define TAGACTION_OUTPUT   2
#define TAGACTION_LOGERROR 3


/* Data types */

/*+ A structure to contain the tagging action. +*/
typedef struct _TaggingAction
{
 int action;                    /*+ A flag to indicate the type of action. +*/

 char *k;                       /*+ The tag key (or NULL). +*/
 char *v;                       /*+ The tag value (or NULL). +*/
}
 TaggingAction;


/*+ A structure to contain the tagging rule. +*/
typedef struct _TaggingRule
{
 char *k;                       /*+ The tag key (or NULL). +*/
 char *v;                       /*+ The tag value (or NULL). +*/

 TaggingAction *actions;        /*+ The actions to take. +*/
 int            nactions;       /*+ The number of actions. +*/
}
 TaggingRule;


/*+ A structure to contain the list of rules and associated information. +*/
typedef struct _TaggingRuleList
{
 TaggingRule *rules;            /*+ The array of rules. +*/
 int          nrules;           /*+ The number of rules. +*/
}
 TaggingRuleList;


/*+ A structure to hold a list of tags to be processed. +*/
typedef struct _TagList
{
 int ntags;                     /*+ The number of tags. +*/

 char **k;                      /*+ The list of tag keys. +*/
 char **v;                      /*+ The list of tag values. +*/
}
 TagList;


/* Global variables */

extern TaggingRuleList NodeRules;
extern TaggingRuleList WayRules;
extern TaggingRuleList RelationRules;


/* Functions in tagging.c */

int ParseXMLTaggingRules(const char *filename);
void DeleteXMLTaggingRules(void);

TaggingRule *AppendTaggingRule(TaggingRuleList *rules,const char *k,const char *v);
void AppendTaggingAction(TaggingRule *rule,const char *k,const char *v,int action);
void DeleteTaggingRuleList(TaggingRuleList *rules);

TagList *NewTagList(void);
void DeleteTagList(TagList *tags);

void AppendTag(TagList *tags,const char *k,const char *v);
void ModifyTag(TagList *tags,const char *k,const char *v);
void DeleteTag(TagList *tags,const char *k);

TagList *ApplyTaggingRules(TaggingRuleList *rules,TagList *tags,node_t id);


#endif /* TAGGING_H */
