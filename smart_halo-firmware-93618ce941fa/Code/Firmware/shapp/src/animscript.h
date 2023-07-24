#ifndef _ANIMSCRIPT_H
#define _ANIMSCRIPT_H

void ascr_anim_step_render(bool reset);

void ascr_cmd_load(uint8_t *buf, uint32_t len);
void ascr_cmd_run(uint8_t *buf, uint32_t len);
void ascr_cmd_stop(uint8_t *buf, uint32_t len);

void ascr_init();

#endif