#ifndef _RTC_H_
#define _RTC_H_

/* Initializes timekeeping. Definition in RTC.c.
 * get_fattime() is declared by FatFS (ff.h) and defined in RTC.c. */
void time_init(void);

#endif /* _RTC_H_ */
