/*
 * SH_application_device_revisions.h
 *
 *  Created on: Sep 22, 2016
 *      Author: Sean
 */

#ifndef SH_FIRMWARE_CODE_SH_APPLICATION_DEVICE_REVISIONS_H_
#define SH_FIRMWARE_CODE_SH_APPLICATION_DEVICE_REVISIONS_H_

#define APPLICATION_REVISION_MAJOR       0x00            						    /** DFU Major revision number to be exposed. */
#define APPLICATION_REVISION_MINOR       0x07						                /** DFU Minor revision number to be exposed. */
#define DFU_REVISION                     ((APPLICATION_REVISION_MAJOR << 8) | APPLICATION_REVISION_MINOR)     /** DFU Revision number to be exposed. Combined of major and minor versions. */

//#define




#endif /* SH_FIRMWARE_CODE_SH_APPLICATION_DEVICE_REVISIONS_H_ */
