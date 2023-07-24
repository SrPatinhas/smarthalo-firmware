/*
 * SH_compile_flags_conflicts.h
 *
 *  Created on: Jun 20, 2016
 *      Author: Sean Beitz
 */

#ifndef SH_FIRMWARE_CODE_SH_COMPILE_FLAGS_CONFLICTS_H_
#define SH_FIRMWARE_CODE_SH_COMPILE_FLAGS_CONFLICTS_H_


//#ifdef FACTORY_PROGRAMMED_FIRMWARE
//#define FACTORY_FAILSAFE			0x01
//#else
//#define FACTORY_FAILSAFE			0x00
//#endif
//
//#if defined(UART_DEBUG_MESSAGES) ||  defined(DEBUG_MODE)
//#define DEBUG_FAILSAFE				0x01
//#else
//#define DEBUG_FAILSAFE				0x00
//#endif
//
//#if defined(DFU_UPDATE_READY_MODE)
//#if (FACTORY_FAILSAFE||DEBUG_FAILSAFE)
//#error("Compilation failsafes error")
//
//#endif

//#if defined(FACTORY_MODE_READY) && (DEBUG_FAILSAFE)
//#error("Compilation failsafes error:")
//#endif
//
//#if defined(FACTORY_MODE_READY) && (!FACTORY_FAILSAFE)
//#error("Compilation failsafes error: not in factory mode")
//#endif
//
//#if defined(FACTORY_MODE_READY) && defined(DFU_UPDATE_READY_MODE)
//#error("Compilation failsafes error: both factory and dfu update modes are selected")
//#endif

//#if defined(FACTORY_MODE_READY) ^ defined(DFU_UPDATE_READY_MODE)
//#error("Compilation failsafes error:select only one mode (debug, factory or dfu ready)")
//#elif defined(FACTORY_MODE_READY) ^ defined(DEBUG_MODE)
//#error("Compilation failsafes error:select only one mode (debug, factory or dfu ready)")
//#elif defined(DFU_UPDATE_READY_MODE) ^ defined(DEBUG_MODE)
//#error("Compilation failsafes error:select only one mode (debug, factory or dfu ready)")
//#endif

//#define SAFETY_CHECK ((defined(FACTORY_MODE_READY) ^ defined(DFU_UPDATE_READY_MODE)) ^ defined(DEBUG_MODE))
//#if (SAFETY_CHECK == 1)
//#error("Compilation failsafes error:select only one mode (debug, factory or dfu ready)")
//#endif

#endif /* SH_FIRMWARE_CODE_SH_COMPILE_FLAGS_CONFLICTS_H_ */
