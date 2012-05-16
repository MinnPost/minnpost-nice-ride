/***************************************
 Functions to handle files.

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


#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>

#include "files.h"


/*+ A structure to contain the list of memory mapped files. +*/
struct mmapinfo
{
 const char  *filename;         /*+ The name of the file (the index of the list). +*/
       int    fd;               /*+ The file descriptor used when it was opened. +*/
       void  *address;          /*+ The address the file was mapped to. +*/
       size_t length;           /*+ The length of the file. +*/
};

/*+ The list of memory mapped files. +*/
static struct mmapinfo *mappedfiles;

/*+ The number of mapped files. +*/
static int nmappedfiles=0;


/*++++++++++++++++++++++++++++++++++++++
  Return a filename composed of the dirname, prefix and name.

  char *FileName Returns a pointer to memory allocated to the filename.

  const char *dirname The directory name.

  const char *prefix The file prefix.

  const char *name The main part of the name.
  ++++++++++++++++++++++++++++++++++++++*/

char *FileName(const char *dirname,const char *prefix, const char *name)
{
 char *filename=(char*)malloc((dirname?strlen(dirname):0)+1+(prefix?strlen(prefix):0)+1+strlen(name)+1);

 sprintf(filename,"%s%s%s%s%s",dirname?dirname:"",dirname?"/":"",prefix?prefix:"",prefix?"-":"",name);

 return(filename);
}


/*++++++++++++++++++++++++++++++++++++++
  Open a file read-only and map it into memory.

  void *MapFile Returns the address of the file or exits in case of an error.

  const char *filename The name of the file to open.
  ++++++++++++++++++++++++++++++++++++++*/

void *MapFile(const char *filename)
{
 int fd;
 off_t size;
 void *address;

 /* Open the file and get its size */

 fd=ReOpenFile(filename);

 size=SizeFile(filename);

 /* Map the file */

 address=mmap(NULL,size,PROT_READ,MAP_SHARED,fd,0);

 if(address==MAP_FAILED)
   {
    close(fd);

    fprintf(stderr,"Cannot mmap file '%s' for reading [%s].\n",filename,strerror(errno));
    exit(EXIT_FAILURE);
   }

 /* Store the information about the mapped file */

 mappedfiles=(struct mmapinfo*)realloc((void*)mappedfiles,(nmappedfiles+1)*sizeof(struct mmapinfo));

 mappedfiles[nmappedfiles].filename=filename;
 mappedfiles[nmappedfiles].fd=fd;
 mappedfiles[nmappedfiles].address=address;
 mappedfiles[nmappedfiles].length=size;

 nmappedfiles++;

 return(address);
}


/*++++++++++++++++++++++++++++++++++++++
  Open a file read-write and map it into memory.

  void *MapFileWriteable Returns the address of the file or exits in case of an error.

  const char *filename The name of the file to open.
  ++++++++++++++++++++++++++++++++++++++*/

void *MapFileWriteable(const char *filename)
{
 int fd;
 off_t size;
 void *address;

 /* Open the file and get its size */

 fd=ReOpenFileWriteable(filename);

 size=SizeFile(filename);

 /* Map the file */

 address=mmap(NULL,size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);

 if(address==MAP_FAILED)
   {
    close(fd);

    fprintf(stderr,"Cannot mmap file '%s' for reading and writing [%s].\n",filename,strerror(errno));
    exit(EXIT_FAILURE);
   }

 /* Store the information about the mapped file */

 mappedfiles=(struct mmapinfo*)realloc((void*)mappedfiles,(nmappedfiles+1)*sizeof(struct mmapinfo));

 mappedfiles[nmappedfiles].filename=filename;
 mappedfiles[nmappedfiles].fd=fd;
 mappedfiles[nmappedfiles].address=address;
 mappedfiles[nmappedfiles].length=size;

 nmappedfiles++;

 return(address);
}


/*++++++++++++++++++++++++++++++++++++++
  Unmap a file and close it.

  void *UnmapFile Returns NULL (for similarity to the MapFile function).

  const char *filename The name of the file when it was opened.
  ++++++++++++++++++++++++++++++++++++++*/

void *UnmapFile(const char *filename)
{
 int i;

 for(i=0;i<nmappedfiles;i++)
    if(!strcmp(mappedfiles[i].filename,filename))
       break;

 if(i==nmappedfiles)
   {
    fprintf(stderr,"The file '%s' was not mapped using MapFile().\n",filename);
    exit(EXIT_FAILURE);
   }

 /* Close the file */

 close(mappedfiles[i].fd);

 /* Unmap the file */

 munmap(mappedfiles[i].address,mappedfiles[i].length);

 /* Shuffle the list of files */

 nmappedfiles--;

 if(nmappedfiles>i)
    memmove(&mappedfiles[i],&mappedfiles[i+1],(nmappedfiles-i)*sizeof(struct mmapinfo));

 return(NULL);
}


/*++++++++++++++++++++++++++++++++++++++
  Open a new file on disk for writing.

  int OpenFileNew Returns the file descriptor if OK or exits in case of an error.

  const char *filename The name of the file to create.
  ++++++++++++++++++++++++++++++++++++++*/

int OpenFileNew(const char *filename)
{
 int fd;

 /* Open the file */

 fd=open(filename,O_RDWR|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);

 if(fd<0)
   {
    fprintf(stderr,"Cannot open file '%s' for writing [%s].\n",filename,strerror(errno));
    exit(EXIT_FAILURE);
   }

 return(fd);
}


/*++++++++++++++++++++++++++++++++++++++
  Open a new or existing file on disk for reading and appending.

  int OpenFileAppend Returns the file descriptor if OK or exits in case of an error.

  const char *filename The name of the file to create or open.
  ++++++++++++++++++++++++++++++++++++++*/

int OpenFileAppend(const char *filename)
{
 int fd;

 /* Open the file */

 fd=open(filename,O_RDWR|O_CREAT|O_APPEND,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);

 if(fd<0)
   {
    fprintf(stderr,"Cannot open file '%s' for appending [%s].\n",filename,strerror(errno));
    exit(EXIT_FAILURE);
   }

 return(fd);
}


/*++++++++++++++++++++++++++++++++++++++
  Open an existing file on disk for reading.

  int ReOpenFile Returns the file descriptor if OK or exits in case of an error.

  const char *filename The name of the file to open.
  ++++++++++++++++++++++++++++++++++++++*/

int ReOpenFile(const char *filename)
{
 int fd;

 /* Open the file */

 fd=open(filename,O_RDONLY);

 if(fd<0)
   {
    fprintf(stderr,"Cannot open file '%s' for reading [%s].\n",filename,strerror(errno));
    exit(EXIT_FAILURE);
   }

 return(fd);
}


/*++++++++++++++++++++++++++++++++++++++
  Open an existing file on disk for reading or writing.

  int ReOpenFileWriteable Returns the file descriptor if OK or exits in case of an error.

  const char *filename The name of the file to open.
  ++++++++++++++++++++++++++++++++++++++*/

int ReOpenFileWriteable(const char *filename)
{
 int fd;

 /* Open the file */

 fd=open(filename,O_RDWR);

 if(fd<0)
   {
    fprintf(stderr,"Cannot open file '%s' for reading and writing [%s].\n",filename,strerror(errno));
    exit(EXIT_FAILURE);
   }

 return(fd);
}


/*++++++++++++++++++++++++++++++++++++++
  Get the size of a file.

  off_t SizeFile Returns the file size if OK or exits in case of an error.

  const char *filename The name of the file to check.
  ++++++++++++++++++++++++++++++++++++++*/

off_t SizeFile(const char *filename)
{
 struct stat buf;

 if(stat(filename,&buf))
   {
    fprintf(stderr,"Cannot stat file '%s' [%s].\n",filename,strerror(errno));
    exit(EXIT_FAILURE);
   }

 return(buf.st_size);
}


/*++++++++++++++++++++++++++++++++++++++
  Check if a file exists.

  int ExistsFile Returns 1 if the file exists and 0 if not.

  const char *filename The name of the file to check.
  ++++++++++++++++++++++++++++++++++++++*/

int ExistsFile(const char *filename)
{
 struct stat buf;

 if(stat(filename,&buf))
    return(0);
 else
    return(1);
}


/*++++++++++++++++++++++++++++++++++++++
  Close a file on disk.

  int CloseFile returns -1 (for similarity to the *OpenFile* functions).

  int fd The file descriptor to close.
  ++++++++++++++++++++++++++++++++++++++*/

int CloseFile(int fd)
{
 close(fd);

 return(-1);
}


/*++++++++++++++++++++++++++++++++++++++
  Delete a file from disk.

  int DeleteFile Returns 0 if OK.

  char *filename The name of the file to delete.
  ++++++++++++++++++++++++++++++++++++++*/

int DeleteFile(char *filename)
{
 unlink(filename);

 return(0);
}
