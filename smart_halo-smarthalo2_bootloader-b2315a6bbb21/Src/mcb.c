/*
 * mcb.c
 */

#include "mcb.h"
#include "log.h"
#include "memory_map.h"
#include "FlashMem.h"
#include "ExtFlash.h"

mcb_t *MCB = (mcb_t *)MCB_MAGIC;

/*! @brief  Boot loader version data

    This structure is compiled in. It is copied
    to the MCB after first boot.
*/
static const blVersion_t gVersion = {
    .blVersionMajor     = BL_VERS_MAJOR,
    .blVersionMinor     = BL_VERS_MINOR,
    .blVersionRevision  = BL_VERS_REVISION,
    .blVersionCommit    = BL_VERS_COMMIT
};

static bool writeMcb();


/*! @brief  Return true if this is the first boot after a
            the flash was initially setup in manufacturing
*/
bool McbFirstboot()
{
    LOG("MCB_FIRSTBOOT: %08x\n", MCB->flags);
    if (MCB->flags == 0xffffffff) {
        return true;
    }
    return false;
}

/*! @brief  Copy the crc/len info into backup fw metadata location

    After the very first boot, we have copied the image on the internal
    flash into the backup/golden area of the external flash. This function
    will update the MCB entry for the golden region.

    Additionally, set the pointer in the MCB to the global version number
    (no need to copy the thing, the bootloader is always resident)
*/
bool McbUpdateGoldenCRC()
{
    // Set buffer to erased flash
    memset(u8Buffer, 0xff, sizeof(u8Buffer));
    // Copy MCB structure into buffer
    mcb_t     *newMcb = (mcb_t *)u8Buffer;
    *newMcb = *MCB;

    newMcb->flags = 0x7fffffff;
    newMcb->fwData[MCB_EXT_BACKUP_IDX].crc = MCB_CRC(MCB_INTERNAL_IDX);
    newMcb->fwData[MCB_EXT_BACKUP_IDX].len = MCB_LEN(MCB_INTERNAL_IDX);

    // Put the bootloader version number into the MCB
    newMcb->version.blVersion = gVersion.blVersion;

    return writeMcb();
}

/*! @brief  Update the MCB metadata for the installed FW

    Firmware has just been installed to the internal flash
    from one of the external flash regions. Update the MCB with
    the metadata (crc and length) associated with that idx.
*/
bool McbUpdateInteralMetadata(int idx)
{
    memset(u8Buffer, 0xff, sizeof(u8Buffer));

    // copy existing MCB
    mcb_t   *newMcb = (mcb_t *)u8Buffer;
    *newMcb = *MCB;

    newMcb->fwData[MCB_INTERNAL_IDX] = MCB->fwData[idx];

    return writeMcb();
}

/*! @brief  Update MCB Metadata with the freshly installed update

    An update FW has been received from SH2OS (written to the
    update area of the external flash). Bootloader has copied
    it to the internal flash. This function performs the necessary
    updates to the MCB.
*/
bool McbUpdateMCBWithUpdate(uint32_t crc, uint32_t len)
{
    memset(u8Buffer, 0xff, sizeof(u8Buffer));

    // copy existing MCB
    mcb_t   *newMcb = (mcb_t *)u8Buffer;
    *newMcb = *MCB;

    // Put crc/len into update metadata member
    newMcb->fwData[MCB_EXT_UPDATE_IDX].crc = crc;
    newMcb->fwData[MCB_EXT_UPDATE_IDX].len = len;
    // Copy that to the internal
    newMcb->fwData[MCB_INTERNAL_IDX] = newMcb->fwData[MCB_EXT_UPDATE_IDX];

    return writeMcb();
}

/*! @brief  Do the actual flashing of the MCB
*/
static bool writeMcb()
{
    bool    bData, res;

    if (!FlashMemErasePage(MCB_MAGIC, 1)) {
        ERR("FlashMemErasePage failed at %08x\n", MCB_MAGIC);
        return false;
    }

    res = FlashMemWriteFullPage(MCB_MAGIC, u8Buffer, &bData);
    if (res == false) {
        ERR("FlashMemWriteFullPage returned %s\n", res ? "true" : "false");
        return false;
    }
    if (!bData) {
        ERR("FlashMemWriteFullPage set bData to %s\n", bData ? "true" : "false");
        return false;
    }

    return true;
}
