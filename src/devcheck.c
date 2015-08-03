#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <linux/fs.h>
#include <strings.h>

#define DEBUG 0

#if DEBUG > 0
/* For debugging, only a few sectors are needed.  */
#  define DBG_LIMIT i<1000
#else
#  define DBG_LIMIT
#endif

/* Only enable this if you are sure you will overwrite the device... */
#define ENABLE_WRITING 0

typedef struct _info_t {
  int ssize;
  uint64_t size;
  int32_t nsect;
} info_t;

void usage(char argv0[]){
  printf("Usage: %s <device>", argv0);
}

info_t *get_info(int dev_fd){
  info_t *info; 
  int ret;
  
  info = malloc(sizeof(info_t));
  bzero(info, sizeof(info_t));
  
  if (!info){
    perror("malloc");
    return NULL;
  }

  ret = ioctl(dev_fd, BLKSSZGET, &(info->ssize));
  if (ret == -1){
    perror("ioctl");
    exit(1);
  }
  ret = ioctl(dev_fd, BLKGETSIZE64, &(info->size));
    if (ret == -1){
    perror("ioctl");
    exit(1);
  }
  ret = ioctl(dev_fd, BLKGETSIZE, &(info->nsect));
    if (ret == -1){
    perror("ioctl");
    exit(1);
  }
  
  return info;
}

void print_info(info_t *info){
  if (!info) return;

  fprintf(stderr, "Sector size: %d\nNumber of sectors: %d\nNumber of bytes: %lld\n",
	  info->ssize, info->nsect, info->size);
  return;
}

int main(int argc, char **argv){
  int fd;
  info_t *dev_info;
  
  if (argc != 2) {
    usage(argv[0]);
    exit(0);
  }

  fprintf(stderr, "Using device: %s\n", argv[1]);
  fd = open(argv[1], O_RDWR);
  if (fd < 0){
    perror("open");
    return 1;
  }

  dev_info = get_info(fd);
  print_info(dev_info);
  
#if ENABLE_WRITING

  fprintf(stderr, "Write zeroes\n");
  for (uint64_t i=0; DBG_LIMIT; i++){
    if (lseek(fd, i*dev_info->ssize, SEEK_SET) < 0){
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

    if (lseek(fd, i*dev_info->ssize, SEEK_SET) < 0){
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
    if (lseek(fd, i*dev_info->ssize, SEEK_SET) < 0){
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

    if (lseek(fd, i*dev_info->ssize, SEEK_SET) < 0){
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
#endif // 0
  
  free(dev_info);
  close(fd);
  
  return 0;
}
