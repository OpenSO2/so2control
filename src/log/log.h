#ifndef __LOG__
#define __LOG__
int log_init(void);
int log_message(char *message);
int log_debug(char *message, ...);
int log_error(char *message);
int log_uninit(void);
#endif
