// Generic stuff


int openForOutput(const char* path) {
  int f = open(path,O_WRONLY | O_CREAT | O_TRUNC, 0644); 
  if (f < 0) {
    printf("Unable to open destination file (%s)!\n",path);
    exit(1);
  }
  return f;
}

int openForInput(const char* path) {
  int f = open(path,O_RDONLY);
  if (f < 0) {
    printf("Unable to open source file (%s)!\n",path);
    exit(1);
  }
  return f;
}

