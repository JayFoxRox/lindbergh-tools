#include <stropts.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// SHM via Baseboard

static int lbb = -1;

void closeBaseboard(void) {
  if (lbb >= 0) {
    close(lbb);
    lbb = -1;
  }
}

int openBaseboard(void) {
  lbb = open("/dev/lbb",O_RDWR);
  if (lbb < 0) {
    printf("Failed to access baseboard!\n");
    exit(1);
  }
  atexit(closeBaseboard);
  return lbb;
}

int seekShm(unsigned int offset) {
  return ioctl(lbb,0x400,offset);
}

int writeShm(unsigned int offset, void* buffer, unsigned int size) {
  //FIXME: Mutex
  seekShm(offset);
  return write(lbb,buffer,size);
}

void writeShmFromFile(unsigned int offset, int fd, unsigned int size) {
  void* buffer = malloc(size);
//lseek(fd,offset,SEEK_SET);
  lseek(fd,0,SEEK_SET);
  read(fd,buffer,size);
  writeShm(offset,buffer,size);
  free(buffer);
  return;
}

int readShm(void* buffer, unsigned int offset, unsigned int size) {
  //FIXME: Mutex
  seekShm(offset);
  return read(lbb,buffer,size);
}

void readShmToFile(int fd, unsigned int offset, unsigned int size) {
  void* buffer = malloc(size);
  readShm(buffer,offset,size);
//lseek(fd,offset,SEEK_SET);
  lseek(fd,0,SEEK_SET);
  write(fd,buffer,size);
  free(buffer);
  return;
}

int readSram(void* buffer, unsigned int offset, unsigned int size) {
  typedef struct {
    uint32_t data;
    uint32_t offset;
    uint32_t size;
  } readData_t;

  uint8_t* bytes = buffer;
  static unsigned int chunkSize = 0x80;
  while (size > chunkSize) {
    readSram(bytes,offset,chunkSize);
    bytes = &bytes[chunkSize];
    offset += chunkSize;
    size -= chunkSize;
  }

  readData_t d_;
  readData_t* d = &d_;
  d->data = (uintptr_t)bytes;
  d->offset = offset;
  d->size = size;

  int ret = ioctl(lbb,0x601,d);
  if (ret < 0) {
    printf("Something went wrong while reading %i bytes from 0x%X from SRAM (%i)\n",size,offset,ret);
  }
//usleep(1000);
  return ret;
}

void readSramToFile(int fd, unsigned int offset, unsigned int size) {
  void* buffer = malloc(size);
  readSram(buffer,offset,size);
//lseek(fd,offset,SEEK_SET);
  lseek(fd,0,SEEK_SET);
  write(fd,buffer,size);
  free(buffer);
  return;
}

int writeSram(unsigned int offset, void* buffer, unsigned int size) {
  typedef struct {
    uint32_t offset;
    uint32_t data;
    uint32_t size;
  } writeData_t;

  uint8_t* bytes = buffer;
  static unsigned int chunkSize = 0x80;
  while (size > chunkSize) {
    writeSram(offset,bytes,chunkSize);
    bytes = &bytes[chunkSize];
    offset += chunkSize;
    size -= chunkSize;
  }

  writeData_t d_;
  writeData_t* d = &d_;
  d->offset = offset;
  d->data = (uintptr_t)bytes;
  d->size = size;

  int ret = ioctl(lbb,0x600,d);
  if (ret < 0) {
    printf("Something went wrong while writing %i bytes to 0x%X from SRAM (%i)\n",size,offset,ret);
  }
//usleep(1000);
  return ret;
}

void writeSramFromFile(unsigned int offset, int fd, unsigned int size) {
  void* buffer = malloc(size);
//lseek(fd,offset,SEEK_SET);
  lseek(fd,0,SEEK_SET);
  read(fd,buffer,size);
  writeSram(offset,buffer,size);
  free(buffer);
  return;
}
