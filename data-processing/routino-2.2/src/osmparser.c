/***************************************
 OSM XML file parser (either JOSM or planet)

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2008-2012 Andrew M. Bishop

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


#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "typesx.h"
#include "nodesx.h"
#include "segmentsx.h"
#include "waysx.h"
#include "relationsx.h"

#include "osmparser.h"

#include "xmlparse.h"
#include "tagging.h"

#include "logging.h"


/* Macros */

/*+ Checks if a value in the XML is one of the allowed values for true. +*/
#define ISTRUE(xx)  (!strcmp(xx,"true") || !strcmp(xx,"yes") || !strcmp(xx,"1"))

/*+ Checks if a value in the XML is one of the allowed values for false. +*/
#define ISFALSE(xx) (!strcmp(xx,"false") || !strcmp(xx,"no") || !strcmp(xx,"0"))


/* Local variables */

static index_t nnodes=0;
static index_t nways=0;
static index_t nrelations=0;

static TagList *current_tags=NULL;

static node_t *way_nodes=NULL;
static int     way_nnodes=0;

static node_t     *relation_nodes=NULL;
static int         relation_nnodes=0;
static way_t      *relation_ways=NULL;
static int         relation_nways=0;
static relation_t *relation_relations=NULL;
static int         relation_nrelations=0;
static way_t       relation_from=NO_WAY_ID;
static way_t       relation_to=NO_WAY_ID;
static node_t      relation_via=NO_NODE_ID;

static NodesX     *nodes;
static SegmentsX  *segments;
static WaysX      *ways;
static RelationsX *relations;


/* Local functions */

static void process_node_tags(TagList *tags,node_t id,double latitude,double longitude);
static void process_way_tags(TagList *tags,way_t id);
static void process_relation_tags(TagList *tags,relation_t id);

static double parse_speed(way_t id,const char *k,const char *v);
static double parse_weight(way_t id,const char *k,const char *v);
static double parse_length(way_t id,const char *k,const char *v);


/* The XML tag processing function prototypes */

//static int xmlDeclaration_function(const char *_tag_,int _type_,const char *version,const char *encoding);
static int osmType_function(const char *_tag_,int _type_,const char *version);
static int relationType_function(const char *_tag_,int _type_,const char *id);
static int wayType_function(const char *_tag_,int _type_,const char *id);
static int memberType_function(const char *_tag_,int _type_,const char *type,const char *ref,const char *role);
static int ndType_function(const char *_tag_,int _type_,const char *ref);
static int nodeType_function(const char *_tag_,int _type_,const char *id,const char *lat,const char *lon);
static int tagType_function(const char *_tag_,int _type_,const char *k,const char *v);
//static int boundType_function(const char *_tag_,int _type_);
//static int boundsType_function(const char *_tag_,int _type_);


/* The XML tag definitions */

/*+ The boundsType type tag. +*/
static xmltag boundsType_tag=
              {"bounds",
               0, {NULL},
               NULL,
               {NULL}};

/*+ The boundType type tag. +*/
static xmltag boundType_tag=
              {"bound",
               0, {NULL},
               NULL,
               {NULL}};

/*+ The tagType type tag. +*/
static xmltag tagType_tag=
              {"tag",
               2, {"k","v"},
               tagType_function,
               {NULL}};

/*+ The nodeType type tag. +*/
static xmltag nodeType_tag=
              {"node",
               3, {"id","lat","lon"},
               nodeType_function,
               {&tagType_tag,NULL}};

/*+ The ndType type tag. +*/
static xmltag ndType_tag=
              {"nd",
               1, {"ref"},
               ndType_function,
               {NULL}};

/*+ The memberType type tag. +*/
static xmltag memberType_tag=
              {"member",
               3, {"type","ref","role"},
               memberType_function,
               {NULL}};

/*+ The wayType type tag. +*/
static xmltag wayType_tag=
              {"way",
               1, {"id"},
               wayType_function,
               {&ndType_tag,&tagType_tag,NULL}};

/*+ The relationType type tag. +*/
static xmltag relationType_tag=
              {"relation",
               1, {"id"},
               relationType_function,
               {&memberType_tag,&tagType_tag,NULL}};

/*+ The osmType type tag. +*/
static xmltag osmType_tag=
              {"osm",
               1, {"version"},
               osmType_function,
               {&boundsType_tag,&boundType_tag,&nodeType_tag,&wayType_tag,&relationType_tag,NULL}};

/*+ The xmlDeclaration type tag. +*/
static xmltag xmlDeclaration_tag=
              {"xml",
               2, {"version","encoding"},
               NULL,
               {NULL}};


/*+ The complete set of tags at the top level. +*/
static xmltag *xml_toplevel_tags[]={&xmlDeclaration_tag,&osmType_tag,NULL};


/* The XML tag processing functions */


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the boundsType XSD type is seen

  int boundsType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.
  ++++++++++++++++++++++++++++++++++++++*/

//static int boundsType_function(const char *_tag_,int _type_)
//{
// return(0);
//}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the boundType XSD type is seen

  int boundType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.
  ++++++++++++++++++++++++++++++++++++++*/

//static int boundType_function(const char *_tag_,int _type_)
//{
// return(0);
//}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the tagType XSD type is seen

  int tagType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *k The contents of the 'k' attribute (or NULL if not defined).

  const char *v The contents of the 'v' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int tagType_function(const char *_tag_,int _type_,const char *k,const char *v)
{
 if(_type_&XMLPARSE_TAG_START && current_tags)
   {
    XMLPARSE_ASSERT_STRING(_tag_,k);
    XMLPARSE_ASSERT_STRING(_tag_,v);

    AppendTag(current_tags,k,v);
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the nodeType XSD type is seen

  int nodeType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *id The contents of the 'id' attribute (or NULL if not defined).

  const char *lat The contents of the 'lat' attribute (or NULL if not defined).

  const char *lon The contents of the 'lon' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int nodeType_function(const char *_tag_,int _type_,const char *id,const char *lat,const char *lon)
{
 static node_t node_id;
 static double latitude,longitude;

 if(_type_&XMLPARSE_TAG_START)
   {
    long long llid;

    nnodes++;

    if(!(nnodes%10000))
       printf_middle("Reading: Lines=%llu Nodes=%"Pindex_t" Ways=%"Pindex_t" Relations=%"Pindex_t,ParseXML_LineNumber(),nnodes,nways,nrelations);

    current_tags=NewTagList();

    /* Handle the node information */

    XMLPARSE_ASSERT_INTEGER(_tag_,id);   llid=atoll(id); /* need long long conversion */
    node_id=(node_t)llid;
    assert((long long)node_id==llid);      /* check node id can be stored in node_t data type. */

    XMLPARSE_ASSERT_FLOATING(_tag_,lat); latitude =atof(lat);
    XMLPARSE_ASSERT_FLOATING(_tag_,lon); longitude=atof(lon);
   }

 if(_type_&XMLPARSE_TAG_END)
   {
    TagList *result=ApplyTaggingRules(&NodeRules,current_tags,node_id);

    process_node_tags(result,node_id,latitude,longitude);

    DeleteTagList(current_tags);
    DeleteTagList(result);
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the ndType XSD type is seen

  int ndType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *ref The contents of the 'ref' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int ndType_function(const char *_tag_,int _type_,const char *ref)
{
 if(_type_&XMLPARSE_TAG_START)
   {
    long long llid;
    node_t node_id;

    XMLPARSE_ASSERT_INTEGER(_tag_,ref); llid=atoll(ref); /* need long long conversion */
    node_id=(node_t)llid;
    assert((long long)node_id==llid);      /* check node id can be stored in node_t data type. */

    if(way_nnodes && (way_nnodes%256)==0)
       way_nodes=(node_t*)realloc((void*)way_nodes,(way_nnodes+256)*sizeof(node_t));

    way_nodes[way_nnodes++]=node_id;
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the memberType XSD type is seen

  int memberType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *type The contents of the 'type' attribute (or NULL if not defined).

  const char *ref The contents of the 'ref' attribute (or NULL if not defined).

  const char *role The contents of the 'role' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int memberType_function(const char *_tag_,int _type_,const char *type,const char *ref,const char *role)
{
 if(_type_&XMLPARSE_TAG_START)
   {
    long long llid;

    XMLPARSE_ASSERT_STRING(_tag_,type);
    XMLPARSE_ASSERT_INTEGER(_tag_,ref); llid=atoll(ref); /* need long long conversion */

    if(!strcmp(type,"node"))
      {
       node_t node_id;

       node_id=(node_t)llid;
       assert((long long)node_id==llid);   /* check node id can be stored in node_t data type. */

       if(relation_nnodes && (relation_nnodes%256)==0)
          relation_nodes=(node_t*)realloc((void*)relation_nodes,(relation_nnodes+256)*sizeof(node_t));

       relation_nodes[relation_nnodes++]=node_id;

       if(role)
         {
          if(!strcmp(role,"via"))
             relation_via=node_id;
         }
      }
    else if(!strcmp(type,"way"))
      {
       way_t way_id;

       way_id=(way_t)llid;
       assert((long long)way_id==llid);   /* check way id can be stored in way_t data type. */

       if(relation_nways && (relation_nways%256)==0)
          relation_ways=(way_t*)realloc((void*)relation_ways,(relation_nways+256)*sizeof(way_t));

       relation_ways[relation_nways++]=way_id;

       if(role)
         {
          if(!strcmp(role,"from"))
             relation_from=way_id;
          if(!strcmp(role,"to"))
             relation_to=way_id;
         }
      }
    else if(!strcmp(type,"relation"))
      {
       relation_t relation_id;

       relation_id=(relation_t)llid;
       assert((long long)relation_id==llid);   /* check relation id can be stored in relation_t data type. */

       if(relation_nrelations && (relation_nrelations%256)==0)
          relation_relations=(relation_t*)realloc((void*)relation_relations,(relation_nrelations+256)*sizeof(relation_t));

       relation_relations[relation_nrelations++]=relation_id;
      }
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the wayType XSD type is seen

  int wayType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *id The contents of the 'id' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int wayType_function(const char *_tag_,int _type_,const char *id)
{
 static way_t way_id;

 if(_type_&XMLPARSE_TAG_START)
   {
    long long llid;

    nways++;

    if(!(nways%1000))
       printf_middle("Reading: Lines=%llu Nodes=%"Pindex_t" Ways=%"Pindex_t" Relations=%"Pindex_t,ParseXML_LineNumber(),nnodes,nways,nrelations);

    current_tags=NewTagList();

    way_nnodes=0;

    /* Handle the way information */

    XMLPARSE_ASSERT_INTEGER(_tag_,id); llid=atoll(id); /* need long long conversion */

    way_id=(way_t)llid;
    assert((long long)way_id==llid);   /* check way id can be stored in way_t data type. */
   }

 if(_type_&XMLPARSE_TAG_END)
   {
    TagList *result=ApplyTaggingRules(&WayRules,current_tags,way_id);

    process_way_tags(result,way_id);

    DeleteTagList(current_tags);
    DeleteTagList(result);
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the relationType XSD type is seen

  int relationType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *id The contents of the 'id' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int relationType_function(const char *_tag_,int _type_,const char *id)
{
 static relation_t relation_id;

 if(_type_&XMLPARSE_TAG_START)
   {
    long long llid;

    nrelations++;

    if(!(nrelations%1000))
       printf_middle("Reading: Lines=%llu Nodes=%"Pindex_t" Ways=%"Pindex_t" Relations=%"Pindex_t,ParseXML_LineNumber(),nnodes,nways,nrelations);

    current_tags=NewTagList();

    relation_nnodes=relation_nways=relation_nrelations=0;

    relation_from=NO_WAY_ID;
    relation_to=NO_WAY_ID;
    relation_via=NO_NODE_ID;

    /* Handle the relation information */

    XMLPARSE_ASSERT_INTEGER(_tag_,id); llid=atoll(id); /* need long long conversion */

    relation_id=(relation_t)llid;
    assert((long long)relation_id==llid);   /* check relation id can be stored in relation_t data type. */
   }

 if(_type_&XMLPARSE_TAG_END)
   {
    TagList *result=ApplyTaggingRules(&RelationRules,current_tags,relation_id);

    process_relation_tags(result,relation_id);

    DeleteTagList(current_tags);
    DeleteTagList(result);
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  The function that is called when the osmType XSD type is seen

  int osmType_function Returns 0 if no error occured or something else otherwise.

  const char *_tag_ Set to the name of the element tag that triggered this function call.

  int _type_ Set to XMLPARSE_TAG_START at the start of a tag and/or XMLPARSE_TAG_END at the end of a tag.

  const char *version The contents of the 'version' attribute (or NULL if not defined).
  ++++++++++++++++++++++++++++++++++++++*/

static int osmType_function(const char *_tag_,int _type_,const char *version)
{
 if(_type_&XMLPARSE_TAG_START)
   {
    if(!version || strcmp(version,"0.6"))
       XMLPARSE_MESSAGE(_tag_,"Invalid value for 'version' (only '0.6' accepted)");
   }

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

//static int xmlDeclaration_function(const char *_tag_,int _type_,const char *version,const char *encoding)
//{
// return(0);
//}


/*++++++++++++++++++++++++++++++++++++++
  Parse an OSM XML file (from JOSM or planet download).

  int ParseOSM Returns 0 if OK or something else in case of an error.

  FILE *file The file to read from.

  NodesX *OSMNodes The data structure of nodes to fill in.

  SegmentsX *OSMSegments The data structure of segments to fill in.

  WaysX *OSMWays The data structure of ways to fill in.

  RelationsX *OSMRelations The data structure of relations to fill in.
  ++++++++++++++++++++++++++++++++++++++*/

int ParseOSM(FILE *file,NodesX *OSMNodes,SegmentsX *OSMSegments,WaysX *OSMWays,RelationsX *OSMRelations)
{
 int retval;

 /* Copy the function parameters and initialise the variables. */

 nodes=OSMNodes;
 segments=OSMSegments;
 ways=OSMWays;
 relations=OSMRelations;

 way_nodes=(node_t*)malloc(256*sizeof(node_t));

 relation_nodes    =(node_t    *)malloc(256*sizeof(node_t));
 relation_ways     =(way_t     *)malloc(256*sizeof(way_t));
 relation_relations=(relation_t*)malloc(256*sizeof(relation_t));

 /* Parse the file */

 nnodes=0,nways=0,nrelations=0;

 printf_first("Reading: Lines=0 Nodes=0 Ways=0 Relations=0");

 retval=ParseXML(file,xml_toplevel_tags,XMLPARSE_UNKNOWN_ATTR_IGNORE);

 printf_last("Read: Lines=%llu Nodes=%"Pindex_t" Ways=%"Pindex_t" Relations=%"Pindex_t,ParseXML_LineNumber(),nnodes,nways,nrelations);

 free(way_nodes);

 free(relation_nodes);
 free(relation_ways);
 free(relation_relations);

 return(retval);
}


/*++++++++++++++++++++++++++++++++++++++
  Process the tags associated with a node.

  TagList *tags The list of node tags.

  node_t id The id of the node.

  double latitude The latitude of the node.

  double longitude The longitude of the node.
  ++++++++++++++++++++++++++++++++++++++*/

static void process_node_tags(TagList *tags,node_t id,double latitude,double longitude)
{
 transports_t allow=Transports_ALL;
 uint16_t flags=0;
 int i;

 /* Parse the tags */

 for(i=0;i<tags->ntags;i++)
   {
    char *k=tags->k[i];
    char *v=tags->v[i];

    switch(*k)
      {
      case 'b':
       if(!strcmp(k,"bicycle"))
         {
          if(ISFALSE(v))
             allow&=~Transports_Bicycle;
          else if(!ISTRUE(v))
             logerror("Node %"Pnode_t" has an unrecognised tag value 'bicycle' = '%s' (after tagging rules); using 'yes'.\n",id,v);
         }

       break;

      case 'f':
       if(!strcmp(k,"foot"))
         {
          if(ISFALSE(v))
             allow&=~Transports_Foot;
          else if(!ISTRUE(v))
             logerror("Node %"Pnode_t" has an unrecognised tag value 'foot' = '%s' (after tagging rules); using 'yes'.\n",id,v);
         }

       break;

      case 'g':
       if(!strcmp(k,"goods"))
         {
          if(ISFALSE(v))
             allow&=~Transports_Goods;
          else if(!ISTRUE(v))
             logerror("Node %"Pnode_t" has an unrecognised tag value 'goods' = '%s' (after tagging rules); using 'yes'.\n",id,v);
         }

       break;

      case 'h':
       if(!strcmp(k,"highway"))
          if(!strcmp(v,"mini_roundabout"))
             flags|=NODE_MINIRNDBT;

       if(!strcmp(k,"horse"))
         {
          if(ISFALSE(v))
             allow&=~Transports_Horse;
          else if(!ISTRUE(v))
             logerror("Node %"Pnode_t" has an unrecognised tag value 'horse' = '%s' (after tagging rules); using 'yes'.\n",id,v);
         }

       if(!strcmp(k,"hgv"))
         {
          if(ISFALSE(v))
             allow&=~Transports_HGV;
          else if(!ISTRUE(v))
             logerror("Node %"Pnode_t" has an unrecognised tag value 'hgv' = '%s' (after tagging rules); using 'yes'.\n",id,v);
         }

       break;

      case 'm':
       if(!strcmp(k,"moped"))
         {
          if(ISFALSE(v))
             allow&=~Transports_Moped;
          else if(!ISTRUE(v))
             logerror("Node %"Pnode_t" has an unrecognised tag value 'moped' = '%s' (after tagging rules); using 'yes'.\n",id,v);
         }

       if(!strcmp(k,"motorbike"))
         {
          if(ISFALSE(v))
             allow&=~Transports_Motorbike;
          else if(!ISTRUE(v))
             logerror("Node %"Pnode_t" has an unrecognised tag value 'motorbike' = '%s' (after tagging rules); using 'yes'.\n",id,v);
         }

       if(!strcmp(k,"motorcar"))
         {
          if(ISFALSE(v))
             allow&=~Transports_Motorcar;
          else if(!ISTRUE(v))
             logerror("Node %"Pnode_t" has an unrecognised tag value 'motorcar' = '%s' (after tagging rules); using 'yes'.\n",id,v);
         }

       break;

      case 'p':
       if(!strcmp(k,"psv"))
         {
          if(ISFALSE(v))
             allow&=~Transports_PSV;
          else if(!ISTRUE(v))
             logerror("Node %"Pnode_t" has an unrecognised tag value 'psv' = '%s' (after tagging rules); using 'yes'.\n",id,v);
         }

       break;

      case 'w':
       if(!strcmp(k,"wheelchair"))
         {
          if(ISFALSE(v))
             allow&=~Transports_Wheelchair;
          else if(!ISTRUE(v))
             logerror("Node %"Pnode_t" has an unrecognised tag value 'wheelchair' = '%s' (after tagging rules); using 'yes'.\n",id,v);
         }

       break;

      default:
       logerror("Node %"Pnode_t" has an unrecognised tag '%s' = '%s' (after tagging rules); ignoring it.\n",id,k,v);
      }
   }

 /* Create the node */

 AppendNode(nodes,id,degrees_to_radians(latitude),degrees_to_radians(longitude),allow,flags);
}


/*++++++++++++++++++++++++++++++++++++++
  Process the tags associated with a way.

  TagList *tags The list of way tags.

  way_t id The id of the way.
  ++++++++++++++++++++++++++++++++++++++*/

static void process_way_tags(TagList *tags,way_t id)
{
 Way   way={0};
 int   oneway=0,roundabout=0,area=0;
 char *name=NULL,*ref=NULL,*refname=NULL;
 int i;

 /* Parse the tags - just look for highway */

 for(i=0;i<tags->ntags;i++)
   {
    char *k=tags->k[i];
    char *v=tags->v[i];

    if(!strcmp(k,"highway"))
      {
       way.type=HighwayType(v);

       if(way.type==Way_Count)
          logerror("Way %"Pway_t" has an unrecognised highway type '%s' (after tagging rules); ignoring it.\n",id,v);
      }
   }

 /* Don't continue if this is not a highway (bypass error logging) */

 if(way.type==0 || way.type==Way_Count)
    return;

 /* Parse the tags - look for the others */

 for(i=0;i<tags->ntags;i++)
   {
    char *k=tags->k[i];
    char *v=tags->v[i];

    switch(*k)
      {
      case 'a':
       if(!strcmp(k,"area"))
         {
          if(ISTRUE(v))
             area=1;
          else if(!ISFALSE(v))
             logerror("Way %"Pway_t" has an unrecognised tag value 'area' = '%s' (after tagging rules); using 'no'.\n",id,v);
         }

       break;

      case 'b':
       if(!strcmp(k,"bicycle"))
         {
          if(ISTRUE(v))
             way.allow|=Transports_Bicycle;
          else if(!ISFALSE(v))
             logerror("Way %"Pway_t" has an unrecognised tag value 'bicycle' = '%s' (after tagging rules); using 'no'.\n",id,v);
         }

       if(!strcmp(k,"bicycleroute"))
         {
          if(ISTRUE(v))
             way.props|=Properties_BicycleRoute;
          else if(!ISFALSE(v))
             logerror("Way %"Pway_t" has an unrecognised tag value 'bicycleroute' = '%s' (after tagging rules); using 'no'.\n",id,v);
         }

       if(!strcmp(k,"bridge"))
         {
          if(ISTRUE(v))
             way.props|=Properties_Bridge;
          else if(!ISFALSE(v))
             logerror("Way %"Pway_t" has an unrecognised tag value 'bridge' = '%s' (after tagging rules); using 'no'.\n",id,v);
         }

       break;

      case 'f':
       if(!strcmp(k,"foot"))
         {
          if(ISTRUE(v))
             way.allow|=Transports_Foot;
          else if(!ISFALSE(v))
             logerror("Way %"Pway_t" has an unrecognised tag value 'foot' = '%s' (after tagging rules); using 'no'.\n",id,v);
         }

       if(!strcmp(k,"footroute"))
         {
          if(ISTRUE(v))
             way.props|=Properties_FootRoute;
          else if(!ISFALSE(v))
             logerror("Way %"Pway_t" has an unrecognised tag value 'footroute' = '%s' (after tagging rules); using 'no'.\n",id,v);
         }

       break;

      case 'g':
       if(!strcmp(k,"goods"))
         {
          if(ISTRUE(v))
             way.allow|=Transports_Goods;
          else if(!ISFALSE(v))
             logerror("Way %"Pway_t" has an unrecognised tag value 'goods' = '%s' (after tagging rules); using 'no'.\n",id,v);
         }

       break;

      case 'h':
       if(!strcmp(k,"highway"))
          ;

       if(!strcmp(k,"horse"))
         {
          if(ISTRUE(v))
             way.allow|=Transports_Horse;
          else if(!ISFALSE(v))
             logerror("Way %"Pway_t" has an unrecognised tag value 'horse' = '%s' (after tagging rules); using 'no'.\n",id,v);
         }

       if(!strcmp(k,"hgv"))
         {
          if(ISTRUE(v))
             way.allow|=Transports_HGV;
          else if(!ISFALSE(v))
             logerror("Way %"Pway_t" has an unrecognised tag value 'hgv' = '%s' (after tagging rules); using 'no'.\n",id,v);
         }

       break;

      case 'm':
       if(!strcmp(k,"maxspeed"))
          way.speed=kph_to_speed(parse_speed(id,k,v));

       if(!strcmp(k,"maxweight"))
          way.weight=tonnes_to_weight(parse_weight(id,k,v));

       if(!strcmp(k,"maxheight"))
          way.height=metres_to_height(parse_length(id,k,v));

       if(!strcmp(k,"maxwidth"))
          way.width=metres_to_height(parse_length(id,k,v));

       if(!strcmp(k,"maxlength"))
          way.length=metres_to_height(parse_length(id,k,v));

       if(!strcmp(k,"moped"))
         {
          if(ISTRUE(v))
             way.allow|=Transports_Moped;
          else if(!ISFALSE(v))
             logerror("Way %"Pway_t" has an unrecognised tag value 'moped' = '%s' (after tagging rules); using 'no'.\n",id,v);
         }

       if(!strcmp(k,"motorbike"))
         {
          if(ISTRUE(v))
             way.allow|=Transports_Motorbike;
          else if(!ISFALSE(v))
             logerror("Way %"Pway_t" has an unrecognised tag value 'motorbike' = '%s' (after tagging rules); using 'no'.\n",id,v);
         }

       if(!strcmp(k,"motorcar"))
         {
          if(ISTRUE(v))
             way.allow|=Transports_Motorcar;
          else if(!ISFALSE(v))
             logerror("Way %"Pway_t" has an unrecognised tag value 'motorcar' = '%s' (after tagging rules); using 'no'.\n",id,v);
         }

       if(!strcmp(k,"multilane"))
         {
          if(ISTRUE(v))
             way.props|=Properties_Multilane;
          else if(!ISFALSE(v))
             logerror("Way %"Pway_t" has an unrecognised tag value 'multilane' = '%s' (after tagging rules); using 'no'.\n",id,v);
         }

       break;

      case 'n':
       if(!strcmp(k,"name"))
          name=v;

       break;

      case 'o':
       if(!strcmp(k,"oneway"))
         {
          if(ISTRUE(v))
             oneway=1;
          else if(!strcmp(v,"-1"))
             oneway=-1;
          else if(!ISFALSE(v))
             logerror("Way %"Pway_t" has an unrecognised tag value 'oneway' = '%s' (after tagging rules); using 'no'.\n",id,v);
         }

       break;

      case 'p':
       if(!strcmp(k,"paved"))
         {
          if(ISTRUE(v))
             way.props|=Properties_Paved;
          else if(!ISFALSE(v))
             logerror("Way %"Pway_t" has an unrecognised tag value 'paved' = '%s' (after tagging rules); using 'no'.\n",id,v);
         }

       if(!strcmp(k,"psv"))
         {
          if(ISTRUE(v))
             way.allow|=Transports_PSV;
          else if(!ISFALSE(v))
             logerror("Way %"Pway_t" has an unrecognised tag value 'psv' = '%s' (after tagging rules); using 'no'.\n",id,v);
         }

       break;

      case 'r':
       if(!strcmp(k,"ref"))
          ref=v;

       if(!strcmp(k,"roundabout"))
         {
          if(ISTRUE(v))
             roundabout=1;
          else if(!ISFALSE(v))
             logerror("Way %"Pway_t" has an unrecognised tag value 'roundabout' = '%s' (after tagging rules); using 'no'.\n",id,v);
         }

       break;

      case 't':
       if(!strcmp(k,"tunnel"))
         {
          if(ISTRUE(v))
             way.props|=Properties_Tunnel;
          else if(!ISFALSE(v))
             logerror("Way %"Pway_t" has an unrecognised tag value 'tunnel' = '%s' (after tagging rules); using 'no'.\n",id,v);
         }

       break;

      case 'w':
       if(!strcmp(k,"wheelchair"))
         {
          if(ISTRUE(v))
             way.allow|=Transports_Wheelchair;
          else if(!ISFALSE(v))
             logerror("Way %"Pway_t" has an unrecognised tag value 'wheelchair' = '%s' (after tagging rules); using 'no'.\n",id,v);
         }

       break;

      default:
       logerror("Way %"Pway_t" has an unrecognised tag '%s' = '%s' (after tagging rules); ignoring it.\n",id,k,v);
      }
   }

 /* Create the way */

 if(!way.allow)
    return;

 if(way_nnodes==0)
   {
    logerror("Way %"Pway_t" has no nodes.\n",id);

    return;
   }

 if(oneway)
    way.type|=Way_OneWay;

 if(roundabout)
    way.type|=Way_Roundabout;

 if(ref && name)
   {
    refname=(char*)malloc(strlen(ref)+strlen(name)+4);
    sprintf(refname,"%s (%s)",name,ref);
   }
 else if(ref && !name)
    refname=ref;
 else if(!ref && name)
    refname=name;
 else /* if(!ref && !name) */
    refname="";

 AppendWay(ways,id,&way,refname);

 if(ref && name)
    free(refname);

 for(i=1;i<way_nnodes;i++)
   {
    node_t from=way_nodes[i-1];
    node_t to  =way_nodes[i];

    if(oneway>0)
       AppendSegment(segments,id,from,to,area+ONEWAY_1TO2);
    else if(oneway<0)
       AppendSegment(segments,id,from,to,area+ONEWAY_2TO1);
    else
       AppendSegment(segments,id,from,to,area);
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Process the tags associated with a relation.

  TagList *tags The list of relation tags.

  relation_t id The id of the relation.
  ++++++++++++++++++++++++++++++++++++++*/

static void process_relation_tags(TagList *tags,relation_t id)
{
 transports_t routes=Transports_None;
 transports_t except=Transports_None;
 int relation_turn_restriction=0;
 TurnRestriction restriction=TurnRestrict_None;
 int i;

 /* Parse the tags */

 for(i=0;i<tags->ntags;i++)
   {
    char *k=tags->k[i];
    char *v=tags->v[i];

    switch(*k)
      {
      case 'b':
       if(!strcmp(k,"bicycleroute"))
         {
          if(ISTRUE(v))
             routes|=Transports_Bicycle;
          else if(!ISFALSE(v))
             logerror("Relation %"Prelation_t" has an unrecognised tag value 'bicycleroute' = '%s' (after tagging rules); using 'no'.\n",id,v);
         }

       break;

      case 'e':
       if(!strcmp(k,"except"))
         {
          for(i=1;i<Transport_Count;i++)
             if(strstr(v,TransportName(i)))
                except|=TRANSPORTS(i);

          if(except==Transports_None)
             logerror("Relation %"Prelation_t" has an unrecognised tag value 'except' = '%s' (after tagging rules); ignoring it.\n",id,v);
         }

       break;

      case 'f':
       if(!strcmp(k,"footroute"))
         {
          if(ISTRUE(v))
             routes|=Transports_Foot;
          else if(!ISFALSE(v))
             logerror("Relation %"Prelation_t" has an unrecognised tag value 'footroute' = '%s' (after tagging rules); using 'no'.\n",id,v);
         }

       break;

      case 'r':
       if(!strcmp(k,"restriction"))
         {
          if(!strcmp(v,"no_right_turn"   )) restriction=TurnRestrict_no_right_turn;
          if(!strcmp(v,"no_left_turn"    )) restriction=TurnRestrict_no_left_turn;
          if(!strcmp(v,"no_u_turn"       )) restriction=TurnRestrict_no_u_turn;
          if(!strcmp(v,"no_straight_on"  )) restriction=TurnRestrict_no_straight_on;
          if(!strcmp(v,"only_right_turn" )) restriction=TurnRestrict_only_right_turn;
          if(!strcmp(v,"only_left_turn"  )) restriction=TurnRestrict_only_left_turn;
          if(!strcmp(v,"only_straight_on")) restriction=TurnRestrict_only_straight_on;

          if(restriction==TurnRestrict_None)
             logerror("Relation %"Prelation_t" has an unrecognised tag value 'restriction' = '%s' (after tagging rules); ignoring it.\n",id,v);
         }

       break;

      case 't':
       if(!strcmp(k,"type"))
          if(!strcmp(v,"restriction"))
             relation_turn_restriction=1;
       break;

      default:
       logerror("Relation %"Prelation_t" has an unrecognised tag '%s' = '%s' (after tagging rules); ignoring it.\n",id,k,v);
      }
   }

 /* Create the route relation (must store all relations that have ways or
    relations even if they are not routes because they might be referenced by
    other relations that are routes) */

 if((relation_nways || relation_nrelations) && !relation_turn_restriction)
    AppendRouteRelation(relations,id,routes,
                        relation_ways,relation_nways,
                        relation_relations,relation_nrelations);

 /* Create the turn restriction relation. */

 if(relation_turn_restriction && restriction!=TurnRestrict_None)
   {
    if(relation_from==NO_WAY_ID)
       logerror("Relation %"Prelation_t" is a turn restriction but has no 'from' way.\n",id);
    else if(relation_to==NO_WAY_ID)
       logerror("Relation %"Prelation_t" is a turn restriction but has no 'to' way.\n",id);
    else if(relation_via==NO_NODE_ID)
       logerror("Relation %"Prelation_t" is a turn restriction but has no 'via' node.\n",id);
    else
       AppendTurnRestrictRelation(relations,id,
                                  relation_from,relation_to,relation_via,
                                  restriction,except);
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Convert a string containing a speed into a double precision.

  double parse_speed Returns the speed in km/h if it can be parsed.

  way_t id The way being processed.

  const char *k The tag key.

  const char *v The tag value.
  ++++++++++++++++++++++++++++++++++++++*/

static double parse_speed(way_t id,const char *k,const char *v)
{
 char *ev;
 double value=strtod(v,&ev);

 if(v==ev)
    logerror("Way %"Pway_t" has an unrecognised tag value '%s' = '%s' (after tagging rules); ignoring it.\n",id,k,v);
 else
   {
    while(isspace(*ev)) ev++;

    if(!strcmp(ev,"mph"))
       return(1.609*value);

    if(*ev==0 || !strcmp(ev,"kph"))
       return(value);

    logerror("Way %"Pway_t" has an un-parseable tag value '%s' = '%s' (after tagging rules); ignoring it.\n",id,k,v);
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  Convert a string containing a weight into a double precision.

  double parse_weight Returns the weight in tonnes if it can be parsed.

  way_t id The way being processed.

  const char *k The tag key.

  const char *v The tag value.
  ++++++++++++++++++++++++++++++++++++++*/

static double parse_weight(way_t id,const char *k,const char *v)
{
 char *ev;
 double value=strtod(v,&ev);

 if(v==ev)
    logerror("Way %"Pway_t" has an unrecognised tag value '%s' = '%s' (after tagging rules); ignoring it.\n",id,k,v);
 else
   {
    while(isspace(*ev)) ev++;

    if(!strcmp(ev,"kg"))
       return(value/1000.0);

    if(*ev==0 || !strcmp(ev,"T") || !strcmp(ev,"t")
              || !strcmp(ev,"ton") || !strcmp(ev,"tons")
              || !strcmp(ev,"tonne") || !strcmp(ev,"tonnes"))
       return(value);

    logerror("Way %"Pway_t" has an un-parseable tag value '%s' = '%s' (after tagging rules); ignoring it.\n",id,k,v);
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  Convert a string containing a length into a double precision.

  double parse_length Returns the length in metres if it can be parsed.

  way_t id The way being processed.

  const char *k The tag key.

  const char *v The tag value.
  ++++++++++++++++++++++++++++++++++++++*/

static double parse_length(way_t id,const char *k,const char *v)
{
 char *ev;
 double value=strtod(v,&ev);

 if(v==ev)
    logerror("Way %"Pway_t" has an unrecognised tag value '%s' = '%s' (after tagging rules); ignoring it.\n",id,k,v);
 else
   {
    int en=0;
    int feet=0,inches=0;

    if(sscanf(v,"%d' %d\"%n",&feet,&inches,&en)==2 && en && !v[en])
       return((feet+(double)inches/12.0)*0.254);

    if(sscanf(v,"%d - %d%n",&feet,&inches,&en)==2 && en && !v[en])
       return((feet+(double)inches/12.0)*0.254);

    if(sscanf(v,"%d ft %d in%n",&feet,&inches,&en)==2 && en && !v[en])
       return((feet+(double)inches/12.0)*0.254);

    if(sscanf(v,"%d feet %d inches%n",&feet,&inches,&en)==2 && en && !v[en])
       return((feet+(double)inches/12.0)*0.254);

    if(!strcmp(ev,"'"))
       return(feet*0.254);

    while(isspace(*ev)) ev++;

    if(!strcmp(ev,"ft") || !strcmp(ev,"feet"))
       return(value*0.254);

    if(*ev==0 || !strcmp(ev,"m") || !strcmp(ev,"metre") || !strcmp(ev,"metres"))
       return(value);

    logerror("Way %"Pway_t" has an un-parseable tag value '%s' = '%s' (after tagging rules); ignoring it.\n",id,k,v);
   }

 return(0);
}
