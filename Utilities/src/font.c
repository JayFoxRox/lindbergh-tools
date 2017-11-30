#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  uint32_t renderOffset; //?
  uint32_t renderHeight; //?
  uint32_t width;
  uint32_t height;
  uint32_t lineHeight; //?
  uint16_t firstSymbol;
  uint16_t lastSymbol;
  uint32_t symbols;
} AbcHeader;

typedef struct {
  float u1;
  float v1;
  float u2;
  float v2;
  uint16_t leftMargin; // left margin?
  uint16_t width;
  uint16_t rightMargin; // right margin?
  uint16_t pad;
} AbcSymbol;

int main() {

  // Open the file
  FILE* f = fopen("/home/fox/Data/Projects/The Firm/Lindbergh Replacement Kit/software/device-emulator/fs/usr/lib/boot/LucidaConsole_12.abc","r");

  // Check if the file was opened correctly
  if (f == NULL) {
    return 1;
  }

  // Now read the header
  AbcHeader header;
  fread(&header,sizeof(header),1,f);

  // Image description
  printf("Image: %ix%i\n",header.width,header.height);
  printf("Symbols: %i (%i to %i)\n",header.symbols,header.firstSymbol,header.lastSymbol);
//  printf("Font height?: %i, %i, %i\n",header.unk2,header.unk1,header.unk0);

  // Symbol data follows
  char currentSymbol = header.firstSymbol;
  while(!feof(f)) { 
    AbcSymbol symbol;
    fread(&symbol,sizeof(symbol),1,f);
    unsigned int u1 = symbol.u1*header.width;
    unsigned int u2 = symbol.u2*header.width;
    unsigned int v1 = symbol.v1*header.height;
    unsigned int v2 = symbol.v2*header.height;
    printf("%f to %f; %f to %f\t(%i,%i - %i,%i)\t%ix%i\t%i to %i, width: %i, %i # %c\n",symbol.u1,symbol.u2,symbol.v1,symbol.v2,u1,v1,u2,v2,u2-u1,v2-v1,symbol.leftMargin,symbol.rightMargin,symbol.width,currentSymbol++);
  }

  // Now we can close and return succesfully
  fclose(f);
  return 0;

}
