#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>

/* Some constants, should check dynamically */
#define BLOCK_SIZE 512
#define DEBUG 0

#if DEBUG > 0
#  define DBG_LIMIT i<1000
#else
#  define DBG_LIMIT
#endif

void usage(char argv0[]){
  printf("Usage: %s <device>", argv0);
}

int main(int argc, char **argv){
  int fd;
  
  if (argc != 2) {
    usage(argv[0]);
    exit(0);
  }
      
  fd = open(argv[1], O_RDWR);
  if (fd < 0){
    perror("open");
    return 1;
  }

  /* Zero all bytes on the device */
  /* Implement later; for now, I do this manually */

  fprintf(stderr, "Write zeroes\n");
  for (uint64_t i=0; DBG_LIMIT; i++){
    if (lseek(fd, i*BLOCK_SIZE, SEEK_SET) < 0){
      if (errno == EINVAL) {
	break;
      } else {
	perror("lseek");
	fprintf(stderr, "Nulling: Sector index: 0x%016llx, errno=%d\n",
		i, errno);
	exit(1);
      }
    }

    if (write(fd, "\x0\x0\x0\x0\x0\x0\x0\x0", 8) < 0){
      perror("write");
      fprintf(stderr, "Nulling: Sector index 0x%16llx\n", i);
      exit(1);
    }
  }

  /* Check 1:
     Check if first byte in sector is zero before writing sector number. 
     If not zero, give warning on stderr and sector number and value read 
     to stdout. */

  fprintf(stderr, "Start check 1\n");
  for (uint64_t i=0; DBG_LIMIT; i++){
    uint64_t buf;

    if (lseek(fd, i*BLOCK_SIZE, SEEK_SET) < 0){
      if (errno == EINVAL) {
	break;
      } else {
	perror("lseek");
	fprintf(stderr, "Check 1: Sector index: 0x%016llx, errno=%d\n",
		i, errno);
	exit(1);
      }
    }
    
    if (read(fd, &buf, 8) < 0){
      perror("read");
      exit(1);
    }

    if (memcmp((void *) &buf, "\x0\x0\x0\x0\x0\x0\x0\x0", 8) != 0)
      printf("1 %016llx %016llx\n", i, (uint64_t) buf);

    /* Write index to file */
    if (lseek(fd, i*BLOCK_SIZE, SEEK_SET) < 0){
      perror("lseek 2");
      exit(1);
    }
    
    if (write(fd, &i, 8) < 0){
      perror("write");
      exit(1);
    }
  }

  /* Check 2:
     Check integrity of sector numbers. If not intact, write 
     sector number and value to stdout. */

  fprintf(stderr, "Start check 2\n");
  for (uint64_t i; DBG_LIMIT; i++){
    char buf[8];

    if (lseek(fd, i*BLOCK_SIZE, SEEK_SET) < 0){
      if (errno == EINVAL) {
	break;
      } else {
	perror("lseek");
	fprintf(stderr, "Check 2: Sector index: 0x%016llx, errno=%d\n",
		i, errno);
	exit(1);
      }
    }
    
    if (read(fd, &buf, 8) < 0){
      perror("read");
      exit(1);
    }

    if (memcmp(buf, &i, 8) != 0)
      printf("2 %016llx %016llx\n", i, (uint64_t) *buf);
  }
  
  close(fd);
  
  return 0;
}
