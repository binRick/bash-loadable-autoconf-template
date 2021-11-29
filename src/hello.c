#include <config.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <sys/utsname.h>

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#include "builtins.h"
#include "shell.h"

#include "utils.h"

#include "bashgetopt.h"

#include "common.h"

#include "log.h"
#include "log.c"

#include "ini.h"
#include "ini.c"

#include "process_group.h"
#include "process_group.c"

#include "uuid4.h"
#include "uuid4.c"

#include "process_iterator.h"
#include "process_iterator.c"

int hello_builtin (list) WORD_LIST *list; {
  printf("hello world\n");
  fflush (stdout);
  return (EXECUTION_SUCCESS);
}

int hello_builtin_load (s) char *s; {
  printf ("hello builtin loaded\n");
  fflush (stdout);
  return (1);
}

void hello_builtin_unload (s) char *s; {
  printf ("hello builtin unloaded\n");
  fflush (stdout);
}

char *hello_doc[] = {
	"Sample builtin.",
	"",
	"this is the long doc for the sample hello builtin",
	(char *)NULL
};

struct builtin hello_struct = {
	"hello",
	hello_builtin,
	BUILTIN_ENABLED,
	hello_doc,
	"hello",
	0
};
