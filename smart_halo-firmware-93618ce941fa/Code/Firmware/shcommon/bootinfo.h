#ifndef _BOOTINFO_H
#define _BOOTINFO_H

typedef void (*binfo_cb_t)( void );

uint32_t binfo_getWord(void);

void binfo_setWord(uint32_t w, binfo_cb_t cb);

//void binfo_test(void);
//void binfo_dump(void);

void binfo_init(void);

#define BINFO_IS_ALARM_ON(x) (((x) & 2) == 0)
#define BINFO_IS_STAY_BL(x) (((x) & 1) == 0)

#define BINFO_ALARM(x,v) ((v) ? ((x) & 0x7ffffffd) : ((x) | 0x00000002))
#define BINFO_STAY_BL(x,v) ((v) ? ((x) & 0x7ffffffe) : ((x) | 0x00000001))

#endif