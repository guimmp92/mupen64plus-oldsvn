#define HOST_REGS 8
#define HOST_CCREG 6
#define EXCLUDE_REG 4

#define IMM_PREFETCH 1
#define IMM_WRITE 1
#define HOST_IMM_ADDR32 1
#define INVERTED_CARRY 1
#define DESTRUCTIVE_WRITEBACK 1
#define DESTRUCTIVE_SHIFT 1

#define USE_MINI_HT 1

#define BASE_ADDR 0x70000000 // Code generator target address

/* x86 calling convention:
   caller-save: %eax %ecx %edx
   callee-save: %ebp %ebx %esi %edi */