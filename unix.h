#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>
#include <stdint.h>
#include <semaphore.h>
#include <math.h>
#include <stdbool.h>
#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

#define ADDRESS      "SO2079620"

#define UNIXSTR_PATH "/tmp/socket.unix.stream2080320"
#define UNIXDG_PATH  "/tmp/socket.unix.datagram"
#define UNIXDG_TMP   "/tmp/socket.unix.dgXXXXXXX"

#define MAXLINE 512
