#ifndef __SYSCALLS_H__
#define __SYSCALLS_H__

#include <stdlib.h>
#include <sys/stat.h>
#include <sys/times.h>

void *_sbrk(ptrdiff_t incr);
void initialise_monitor_handles(void);
int _getpid(void);
int _kill(int pid, int sig);
void _exit (int status);
int _read(int file, char *ptr, int len);
int _write(int file, char *ptr, int len);
int _close(int file);
int _fstat(int file, struct stat *st);
int _isatty(int file);
int _lseek(int file, int ptr, int dir);
int _open(char *path, int flags, ...);
int _wait(int *status);
int _unlink(char *name);
int _times(struct tms *buf);
int _stat(char *file, struct stat *st);
int _link(char *old, char *new);
int _fork(void);
int _execve(char *name, char **argv, char **env);

#endif
