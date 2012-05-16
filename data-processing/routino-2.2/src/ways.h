/***************************************
 A header file for the ways.

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2008-2011 Andrew M. Bishop

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


#ifndef WAYS_H
#define WAYS_H    /*+ To stop multiple inclusions. +*/

#include <stdint.h>
#include <stdlib.h>

#include "types.h"

#include "files.h"


/* Data structures */


/*+ A structure containing a single way (members ordered to minimise overall size). +*/
struct _Way
{
 index_t      name;             /*+ The offset of the name of the way in the names array. +*/

 transports_t allow;            /*+ The type of traffic allowed on the way. +*/

 highway_t    type;             /*+ The highway type of the way. +*/

 properties_t props;            /*+ The properties of the way. +*/

 speed_t      speed;            /*+ The defined maximum speed limit of the way. +*/

 weight_t     weight;           /*+ The defined maximum weight of traffic on the way. +*/
 height_t     height;           /*+ The defined maximum height of traffic on the way. +*/
 width_t      width;            /*+ The defined maximum width of traffic on the way. +*/
 length_t     length;           /*+ The defined maximum length of traffic on the way. +*/
};


/*+ A structure containing the header from the file. +*/
typedef struct _WaysFile
{
 index_t      number;           /*+ The number of ways stored. +*/
 index_t      onumber;          /*+ The number of ways originally. +*/

 highways_t   highways;         /*+ The types of highways that were seen when parsing. +*/
 transports_t allow;            /*+ The types of traffic that were seen when parsing. +*/
 properties_t props;            /*+ The properties that were seen when parsing. +*/
}
 WaysFile;


/*+ A structure containing a set of ways (and pointers to mmap file). +*/
struct _Ways
{
 WaysFile file;                 /*+ The header data from the file. +*/

#if !SLIM

 void      *data;               /*+ The memory mapped data. +*/

 Way       *ways;               /*+ An array of ways. +*/
 char      *names;              /*+ An array of characters containing the names. +*/

#else

 int        fd;                 /*+ The file descriptor for the file. +*/
 off_t      namesoffset;        /*+ The offset of the names within the file. +*/

 Way        cached[3];          /*+ Two cached nodes read from the file in slim mode. +*/
 index_t    incache[3];         /*+ The indexes of the cached ways. +*/

 char      *ncached[3];         /*+ The cached way name. +*/

#endif
};


/* Functions in ways.c */

Ways *LoadWayList(const char *filename);

int WaysCompare(Way *way1,Way *way2);


/* Macros and inline functions */

#if !SLIM

/*+ Return a Way* pointer given a set of ways and an index. +*/
#define LookupWay(xxx,yyy,zzz)     (&(xxx)->ways[yyy])

/*+ Return the name of a way given the Way pointer and a set of ways. +*/
#define WayName(xxx,yyy)           (&(xxx)->names[(yyy)->name])

#else

static Way *LookupWay(Ways *ways,index_t index,int position);

static char *WayName(Ways *ways,Way *way);


/*++++++++++++++++++++++++++++++++++++++
  Find the Way information for a particular way.

  Way *LookupWay Returns a pointer to the cached way information.

  Ways *ways The set of ways to use.

  index_t index The index of the way.

  int position The position in the cache to store the value.
  ++++++++++++++++++++++++++++++++++++++*/

static inline Way *LookupWay(Ways *ways,index_t index,int position)
{
 if(ways->incache[position-1]!=index)
   {
    SeekReadFile(ways->fd,&ways->cached[position-1],sizeof(Way),sizeof(WaysFile)+(off_t)index*sizeof(Way));

    ways->incache[position-1]=index;
   }

 return(&ways->cached[position-1]);
}


/*++++++++++++++++++++++++++++++++++++++
  Find the name of a way.

  char *WayName Returns a pointer to the name of the way.

  Ways *ways The set of ways to use.

  Way *way The Way pointer.
  ++++++++++++++++++++++++++++++++++++++*/

static inline char *WayName(Ways *ways,Way *way)
{
 int position=way-&ways->cached[-1];

 int n=0;

 SeekFile(ways->fd,ways->namesoffset+way->name);

 if(!ways->ncached[position-1])
    ways->ncached[position-1]=(char*)malloc(32);

 while(1)
   {
    int i;
    int m=ReadFile(ways->fd,ways->ncached[position-1]+n,32);

    if(m<0)
       break;
    
    for(i=n;i<n+32;i++)
       if(ways->ncached[position-1][i]==0)
          goto exitloop;

    n+=32;

    ways->ncached[position-1]=(char*)realloc((void*)ways->ncached[position-1],n+32);
   }

 exitloop:

 return(ways->ncached[position-1]);
}

#endif


#endif /* WAYS_H */
