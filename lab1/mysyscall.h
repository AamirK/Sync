#ifndef __MYSYSCALL_H__
#define __MYSYSCALL_H__
#include <asm/unistd.h>
#include <errno.h>

// Exercise 5: Your code here
// Populate each of these functions with appropriate
// assembly code for each number of system call arguments.
//
// Go ahead and fill in all 7 variants, as you will eventually
// need them.
//
// Friendly advice: as you figure out the signature of a system
// call, you might consider writing a macro for it for future reference, 
// like:
//
// #define MY_GETPID(...) MY_SYSCALL...(...)
#define MY_OPEN(ARG1, ARG2) \
({ \
MY_SYSCALL2(5, ARG1, ARG2); \
})
#define MY_OPEN2(ARG1, ARG2, ARG3) \
({ \
MY_SYSCALL3(5, ARG1, ARG2,ARG3); \
})
#define MY_WRITE(ARG1, ARG2, ARG3)\
 ({\
MY_SYSCALL3(4, ARG1, ARG2, ARG3);\
})

#define MY_READ(ARG1, ARG2, ARG3) \
({ \
MY_SYSCALL3(3,ARG1,ARG2,ARG3); \
})


#define MY_EXIT(ARG1)	\
	({	\
	MY_SYSCALL1(1, ARG1); \
	})

#define MY_SYSCALL0(NUM)                                \
   ({                                                   \
    int rv = -ENOSYS;                                   \
    asm volatile ("int $0x80\n\t"				\
				:"=a"(rv) :"a"(NUM));                    \
    rv;                                                 \
  })


  
#define  MY_SYSCALL1(NUM, ARG1)                          \
  ({                                                    \
    int rv = -ENOSYS;                                   \
	asm volatile ("int $0x80\n\t"				\
				  :"=a"(rv) :"a"(NUM),"b"(ARG1):"memory");                    \
    rv;                                                 \
  })


#define MY_SYSCALL2(NUM, ARG1, ARG2)                    \
   ({                                                   \
     int rv = -ENOSYS;                                  \
     asm volatile ( "int $0x80\n\t"				\
				  :"=a"(rv) :"a"(NUM),"b"(ARG1),"c"(ARG2):"memory"); \
	rv;                                                \
   })

   
#define MY_SYSCALL3(NUM, ARG1, ARG2, ARG3)              \
   ({                                                   \
     int rv = -ENOSYS;                                  \
          asm volatile ( "int $0x80\n\t"				\
				  :"=a"(rv) :"a"(NUM),"b"(ARG1),"c"(ARG2),"d"(ARG3):"memory"); \
     rv;                                                \
   })
   
#define MY_SYSCALL4(NUM, ARG1, ARG2, ARG3, ARG4)        \
   ({                                                   \
     int rv = -ENOSYS;                                  \
               asm volatile ( "movl %5, %%esi\n\t"	\
				"movl %4, %%edx\n\t"			\
				"movl %3, %%ecx\n\t" \
				"movl %2, %%ebx\n\t"					\
				"movl %1, %%eax\n\t"						\
				"int $0x80\n\t"	\
				:"=a"(rv) :"i"(NUM),"g"(ARG1),"g"(ARG2),"g"(ARG3),"g"(ARG4):"memory"); \
     rv;                                                \
   })

#define MY_SYSCALL5(NUM, ARG1, ARG2, ARG3, ARG4, ARG5)  \
   ({                                                   \
     int rv = -ENOSYS;                                  \
	asm volatile("movl %6, %%edi\n\t" \
				"movl %5, %%esi\n\t" \
				"movl %4, %%edx\n\t" \
				"movl %3, %%ecx\n\t" \
				"movl %2, %%ebx\n\t" \
				"movl %1, %%eax\n\t" \
				"int $0x80\n\t"	\
				:"=a"(rv) :"i"(NUM),"g"(ARG1),"g"(ARG2),"g"(ARG3),"g"(ARG4),"g"(ARG5):"memory"); \
    rv;                                                 \
   })
#define exit()
   
   
#endif // __MYSYSCALL_H__
