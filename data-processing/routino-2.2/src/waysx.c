/***************************************
 Extended Way data type functions.

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
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "ways.h"

#include "typesx.h"
#include "waysx.h"

#include "files.h"
#include "logging.h"
#include "sorting.h"


/* Global variables */

/*+ The command line '--tmpdir' option or its default value. +*/
extern char *option_tmpdirname;


/* Local variables */

/*+ A temporary file-local variable for use by the sort functions. +*/
static WaysX *sortwaysx;


/* Functions */

static int sort_by_id(WayX *a,WayX *b);
static int sort_by_name_and_id(WayX *a,WayX *b);
static int sort_by_name_and_prop_and_id(WayX *a,WayX *b);

static int deduplicate_and_index_by_id(WayX *wayx,index_t index);


/*++++++++++++++++++++++++++++++++++++++
  Allocate a new way list (create a new file or open an existing one).

  WaysX *NewWayList Returns the way list.

  int append Set to 1 if the file is to be opened for appending (now or later).
  ++++++++++++++++++++++++++++++++++++++*/

WaysX *NewWayList(int append)
{
 WaysX *waysx;

 waysx=(WaysX*)calloc(1,sizeof(WaysX));

 assert(waysx); /* Check calloc() worked */

 waysx->filename=(char*)malloc(strlen(option_tmpdirname)+32);

 if(append)
    sprintf(waysx->filename,"%s/waysx.input.tmp",option_tmpdirname);
 else
    sprintf(waysx->filename,"%s/waysx.%p.tmp",option_tmpdirname,(void*)waysx);

 if(append)
   {
    off_t size,position=0;

    waysx->fd=OpenFileAppend(waysx->filename);

    size=SizeFile(waysx->filename);

    while(position<size)
      {
       FILESORT_VARINT waysize;

       SeekReadFile(waysx->fd,&waysize,FILESORT_VARSIZE,position);

       waysx->number++;
       position+=waysize+FILESORT_VARSIZE;
      }

    SeekFile(waysx->fd,size);
   }
 else
    waysx->fd=OpenFileNew(waysx->filename);

 waysx->nfilename=(char*)malloc(strlen(option_tmpdirname)+32);
 sprintf(waysx->nfilename,"%s/waynames.%p.tmp",option_tmpdirname,(void*)waysx);

 return(waysx);
}


/*++++++++++++++++++++++++++++++++++++++
  Free a way list.

  WaysX *waysx The set of ways to be freed.

  int keep Set to 1 if the file is to be kept (for appending later).
  ++++++++++++++++++++++++++++++++++++++*/

void FreeWayList(WaysX *waysx,int keep)
{
 if(!keep)
    DeleteFile(waysx->filename);

 free(waysx->filename);

 if(waysx->idata)
    free(waysx->idata);

 DeleteFile(waysx->nfilename);

 free(waysx->nfilename);

 free(waysx);
}


/*++++++++++++++++++++++++++++++++++++++
  Append a single way to an unsorted way list.

  WaysX *waysx The set of ways to process.

  way_t id The ID of the way.

  Way *way The way data itself.

  const char *name The name or reference of the way.
  ++++++++++++++++++++++++++++++++++++++*/

void AppendWay(WaysX *waysx,way_t id,Way *way,const char *name)
{
 WayX wayx;
 FILESORT_VARINT size;

 wayx.id=id;
 wayx.prop=0;
 wayx.way=*way;

 size=sizeof(WayX)+strlen(name)+1;

 WriteFile(waysx->fd,&size,FILESORT_VARSIZE);
 WriteFile(waysx->fd,&wayx,sizeof(WayX));
 WriteFile(waysx->fd,name,strlen(name)+1);

 waysx->number++;

 assert(!(waysx->number==0)); /* Zero marks the high-water mark for ways. */
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the list of ways.

  WaysX *waysx The set of ways to process.
  ++++++++++++++++++++++++++++++++++++++*/

void SortWayList(WaysX *waysx)
{
 index_t i,xnumber;
 int fd;
 char *names[2]={NULL,NULL};
 int namelen[2]={0,0};
 int nnames=0;
 uint32_t lastlength=0;

 /* Print the start message */

 printf_first("Sorting Ways by Name");

 /* Close the file (finished appending) */

 waysx->fd=CloseFile(waysx->fd);

 /* Re-open the file read-only and a new file writeable */

 waysx->fd=ReOpenFile(waysx->filename);

 DeleteFile(waysx->filename);

 fd=OpenFileNew(waysx->filename);

 /* Sort the ways to allow separating the names */

 filesort_vary(waysx->fd,fd,(int (*)(const void*,const void*))sort_by_name_and_id,NULL);

 /* Close the files */

 waysx->fd=CloseFile(waysx->fd);
 CloseFile(fd);

 /* Print the final message */

 printf_last("Sorted Ways by Name: Ways=%"Pindex_t,waysx->number);


 /* Print the start message */

 printf_first("Separating Way Names: Ways=0 Names=0");

 /* Re-open the file read-only and new files writeable */

 waysx->fd=ReOpenFile(waysx->filename);

 DeleteFile(waysx->filename);

 fd=OpenFileNew(waysx->filename);

 waysx->nfd=OpenFileNew(waysx->nfilename);

 /* Copy from the single file into two files */

 for(i=0;i<waysx->number;i++)
   {
    WayX wayx;
    FILESORT_VARINT size;

    ReadFile(waysx->fd,&size,FILESORT_VARSIZE);

    if(namelen[nnames%2]<size)
       names[nnames%2]=(char*)realloc((void*)names[nnames%2],namelen[nnames%2]=size);

    ReadFile(waysx->fd,&wayx,sizeof(WayX));
    ReadFile(waysx->fd,names[nnames%2],size-sizeof(WayX));

    if(nnames==0 || strcmp(names[0],names[1]))
      {
       WriteFile(waysx->nfd,names[nnames%2],size-sizeof(WayX));

       lastlength=waysx->nlength;
       waysx->nlength+=size-sizeof(WayX);

       nnames++;
      }

    wayx.way.name=lastlength;

    WriteFile(fd,&wayx,sizeof(WayX));

    if(!((i+1)%1000))
       printf_middle("Separating Way Names: Ways=%"Pindex_t" Names=%"Pindex_t,i+1,nnames);
   }

 if(names[0]) free(names[0]);
 if(names[1]) free(names[1]);

 /* Close the files */

 waysx->fd=CloseFile(waysx->fd);
 CloseFile(fd);

 waysx->nfd=CloseFile(waysx->nfd);

 /* Print the final message */

 printf_last("Separated Way Names: Ways=%"Pindex_t" Names=%"Pindex_t" ",waysx->number,nnames);


 /* Print the start message */

 printf_first("Sorting Ways");

 /* Re-open the file read-only and a new file writeable */

 waysx->fd=ReOpenFile(waysx->filename);

 DeleteFile(waysx->filename);

 fd=OpenFileNew(waysx->filename);

 /* Allocate the array of indexes */

 waysx->idata=(way_t*)malloc(waysx->number*sizeof(way_t));

 assert(waysx->idata); /* Check malloc() worked */

 /* Sort the ways by index and index them */

 xnumber=waysx->number;
 waysx->number=0;

 sortwaysx=waysx;

 waysx->number=filesort_fixed(waysx->fd,fd,sizeof(WayX),(int (*)(const void*,const void*))sort_by_id,(int (*)(void*,index_t))deduplicate_and_index_by_id);

 /* Close the files */

 waysx->fd=CloseFile(waysx->fd);
 CloseFile(fd);

 /* Print the final message */

 printf_last("Sorted Ways: Ways=%"Pindex_t" Duplicates=%"Pindex_t,xnumber,xnumber-waysx->number);
}


/*++++++++++++++++++++++++++++++++++++++
  Compact the list of ways.

  WaysX *waysx The set of ways to process.
  ++++++++++++++++++++++++++++++++++++++*/

void CompactWayList(WaysX *waysx)
{
 index_t i;
 int fd;
 Way lastway;

 /* Print the start message */

 printf_first("Sorting Ways by Properties");

 /* Re-open the file read-only and a new file writeable */

 waysx->fd=ReOpenFile(waysx->filename);

 DeleteFile(waysx->filename);

 fd=OpenFileNew(waysx->filename);

 /* Sort the ways to allow compacting according to he properties */

 filesort_fixed(waysx->fd,fd,sizeof(WayX),(int (*)(const void*,const void*))sort_by_name_and_prop_and_id,NULL);

 /* Close the files */

 waysx->fd=CloseFile(waysx->fd);
 CloseFile(fd);

 /* Print the final message */

 printf_last("Sorted Ways by Properties: Ways=%"Pindex_t,waysx->number);


 /* Print the start message */

 printf_first("Compacting Ways: Ways=0 Properties=0");

 /* Re-open the file read-only and a new file writeable */

 waysx->fd=ReOpenFile(waysx->filename);

 DeleteFile(waysx->filename);

 fd=OpenFileNew(waysx->filename);

 /* Update the way as we go using the sorted index */

 waysx->cnumber=0;

 for(i=0;i<waysx->number;i++)
   {
    WayX wayx;

    ReadFile(waysx->fd,&wayx,sizeof(WayX));

    if(waysx->cnumber==0 || wayx.way.name!=lastway.name || WaysCompare(&lastway,&wayx.way))
      {
       lastway=wayx.way;

       waysx->cnumber++;
      }

    wayx.prop=waysx->cnumber-1;

    WriteFile(fd,&wayx,sizeof(WayX));

    if(!((i+1)%1000))
       printf_middle("Compacting Ways: Ways=%"Pindex_t" Properties=%"Pindex_t,i+1,waysx->cnumber);
   }

 /* Close the files */

 waysx->fd=CloseFile(waysx->fd);
 CloseFile(fd);

 /* Print the final message */

 printf_last("Compacted Ways: Ways=%"Pindex_t" Properties=%"Pindex_t" ",waysx->number,waysx->cnumber);


 /* Print the start message */

 printf_first("Sorting Ways");

 /* Re-open the file read-only and a new file writeable */

 waysx->fd=ReOpenFile(waysx->filename);

 DeleteFile(waysx->filename);

 fd=OpenFileNew(waysx->filename);

 /* Sort the ways by index */

 filesort_fixed(waysx->fd,fd,sizeof(WayX),(int (*)(const void*,const void*))sort_by_id,NULL);

 /* Close the files */

 waysx->fd=CloseFile(waysx->fd);
 CloseFile(fd);

 /* Print the final message */

 printf_last("Sorted Ways: Ways=%"Pindex_t,waysx->number);
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the ways into id order.

  int sort_by_id Returns the comparison of the id fields.

  WayX *a The first extended way.

  WayX *b The second extended way.
  ++++++++++++++++++++++++++++++++++++++*/

static int sort_by_id(WayX *a,WayX *b)
{
 way_t a_id=a->id;
 way_t b_id=b->id;

 if(a_id<b_id)
    return(-1);
 else if(a_id>b_id)
    return(1);
 else
    return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the ways into name order and then id order.

  int sort_by_name_and_id Returns the comparison of the name and id fields.

  WayX *a The first extended Way.

  WayX *b The second extended Way.
  ++++++++++++++++++++++++++++++++++++++*/

static int sort_by_name_and_id(WayX *a,WayX *b)
{
 int compare;
 char *a_name=(char*)a+sizeof(WayX);
 char *b_name=(char*)b+sizeof(WayX);

 compare=strcmp(a_name,b_name);

 if(compare)
    return(compare);

 return(sort_by_id(a,b));
}


/*++++++++++++++++++++++++++++++++++++++
  Sort the ways into name, properties and id order.

  int sort_by_name_and_prop_and_id Returns the comparison of the name, properties and id fields.

  WayX *a The first extended Way.

  WayX *b The second extended Way.
  ++++++++++++++++++++++++++++++++++++++*/

static int sort_by_name_and_prop_and_id(WayX *a,WayX *b)
{
 int compare;
 index_t a_name=a->way.name;
 index_t b_name=b->way.name;

 if(a_name<b_name)
    return(-1);
 else if(a_name>b_name)
    return(1);

 compare=WaysCompare(&a->way,&b->way);

 if(compare)
    return(compare);

 return(sort_by_id(a,b));
}


/*++++++++++++++++++++++++++++++++++++++
  Create the index of identifiers and discard duplicate ways.

  int deduplicate_and_index_by_id Return 1 if the value is to be kept, otherwise 0.

  WayX *wayx The extended way.

  index_t index The index of this way in the total.
  ++++++++++++++++++++++++++++++++++++++*/

static int deduplicate_and_index_by_id(WayX *wayx,index_t index)
{
 static way_t previd;

 if(index==0 || wayx->id!=previd)
   {
    previd=wayx->id;

    sortwaysx->idata[index]=wayx->id;

    return(1);
   }
 else
   {
    logerror("Way %"Pway_t" is duplicated.\n",wayx->id);

    return(0);
   }
}


/*++++++++++++++++++++++++++++++++++++++
  Find a particular way index.

  index_t IndexWayX Returns the index of the extended way with the specified id.

  WaysX *waysx The set of ways to process.

  way_t id The way id to look for.
  ++++++++++++++++++++++++++++++++++++++*/

index_t IndexWayX(WaysX *waysx,way_t id)
{
 index_t start=0;
 index_t end=waysx->number-1;
 index_t mid;

 /* Binary search - search key exact match only is required.
  *
  *  # <- start  |  Check mid and move start or end if it doesn't match
  *  #           |
  *  #           |  Since an exact match is wanted we can set end=mid-1
  *  # <- mid    |  or start=mid+1 because we know that mid doesn't match.
  *  #           |
  *  #           |  Eventually either end=start or end=start+1 and one of
  *  # <- end    |  start or end is the wanted one.
  */

 if(end<start)                   /* There are no ways */
    return(NO_WAY);
 else if(id<waysx->idata[start]) /* Check key is not before start */
    return(NO_WAY);
 else if(id>waysx->idata[end])   /* Check key is not after end */
    return(NO_WAY);
 else
   {
    do
      {
       mid=(start+end)/2;            /* Choose mid point */

       if(waysx->idata[mid]<id)      /* Mid point is too low */
          start=mid+1;
       else if(waysx->idata[mid]>id) /* Mid point is too high */
          end=mid?(mid-1):mid;
       else                          /* Mid point is correct */
          return(mid);
      }
    while((end-start)>1);

    if(waysx->idata[start]==id)      /* Start is correct */
       return(start);

    if(waysx->idata[end]==id)        /* End is correct */
       return(end);
   }

 return(NO_WAY);
}


/*++++++++++++++++++++++++++++++++++++++
  Save the way list to a file.

  WaysX *waysx The set of ways to save.

  const char *filename The name of the file to save.
  ++++++++++++++++++++++++++++++++++++++*/

void SaveWayList(WaysX *waysx,const char *filename)
{
 index_t i;
 int fd;
 int position=0;
 WaysFile waysfile={0};
 highways_t   highways=0;
 transports_t allow=0;
 properties_t props=0;

 /* Print the start message */

 printf_first("Writing Ways: Ways=0");

 /* Map into memory /  open the file */

#if !SLIM
 waysx->data=MapFile(waysx->filename);
#else
 waysx->fd=ReOpenFile(waysx->filename);
#endif

 /* Write out the ways data */

 fd=OpenFileNew(filename);

 SeekFile(fd,sizeof(WaysFile));

 for(i=0;i<waysx->number;i++)
   {
    WayX *wayx=LookupWayX(waysx,i,1);

    highways|=HIGHWAYS(wayx->way.type);
    allow   |=wayx->way.allow;
    props   |=wayx->way.props;

    SeekWriteFile(fd,&wayx->way,sizeof(Way),sizeof(WaysFile)+(off_t)wayx->prop*sizeof(Way));

    if(!((i+1)%1000))
       printf_middle("Writing Ways: Ways=%"Pindex_t,i+1);
   }

 /* Unmap from memory / close the file */

#if !SLIM
 waysx->data=UnmapFile(waysx->filename);
#else
 waysx->fd=CloseFile(waysx->fd);
#endif

 /* Write out the ways names */

 SeekFile(fd,sizeof(WaysFile)+(off_t)waysx->cnumber*sizeof(Way));

 waysx->nfd=ReOpenFile(waysx->nfilename);

 while(position<waysx->nlength)
   {
    int len=1024;
    char temp[1024];

    if((waysx->nlength-position)<1024)
       len=waysx->nlength-position;

    ReadFile(waysx->nfd,temp,len);
    WriteFile(fd,temp,len);

    position+=len;
   }

 /* Close the file */

 waysx->nfd=CloseFile(waysx->nfd);

 /* Write out the header structure */

 waysfile.number =waysx->cnumber;
 waysfile.onumber=waysx->number;

 waysfile.highways=highways;
 waysfile.allow   =allow;
 waysfile.props   =props;

 SeekFile(fd,0);
 WriteFile(fd,&waysfile,sizeof(WaysFile));

 CloseFile(fd);

 /* Print the final message */

 printf_last("Wrote Ways: Ways=%"Pindex_t,waysx->number);
}
