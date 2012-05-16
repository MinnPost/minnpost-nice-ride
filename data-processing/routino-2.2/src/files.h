/***************************************
 Header file for file function prototypes

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


#ifndef FILES_H
#define FILES_H    /*+ To stop multiple inclusions. +*/

/* If your system does not have the pread() and pwrite() system calls then you
 * will need to change this line to the value 0 so that seek() and
 * read()/write() are used instead of pread()/pwrite(). */
#define HAVE_PREAD_PWRITE 1


#include <assert.h>
#include <unistd.h>
#include <sys/types.h>


/* Functions in files.c */

char *FileName(const char *dirname,const char *prefix, const char *name);

void *MapFile(const char *filename);
void *MapFileWriteable(const char *filename);
void *UnmapFile(const char *filename);

int OpenFileNew(const char *filename);
int OpenFileAppend(const char *filename);
int ReOpenFile(const char *filename);
int ReOpenFileWriteable(const char *filename);

static int WriteFile(int fd,const void *address,size_t length);
static int ReadFile(int fd,void *address,size_t length);

static int SeekWriteFile(int fd,const void *address,size_t length,off_t position);
static int SeekReadFile(int fd,void *address,size_t length,off_t position);

off_t SizeFile(const char *filename);
int ExistsFile(const char *filename);

static int SeekFile(int fd,off_t position);

int CloseFile(int fd);

int DeleteFile(char *filename);


/* Inline the frequently called functions */

/*++++++++++++++++++++++++++++++++++++++
  Write data to a file descriptor.

  int WriteFile Returns 0 if OK or something else in case of an error.

  int fd The file descriptor to write to.

  const void *address The address of the data to be written.

  size_t length The length of data to write.
  ++++++++++++++++++++++++++++++++++++++*/

static inline int WriteFile(int fd,const void *address,size_t length)
{
 assert(fd!=-1);

 /* Write the data */

 if(write(fd,address,length)!=length)
    return(-1);

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  Read data from a file descriptor.

  int ReadFile Returns 0 if OK or something else in case of an error.

  int fd The file descriptor to read from.

  void *address The address the data is to be read into.

  size_t length The length of data to read.
  ++++++++++++++++++++++++++++++++++++++*/

static inline int ReadFile(int fd,void *address,size_t length)
{
 assert(fd!=-1);

 /* Read the data */

 if(read(fd,address,length)!=length)
    return(-1);

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  Seek to a position in a file descriptor.

  int SeekFile Returns 0 if OK or something else in case of an error.

  int fd The file descriptor to seek within.

  off_t position The position to seek to.
  ++++++++++++++++++++++++++++++++++++++*/

static inline int SeekFile(int fd,off_t position)
{
 assert(fd!=-1);

 /* Seek the data */

 if(lseek(fd,position,SEEK_SET)!=position)
    return(-1);

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  Write data to a file descriptor after seeking to a position.

  int SeekWriteFile Returns 0 if OK or something else in case of an error.

  int fd The file descriptor to write to.

  const void *address The address of the data to be written.

  size_t length The length of data to write.

  off_t position The position to seek to.
  ++++++++++++++++++++++++++++++++++++++*/

static inline int SeekWriteFile(int fd,const void *address,size_t length,off_t position)
{
 assert(fd!=-1);

 /* Seek and write the data */

#if HAVE_PREAD_PWRITE

 if(pwrite(fd,address,length,position)!=length)
    return(-1);

#else

 if(lseek(fd,position,SEEK_SET)!=position)
    return(-1);

 if(write(fd,address,length)!=length)
    return(-1);

#endif

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  Read data from a file descriptor after seeking to a position.

  int SeekReadFile Returns 0 if OK or something else in case of an error.

  int fd The file descriptor to read from.

  void *address The address the data is to be read into.

  size_t length The length of data to read.

  off_t position The position to seek to.
  ++++++++++++++++++++++++++++++++++++++*/

static inline int SeekReadFile(int fd,void *address,size_t length,off_t position)
{
 assert(fd!=-1);

 /* Seek and read the data */

#if HAVE_PREAD_PWRITE

 if(pread(fd,address,length,position)!=length)
    return(-1);

#else

 if(lseek(fd,position,SEEK_SET)!=position)
    return(-1);

 if(read(fd,address,length)!=length)
    return(-1);

#endif

 return(0);
}


#endif /* FILES_H */
