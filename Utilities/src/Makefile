CC=clang -m32
LD=clang -m32
CFLAGS=-O0 -g
LDFLAGS=$(CFLAGS)

all: ../bin/ini ../bin/jvs ../bin/backup ../bin/sound

clean:
	rm -f *.o ../bin/ini ../bin/jvs ../bin/backup ../bin/sound

../bin/ini: ini.o ../inc/ini.o
	$(LD) $(LDFLAGS) -o $@ $^
../bin/jvs: jvs.o
	$(LD) $(LDFLAGS) -o $@ $^
../bin/backup: backup.o
	$(LD) $(LDFLAGS) -o $@ $^
../bin/sound: sound.o
	$(LD) $(LDFLAGS) -L../inc/  $^ -lsegaapi -lstdc++ -o $@

