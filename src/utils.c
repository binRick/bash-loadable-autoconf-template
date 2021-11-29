


const char *
container_type (void)
{
	struct stat  statbuf;
	char         buffer[1024];
	FILE        *f;
#if defined (PROCENV_PLATFORM_LINUX)
	dev_t        expected;

	expected = makedev (5, 1);
#endif

	if (stat ("/dev/console", &statbuf) < 0)
		goto out;

#if defined (PROCENV_PLATFORM_FREEBSD)
	if (misc.in_jail)
		return "jail";
#endif

	/* LXC's /dev/console is actually a pty */
#if defined (PROCENV_PLATFORM_LINUX)
	if (major (statbuf.st_rdev) != major (expected)
			|| (minor (statbuf.st_rdev)) != minor (expected))
		return "lxc";
#endif

	if (! stat ("/proc/vz", &statbuf) && stat ("/proc/bc", &statbuf) < 0)
		return "openvz";

	f = fopen ("/proc/self/status", "r");
	if (! f)
		goto out;

	while (fgets (buffer, sizeof (buffer), f)) {
		size_t len = strlen (buffer);
		buffer[len-1] = '\0';

		if (strstr (buffer, "VxID") == buffer) {
			fclose (f);
			return "vserver";
		}
	}

	fclose (f);

out:
	return "";
}



char *username, *osname, *shellname, *pkgCount;
char *krnlver;
long uptimeH, uptimeM;


static int get_ncpu() {
	int ncpu;
#ifdef _SC_NPROCESSORS_ONLN
	ncpu = sysconf(_SC_NPROCESSORS_ONLN);
#elif defined __APPLE__
	int mib[2] = {CTL_HW, HW_NCPU};
	size_t len = sizeof(ncpu);
	sysctl(mib, 2, &ncpu, &len, NULL, 0);
#elif defined _GNU_SOURCE
	ncpu = get_nprocs();
#else
	ncpu = -1;
#endif
	return ncpu;
}

int get_pid_max()
{
#ifdef __linux__
	//read /proc/sys/kernel/pid_max
	static char buffer[1024];
	FILE *fd = fopen("/proc/sys/kernel/pid_max", "r");
	if (fd==NULL) return -1;
	if (fgets(buffer, sizeof(buffer), fd)==NULL) {
		fclose(fd);
		return -1;
	}
	fclose(fd);
	return atoi(buffer);
#elif defined __FreeBSD__
	return 99998;
#elif defined __APPLE__
	return 99998;
#endif
}


int
fd_valid (int fd)
{
	int flags = 0;

	if (fd < 0)
		return 0;

	errno = 0;
	flags = fcntl (fd, F_GETFL);

	if (flags < 0)
		return 0;

	/* redundant really */
	if (errno == EBADF)
		return 0;

	return 1;
}



static int tty_height(void)
{
#ifdef TIOCGWINSZ
	const int fd = 0;
	struct winsize ws;

	(void)memset(&ws, 0, sizeof(ws));
	/* if tty and we can get a sane width, return it */
	if (isatty(fd) &&
	    (ioctl(fd, TIOCGWINSZ, &ws) != -1) &&
	    (0 < ws.ws_row) &&
	    (ws.ws_row == (size_t)ws.ws_row))
		return ws.ws_row;
#endif
	return 25;	/* else standard tty 80x25 */
}






ssize_t getcwd_by_pid(pid_t pid, char *buf, size_t bufsiz)
{
    char link[128];

    sprintf(link, "/proc/%d/cwd", pid);

    return readlink(link, buf, bufsiz);
}








static const char *cmd_lookup(const char *cmd)
{
    struct stat s;
    int plen = 0, clen = strlen(cmd) + 1;
    char *search, *p;
    static char path[PATH_MAX];

    if (!stat(cmd, &s) && S_ISREG(s.st_mode))
        return cmd;

    search = getenv("PATH");

    if (!search)
        search = "/bin:/usr/bin:/sbin:/usr/sbin";

    p = search;

    do {
        if (*p != ':' && *p != '\0')
            continue;

        plen = p - search;

        if ((plen + clen) >= sizeof(path))
            continue;

        strncpy(path, search, plen);
        sprintf(path + plen, "/%s", cmd);

        if (!stat(path, &s) && S_ISREG(s.st_mode))
            return path;

        search = p + 1;
    } while (*p++);

    return NULL;
}








char *os()
{
	static struct utsname sysInfo;
	uname(&sysInfo);
	/* start */
	/* This whole section could probably be rewritten - it seems
	   like a bit of a mess right now */
	if (strncmp(sysInfo.sysname, "Linux", 5) == 0) {
		char *osContents = malloc(512);
		char *newContents = malloc(512);
		int line = 0;
		FILE *f = fopen("/etc/os-release", "rt");
		if (f == NULL || osContents == NULL)
			return "Linux";
		/* look through each line of /etc/os-release until we're on the
		 * NAME= line */
		while (fgets(osContents, 512, f)) {
			snprintf(newContents, 512, "%.*s", 511, osContents + 4);
			if (strncmp(newContents, "=", 1) == 0)
				break;
			line++;
		}
		fclose(f);
		free(osContents);
		if (strncmp(newContents, "=", 1) == 0) {
			int len = strlen(newContents);
			for (int i = 0; i < len; i++) {
				if (newContents[i] == '\"' ||
				    newContents[i] == '=') {
					for (int ii = 0; ii < len; ii++)
						newContents[ii] =
						    newContents[ii + 1];
					newContents[strlen(newContents) - 1] =
					    '\0';
				}
			}
		}
		if (osname == NULL)
			osname = malloc(512);
		strcpy(osname, newContents);
		free(newContents);
    return osname;
  }
}

char *kernel()
{
	static struct utsname kernelData;
	uname(&kernelData);
  return kernelData.release;
//	return NULL;
}




char *shell()
{
	char *shell = getenv("SHELL");
	char *slash = strrchr(shell, '/');
	if (slash) {
		shell = slash + 1;
	}
  return shell;
}









void lowerCase(char *str)
{
	const int len = strlen(str);
	int i;
	for (i = 0; i < len; i += 1)
		if (str[i] >= 'A' && str[i] <= 'Z')
			str[i] += 'a' - 'A';
}

void upperCase(char *str)
{
	const int len = strlen(str);
	int i;
	for (i = 0; i < len; i += 1)
		if (str[i] >= 'a' && str[i] <= 'z')
			str[i] += 'A' - 'a';
}
