// ini <file> <section> <key>
// Examples: ini test.ini sec ke
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../inc/ini.h"

static int handler(void* user, const char* section, const char* name, const char* value) {

  char** argv = (char**)user;

  const char* _section = argv[2];
  const char* key = argv[3];

  #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
  if (MATCH(_section, key)) {
    printf("%s",value);
    exit(0);
  }

  return 0;

}

int main(int argc, char* argv[]) {
  
  if (argc != 4) {
    printf("invalid arguments (%i)\n",argc);
    return 1;
  }

  const char* file = argv[1];

  if (ini_parse(file, handler, argv) < 0) {
    printf("invalid file (%s)\n",file);
    return 1;
  }

  const char* _section = argv[2];
  const char* key = argv[3];

  printf("invalid section/key (%s/%s)\n",_section,key);
  return 1;

}


