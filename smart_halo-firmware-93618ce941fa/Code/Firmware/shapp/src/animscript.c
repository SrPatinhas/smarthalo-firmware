
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "platform.h"

#include "scheduler.h"
#include "dispatch.h"
#include "leds.h"
#include "bslink.h"

#include "animscript.h"

#define MOD(a,b) ((((a)%(b))+(b))%(b))

#define ASCR_ACCMAX 16

#define ASCR_DATAMAX 1024
#define ASCR_DATA32MAX 32
#define ASCR_DATA8MAX (ASCR_DATAMAX - ASCR_DATA32MAX*4)

#define ASCR_CODEMAX 256
#define ASCR_RUNMAX ASCR_CODEMAX*2

#if ASCR_DATA32MAX*4 > ASCR_DATAMAX
#error ASCR_DATA32MAX is too big
#endif

typedef enum {
    ASCR_FAULT_OK = 0,
    ASCR_FAULT_PC_INVALID,
    ASCR_FAULT_OP_INVALID,
    ASCR_FAULT_MEM_INVALID,
} ascr_vm_fault_t;

typedef enum {
    ASCR_MEM_ZERO = 0,
	ASCR_MEM_RESET,
	ASCR_MEM_STEP,
	ASCR_MEM_STOP,
	ASCR_MEM_MOVEMENT,
	ASCR_MEM_TOUCH,
	ASCR_MEM_ANGLE,
} ascr_vm_addrdef_t;

typedef struct {
	uint16_t addr;
	uint8_t win;
	uint8_t dummy;
} ascr_vm_acc_t;
ascr_vm_acc_t ascr_vm_acc[ASCR_ACCMAX];

uint8_t ascr_vm_data[ASCR_DATAMAX] __attribute__ ((aligned (32)));

uint32_t *ascr_vm_32_data = (uint32_t*)ascr_vm_data;
uint8_t *ascr_vm_8_data = ascr_vm_data + ASCR_DATA32MAX*4;

/*

ACC  20 cw wa aa
ACCZ 21 cw wa aa
ADDI 00 cd vv vv
SUBI 02 cd vv vv
MULI 04 cd vv vv
DIVI 06 cd vv vv
MODI 08 cd vv vv
SHLI 0a cd vv vv
SHRI 0c cd vv vv
ANDI 0e cd vv vv
ORI  10 cd vv vv
NOT	 12 cd vv vv
ADDM 01 cd xa aa
SUBM 03 cd xa aa
MULM 05 cd xa aa
DIVM 07 cd xa aa
MODM 09 cd xa aa
SHLM 0b cd xa aa
SHRM 0d cd xa aa
ANDM 0f cd xa aa
ORM  11 cd xa aa
ROLI 40 cd vv vv
RORI 42 cd vv vv
ROLM 41 cd xa aa
RORM 43 cd xa aa
B    60 aa aa aa
BEQ  61 aa aa aa
BNE  62 aa aa aa
BGT  63 aa aa aa
BLT  64 aa aa aa
BGE  65 aa aa aa
BLE  66 aa aa aa
DONE 80 xx xx xx
RDRH 81 xx xa aa
RDRC 82 xx xa aa
DBGD 87 xw wa aa

*/

uint32_t ascr_vm_code[ASCR_CODEMAX]
= {
//	0x87001001, // DBGD 1, 1
	0x20001000, // ACC 0, 1, 0
	0x20118020, // ACC 1, 24, 32
	0x20218038, // ACC 2, 24, 56
	0x20318050, // ACC 3, 24, 80
	0x00010060, // ADD 0, 1, 80
	0x000200FF, // ADD 0, 2, 255
	0x000300FF, // ADD 0, 3, 128
	0x81000020, // RDRH 32
//	0x87018020, // DBGD 24, 1
//	0x87018038, // DBGD 24, 1
//	0x87018050, // DBGD 24, 1
	0x80000000, // DONE
}
;

bool ascr_vm_status_zero;
bool ascr_vm_status_pos;
bool ascr_vm_status_neg;

ascr_vm_fault_t ascr_vm_fault;

uint32_t ascr_vm_pc;
uint32_t ascr_vm_runcnt;

uint32_t ascr_param_entrypoint;

//============================================================================
// Helpers

void ascr_fault(ascr_vm_fault_t fault) {
	printf("ASCR FAULT : %d, %d\r\n", fault, ascr_vm_pc);
	ascr_vm_fault = fault;
	ascr_vm_runcnt = 0;
}

inline bool ascr_help_guard_addr(uint32_t addr) {
	if(addr >= (ASCR_DATA32MAX + (ASCR_DATAMAX - ASCR_DATA32MAX*4)) ) {
		ascr_fault(ASCR_FAULT_MEM_INVALID);
		return true;
	}
	return false;
}

inline uint32_t ascr_help_mem_get(uint32_t addr) {
	if(ascr_help_guard_addr(addr)) {
		return 0;
	}
	return (addr < ASCR_DATA32MAX) ? ascr_vm_32_data[addr] : (uint32_t)ascr_vm_8_data[addr-ASCR_DATA32MAX];
}

inline void ascr_help_mem_set(uint32_t addr, uint32_t data) {
	if(ascr_help_guard_addr(addr)) {
		return;
	}
	if(addr == 0) {
		return;
	}
	if (addr < ASCR_DATA32MAX) {
		ascr_vm_32_data[addr] = data;	
	} else {
		ascr_vm_8_data[addr-ASCR_DATA32MAX] = (uint8_t) (data & 0xFF);
	}
}

//============================================================================
// Opcodes

/***
	ACC cw wa aa
	Set Accumulator pointer to memory address

	ACCZ cw wa aa
	Set Accumulator pointer to memory address and initialize to zero

	c Accumulator Id 4 bit
	w Memory window 8 bit
	a Memory address 12 bit
*/
void ascr_op_acc(uint32_t fetch) {
	uint32_t z,c,a,w;
	z = (fetch >> 24) & 0x1;
	c = (fetch >> 20) & 0xf;
	w = (fetch >> 12) & 0xff;
	a = (fetch) & 0xfff;
	if(ascr_help_guard_addr(a) || ascr_help_guard_addr(a+w-1)) {
		return;
	}
	if(w==0) {
		ascr_fault(ASCR_FAULT_OP_INVALID);
		return;
	}
	if(c==0) {
		//Readonly
		return;
	}
	ascr_vm_acc[c].addr = a;
	ascr_vm_acc[c].win = w;
	if(z) {
		for(int i = a; i < (a+w); i++) {
			ascr_help_mem_set(i, 0);
		}
	}
	//printf("ASCR ACC %d, %d, %d\r\n", c, a, w);
}

/***
	ADDI cd vv vv
	ADDM cd 0a aa
	ADDx,SUBx,MULx,DIVx,MODx,SHLx,SHRx,ANDx,ORx,NOTx
*/

typedef int32_t (*ascr_op_alu_fn_t)(int32_t,int32_t);

int32_t ascr_op_alu_add(int32_t a, int32_t b) { return a+b; }
int32_t ascr_op_alu_sub(int32_t a, int32_t b) { return a-b; }
int32_t ascr_op_alu_mul(int32_t a, int32_t b) { return a*b; }
int32_t ascr_op_alu_div(int32_t a, int32_t b) { return a/b; }
int32_t ascr_op_alu_mod(int32_t a, int32_t b) { return a%b; }
int32_t ascr_op_alu_shl(int32_t a, int32_t b) { return a<<b; }
int32_t ascr_op_alu_shr(int32_t a, int32_t b) { return a>>b; }
int32_t ascr_op_alu_and(int32_t a, int32_t b) { return a&b; }
int32_t ascr_op_alu_or(int32_t a, int32_t b) { return a|b; }
int32_t ascr_op_alu_not(int32_t a, int32_t b) { return ~a; }

ascr_op_alu_fn_t ascr_op_alu_fns[] = {
	ascr_op_alu_add,
	ascr_op_alu_sub,
	ascr_op_alu_mul,
	ascr_op_alu_div,
	ascr_op_alu_mod,
	ascr_op_alu_shl,
	ascr_op_alu_shr,
	ascr_op_alu_and,
	ascr_op_alu_or,
	ascr_op_alu_not,
};
#define ASCR_ALU_MAX (sizeof(ascr_op_alu_fns)/sizeof(ascr_op_alu_fn_t))

void ascr_op_alu(uint32_t fetch) {
	uint32_t aluop,m,c,d,av,ca,cw,da,dw;
	
	aluop = (fetch >> 25) & 0xf;
	m = 	(fetch >> 24) & 0x1;
	c = 	(fetch >> 20) & 0xf;
	d = 	(fetch >> 16) & 0xf;
	av = 	(fetch) & 0xffff;
	ca = 	ascr_vm_acc[c].addr;
	cw = 	ascr_vm_acc[c].win;
	da = 	ascr_vm_acc[d].addr;
	dw = 	ascr_vm_acc[d].win;

	//printf("ASCR ALU %d, %d, %d, %d, %d, %d, %d, %d, %d\r\n", aluop,m,c,d,av,ca,cw,da,dw);
	if((m) && ascr_help_guard_addr(av)) {
		return;
	}
	if(aluop >= ASCR_ALU_MAX) {
		ascr_fault(ASCR_FAULT_OP_INVALID);
		return;
	}
	ascr_op_alu_fn_t alufn = ascr_op_alu_fns[aluop];

	for(int dst_i = 0; dst_i < dw; dst_i++) {
		int src_i = dst_i % cw;
		int32_t arg_a = ascr_help_mem_get(ca + src_i);
		int32_t arg_b = (m) ? ascr_help_mem_get(av + src_i) : av;
		int32_t result = alufn(arg_a,arg_b);
		ascr_vm_status_zero = (result == 0);
		ascr_vm_status_pos = (result > 0);
		ascr_vm_status_neg = (result < 0);
		/*printf("ASCR ALU status: %d,%d,%d\r\n",
			(ascr_vm_status_zero) ? 1 : 0,
			(ascr_vm_status_pos) ? 1 : 0,
			(ascr_vm_status_neg) ? 1 : 0);*/
		//printf("ASCR SET %d, %d\r\n", da + dst_i, result);
		ascr_help_mem_set(da + dst_i, result);
	}
}



/***
	xxxI cd vv vv
	xxxM cd 0a aa
	ROLI, ROLM: Rotate Left values, constant or value in memory
	RORI, RORM: Rotate Right values, constant or value in memory

*/

void ascr_op_array(uint32_t fetch) {
	uint32_t op,m,c,d,av,ca,cw,da,dw;
	
	op =    (fetch >> 25) & 0x1;
	m = 	(fetch >> 24) & 0x1;
	c = 	(fetch >> 20) & 0xf;
	d = 	(fetch >> 16) & 0xf;
	av = 	(fetch) & 0xffff;
	ca = 	ascr_vm_acc[c].addr;
	cw = 	ascr_vm_acc[c].win;
	da = 	ascr_vm_acc[d].addr;
	dw = 	ascr_vm_acc[d].win;

	if((m) && ascr_help_guard_addr(av)) {
		return;
	}
	if((ca >= da && ca < (da + dw)) || (da >= ca && da < (ca + cw))) {
		ascr_fault(ASCR_FAULT_MEM_INVALID);
		return;
	}
	if(cw != dw) {
		ascr_fault(ASCR_FAULT_MEM_INVALID);
		return;
	}

	int32_t rot = (m) ? ascr_help_mem_get(av) : av;
	rot = (op) ? -rot: rot; 

	for(int dst_i = 0; dst_i < (int32_t)dw; dst_i++) {
		int src_i = MOD((dst_i + rot), (int32_t)cw);
		int32_t val = ascr_help_mem_get(ca + src_i);
		ascr_help_mem_set(da + dst_i, val);
		//printf("%d, %d, %d, %d: %d\r\n",ca,src_i,da,dst_i,val);
	}
}

/***
	B   aa aa aa, Unconditional branch to address
	BEQ aa aa aa, (BZ) Branch if equal (zero)
	BNE aa aa aa, (BNZ) Branch if not equal (not zero)
	BGT aa aa aa, Branch if greater than
	BLT aa aa aa, Branch if less than
	BGE aa aa aa, Branch if greater or equal
	BLE aa aa aa, Branch if less or equal

*/
void ascr_op_branch(uint32_t fetch) {
	uint32_t c,a;
	c = (fetch >> 24) & 0x7;
	a = (fetch) & 0xFFFFFF;
	if(a >= ASCR_CODEMAX) {
		ascr_fault(ASCR_FAULT_PC_INVALID);
		return;
	}
	bool branch = false;
	switch(c) {
		case 0: // B
			branch = true;
			break;
		case 1: // BEQ
			branch = ascr_vm_status_zero;
			break;
		case 2: // BNE
			branch = !ascr_vm_status_zero;
			break;
		case 3: // BGT
			branch = ascr_vm_status_pos;
			break;
		case 4: // BLT
			branch = ascr_vm_status_neg;
			break;
		case 5: // BGE
			branch = ascr_vm_status_pos || ascr_vm_status_zero;
			break;
		case 6: // BLE
			branch = ascr_vm_status_neg || ascr_vm_status_zero;
			break;
		default:
			ascr_fault(ASCR_FAULT_OP_INVALID);
			return;
	}
	//printf("branch %d, %d\r\n", (branch) ? 1 : 0, a);
	if(branch) {
		ascr_vm_pc = a;
	}
}


/***
	DONE xx xx xx, Frame done
	STOP xx xx xx, Request to stop animation
	RDRH 00 0a aa, Render Halo HSV at memory address
	RDRC 00 0a aa, Render Center HSV at memory address
	DBGD 0w wa aa, Dump memory
*/
void ascr_op_special(uint32_t fetch) {
	uint32_t op,w,a;
	op = (fetch >> 24) & 0x7;
	w  = (fetch >> 12) & 0xFF;
	a  = (fetch) & 0xFFF;
	if(a >= ASCR_CODEMAX) {
		ascr_fault(ASCR_FAULT_PC_INVALID);
		return;
	}
	if(op == 0) { // DONE
		//printf("ASCR DONE %d\r\n", ASCR_RUNMAX - ascr_vm_runcnt);
		ascr_vm_runcnt = 0;
	}
	if(op == 1) { // STOP
		//printf("ASCR STOP %d\r\n", ASCR_RUNMAX - ascr_vm_runcnt);
		ascr_vm_runcnt = 0;
		leds_anim_off(ANIM_ANIMSCRIPT);
	}
	if(op == 2) { // RDRH
		if(a < ASCR_DATA32MAX) {
			ascr_fault(ASCR_FAULT_MEM_INVALID);
			return;
		}
		a -= ASCR_DATA32MAX;
		leds_util_copyHSVtoRGB(ascr_vm_8_data + a, ascr_vm_8_data + a + 24, ascr_vm_8_data + a + 48);
	}
	if(op == 3) { // RDRC
		if(a < ASCR_DATA32MAX) {
			ascr_fault(ASCR_FAULT_MEM_INVALID);
			return;
		}
		a -= ASCR_DATA32MAX;
		leds_util_central_copyHSVtoRGB(ascr_vm_8_data[a], ascr_vm_8_data[a+1], ascr_vm_8_data[a+2]);
	}
	if(op == 7) { // DBGD
		for(int i = 0; i < w; i++) {
			int val = ascr_help_mem_get(a + i);
			if(i%16 == 0) {
				printf("\r\n");
			}
			printf((a + i < ASCR_DATA32MAX) ? "%08X " : "%02X ", val);
		}
		printf("\r\n");
	}
}


//============================================================================
// Dispatcher

typedef void (*ascr_opfn_t)(uint32_t);

typedef struct {
	uint8_t op;
	uint8_t mask;
	ascr_opfn_t fn;
} ascr_op_entry_t;

ascr_op_entry_t ascr_opfn[] = {
    {0x00, 0xe0, ascr_op_alu}, // ADDI,SUBI,MULI,DIVI,MODI,SHLI,SHRI,ANDI,ORI,NOT,ADDM,SUBM,MULM,DIVM,MODM,SHLM,SHRM,ANDM,ORM,
    {0x20, 0xfe, ascr_op_acc}, // ACC, ACCZ
    {0x40, 0xfc, ascr_op_array}, // ROLI, RORI, ROLM, RORM
    {0x60, 0xf8, ascr_op_branch}, // B,BEQ,BNE,BGT,BLT,BGE,BLE
    {0x80, 0xf8, ascr_op_special}, // RDRH, RDRC, DBGD
};
#define ASCR_OPFN_CNT (sizeof(ascr_opfn)/sizeof(ascr_op_entry_t))

void ascr_execute() {
	//Always zero
	ascr_vm_data[ASCR_MEM_ZERO] = 0;
	//pc
	if(ascr_vm_pc >= ASCR_CODEMAX) {
		ascr_fault(ASCR_FAULT_PC_INVALID);
		return;
	}
	//fetch
	uint32_t fetch = ascr_vm_code[ascr_vm_pc];
	ascr_vm_pc++;
	uint32_t op = fetch >> 24;
	//LUT
	for(int i = 0; i < ASCR_OPFN_CNT; i++) {
		if((op & ascr_opfn[i].mask) == ascr_opfn[i].op) {
			ascr_opfn[i].fn(fetch);
			return;
		}
	}
	//Unknown OP
	ascr_fault(ASCR_FAULT_OP_INVALID);
	return;
}

//============================================================================
// Render Entry point

void ascr_anim_step_render(bool reset) {
	if(ascr_vm_fault) {
		return;
	}
	//Init accumulators
	for(int i = 0; i < ASCR_ACCMAX; i++) {
		ascr_vm_acc[i].addr = 0;
		ascr_vm_acc[i].win = 0;
	}
	ascr_vm_acc[0].win = 1;
	//
	ascr_vm_pc = ascr_param_entrypoint;
	ascr_vm_runcnt = ASCR_RUNMAX;
	ascr_vm_32_data[ASCR_MEM_RESET] = (reset) ? 1 : 0;
	ascr_vm_32_data[ASCR_MEM_STEP] = ascr_vm_32_data[ASCR_MEM_STEP]+1;
	while(ascr_vm_runcnt--) {
		ascr_execute();
	}
}


//============================================================================
// Cmds

void ascr_cmd_load(uint8_t *buf, uint32_t len) {
    uint8_t reply[1];
    uint32_t ptr = 0;

    printf("ascr_cmd_load\r\n");

    if(len < 2) {
	    reply[ptr++] = RET_FAIL;
	    bslink_write(reply, ptr);
	    return;
    }

    uint32_t mem = buf[0] >> 7;
    uint32_t addr = (((uint32_t)buf[0] << 8) & 0x7f) + (uint32_t)buf[1];
    buf += 2;
    len -= 2;

    if(!mem) {

	    if((len%4) || 
	    	(addr+(len>>2) > ASCR_CODEMAX)
	    	) {
		    reply[ptr++] = RET_FAIL;
		    bslink_write(reply, ptr);
		    return;
	    }

		for(int i = 0; i < len>>2; i++) {
			ascr_vm_code[addr+i] = 	((uint32_t)buf[(i*4)+0]<<24) + 
									((uint32_t)buf[(i*4)+1]<<16) + 
									((uint32_t)buf[(i*4)+2]<<8) + 
									(uint32_t)buf[(i*4)+3];
		}    

    } else {

	    if(addr+len > ASCR_DATAMAX) {
		    reply[ptr++] = RET_FAIL;
		    bslink_write(reply, ptr);
		    return;
	    }

		for(int i = 0; i < len; i++) {
			ascr_help_mem_set(addr+ASCR_DATA32MAX+i, buf[i]);
		}    

    }


	/*for(int i = 0; i < len>>2; i++) {
		printf("%08X\r\n", ascr_vm_code[addr+i]);
	}*/

    reply[ptr++] = RET_OK;
    bslink_write(reply, ptr);
}

void ascr_cmd_run(uint8_t *buf, uint32_t len) {

    uint8_t reply[1];
    uint32_t ptr = 0;

    printf("ascr_cmd_run\r\n");

    if((len < 4) || (len-4 > ASCR_DATA8MAX)) {
	    reply[ptr++] = RET_FAIL;
	    bslink_write(reply, ptr);
	    return;
    }

    int timeout = ((uint32_t)buf[0] << 8) + (uint32_t)buf[1];
    ascr_param_entrypoint = ((uint32_t)buf[2] << 8) + (uint32_t)buf[3];
    buf += 4;
    len -= 4;
    printf("len %d \r\n", len);
    memcpy(ascr_vm_8_data, buf, len);

	ascr_vm_32_data[ASCR_MEM_STOP] = 0;

	ascr_vm_fault = 0;
	leds_anim_on(ANIM_ANIMSCRIPT, timeout);

    reply[ptr++] = RET_OK;
    bslink_write(reply, ptr);
}


void ascr_cmd_stop(uint8_t *buf, uint32_t len) {

    uint8_t reply[1];
    uint32_t ptr = 0;

    printf("ascr_cmd_stop\r\n");

    bool force = (len) ? (buf[0]) ? 1 : 0 : 0;

    if(force) {
    	leds_anim_off(ANIM_ANIMSCRIPT);
    } else {
		ascr_vm_32_data[ASCR_MEM_STOP] = 1;
    }

    reply[ptr++] = RET_OK;
    bslink_write(reply, ptr);
}


disp_cmdfn_t ascr_cmdfn[] = {
    ascr_cmd_load,
    ascr_cmd_run,
    ascr_cmd_stop,
 };
#define ASCR_CMDFN_CNT (sizeof(ascr_cmdfn)/sizeof(disp_cmdfn_t))

void ascr_dispatch(uint8_t *buf, uint32_t len) {
    if(buf[0] >= ASCR_CMDFN_CNT) {
        uint8_t reply[1];
        uint32_t ptr = 0;
        reply[ptr++] = RET_UNIMPLEMENTED;
        bslink_write(reply, ptr);
    } else {
        ascr_cmdfn[buf[0]](buf+1, len-1);
    }
}


void ascr_init() {
	ascr_vm_fault = 0;
	memset(ascr_vm_data, 0, ASCR_DATAMAX);

	//disp_register(5, 1, ascr_dispatch);
}