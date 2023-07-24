# Smarthalo E STM Code Repository

The current status of the code here is minimal operation to prove that the hardware is functionnal. There is a single task setup for FreeRtos CMSIS V2 called GuiTask. This is the task responsible for calling the touchgfx code, which is not plug and play operation. 

## Processor Specific Memory Particularity
the processor chosen has 128k of ram, and 1024k of SRAM internally. By deafult the bss section is mapped to the ram, but the frame buffers will not fit in there. it is therefore necessary to map those to the AXI_SRAM. See the attribute of GFXMMU_PHY_BUF_0. Currently there is only one frame buffer here, another will need to be added to allow for animations instead of static images. Check the gfxmmu section for easy optimisations of memory use by the frame buffers. 

For this reason, I created a section .frame_buffers in the linker script that is mapped to the AXI_SRAM. GFXMMU_PHY_BUF_0 is given the attributes of aligned to 16 bits, and section ".frame_buffers" to specifiy that it should rest in the frame buffer section of memory. this section is aligned to 4 bytes in the linker script, to allow the storage of any other non frame buffer variables aligned to 16 bits.

## LCD driver
The lcd receives it's image date via the parallel rgb565 lines and its commands via spi. There is a magical sequence of commands and parameters to be sent to the display that is given mysteriously by the lcd panel manufacturer, but only if you ask. This sequence sets up internal power supplies, gamma correction and the likes. Dont touch it, its fine and it works.

## LCD and Images Modules Explanations

There are multiple modules that must work in unison to allow an image to be displayed on the lcd screen. 

Touchgfx will generate images as bitmaps that will be stored in a frame buffer by the DMA2D module that is then piped to the LTDC module to be sent to the lcd display. The frame buffer is defined by the gfxmmu and a user made array. See below for more details.

### LTDC module

This is a module that is responsible for outputting the physical transport layer of the RGB565 parallel signal. If this module is disabled, there will be no output on the parallel signals to the lcd display.
[LTDC APPLICATION NOTE](https://drive.google.com/file/d/1-DfbGSU0Q_96ODMqvLUZQEQzXK6xFwWk/view?usp=sharing)

### GFXMMU

The graphical memory management unit is used to convert a frame buffer from a fixed rectangular array to a virtual array of any shape, for example a circle. In order to use this module adequately, one must first map an excel file in CubeMX in the GFXMMU section. This is detailed in the below application note. Currently the excel file mapped is a 480x480 square, see excel file in root project folder. This will need to be mapped to a circle instead to save some SRAM, and it will reduce the size of GFXMMU_FB_SIZE from 460K to give or take 370K of ram. 
Frame buffer 1 is currently stored at 0x30000000, and frame buffer 2 is set to +0x70800 (480x480, 16 bits, aligned). when reducing the size of the frame buffer, reduce the offset in memory locations accordingly.
[GFXMMU APPLICATION NOTE](https://www.st.com/resource/en/application_note/dm00407777-graphic-memory-optimization-with-stm32-chromgrc-stmicroelectronics.pdf)

### DMA2D (AKA Chrome-Art)

This is a hardware accelerator module that rasterixes images into frames (bitmaps) by combining images or converting image formats and storing the results into a memory location determined by the frame buffer address. The rate of rasterisation is one pixel per clock cycle independent of the format. 
[DMA2D APPLICATION NOTE](https://drive.google.com/file/d/1-ZwlMv4-iTOiNeTWSQ9s9DnY63_X7upT/view?usp=sharing)

## Work to complete

As of the handover from Sean's departure, the work to be compelted to allow images and animations on the lcd display is the following:

1- There is a broken link in between the generation of images and animations in TouchGFX and the outputted image on the lcd display. Currently the way to display an image is to copy the image's bitmap, found in smarthaloe-stm-firmware/TouchGFX/generated/images/src into the frame buffer located at memory address 0x30000000 (user defined as GFXMMU_PHY_BUF_0 in the main.c, can be conveniently set as a user label in cubemx when writing memory address location for frame buffers). attempting to generate an image directly from touchgfx to the lcd yields a squished image. this means that the likeliest place to look for an error is in the GFXMMU module setup, or less likely but maybe the DMA2D.

2- Most of the peripheral modules are setup in blocking mode at the moment. these should be swapped to DMA or IT based for non blocking operation

3- Look for all //SBEITZ tags, these indicate a to do or important information. Also dont spend too much time looking around, just give Sean a call for a boost in the right direction: 514 435-6090. 




