// reset && clear && clang -m32 apihijack.c -lelf -ldl -shared -fPIC -o apihijack.so 
// LD_LIBRARY_PATH=. LD_PRELOAD=apihijack.so ./abc

#define _GNU_SOURCE
#include <dlfcn.h>
#include <sys/mman.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <libelf.h>
#include <gelf.h>

uint8_t* originalMain = (uint8_t*)0x8061b60;
/*
#define API(address, function, returnType, ...) \
returnType(*function)(__VA_ARGS__) = (returnType(*)(__VA_ARGS__))address;
*/
/*
#define API(address, function, returnType, ...) \
extern returnType function(__VA_ARGS__);
*/


void* findSymbol(const char* title) {
  //0x8048000 ELF address

  Elf *elf;                       /* Our Elf pointer for libelf */
  Elf_Scn *scn;                   /* Section Descriptor */
  Elf_Data *edata;                /* Data Descriptor */
  GElf_Sym sym;			/* Symbol */
  GElf_Shdr shdr;                 /* Section Header */

  // Iterate through section headers again this time well stop when we find symbols 
  elf = elf_memory((void*)0x8048000, 1024*1024*128); // 128 MB image max..

printf("Loaded elf: %i\n",elf);

  int symbol_count;
  int i;

  while((scn = elf_nextscn(elf, scn)) != NULL) {
    printf("Checking section\n");
    gelf_getshdr(scn, &shdr);

    // When we find a section header marked SHT_SYMTAB stop and get symbols
    if(shdr.sh_type == SHT_SYMTAB) {
      // edata points to our symbol table
      edata = elf_getdata(scn, edata);

      // how many symbols are there? this number comes from the size of
      // the section divided by the entry size
      symbol_count = shdr.sh_size / shdr.sh_entsize;

      // loop through to grab all symbols
      for(i = 0; i < symbol_count; i++) {			
      // libelf grabs the symbol data using gelf_getsym()
        gelf_getsym(edata, i, &sym);

        // print out the value and size
        printf("%08x %08d ", sym.st_value, sym.st_size);

        printf("%s\n", elf_strptr(elf, shdr.sh_link, sym.st_name));

      }
    }
  }

  elf_end(elf);

  return NULL;
}

#define API(address, function, returnType, ...) \
returnType(*function ## x)(__VA_ARGS__) = NULL; \
void __attribute__ ((constructor)) resolve ## function(void) { \
/*function ## x = dlsym(RTLD_DEFAULT, #function); */ \
  function ## x = address; /*findSymbol(#function);*/ \
  if (!function ## x) { \
    printf("Unable to resolve '%s'!\n",#function); \
    exit(1); \
  } \
  printf("Resolved '%s' to 0x%08X\n",#function,(uintptr_t)function ## x); \
  return; \
}



// API

API(0x081e3424, amDongleInit, int, void) //__cdecl 
/*
API(0x081e360b, amDongleExit)
API(0x081e369e, amDongleUpdate)
API(0x081e3772, amDongleIsAvailable)
API(0x081e3792, amDongleIsDevelop)
API(0x081e37b2, amDongleSetWorkaround)
API(0x081e380b, amDongleChipInfo)
API(0x081e389e, amDongleUserInfo)
API(0x081e394d, amDongleUserInfoEx)
API(0x081e39c7, amDongleReadEeprom)
API(0x081e3a78, amDongleReadEepromEx)
API(0x081e3af2, amDongleWriteEeprom)
API(0x081e3b68, amDongleWriteEepromEx)
API(0x081e3be2, amDongleReadSram)
API(0x081e3ca4, amDongleReadSramEx)
API(0x081e3d1e, amDongleWriteSram)
API(0x081e3da5, amDongleWriteSramEx)
API(0x081e3e1f, amDongleSetIv)
API(0x081e3e75, amDongleSetSeedSend)
API(0x081e3f0a, amDongleSetSeedRecv)
API(0x081e3f9e, amDongleDecrypt)
API(0x081e4057, amDongleDecryptEx)
API(0x081e4184, amDongleEncrypt)
API(0x081e423e, amDongleEncryptEx)
API(0x081e4369, amDongleReceive)
API(0x081e449e, amDongleReceiveEx)
API(0x081e44fd, amDongleSend)
API(0x081e46a1, amDongleSendEx)
*/
/*
0x081e55b8  amJvsInit
0x081e5a0d  amJvsExit
0x081e5ab3  amJvsCheckInit
0x081e5ae3  amJvsGetNodes
0x081e5b03  amJvsSendRequest
0x081e5c9e  amJvsRecvAcknowledge
0x081e5f06  amJvsGetSense
0x081e5f3c  amJvspClearPacket
0x081e5f7f  amJvspAddCommand
0x081e6161  amJvspCheckCommand
0x081e619d  amJvspCheckAcknowledge
0x081e6218  amJvspMakeReportLen
0x081e62bd  amJvspMakeReportIndex
0x081e6453  amJvspReqReset
0x081e648b  amJvspReqAddressSet
0x081e64c5  amJvspAckAddressSet
0x081e6506  amJvspReqIoId
0x081e6537  amJvspAckIoId
0x081e65bb  amJvspReqCommandRevision
0x081e65ec  amJvspAckCommandRevision
0x081e6658  amJvspReqJvRevision
0x081e6689  amJvspAckJvRevision
0x081e66f0  amJvspReqTransVersion
0x081e6721  amJvspAckTransVersion
0x081e6788  amJvspReqNodeFunctionInfo
0x081e67b9  amJvspAckNodeFunctionInfo
0x081e69d3  amJvspReqMainId
0x081e6a62  amJvspAckMainId
0x081e6aa3  amJvspReqSwInput
0x081e6ae6  amJvspAckSwInput
0x081e6be6  amJvspReqCoinInput
0x081e6c1d  amJvspAckCoinInput
0x081e6cc8  amJvspReqAnalogInput
0x081e6cff  amJvspAckAnalogInput
0x081e6da6  amJvspReqRotaryInput
0x081e6ddd  amJvspAckRotaryInput
0x081e6e95  amJvspReqKeyCodeInput
0x081e6ec6  amJvspAckKeyCodeInput
0x081e6f2d  amJvspReqDispPosInput
0x081e6f64  amJvspAckDispPosInput
0x081e6fe5  amJvspReqGeneralSwInput
0x081e701c  amJvspAckGeneralSwInput
0x081e70e2  amJvspReqPayoutBalance
0x081e7119  amJvspAckPayoutBalance
0x081e71a3  amJvspReqCoinDecrement
0x081e71ef  amJvspAckCoinDecrement
0x081e7230  amJvspReqPayoutIncrement
0x081e727c  amJvspAckPayoutIncrement
0x081e72bd  amJvspReqGeneralOutput1
0x081e7332  amJvspAckGeneralOutput1
0x081e7373  amJvspReqGeneralOutput2
0x081e73b6  amJvspAckGeneralOutput2
0x081e73f7  amJvspReqGeneralOutput3
0x081e743a  amJvspAckGeneralOutput3
0x081e747b  amJvspReqAnalogOutput
0x081e74f3  amJvspAckAnalogOutput
0x081e7534  amJvspReqCharacterOutput
0x081e75ac  amJvspAckCharacterOutput
0x081e75ed  amJvspReqCoinIncrement
0x081e7639  amJvspAckCoinIncrement
0x081e767a  amJvspReqPayoutDecrement
0x081e76c6  amJvspAckPayoutDecrement
0x081e7707  amJvspReqMakerUnique
0x081e7732  amJvspAckSetMakerUnique
0x081e774b  amJvspReqNodeInfo
0x081e779e  amJvspAckNodeInfo
0x081e784d  amJvspReqNodeInput
*/
/*
0x081ebea4  bcLibInit
0x081ebfce  bcLibExit
0x081ed066  bcLibInitOld
*/
/*
0x081e792c  amLibInit
0x081e7ada  amLibExit
0x081e7b2a  amLibIsBasebdAvailable
*/

int main(int argc, char* argv[]);

int __libc_start_main(int (*oldMain) (int, char * *, char * *), int argc, char * * ubp_av, void (*init) (void), void (*fini) (void), void (*rtld_fini) (void), void (* stack_end)) {
  int (*original__libc_start_main)(int (*oldMain) (int, char * *, char * *), int argc, char * * ubp_av, void (*init) (void), void (*fini) (void), void (*rtld_fini) (void), void (* stack_end)) = dlsym(RTLD_NEXT, "__libc_start_main");  \
  return original__libc_start_main((int(*)(int,char**,char**))main,argc, ubp_av,init,fini,rtld_fini,stack_end);
}

/*
void __attribute__ ((constructor)) hookMain(void) {
  mprotect((void*)((uintptr_t)originalMain & (~0xFFF)), 0x2000, PROT_EXEC | PROT_WRITE);

printf("Found main at 0x%08X?\n",(uintptr_t)sym);
/
  printf("Patching main to 0x%08X\n",(uintptr_t)main);
  *&originalMain[0] = 0xE9;
  *(uint32_t*)&originalMain[1] = ((uintptr_t)main-(uintptr_t)originalMain)-5;
  printf("Starting!\n");
/
}
*/

int main(int argc, char* argv[]) {
  printf("amDongleInit: 0x%08X\n",(uintptr_t)amDongleInitx);

  amLibInit();
  bcLibInit();

  amJvsInit();

  amJvspClearPacket(); 
  amJvspReqReset();
  amJvsSendRequest();

  amJvspClearPacket(); 
  amJvspReqReset();
  amJvsSendRequest();
  
  while(amJvsGetSense() != 3) { //FIXME: Might not be 3
    
    amJvspClearPacket(); 
    amJvspReqAddressSet();
    amJvsSendRequest();
  
  }

  return 1;
}
