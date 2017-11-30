// (c)2012 Jannik Vogel

// Compile using: reset && clear && clang backup.c /usr/lib32/crt1.o /usr/lib32/crti.o /usr/lib32/crtn.o -m32 -nostdlib -O0 -g -lgcc_s -lc -o backup && gdb ./backup save

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stropts.h>
#include <unistd.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>



// Game database

typedef struct {
  unsigned int uidSize;
  char uid[8];
  const char* title;
  const char* shortname;
  struct {
    unsigned int sramOffset;
    unsigned int eepromOffset;
    const char* files[];
  } backup;
} game_t;

game_t games[] = {
  { 4, "SBLR", "After Burner Climax", "abc", { 0x0C000, 0x1000 }},
  { 4, "SBKX", "Virtua Tennis 3", "vt3", { 0x0C000, 0x1000 }},
  { 4, "SBMB", "Outrun 2", "or2", { 0x10000, 0x1000 }}
};
const unsigned int gamesCount = sizeof(games) / sizeof(game_t);






#include "shm.h"
#include "eeprom.h"


#include "files.h"






















game_t* getGame(const char uid[8]) {
  unsigned int gameIndex;
  for(gameIndex = 0; gameIndex < gamesCount; gameIndex++) {
    game_t* game = &games[gameIndex];
    if (!memcmp(uid,game->uid,game->uidSize)) {
      return game;
    }  
  }
  return NULL;
}


void readHeader(int fd, unsigned int address) {

  char uid[8];
  uint32_t crc;
  int f = fd;

  lseek(f,address,SEEK_SET);
  read(f,&crc,4);
  printf("CRC: 0x%08X\n",crc);
  {
    read(f,uid,8);
    printf("UID: %.8s",uid);
    game_t* game = getGame(uid);
    if (game == NULL) {
      printf(" (%s)",game->title);
    }
    printf("\n");
  }

  return;

}

int main(int argc, char* argv[]) {
  if (/*(argc != 2) &&*/ (argc != 4)) {
    printf("%s <save/load> <sram-file> <eeprom-file>\n",argv[0]);
    return 1;
  }

  char* shmFile = argv[2];
  char* smbusFile = argv[3];

/*
  if (!strcmp(argv[1],"info")) {

    if (argc == 2) {
      printf("This action requires 4 parameters!\n");
      exit(1);
    }
  
    {
      printf("SRAM Information:\n");
      int f = openForInput(shmFile);
      readHeader(f,0x0C000);
      //TODO: should be 0x10000 in outrun
      //TODO: Record 2?
      close(f);
    }
    printf("\n");
    {
      printf("EEPROM Information:\n");
      int f = openForInput(smbusFile);
      readHeader(f,0x1000);
      //TODO: Record 2?
      close(f);
    }

    return 0;

  }
*/

  openBaseboard();
  openI2c();

/*
  if (argc == 2) {

    uint8_t sramUid[8];
    readSram(sramUid,0x0C004,8);
    // TODO: Located at 0x10000 in outrun 2 instead!
    shmFile = alloca(32);
    sprintf(shmFile,"%.4s-sram.bin",sramUid);

    uint8_t eepromUid[8];
    readSmbus(eepromUid,0x1004,8);
    smbusFile = alloca(32);
    sprintf(smbusFile,"%.4s-eeprom.bin",eepromUid);

    printf("Will use the following files: %s, %s\n",shmFile,smbusFile); 
    //TODO: Use shortname in the future?
  
  }
*/
  
  if (!strcmp(argv[1],"save")) {

    {
      int f = openForOutput(shmFile);
      readSramToFile(f,0x0C000,0x20000);    
      close(f);  
    }
  
    {
      int f = openForOutput(smbusFile);
      readSmbusToFile(f,0x1000,0x100);
      close(f);
    }

    return 0;

  }

  if (!strcmp(argv[1],"load")) {

    {
      int f = openForInput(shmFile);
      writeSramFromFile(0x0C000,f,0x20000);    
      close(f);
    }
  
    {
      int f = openForInput(smbusFile);
      writeSmbusFromFile(0x1000,f,0x100);
      close(f);
    }

    return 0;

  }

  printf("Invalid option: %s\n",argv[1]);
  
  return 1;
}
