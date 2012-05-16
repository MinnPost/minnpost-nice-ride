/***************************************
 Merge sort functions.

 Part of the Routino routing software.
 ******************/ /******************
 This file Copyright 2009-2012 Andrew M. Bishop

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
#include <string.h>
#include <assert.h>

#include "types.h"

#include "files.h"
#include "sorting.h"


/* Global variables */

/*+ The command line '--tmpdir' option or its default value. +*/
extern char *option_tmpdirname;

/*+ The amount of RAM to use for filesorting. +*/
extern size_t option_filesort_ramsize;


/*++++++++++++++++++++++++++++++++++++++
  A function to sort the contents of a file of fixed length objects using a
  limited amount of RAM.

  The data is sorted using a "Merge sort" http://en.wikipedia.org/wiki/Merge_sort
  and in particular an "external sort" http://en.wikipedia.org/wiki/External_sorting.
  The individual sort steps and the merge step both use a "Heap sort"
  http://en.wikipedia.org/wiki/Heapsort.  The combination of the two should work well
  if the data is already partially sorted.

  index_t filesort_fixed Returns the number of objects kept.

  int fd_in The file descriptor of the input file (opened for reading and at the beginning).

  int fd_out The file descriptor of the output file (opened for writing and empty).

  size_t itemsize The size of each item in the file that needs sorting.

  int (*compare)(const void*, const void*) The comparison function (identical to qsort if the
                                           data to be sorted is an array of things not pointers).

  int (*keep)(void *,index_t) If non-NULL then this function is called for each item, if it
                              returns 1 then the object is kept and written to the output file.
  ++++++++++++++++++++++++++++++++++++++*/

index_t filesort_fixed(int fd_in,int fd_out,size_t itemsize,int (*compare)(const void*,const void*),int (*keep)(void*,index_t))
{
 int *fds=NULL,*heap=NULL;
 int nfiles=0,ndata=0;
 index_t count=0,total=0;
 size_t nitems=option_filesort_ramsize/(itemsize+sizeof(void*));
 void *data=NULL,**datap=NULL;
 char *filename;
 int i,more=1;

 /* Allocate the RAM buffer and other bits */

 data=malloc(nitems*itemsize);
 datap=malloc(nitems*sizeof(void*));

 filename=(char*)malloc(strlen(option_tmpdirname)+24);

 /* Loop around, fill the buffer, sort the data and write a temporary file */

 do
   {
    int fd,n=0;

    /* Read in the data and create pointers */

    for(i=0;i<nitems;i++)
      {
       datap[i]=data+i*itemsize;

       if(ReadFile(fd_in,datap[i],itemsize))
         {
          more=0;
          break;
         }

       total++;
      }

    n=i;

    /* Shortcut if there is no data and no previous files (i.e. no data at all) */

    if(nfiles==0 && n==0)
       goto tidy_and_exit;

    /* No new data read in this time round */

    if(n==0)
       break;

    /* Sort the data pointers using a heap sort */

    filesort_heapsort(datap,n,compare);

    /* Shortcut if all read in and sorted at once */

    if(nfiles==0 && !more)
      {
       for(i=0;i<n;i++)
         {
          if(!keep || keep(datap[i],count))
            {
             WriteFile(fd_out,datap[i],itemsize);
             count++;
            }
         }

       goto tidy_and_exit;
      }

    /* Create a temporary file and write the result */

    sprintf(filename,"%s/filesort.%d.tmp",option_tmpdirname,nfiles);

    fd=OpenFileNew(filename);

    for(i=0;i<n;i++)
       WriteFile(fd,datap[i],itemsize);

    CloseFile(fd);

    nfiles++;
   }
 while(more);

 /* Shortcut if only one file (unlucky for us there must have been exactly
    nitems, lucky for us we still have the data in RAM) */

 if(nfiles==1)
   {
    for(i=0;i<nitems;i++)
      {
       if(!keep || keep(datap[i],count))
         {
          WriteFile(fd_out,datap[i],itemsize);
          count++;
         }
      }

    DeleteFile(filename);

    goto tidy_and_exit;
   }

 /* Check that number of files is less than file size */

 assert(nfiles<nitems);

 /* Open all of the temporary files */

 fds=(int*)malloc(nfiles*sizeof(int));

 for(i=0;i<nfiles;i++)
   {
    sprintf(filename,"%s/filesort.%d.tmp",option_tmpdirname,i);

    fds[i]=ReOpenFile(filename);

    DeleteFile(filename);
   }

 /* Perform an n-way merge using a binary heap */

 heap=(int*)malloc((1+nfiles)*sizeof(int));

 /* Fill the heap to start with */

 for(i=0;i<nfiles;i++)
   {
    int index;

    datap[i]=data+i*itemsize;

    ReadFile(fds[i],datap[i],itemsize);

    index=i+1;

    heap[index]=i;

    /* Bubble up the new value */

    while(index>1)
      {
       int newindex;
       int temp;

       newindex=index/2;

       if(compare(datap[heap[index]],datap[heap[newindex]])>=0)
          break;

       temp=heap[index];
       heap[index]=heap[newindex];
       heap[newindex]=temp;

       index=newindex;
      }
   }

 /* Repeatedly pull out the root of the heap and refill from the same file */

 ndata=nfiles;

 do
   {
    int index=1;

    if(!keep || keep(datap[heap[index]],count))
      {
       WriteFile(fd_out,datap[heap[index]],itemsize);
       count++;
      }

    if(ReadFile(fds[heap[index]],datap[heap[index]],itemsize))
      {
       heap[index]=heap[ndata];
       ndata--;
      }

    /* Bubble down the new value */

    while((2*index)<ndata)
      {
       int newindex;
       int temp;

       newindex=2*index;

       if(compare(datap[heap[newindex]],datap[heap[newindex+1]])>=0)
          newindex=newindex+1;

       if(compare(datap[heap[index]],datap[heap[newindex]])<=0)
          break;

       temp=heap[newindex];
       heap[newindex]=heap[index];
       heap[index]=temp;

       index=newindex;
      }

    if((2*index)==ndata)
      {
       int newindex;
       int temp;

       newindex=2*index;

       if(compare(datap[heap[index]],datap[heap[newindex]])<=0)
          ; /* break */
       else
         {
          temp=heap[newindex];
          heap[newindex]=heap[index];
          heap[index]=temp;
         }
      }
   }
 while(ndata>0);

 /* Tidy up */

 tidy_and_exit:

 if(fds)
   {
    for(i=0;i<nfiles;i++)
       CloseFile(fds[i]);
    free(fds);
   }

 if(heap)
    free(heap);

 free(data);
 free(datap);
 free(filename);

 return(count);
}


/*++++++++++++++++++++++++++++++++++++++
  A function to sort the contents of a file of variable length objects (each
  preceded by its length in FILESORT_VARSIZE bytes) using a limited amount of RAM.

  The data is sorted using a "Merge sort" http://en.wikipedia.org/wiki/Merge_sort
  and in particular an "external sort" http://en.wikipedia.org/wiki/External_sorting.
  The individual sort steps and the merge step both use a "Heap sort"
  http://en.wikipedia.org/wiki/Heapsort.  The combination of the two should work well
  if the data is already partially sorted.

  index_t filesort_vary Returns the number of objects kept.

  int fd_in The file descriptor of the input file (opened for reading and at the beginning).

  int fd_out The file descriptor of the output file (opened for writing and empty).

  int (*compare)(const void*, const void*) The comparison function (identical to qsort if the
                                           data to be sorted is an array of things not pointers).

  int (*keep)(void *,index_t) If non-NULL then this function is called for each item, if it
                              returns 1 then the object is kept and written to the output file.
  ++++++++++++++++++++++++++++++++++++++*/

index_t filesort_vary(int fd_in,int fd_out,int (*compare)(const void*,const void*),int (*keep)(void*,index_t))
{
 int *fds=NULL,*heap=NULL;
 int nfiles=0,ndata=0;
 index_t count=0,total=0;
 FILESORT_VARINT nextitemsize,largestitemsize=0;
 void *data=NULL,**datap=NULL;
 char *filename;
 int i,more=1;

 /* Allocate the RAM buffer and other bits */

 data=malloc(option_filesort_ramsize);

 filename=(char*)malloc(strlen(option_tmpdirname)+24);

 /* Loop around, fill the buffer, sort the data and write a temporary file */

 if(ReadFile(fd_in,&nextitemsize,FILESORT_VARSIZE))    /* Always have the next item size known in advance */
    goto tidy_and_exit;

 do
   {
    int fd,n=0;
    size_t ramused=FILESORT_VARALIGN-FILESORT_VARSIZE;

    datap=data+option_filesort_ramsize;

    /* Read in the data and create pointers */

    while((ramused+FILESORT_VARSIZE+nextitemsize)<=((void*)datap-sizeof(void*)-data))
      {
       FILESORT_VARINT itemsize=nextitemsize;

       if(itemsize>largestitemsize)
          largestitemsize=itemsize;

       *(FILESORT_VARINT*)(data+ramused)=itemsize;

       ramused+=FILESORT_VARSIZE;

       ReadFile(fd_in,data+ramused,itemsize);

       *--datap=data+ramused; /* points to real data */

       ramused+=itemsize;

       ramused =FILESORT_VARALIGN*((ramused+FILESORT_VARSIZE-1)/FILESORT_VARALIGN);
       ramused+=FILESORT_VARALIGN-FILESORT_VARSIZE;

       total++;
       n++;

       if(ReadFile(fd_in,&nextitemsize,FILESORT_VARSIZE))
         {
          more=0;
          break;
         }
      }

    /* No new data read in this time round */

    if(n==0)
       break;

    /* Sort the data pointers using a heap sort */

    filesort_heapsort(datap,n,compare);

    /* Shortcut if all read in and sorted at once */

    if(nfiles==0 && !more)
      {
       for(i=0;i<n;i++)
         {
          if(!keep || keep(datap[i],count))
            {
             FILESORT_VARINT itemsize=*(FILESORT_VARINT*)(datap[i]-FILESORT_VARSIZE);

             WriteFile(fd_out,datap[i]-FILESORT_VARSIZE,itemsize+FILESORT_VARSIZE);
             count++;
            }
         }

       goto tidy_and_exit;
      }

    /* Create a temporary file and write the result */

    sprintf(filename,"%s/filesort.%d.tmp",option_tmpdirname,nfiles);

    fd=OpenFileNew(filename);

    for(i=0;i<n;i++)
      {
       FILESORT_VARINT itemsize=*(FILESORT_VARINT*)(datap[i]-FILESORT_VARSIZE);

       WriteFile(fd,datap[i]-FILESORT_VARSIZE,itemsize+FILESORT_VARSIZE);
      }

    CloseFile(fd);

    nfiles++;
   }
 while(more);

 /* Check that number of files is less than file size */

 largestitemsize=FILESORT_VARALIGN*(1+(largestitemsize+FILESORT_VARALIGN-FILESORT_VARSIZE)/FILESORT_VARALIGN);

 assert(nfiles<((option_filesort_ramsize-nfiles*sizeof(void*))/largestitemsize));

 /* Open all of the temporary files */

 fds=(int*)malloc(nfiles*sizeof(int));

 for(i=0;i<nfiles;i++)
   {
    sprintf(filename,"%s/filesort.%d.tmp",option_tmpdirname,i);

    fds[i]=ReOpenFile(filename);

    DeleteFile(filename);
   }

 /* Perform an n-way merge using a binary heap */

 heap=(int*)malloc((1+nfiles)*sizeof(int));

 datap=data+option_filesort_ramsize-nfiles*sizeof(void*);

 /* Fill the heap to start with */

 for(i=0;i<nfiles;i++)
   {
    int index;
    FILESORT_VARINT itemsize;

    datap[i]=data+FILESORT_VARALIGN-FILESORT_VARSIZE+i*largestitemsize;

    ReadFile(fds[i],&itemsize,FILESORT_VARSIZE);

    *(FILESORT_VARINT*)(datap[i]-FILESORT_VARSIZE)=itemsize;

    ReadFile(fds[i],datap[i],itemsize);

    index=i+1;

    heap[index]=i;

    /* Bubble up the new value */

    while(index>1)
      {
       int newindex;
       int temp;

       newindex=index/2;

       if(compare(datap[heap[index]],datap[heap[newindex]])>=0)
          break;

       temp=heap[index];
       heap[index]=heap[newindex];
       heap[newindex]=temp;

       index=newindex;
      }
   }

 /* Repeatedly pull out the root of the heap and refill from the same file */

 ndata=nfiles;

 do
   {
    int index=1;
    FILESORT_VARINT itemsize;

    if(!keep || keep(datap[heap[index]],count))
      {
       itemsize=*(FILESORT_VARINT*)(datap[heap[index]]-FILESORT_VARSIZE);

       WriteFile(fd_out,datap[heap[index]]-FILESORT_VARSIZE,itemsize+FILESORT_VARSIZE);
       count++;
      }

    if(ReadFile(fds[heap[index]],&itemsize,FILESORT_VARSIZE))
      {
       heap[index]=heap[ndata];
       ndata--;
      }
    else
      {
       *(FILESORT_VARINT*)(datap[heap[index]]-FILESORT_VARSIZE)=itemsize;

       ReadFile(fds[heap[index]],datap[heap[index]],itemsize);
      }

    /* Bubble down the new value */

    while((2*index)<ndata)
      {
       int newindex;
       int temp;

       newindex=2*index;

       if(compare(datap[heap[newindex]],datap[heap[newindex+1]])>=0)
          newindex=newindex+1;

       if(compare(datap[heap[index]],datap[heap[newindex]])<=0)
          break;

       temp=heap[newindex];
       heap[newindex]=heap[index];
       heap[index]=temp;

       index=newindex;
      }

    if((2*index)==ndata)
      {
       int newindex;
       int temp;

       newindex=2*index;

       if(compare(datap[heap[index]],datap[heap[newindex]])<=0)
          ; /* break */
       else
         {
          temp=heap[newindex];
          heap[newindex]=heap[index];
          heap[index]=temp;
         }
      }
   }
 while(ndata>0);

 /* Tidy up */

 tidy_and_exit:

 if(fds)
   {
    for(i=0;i<nfiles;i++)
       CloseFile(fds[i]);
    free(fds);
   }

 if(heap)
    free(heap);

 free(data);
 free(filename);

 return(count);
}


/*++++++++++++++++++++++++++++++++++++++
  A function to sort an array of pointers efficiently.

  The data is sorted using a "Heap sort" http://en.wikipedia.org/wiki/Heapsort,
  in particular, this is good because it can operate in-place and doesn't
  allocate more memory like using qsort() does.

  void **datap A pointer to the array of pointers to sort.

  size_t nitems The number of items of data to sort.

  int(*compare)(const void *, const void *) The comparison function (identical to qsort if the
                                            data to be sorted was an array of things not pointers).
  ++++++++++++++++++++++++++++++++++++++*/

void filesort_heapsort(void **datap,size_t nitems,int(*compare)(const void*, const void*))
{
 void **datap1=&datap[-1];
 int i;

 /* Fill the heap by pretending to insert the data that is already there */

 for(i=2;i<=nitems;i++)
   {
    int index=i;

    /* Bubble up the new value (upside-down, put largest at top) */

    while(index>1)
      {
       int newindex;
       void *temp;

       newindex=index/2;

       if(compare(datap1[index],datap1[newindex])<=0) /* reversed compared to filesort_fixed() above */
          break;

       temp=datap1[index];
       datap1[index]=datap1[newindex];
       datap1[newindex]=temp;

       index=newindex;
      }
   }

 /* Repeatedly pull out the root of the heap and swap with the bottom item */

 for(i=nitems;i>1;i--)
   {
    int index=1;
    void *temp;

    temp=datap1[index];
    datap1[index]=datap1[i];
    datap1[i]=temp;

    /* Bubble down the new value (upside-down, put largest at top) */

    while((2*index)<(i-1))
      {
       int newindex;
       void *temp;

       newindex=2*index;

       if(compare(datap1[newindex],datap1[newindex+1])<=0) /* reversed compared to filesort_fixed() above */
          newindex=newindex+1;

       if(compare(datap1[index],datap1[newindex])>=0) /* reversed compared to filesort_fixed() above */
          break;

       temp=datap1[newindex];
       datap1[newindex]=datap1[index];
       datap1[index]=temp;

       index=newindex;
      }

    if((2*index)==(i-1))
      {
       int newindex;
       void *temp;

       newindex=2*index;

       if(compare(datap1[index],datap1[newindex])>=0) /* reversed compared to filesort_fixed() above */
          ; /* break */
       else
         {
          temp=datap1[newindex];
          datap1[newindex]=datap1[index];
          datap1[index]=temp;
         }
      }
   }
}
