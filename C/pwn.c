#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

#define SUID(u) setresuid(u, u, u)
#define SGID(g) setresgid(g, g, g)

#define FAIL(fmt, ...)                                                         \
	do {                                                                   \
		int _erno = errno;                                             \
		fprintf(stderr, fmt ": %s\n", ##__VA_ARGS__, strerror(_erno)); \
		return 1;                                                      \
	} while (0)

int
main(int argc, char *argv[])
{
	struct stat st;
	if (argc == 1 || (argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0))) {
		printf("pwn - lightweight and simple sudo-like program launcher\n");
		return argc == 2;
	}

	switch (getuid()) {
	case 0:
	case 1000:
		break;
	default:
		fprintf(stderr, "Not allowed!\n");
		return 1;
	}

	if (stat("/proc/self/exe", &st) != 0)
		FAIL("stat(\"%s\")", argv[0]);

	if (SUID(st.st_uid) != 0)
		FAIL("seteuid(%d)", (int)st.st_uid);
	if (SGID(st.st_gid) != 0)
		FAIL("setegid(%d)", (int)st.st_gid);

	execvp(argv[1], argv + 1);
	perror(argv[1]);
	return 1;
}
