#ifndef PD_EXIT_H
#define PD_EXIT_H

#ifdef TARGET_PLAYDATE
// Implemented in game/main.c — logs before terminating
extern void scalanative_pd_exit(int status, const char *file, int line);
extern void scalanative_pd_abort(const char *file, int line);
#define exit(status) scalanative_pd_exit(status, __FILE__, __LINE__)
#define abort() scalanative_pd_abort(__FILE__, __LINE__)
#endif

#endif // PD_EXIT_H
