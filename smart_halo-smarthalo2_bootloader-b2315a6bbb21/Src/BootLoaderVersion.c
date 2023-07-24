/*!
    @file BootLoaderVersion.c

    Pretty print the boot loader version

    @author     Georg Nikodym
    @copyright  Copyright (c) 2019 SmartHalo Inc
*/

#include <OSS/printf.h>
#include "BootLoaderVersion.h"

void printBootLoaderVersion(blVersion_t *vers)
{
    if ((uintptr_t)vers == 0xffffffff) return;

    LOG("%d.%d.%d.%d",
        vers->blVersionMajor,
        vers->blVersionMinor,
        vers->blVersionRevision,
        vers->blVersionCommit);
    // LOG("\n%08x", vers->blVersion);
}