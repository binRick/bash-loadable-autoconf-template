#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef __APPLE__
#include <sys/procfs.h>
#endif
#include <time.h>
#include "process_iterator.h"

#ifdef __linux__

#include "process_iterator_linux.c"

#endif
