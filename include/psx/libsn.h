#pragma once

/* PC host file services; signatures from Psy-Q 3.6 LIBSN.H. */
int PCopen(char *name, int flags, int perms);
int PClseek(int fd, int offset, int mode);
int PCread(int fd, char *buffer, int length);
int PCclose(int fd);
