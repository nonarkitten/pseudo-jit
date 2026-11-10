/* sys/times.h for the Dhrystone guest: the process-time structure dhry_1.c reads; buffee_rt.c's times() returns zeros. */
struct tms { long tms_utime, tms_stime, tms_cutime, tms_cstime; };
extern int times ();
