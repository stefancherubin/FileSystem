/*
  Elan Moyal
  gcc -g -Wall filesystem.c `pkg-config fuse --cflags --libs` -o filesystem
  for permission issues:
  sudo chown root /usr/local/bin/fusermount
  sudo chmod u+s /usr/local/bin/fusermount

*/

#define FUSE_USE_VERSION 30
#define _FILE_OFFSET_BITS  64

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef linux
#define _XOPEN_SOURCE 700
#endif

#include <fuse.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#endif

const int globalMaxBlocks = 10000;
const int globalMaxFileSize = 4096;
char* globalBlockName = "fusedata.";
char* globalFilePath = "/home/elan/Documents/FileSystem/temp/fusedata/";

char* concat(char* first, char* second){
  char* result = malloc(strlen(first) + strlen(second) + 1);
  strcpy(result, first);
  strcat(result, second);
  return result;
}

void* nemInit(){
  printf("Inside Init\n");
  char* fileNameStart = concat(globalFilePath, globalBlockName); // /tmp/fuse
  char* fileNameComplete = concat(fileNameStart, "0");
  FILE* fd = fopen(fileNameComplete, "r");
  if(fd != NULL){
    fclose(fd);
    free(fileNameComplete);
    return NULL;
  }
  free(fd);
  int i;
  for(i = 0; i < globalMaxBlocks; ++i){
    char num[6];
    sprintf(num, "%d", i);
    char* tmpName = concat(fileNameStart, num);
    printf("Inside Loop\n");
    fd = fopen(tmpName, "w+");
    char* buffer[globalMaxFileSize];
    memset(buffer, "0", globalMaxFileSize);
    fwrite(buffer, globalMaxFileSize, sizeof(char), fd);
    fclose(fd);
  }
  return NULL;
}

int nemGetattr(const char* path, struct stat* stbuf){
  /*
  int res = 0;

  memset(stbuf, 0, sizeof(struct stat));
  if(strcmp(path, "/") == 0){
    stbuf -> st_mode = S_IFDIR | 0755;
    stbuf -> st_nlink = 2;
  }
  else if(strcmp(path, globalFilePath) == 0){
    stbuf -> st_mode = S_IFREG | 0777;
    stbuf -> st_nlink = 1;
    stbuf -> st_size = globalMaxFileSize * globalMaxBlocks;
  }
  else{
    res = -ENOENT;
  }
  return res;
  */
  int res;
res = lstat(path, stbuf);
if (res == -1)
        return -errno;
return 0;
}

static int nemReaddir(const char *path, void *buf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info *fi)
{
        (void) offset;
        (void) fi;
        if (strcmp(path, "/") != 0)
                return -ENOENT;
        filler(buf, ".", NULL, 0);
        filler(buf, "..", NULL, 0);
        filler(buf, globalFilePath + 1, NULL, 0);
        return 0;
}
static int nemOpen(const char *path, struct fuse_file_info *fi)
{
        if (strcmp(path, globalFilePath) != 0)
                return -ENOENT;
        if ((fi->flags & 3) != O_RDONLY)
                return -EACCES;
        return 0;
}
static int nemRead(const char *path, char *buf, size_t size, off_t offset,
                    struct fuse_file_info *fi)
{
        int fd;
        int res;
        (void) fi;
        fd = open(path, O_RDONLY);
        if (fd == -1)
                return -errno;
        res = pread(fd, buf, size, offset);
        if (res == -1)
                res = -errno;
        close(fd);
        return res;
}

struct fuse_operations nemOperations = {
  //.init = nemInit,
  .getattr = nemGetattr,
  .readdir = nemReaddir,
  .open    = nemOpen,
  .read    = nemRead,
};

int main(int argc, char* argv[]){

  printf("Inside Main\n");
  umask(0);
  nemInit();
  return fuse_main(argc, argv, &nemOperations, NULL);
}
