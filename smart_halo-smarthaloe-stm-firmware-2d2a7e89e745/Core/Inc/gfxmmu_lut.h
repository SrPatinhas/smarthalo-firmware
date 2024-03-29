/**
  ******************************************************************************
  * File Name          : gfxmmu_lut.h
  * Description        : header file for GFX MMU Configuration Table
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __gfxmmu_lut_H
#define __gfxmmu_lut_H
#ifdef __cplusplus
 extern "C" {
#endif
// GFX MMU Configuration Table

  #define GFXMMU_FB_SIZE 460800
  #define GFXMMU_LUT_FIRST 0
  #define GFXMMU_LUT_LAST  479
  #define GFXMMU_LUT_SIZE 480

uint32_t gfxmmu_lut_config[2*GFXMMU_LUT_SIZE] = {
  0x003B0001, //GFXMMU_LUT0L
  0x00000000, //GFXMMU_LUT0H
  0x003B0001, //GFXMMU_LUT1L
  0x000003C0, //GFXMMU_LUT1H
  0x003B0001, //GFXMMU_LUT2L
  0x00000780, //GFXMMU_LUT2H
  0x003B0001, //GFXMMU_LUT3L
  0x00000B40, //GFXMMU_LUT3H
  0x003B0001, //GFXMMU_LUT4L
  0x00000F00, //GFXMMU_LUT4H
  0x003B0001, //GFXMMU_LUT5L
  0x000012C0, //GFXMMU_LUT5H
  0x003B0001, //GFXMMU_LUT6L
  0x00001680, //GFXMMU_LUT6H
  0x003B0001, //GFXMMU_LUT7L
  0x00001A40, //GFXMMU_LUT7H
  0x003B0001, //GFXMMU_LUT8L
  0x00001E00, //GFXMMU_LUT8H
  0x003B0001, //GFXMMU_LUT9L
  0x000021C0, //GFXMMU_LUT9H
  0x003B0001, //GFXMMU_LUT10L
  0x00002580, //GFXMMU_LUT10H
  0x003B0001, //GFXMMU_LUT11L
  0x00002940, //GFXMMU_LUT11H
  0x003B0001, //GFXMMU_LUT12L
  0x00002D00, //GFXMMU_LUT12H
  0x003B0001, //GFXMMU_LUT13L
  0x000030C0, //GFXMMU_LUT13H
  0x003B0001, //GFXMMU_LUT14L
  0x00003480, //GFXMMU_LUT14H
  0x003B0001, //GFXMMU_LUT15L
  0x00003840, //GFXMMU_LUT15H
  0x003B0001, //GFXMMU_LUT16L
  0x00003C00, //GFXMMU_LUT16H
  0x003B0001, //GFXMMU_LUT17L
  0x00003FC0, //GFXMMU_LUT17H
  0x003B0001, //GFXMMU_LUT18L
  0x00004380, //GFXMMU_LUT18H
  0x003B0001, //GFXMMU_LUT19L
  0x00004740, //GFXMMU_LUT19H
  0x003B0001, //GFXMMU_LUT20L
  0x00004B00, //GFXMMU_LUT20H
  0x003B0001, //GFXMMU_LUT21L
  0x00004EC0, //GFXMMU_LUT21H
  0x003B0001, //GFXMMU_LUT22L
  0x00005280, //GFXMMU_LUT22H
  0x003B0001, //GFXMMU_LUT23L
  0x00005640, //GFXMMU_LUT23H
  0x003B0001, //GFXMMU_LUT24L
  0x00005A00, //GFXMMU_LUT24H
  0x003B0001, //GFXMMU_LUT25L
  0x00005DC0, //GFXMMU_LUT25H
  0x003B0001, //GFXMMU_LUT26L
  0x00006180, //GFXMMU_LUT26H
  0x003B0001, //GFXMMU_LUT27L
  0x00006540, //GFXMMU_LUT27H
  0x003B0001, //GFXMMU_LUT28L
  0x00006900, //GFXMMU_LUT28H
  0x003B0001, //GFXMMU_LUT29L
  0x00006CC0, //GFXMMU_LUT29H
  0x003B0001, //GFXMMU_LUT30L
  0x00007080, //GFXMMU_LUT30H
  0x003B0001, //GFXMMU_LUT31L
  0x00007440, //GFXMMU_LUT31H
  0x003B0001, //GFXMMU_LUT32L
  0x00007800, //GFXMMU_LUT32H
  0x003B0001, //GFXMMU_LUT33L
  0x00007BC0, //GFXMMU_LUT33H
  0x003B0001, //GFXMMU_LUT34L
  0x00007F80, //GFXMMU_LUT34H
  0x003B0001, //GFXMMU_LUT35L
  0x00008340, //GFXMMU_LUT35H
  0x003B0001, //GFXMMU_LUT36L
  0x00008700, //GFXMMU_LUT36H
  0x003B0001, //GFXMMU_LUT37L
  0x00008AC0, //GFXMMU_LUT37H
  0x003B0001, //GFXMMU_LUT38L
  0x00008E80, //GFXMMU_LUT38H
  0x003B0001, //GFXMMU_LUT39L
  0x00009240, //GFXMMU_LUT39H
  0x003B0001, //GFXMMU_LUT40L
  0x00009600, //GFXMMU_LUT40H
  0x003B0001, //GFXMMU_LUT41L
  0x000099C0, //GFXMMU_LUT41H
  0x003B0001, //GFXMMU_LUT42L
  0x00009D80, //GFXMMU_LUT42H
  0x003B0001, //GFXMMU_LUT43L
  0x0000A140, //GFXMMU_LUT43H
  0x003B0001, //GFXMMU_LUT44L
  0x0000A500, //GFXMMU_LUT44H
  0x003B0001, //GFXMMU_LUT45L
  0x0000A8C0, //GFXMMU_LUT45H
  0x003B0001, //GFXMMU_LUT46L
  0x0000AC80, //GFXMMU_LUT46H
  0x003B0001, //GFXMMU_LUT47L
  0x0000B040, //GFXMMU_LUT47H
  0x003B0001, //GFXMMU_LUT48L
  0x0000B400, //GFXMMU_LUT48H
  0x003B0001, //GFXMMU_LUT49L
  0x0000B7C0, //GFXMMU_LUT49H
  0x003B0001, //GFXMMU_LUT50L
  0x0000BB80, //GFXMMU_LUT50H
  0x003B0001, //GFXMMU_LUT51L
  0x0000BF40, //GFXMMU_LUT51H
  0x003B0001, //GFXMMU_LUT52L
  0x0000C300, //GFXMMU_LUT52H
  0x003B0001, //GFXMMU_LUT53L
  0x0000C6C0, //GFXMMU_LUT53H
  0x003B0001, //GFXMMU_LUT54L
  0x0000CA80, //GFXMMU_LUT54H
  0x003B0001, //GFXMMU_LUT55L
  0x0000CE40, //GFXMMU_LUT55H
  0x003B0001, //GFXMMU_LUT56L
  0x0000D200, //GFXMMU_LUT56H
  0x003B0001, //GFXMMU_LUT57L
  0x0000D5C0, //GFXMMU_LUT57H
  0x003B0001, //GFXMMU_LUT58L
  0x0000D980, //GFXMMU_LUT58H
  0x003B0001, //GFXMMU_LUT59L
  0x0000DD40, //GFXMMU_LUT59H
  0x003B0001, //GFXMMU_LUT60L
  0x0000E100, //GFXMMU_LUT60H
  0x003B0001, //GFXMMU_LUT61L
  0x0000E4C0, //GFXMMU_LUT61H
  0x003B0001, //GFXMMU_LUT62L
  0x0000E880, //GFXMMU_LUT62H
  0x003B0001, //GFXMMU_LUT63L
  0x0000EC40, //GFXMMU_LUT63H
  0x003B0001, //GFXMMU_LUT64L
  0x0000F000, //GFXMMU_LUT64H
  0x003B0001, //GFXMMU_LUT65L
  0x0000F3C0, //GFXMMU_LUT65H
  0x003B0001, //GFXMMU_LUT66L
  0x0000F780, //GFXMMU_LUT66H
  0x003B0001, //GFXMMU_LUT67L
  0x0000FB40, //GFXMMU_LUT67H
  0x003B0001, //GFXMMU_LUT68L
  0x0000FF00, //GFXMMU_LUT68H
  0x003B0001, //GFXMMU_LUT69L
  0x000102C0, //GFXMMU_LUT69H
  0x003B0001, //GFXMMU_LUT70L
  0x00010680, //GFXMMU_LUT70H
  0x003B0001, //GFXMMU_LUT71L
  0x00010A40, //GFXMMU_LUT71H
  0x003B0001, //GFXMMU_LUT72L
  0x00010E00, //GFXMMU_LUT72H
  0x003B0001, //GFXMMU_LUT73L
  0x000111C0, //GFXMMU_LUT73H
  0x003B0001, //GFXMMU_LUT74L
  0x00011580, //GFXMMU_LUT74H
  0x003B0001, //GFXMMU_LUT75L
  0x00011940, //GFXMMU_LUT75H
  0x003B0001, //GFXMMU_LUT76L
  0x00011D00, //GFXMMU_LUT76H
  0x003B0001, //GFXMMU_LUT77L
  0x000120C0, //GFXMMU_LUT77H
  0x003B0001, //GFXMMU_LUT78L
  0x00012480, //GFXMMU_LUT78H
  0x003B0001, //GFXMMU_LUT79L
  0x00012840, //GFXMMU_LUT79H
  0x003B0001, //GFXMMU_LUT80L
  0x00012C00, //GFXMMU_LUT80H
  0x003B0001, //GFXMMU_LUT81L
  0x00012FC0, //GFXMMU_LUT81H
  0x003B0001, //GFXMMU_LUT82L
  0x00013380, //GFXMMU_LUT82H
  0x003B0001, //GFXMMU_LUT83L
  0x00013740, //GFXMMU_LUT83H
  0x003B0001, //GFXMMU_LUT84L
  0x00013B00, //GFXMMU_LUT84H
  0x003B0001, //GFXMMU_LUT85L
  0x00013EC0, //GFXMMU_LUT85H
  0x003B0001, //GFXMMU_LUT86L
  0x00014280, //GFXMMU_LUT86H
  0x003B0001, //GFXMMU_LUT87L
  0x00014640, //GFXMMU_LUT87H
  0x003B0001, //GFXMMU_LUT88L
  0x00014A00, //GFXMMU_LUT88H
  0x003B0001, //GFXMMU_LUT89L
  0x00014DC0, //GFXMMU_LUT89H
  0x003B0001, //GFXMMU_LUT90L
  0x00015180, //GFXMMU_LUT90H
  0x003B0001, //GFXMMU_LUT91L
  0x00015540, //GFXMMU_LUT91H
  0x003B0001, //GFXMMU_LUT92L
  0x00015900, //GFXMMU_LUT92H
  0x003B0001, //GFXMMU_LUT93L
  0x00015CC0, //GFXMMU_LUT93H
  0x003B0001, //GFXMMU_LUT94L
  0x00016080, //GFXMMU_LUT94H
  0x003B0001, //GFXMMU_LUT95L
  0x00016440, //GFXMMU_LUT95H
  0x003B0001, //GFXMMU_LUT96L
  0x00016800, //GFXMMU_LUT96H
  0x003B0001, //GFXMMU_LUT97L
  0x00016BC0, //GFXMMU_LUT97H
  0x003B0001, //GFXMMU_LUT98L
  0x00016F80, //GFXMMU_LUT98H
  0x003B0001, //GFXMMU_LUT99L
  0x00017340, //GFXMMU_LUT99H
  0x003B0001, //GFXMMU_LUT100L
  0x00017700, //GFXMMU_LUT100H
  0x003B0001, //GFXMMU_LUT101L
  0x00017AC0, //GFXMMU_LUT101H
  0x003B0001, //GFXMMU_LUT102L
  0x00017E80, //GFXMMU_LUT102H
  0x003B0001, //GFXMMU_LUT103L
  0x00018240, //GFXMMU_LUT103H
  0x003B0001, //GFXMMU_LUT104L
  0x00018600, //GFXMMU_LUT104H
  0x003B0001, //GFXMMU_LUT105L
  0x000189C0, //GFXMMU_LUT105H
  0x003B0001, //GFXMMU_LUT106L
  0x00018D80, //GFXMMU_LUT106H
  0x003B0001, //GFXMMU_LUT107L
  0x00019140, //GFXMMU_LUT107H
  0x003B0001, //GFXMMU_LUT108L
  0x00019500, //GFXMMU_LUT108H
  0x003B0001, //GFXMMU_LUT109L
  0x000198C0, //GFXMMU_LUT109H
  0x003B0001, //GFXMMU_LUT110L
  0x00019C80, //GFXMMU_LUT110H
  0x003B0001, //GFXMMU_LUT111L
  0x0001A040, //GFXMMU_LUT111H
  0x003B0001, //GFXMMU_LUT112L
  0x0001A400, //GFXMMU_LUT112H
  0x003B0001, //GFXMMU_LUT113L
  0x0001A7C0, //GFXMMU_LUT113H
  0x003B0001, //GFXMMU_LUT114L
  0x0001AB80, //GFXMMU_LUT114H
  0x003B0001, //GFXMMU_LUT115L
  0x0001AF40, //GFXMMU_LUT115H
  0x003B0001, //GFXMMU_LUT116L
  0x0001B300, //GFXMMU_LUT116H
  0x003B0001, //GFXMMU_LUT117L
  0x0001B6C0, //GFXMMU_LUT117H
  0x003B0001, //GFXMMU_LUT118L
  0x0001BA80, //GFXMMU_LUT118H
  0x003B0001, //GFXMMU_LUT119L
  0x0001BE40, //GFXMMU_LUT119H
  0x003B0001, //GFXMMU_LUT120L
  0x0001C200, //GFXMMU_LUT120H
  0x003B0001, //GFXMMU_LUT121L
  0x0001C5C0, //GFXMMU_LUT121H
  0x003B0001, //GFXMMU_LUT122L
  0x0001C980, //GFXMMU_LUT122H
  0x003B0001, //GFXMMU_LUT123L
  0x0001CD40, //GFXMMU_LUT123H
  0x003B0001, //GFXMMU_LUT124L
  0x0001D100, //GFXMMU_LUT124H
  0x003B0001, //GFXMMU_LUT125L
  0x0001D4C0, //GFXMMU_LUT125H
  0x003B0001, //GFXMMU_LUT126L
  0x0001D880, //GFXMMU_LUT126H
  0x003B0001, //GFXMMU_LUT127L
  0x0001DC40, //GFXMMU_LUT127H
  0x003B0001, //GFXMMU_LUT128L
  0x0001E000, //GFXMMU_LUT128H
  0x003B0001, //GFXMMU_LUT129L
  0x0001E3C0, //GFXMMU_LUT129H
  0x003B0001, //GFXMMU_LUT130L
  0x0001E780, //GFXMMU_LUT130H
  0x003B0001, //GFXMMU_LUT131L
  0x0001EB40, //GFXMMU_LUT131H
  0x003B0001, //GFXMMU_LUT132L
  0x0001EF00, //GFXMMU_LUT132H
  0x003B0001, //GFXMMU_LUT133L
  0x0001F2C0, //GFXMMU_LUT133H
  0x003B0001, //GFXMMU_LUT134L
  0x0001F680, //GFXMMU_LUT134H
  0x003B0001, //GFXMMU_LUT135L
  0x0001FA40, //GFXMMU_LUT135H
  0x003B0001, //GFXMMU_LUT136L
  0x0001FE00, //GFXMMU_LUT136H
  0x003B0001, //GFXMMU_LUT137L
  0x000201C0, //GFXMMU_LUT137H
  0x003B0001, //GFXMMU_LUT138L
  0x00020580, //GFXMMU_LUT138H
  0x003B0001, //GFXMMU_LUT139L
  0x00020940, //GFXMMU_LUT139H
  0x003B0001, //GFXMMU_LUT140L
  0x00020D00, //GFXMMU_LUT140H
  0x003B0001, //GFXMMU_LUT141L
  0x000210C0, //GFXMMU_LUT141H
  0x003B0001, //GFXMMU_LUT142L
  0x00021480, //GFXMMU_LUT142H
  0x003B0001, //GFXMMU_LUT143L
  0x00021840, //GFXMMU_LUT143H
  0x003B0001, //GFXMMU_LUT144L
  0x00021C00, //GFXMMU_LUT144H
  0x003B0001, //GFXMMU_LUT145L
  0x00021FC0, //GFXMMU_LUT145H
  0x003B0001, //GFXMMU_LUT146L
  0x00022380, //GFXMMU_LUT146H
  0x003B0001, //GFXMMU_LUT147L
  0x00022740, //GFXMMU_LUT147H
  0x003B0001, //GFXMMU_LUT148L
  0x00022B00, //GFXMMU_LUT148H
  0x003B0001, //GFXMMU_LUT149L
  0x00022EC0, //GFXMMU_LUT149H
  0x003B0001, //GFXMMU_LUT150L
  0x00023280, //GFXMMU_LUT150H
  0x003B0001, //GFXMMU_LUT151L
  0x00023640, //GFXMMU_LUT151H
  0x003B0001, //GFXMMU_LUT152L
  0x00023A00, //GFXMMU_LUT152H
  0x003B0001, //GFXMMU_LUT153L
  0x00023DC0, //GFXMMU_LUT153H
  0x003B0001, //GFXMMU_LUT154L
  0x00024180, //GFXMMU_LUT154H
  0x003B0001, //GFXMMU_LUT155L
  0x00024540, //GFXMMU_LUT155H
  0x003B0001, //GFXMMU_LUT156L
  0x00024900, //GFXMMU_LUT156H
  0x003B0001, //GFXMMU_LUT157L
  0x00024CC0, //GFXMMU_LUT157H
  0x003B0001, //GFXMMU_LUT158L
  0x00025080, //GFXMMU_LUT158H
  0x003B0001, //GFXMMU_LUT159L
  0x00025440, //GFXMMU_LUT159H
  0x003B0001, //GFXMMU_LUT160L
  0x00025800, //GFXMMU_LUT160H
  0x003B0001, //GFXMMU_LUT161L
  0x00025BC0, //GFXMMU_LUT161H
  0x003B0001, //GFXMMU_LUT162L
  0x00025F80, //GFXMMU_LUT162H
  0x003B0001, //GFXMMU_LUT163L
  0x00026340, //GFXMMU_LUT163H
  0x003B0001, //GFXMMU_LUT164L
  0x00026700, //GFXMMU_LUT164H
  0x003B0001, //GFXMMU_LUT165L
  0x00026AC0, //GFXMMU_LUT165H
  0x003B0001, //GFXMMU_LUT166L
  0x00026E80, //GFXMMU_LUT166H
  0x003B0001, //GFXMMU_LUT167L
  0x00027240, //GFXMMU_LUT167H
  0x003B0001, //GFXMMU_LUT168L
  0x00027600, //GFXMMU_LUT168H
  0x003B0001, //GFXMMU_LUT169L
  0x000279C0, //GFXMMU_LUT169H
  0x003B0001, //GFXMMU_LUT170L
  0x00027D80, //GFXMMU_LUT170H
  0x003B0001, //GFXMMU_LUT171L
  0x00028140, //GFXMMU_LUT171H
  0x003B0001, //GFXMMU_LUT172L
  0x00028500, //GFXMMU_LUT172H
  0x003B0001, //GFXMMU_LUT173L
  0x000288C0, //GFXMMU_LUT173H
  0x003B0001, //GFXMMU_LUT174L
  0x00028C80, //GFXMMU_LUT174H
  0x003B0001, //GFXMMU_LUT175L
  0x00029040, //GFXMMU_LUT175H
  0x003B0001, //GFXMMU_LUT176L
  0x00029400, //GFXMMU_LUT176H
  0x003B0001, //GFXMMU_LUT177L
  0x000297C0, //GFXMMU_LUT177H
  0x003B0001, //GFXMMU_LUT178L
  0x00029B80, //GFXMMU_LUT178H
  0x003B0001, //GFXMMU_LUT179L
  0x00029F40, //GFXMMU_LUT179H
  0x003B0001, //GFXMMU_LUT180L
  0x0002A300, //GFXMMU_LUT180H
  0x003B0001, //GFXMMU_LUT181L
  0x0002A6C0, //GFXMMU_LUT181H
  0x003B0001, //GFXMMU_LUT182L
  0x0002AA80, //GFXMMU_LUT182H
  0x003B0001, //GFXMMU_LUT183L
  0x0002AE40, //GFXMMU_LUT183H
  0x003B0001, //GFXMMU_LUT184L
  0x0002B200, //GFXMMU_LUT184H
  0x003B0001, //GFXMMU_LUT185L
  0x0002B5C0, //GFXMMU_LUT185H
  0x003B0001, //GFXMMU_LUT186L
  0x0002B980, //GFXMMU_LUT186H
  0x003B0001, //GFXMMU_LUT187L
  0x0002BD40, //GFXMMU_LUT187H
  0x003B0001, //GFXMMU_LUT188L
  0x0002C100, //GFXMMU_LUT188H
  0x003B0001, //GFXMMU_LUT189L
  0x0002C4C0, //GFXMMU_LUT189H
  0x003B0001, //GFXMMU_LUT190L
  0x0002C880, //GFXMMU_LUT190H
  0x003B0001, //GFXMMU_LUT191L
  0x0002CC40, //GFXMMU_LUT191H
  0x003B0001, //GFXMMU_LUT192L
  0x0002D000, //GFXMMU_LUT192H
  0x003B0001, //GFXMMU_LUT193L
  0x0002D3C0, //GFXMMU_LUT193H
  0x003B0001, //GFXMMU_LUT194L
  0x0002D780, //GFXMMU_LUT194H
  0x003B0001, //GFXMMU_LUT195L
  0x0002DB40, //GFXMMU_LUT195H
  0x003B0001, //GFXMMU_LUT196L
  0x0002DF00, //GFXMMU_LUT196H
  0x003B0001, //GFXMMU_LUT197L
  0x0002E2C0, //GFXMMU_LUT197H
  0x003B0001, //GFXMMU_LUT198L
  0x0002E680, //GFXMMU_LUT198H
  0x003B0001, //GFXMMU_LUT199L
  0x0002EA40, //GFXMMU_LUT199H
  0x003B0001, //GFXMMU_LUT200L
  0x0002EE00, //GFXMMU_LUT200H
  0x003B0001, //GFXMMU_LUT201L
  0x0002F1C0, //GFXMMU_LUT201H
  0x003B0001, //GFXMMU_LUT202L
  0x0002F580, //GFXMMU_LUT202H
  0x003B0001, //GFXMMU_LUT203L
  0x0002F940, //GFXMMU_LUT203H
  0x003B0001, //GFXMMU_LUT204L
  0x0002FD00, //GFXMMU_LUT204H
  0x003B0001, //GFXMMU_LUT205L
  0x000300C0, //GFXMMU_LUT205H
  0x003B0001, //GFXMMU_LUT206L
  0x00030480, //GFXMMU_LUT206H
  0x003B0001, //GFXMMU_LUT207L
  0x00030840, //GFXMMU_LUT207H
  0x003B0001, //GFXMMU_LUT208L
  0x00030C00, //GFXMMU_LUT208H
  0x003B0001, //GFXMMU_LUT209L
  0x00030FC0, //GFXMMU_LUT209H
  0x003B0001, //GFXMMU_LUT210L
  0x00031380, //GFXMMU_LUT210H
  0x003B0001, //GFXMMU_LUT211L
  0x00031740, //GFXMMU_LUT211H
  0x003B0001, //GFXMMU_LUT212L
  0x00031B00, //GFXMMU_LUT212H
  0x003B0001, //GFXMMU_LUT213L
  0x00031EC0, //GFXMMU_LUT213H
  0x003B0001, //GFXMMU_LUT214L
  0x00032280, //GFXMMU_LUT214H
  0x003B0001, //GFXMMU_LUT215L
  0x00032640, //GFXMMU_LUT215H
  0x003B0001, //GFXMMU_LUT216L
  0x00032A00, //GFXMMU_LUT216H
  0x003B0001, //GFXMMU_LUT217L
  0x00032DC0, //GFXMMU_LUT217H
  0x003B0001, //GFXMMU_LUT218L
  0x00033180, //GFXMMU_LUT218H
  0x003B0001, //GFXMMU_LUT219L
  0x00033540, //GFXMMU_LUT219H
  0x003B0001, //GFXMMU_LUT220L
  0x00033900, //GFXMMU_LUT220H
  0x003B0001, //GFXMMU_LUT221L
  0x00033CC0, //GFXMMU_LUT221H
  0x003B0001, //GFXMMU_LUT222L
  0x00034080, //GFXMMU_LUT222H
  0x003B0001, //GFXMMU_LUT223L
  0x00034440, //GFXMMU_LUT223H
  0x003B0001, //GFXMMU_LUT224L
  0x00034800, //GFXMMU_LUT224H
  0x003B0001, //GFXMMU_LUT225L
  0x00034BC0, //GFXMMU_LUT225H
  0x003B0001, //GFXMMU_LUT226L
  0x00034F80, //GFXMMU_LUT226H
  0x003B0001, //GFXMMU_LUT227L
  0x00035340, //GFXMMU_LUT227H
  0x003B0001, //GFXMMU_LUT228L
  0x00035700, //GFXMMU_LUT228H
  0x003B0001, //GFXMMU_LUT229L
  0x00035AC0, //GFXMMU_LUT229H
  0x003B0001, //GFXMMU_LUT230L
  0x00035E80, //GFXMMU_LUT230H
  0x003B0001, //GFXMMU_LUT231L
  0x00036240, //GFXMMU_LUT231H
  0x003B0001, //GFXMMU_LUT232L
  0x00036600, //GFXMMU_LUT232H
  0x003B0001, //GFXMMU_LUT233L
  0x000369C0, //GFXMMU_LUT233H
  0x003B0001, //GFXMMU_LUT234L
  0x00036D80, //GFXMMU_LUT234H
  0x003B0001, //GFXMMU_LUT235L
  0x00037140, //GFXMMU_LUT235H
  0x003B0001, //GFXMMU_LUT236L
  0x00037500, //GFXMMU_LUT236H
  0x003B0001, //GFXMMU_LUT237L
  0x000378C0, //GFXMMU_LUT237H
  0x003B0001, //GFXMMU_LUT238L
  0x00037C80, //GFXMMU_LUT238H
  0x003B0001, //GFXMMU_LUT239L
  0x00038040, //GFXMMU_LUT239H
  0x003B0001, //GFXMMU_LUT240L
  0x00038400, //GFXMMU_LUT240H
  0x003B0001, //GFXMMU_LUT241L
  0x000387C0, //GFXMMU_LUT241H
  0x003B0001, //GFXMMU_LUT242L
  0x00038B80, //GFXMMU_LUT242H
  0x003B0001, //GFXMMU_LUT243L
  0x00038F40, //GFXMMU_LUT243H
  0x003B0001, //GFXMMU_LUT244L
  0x00039300, //GFXMMU_LUT244H
  0x003B0001, //GFXMMU_LUT245L
  0x000396C0, //GFXMMU_LUT245H
  0x003B0001, //GFXMMU_LUT246L
  0x00039A80, //GFXMMU_LUT246H
  0x003B0001, //GFXMMU_LUT247L
  0x00039E40, //GFXMMU_LUT247H
  0x003B0001, //GFXMMU_LUT248L
  0x0003A200, //GFXMMU_LUT248H
  0x003B0001, //GFXMMU_LUT249L
  0x0003A5C0, //GFXMMU_LUT249H
  0x003B0001, //GFXMMU_LUT250L
  0x0003A980, //GFXMMU_LUT250H
  0x003B0001, //GFXMMU_LUT251L
  0x0003AD40, //GFXMMU_LUT251H
  0x003B0001, //GFXMMU_LUT252L
  0x0003B100, //GFXMMU_LUT252H
  0x003B0001, //GFXMMU_LUT253L
  0x0003B4C0, //GFXMMU_LUT253H
  0x003B0001, //GFXMMU_LUT254L
  0x0003B880, //GFXMMU_LUT254H
  0x003B0001, //GFXMMU_LUT255L
  0x0003BC40, //GFXMMU_LUT255H
  0x003B0001, //GFXMMU_LUT256L
  0x0003C000, //GFXMMU_LUT256H
  0x003B0001, //GFXMMU_LUT257L
  0x0003C3C0, //GFXMMU_LUT257H
  0x003B0001, //GFXMMU_LUT258L
  0x0003C780, //GFXMMU_LUT258H
  0x003B0001, //GFXMMU_LUT259L
  0x0003CB40, //GFXMMU_LUT259H
  0x003B0001, //GFXMMU_LUT260L
  0x0003CF00, //GFXMMU_LUT260H
  0x003B0001, //GFXMMU_LUT261L
  0x0003D2C0, //GFXMMU_LUT261H
  0x003B0001, //GFXMMU_LUT262L
  0x0003D680, //GFXMMU_LUT262H
  0x003B0001, //GFXMMU_LUT263L
  0x0003DA40, //GFXMMU_LUT263H
  0x003B0001, //GFXMMU_LUT264L
  0x0003DE00, //GFXMMU_LUT264H
  0x003B0001, //GFXMMU_LUT265L
  0x0003E1C0, //GFXMMU_LUT265H
  0x003B0001, //GFXMMU_LUT266L
  0x0003E580, //GFXMMU_LUT266H
  0x003B0001, //GFXMMU_LUT267L
  0x0003E940, //GFXMMU_LUT267H
  0x003B0001, //GFXMMU_LUT268L
  0x0003ED00, //GFXMMU_LUT268H
  0x003B0001, //GFXMMU_LUT269L
  0x0003F0C0, //GFXMMU_LUT269H
  0x003B0001, //GFXMMU_LUT270L
  0x0003F480, //GFXMMU_LUT270H
  0x003B0001, //GFXMMU_LUT271L
  0x0003F840, //GFXMMU_LUT271H
  0x003B0001, //GFXMMU_LUT272L
  0x0003FC00, //GFXMMU_LUT272H
  0x003B0001, //GFXMMU_LUT273L
  0x0003FFC0, //GFXMMU_LUT273H
  0x003B0001, //GFXMMU_LUT274L
  0x00040380, //GFXMMU_LUT274H
  0x003B0001, //GFXMMU_LUT275L
  0x00040740, //GFXMMU_LUT275H
  0x003B0001, //GFXMMU_LUT276L
  0x00040B00, //GFXMMU_LUT276H
  0x003B0001, //GFXMMU_LUT277L
  0x00040EC0, //GFXMMU_LUT277H
  0x003B0001, //GFXMMU_LUT278L
  0x00041280, //GFXMMU_LUT278H
  0x003B0001, //GFXMMU_LUT279L
  0x00041640, //GFXMMU_LUT279H
  0x003B0001, //GFXMMU_LUT280L
  0x00041A00, //GFXMMU_LUT280H
  0x003B0001, //GFXMMU_LUT281L
  0x00041DC0, //GFXMMU_LUT281H
  0x003B0001, //GFXMMU_LUT282L
  0x00042180, //GFXMMU_LUT282H
  0x003B0001, //GFXMMU_LUT283L
  0x00042540, //GFXMMU_LUT283H
  0x003B0001, //GFXMMU_LUT284L
  0x00042900, //GFXMMU_LUT284H
  0x003B0001, //GFXMMU_LUT285L
  0x00042CC0, //GFXMMU_LUT285H
  0x003B0001, //GFXMMU_LUT286L
  0x00043080, //GFXMMU_LUT286H
  0x003B0001, //GFXMMU_LUT287L
  0x00043440, //GFXMMU_LUT287H
  0x003B0001, //GFXMMU_LUT288L
  0x00043800, //GFXMMU_LUT288H
  0x003B0001, //GFXMMU_LUT289L
  0x00043BC0, //GFXMMU_LUT289H
  0x003B0001, //GFXMMU_LUT290L
  0x00043F80, //GFXMMU_LUT290H
  0x003B0001, //GFXMMU_LUT291L
  0x00044340, //GFXMMU_LUT291H
  0x003B0001, //GFXMMU_LUT292L
  0x00044700, //GFXMMU_LUT292H
  0x003B0001, //GFXMMU_LUT293L
  0x00044AC0, //GFXMMU_LUT293H
  0x003B0001, //GFXMMU_LUT294L
  0x00044E80, //GFXMMU_LUT294H
  0x003B0001, //GFXMMU_LUT295L
  0x00045240, //GFXMMU_LUT295H
  0x003B0001, //GFXMMU_LUT296L
  0x00045600, //GFXMMU_LUT296H
  0x003B0001, //GFXMMU_LUT297L
  0x000459C0, //GFXMMU_LUT297H
  0x003B0001, //GFXMMU_LUT298L
  0x00045D80, //GFXMMU_LUT298H
  0x003B0001, //GFXMMU_LUT299L
  0x00046140, //GFXMMU_LUT299H
  0x003B0001, //GFXMMU_LUT300L
  0x00046500, //GFXMMU_LUT300H
  0x003B0001, //GFXMMU_LUT301L
  0x000468C0, //GFXMMU_LUT301H
  0x003B0001, //GFXMMU_LUT302L
  0x00046C80, //GFXMMU_LUT302H
  0x003B0001, //GFXMMU_LUT303L
  0x00047040, //GFXMMU_LUT303H
  0x003B0001, //GFXMMU_LUT304L
  0x00047400, //GFXMMU_LUT304H
  0x003B0001, //GFXMMU_LUT305L
  0x000477C0, //GFXMMU_LUT305H
  0x003B0001, //GFXMMU_LUT306L
  0x00047B80, //GFXMMU_LUT306H
  0x003B0001, //GFXMMU_LUT307L
  0x00047F40, //GFXMMU_LUT307H
  0x003B0001, //GFXMMU_LUT308L
  0x00048300, //GFXMMU_LUT308H
  0x003B0001, //GFXMMU_LUT309L
  0x000486C0, //GFXMMU_LUT309H
  0x003B0001, //GFXMMU_LUT310L
  0x00048A80, //GFXMMU_LUT310H
  0x003B0001, //GFXMMU_LUT311L
  0x00048E40, //GFXMMU_LUT311H
  0x003B0001, //GFXMMU_LUT312L
  0x00049200, //GFXMMU_LUT312H
  0x003B0001, //GFXMMU_LUT313L
  0x000495C0, //GFXMMU_LUT313H
  0x003B0001, //GFXMMU_LUT314L
  0x00049980, //GFXMMU_LUT314H
  0x003B0001, //GFXMMU_LUT315L
  0x00049D40, //GFXMMU_LUT315H
  0x003B0001, //GFXMMU_LUT316L
  0x0004A100, //GFXMMU_LUT316H
  0x003B0001, //GFXMMU_LUT317L
  0x0004A4C0, //GFXMMU_LUT317H
  0x003B0001, //GFXMMU_LUT318L
  0x0004A880, //GFXMMU_LUT318H
  0x003B0001, //GFXMMU_LUT319L
  0x0004AC40, //GFXMMU_LUT319H
  0x003B0001, //GFXMMU_LUT320L
  0x0004B000, //GFXMMU_LUT320H
  0x003B0001, //GFXMMU_LUT321L
  0x0004B3C0, //GFXMMU_LUT321H
  0x003B0001, //GFXMMU_LUT322L
  0x0004B780, //GFXMMU_LUT322H
  0x003B0001, //GFXMMU_LUT323L
  0x0004BB40, //GFXMMU_LUT323H
  0x003B0001, //GFXMMU_LUT324L
  0x0004BF00, //GFXMMU_LUT324H
  0x003B0001, //GFXMMU_LUT325L
  0x0004C2C0, //GFXMMU_LUT325H
  0x003B0001, //GFXMMU_LUT326L
  0x0004C680, //GFXMMU_LUT326H
  0x003B0001, //GFXMMU_LUT327L
  0x0004CA40, //GFXMMU_LUT327H
  0x003B0001, //GFXMMU_LUT328L
  0x0004CE00, //GFXMMU_LUT328H
  0x003B0001, //GFXMMU_LUT329L
  0x0004D1C0, //GFXMMU_LUT329H
  0x003B0001, //GFXMMU_LUT330L
  0x0004D580, //GFXMMU_LUT330H
  0x003B0001, //GFXMMU_LUT331L
  0x0004D940, //GFXMMU_LUT331H
  0x003B0001, //GFXMMU_LUT332L
  0x0004DD00, //GFXMMU_LUT332H
  0x003B0001, //GFXMMU_LUT333L
  0x0004E0C0, //GFXMMU_LUT333H
  0x003B0001, //GFXMMU_LUT334L
  0x0004E480, //GFXMMU_LUT334H
  0x003B0001, //GFXMMU_LUT335L
  0x0004E840, //GFXMMU_LUT335H
  0x003B0001, //GFXMMU_LUT336L
  0x0004EC00, //GFXMMU_LUT336H
  0x003B0001, //GFXMMU_LUT337L
  0x0004EFC0, //GFXMMU_LUT337H
  0x003B0001, //GFXMMU_LUT338L
  0x0004F380, //GFXMMU_LUT338H
  0x003B0001, //GFXMMU_LUT339L
  0x0004F740, //GFXMMU_LUT339H
  0x003B0001, //GFXMMU_LUT340L
  0x0004FB00, //GFXMMU_LUT340H
  0x003B0001, //GFXMMU_LUT341L
  0x0004FEC0, //GFXMMU_LUT341H
  0x003B0001, //GFXMMU_LUT342L
  0x00050280, //GFXMMU_LUT342H
  0x003B0001, //GFXMMU_LUT343L
  0x00050640, //GFXMMU_LUT343H
  0x003B0001, //GFXMMU_LUT344L
  0x00050A00, //GFXMMU_LUT344H
  0x003B0001, //GFXMMU_LUT345L
  0x00050DC0, //GFXMMU_LUT345H
  0x003B0001, //GFXMMU_LUT346L
  0x00051180, //GFXMMU_LUT346H
  0x003B0001, //GFXMMU_LUT347L
  0x00051540, //GFXMMU_LUT347H
  0x003B0001, //GFXMMU_LUT348L
  0x00051900, //GFXMMU_LUT348H
  0x003B0001, //GFXMMU_LUT349L
  0x00051CC0, //GFXMMU_LUT349H
  0x003B0001, //GFXMMU_LUT350L
  0x00052080, //GFXMMU_LUT350H
  0x003B0001, //GFXMMU_LUT351L
  0x00052440, //GFXMMU_LUT351H
  0x003B0001, //GFXMMU_LUT352L
  0x00052800, //GFXMMU_LUT352H
  0x003B0001, //GFXMMU_LUT353L
  0x00052BC0, //GFXMMU_LUT353H
  0x003B0001, //GFXMMU_LUT354L
  0x00052F80, //GFXMMU_LUT354H
  0x003B0001, //GFXMMU_LUT355L
  0x00053340, //GFXMMU_LUT355H
  0x003B0001, //GFXMMU_LUT356L
  0x00053700, //GFXMMU_LUT356H
  0x003B0001, //GFXMMU_LUT357L
  0x00053AC0, //GFXMMU_LUT357H
  0x003B0001, //GFXMMU_LUT358L
  0x00053E80, //GFXMMU_LUT358H
  0x003B0001, //GFXMMU_LUT359L
  0x00054240, //GFXMMU_LUT359H
  0x003B0001, //GFXMMU_LUT360L
  0x00054600, //GFXMMU_LUT360H
  0x003B0001, //GFXMMU_LUT361L
  0x000549C0, //GFXMMU_LUT361H
  0x003B0001, //GFXMMU_LUT362L
  0x00054D80, //GFXMMU_LUT362H
  0x003B0001, //GFXMMU_LUT363L
  0x00055140, //GFXMMU_LUT363H
  0x003B0001, //GFXMMU_LUT364L
  0x00055500, //GFXMMU_LUT364H
  0x003B0001, //GFXMMU_LUT365L
  0x000558C0, //GFXMMU_LUT365H
  0x003B0001, //GFXMMU_LUT366L
  0x00055C80, //GFXMMU_LUT366H
  0x003B0001, //GFXMMU_LUT367L
  0x00056040, //GFXMMU_LUT367H
  0x003B0001, //GFXMMU_LUT368L
  0x00056400, //GFXMMU_LUT368H
  0x003B0001, //GFXMMU_LUT369L
  0x000567C0, //GFXMMU_LUT369H
  0x003B0001, //GFXMMU_LUT370L
  0x00056B80, //GFXMMU_LUT370H
  0x003B0001, //GFXMMU_LUT371L
  0x00056F40, //GFXMMU_LUT371H
  0x003B0001, //GFXMMU_LUT372L
  0x00057300, //GFXMMU_LUT372H
  0x003B0001, //GFXMMU_LUT373L
  0x000576C0, //GFXMMU_LUT373H
  0x003B0001, //GFXMMU_LUT374L
  0x00057A80, //GFXMMU_LUT374H
  0x003B0001, //GFXMMU_LUT375L
  0x00057E40, //GFXMMU_LUT375H
  0x003B0001, //GFXMMU_LUT376L
  0x00058200, //GFXMMU_LUT376H
  0x003B0001, //GFXMMU_LUT377L
  0x000585C0, //GFXMMU_LUT377H
  0x003B0001, //GFXMMU_LUT378L
  0x00058980, //GFXMMU_LUT378H
  0x003B0001, //GFXMMU_LUT379L
  0x00058D40, //GFXMMU_LUT379H
  0x003B0001, //GFXMMU_LUT380L
  0x00059100, //GFXMMU_LUT380H
  0x003B0001, //GFXMMU_LUT381L
  0x000594C0, //GFXMMU_LUT381H
  0x003B0001, //GFXMMU_LUT382L
  0x00059880, //GFXMMU_LUT382H
  0x003B0001, //GFXMMU_LUT383L
  0x00059C40, //GFXMMU_LUT383H
  0x003B0001, //GFXMMU_LUT384L
  0x0005A000, //GFXMMU_LUT384H
  0x003B0001, //GFXMMU_LUT385L
  0x0005A3C0, //GFXMMU_LUT385H
  0x003B0001, //GFXMMU_LUT386L
  0x0005A780, //GFXMMU_LUT386H
  0x003B0001, //GFXMMU_LUT387L
  0x0005AB40, //GFXMMU_LUT387H
  0x003B0001, //GFXMMU_LUT388L
  0x0005AF00, //GFXMMU_LUT388H
  0x003B0001, //GFXMMU_LUT389L
  0x0005B2C0, //GFXMMU_LUT389H
  0x003B0001, //GFXMMU_LUT390L
  0x0005B680, //GFXMMU_LUT390H
  0x003B0001, //GFXMMU_LUT391L
  0x0005BA40, //GFXMMU_LUT391H
  0x003B0001, //GFXMMU_LUT392L
  0x0005BE00, //GFXMMU_LUT392H
  0x003B0001, //GFXMMU_LUT393L
  0x0005C1C0, //GFXMMU_LUT393H
  0x003B0001, //GFXMMU_LUT394L
  0x0005C580, //GFXMMU_LUT394H
  0x003B0001, //GFXMMU_LUT395L
  0x0005C940, //GFXMMU_LUT395H
  0x003B0001, //GFXMMU_LUT396L
  0x0005CD00, //GFXMMU_LUT396H
  0x003B0001, //GFXMMU_LUT397L
  0x0005D0C0, //GFXMMU_LUT397H
  0x003B0001, //GFXMMU_LUT398L
  0x0005D480, //GFXMMU_LUT398H
  0x003B0001, //GFXMMU_LUT399L
  0x0005D840, //GFXMMU_LUT399H
  0x003B0001, //GFXMMU_LUT400L
  0x0005DC00, //GFXMMU_LUT400H
  0x003B0001, //GFXMMU_LUT401L
  0x0005DFC0, //GFXMMU_LUT401H
  0x003B0001, //GFXMMU_LUT402L
  0x0005E380, //GFXMMU_LUT402H
  0x003B0001, //GFXMMU_LUT403L
  0x0005E740, //GFXMMU_LUT403H
  0x003B0001, //GFXMMU_LUT404L
  0x0005EB00, //GFXMMU_LUT404H
  0x003B0001, //GFXMMU_LUT405L
  0x0005EEC0, //GFXMMU_LUT405H
  0x003B0001, //GFXMMU_LUT406L
  0x0005F280, //GFXMMU_LUT406H
  0x003B0001, //GFXMMU_LUT407L
  0x0005F640, //GFXMMU_LUT407H
  0x003B0001, //GFXMMU_LUT408L
  0x0005FA00, //GFXMMU_LUT408H
  0x003B0001, //GFXMMU_LUT409L
  0x0005FDC0, //GFXMMU_LUT409H
  0x003B0001, //GFXMMU_LUT410L
  0x00060180, //GFXMMU_LUT410H
  0x003B0001, //GFXMMU_LUT411L
  0x00060540, //GFXMMU_LUT411H
  0x003B0001, //GFXMMU_LUT412L
  0x00060900, //GFXMMU_LUT412H
  0x003B0001, //GFXMMU_LUT413L
  0x00060CC0, //GFXMMU_LUT413H
  0x003B0001, //GFXMMU_LUT414L
  0x00061080, //GFXMMU_LUT414H
  0x003B0001, //GFXMMU_LUT415L
  0x00061440, //GFXMMU_LUT415H
  0x003B0001, //GFXMMU_LUT416L
  0x00061800, //GFXMMU_LUT416H
  0x003B0001, //GFXMMU_LUT417L
  0x00061BC0, //GFXMMU_LUT417H
  0x003B0001, //GFXMMU_LUT418L
  0x00061F80, //GFXMMU_LUT418H
  0x003B0001, //GFXMMU_LUT419L
  0x00062340, //GFXMMU_LUT419H
  0x003B0001, //GFXMMU_LUT420L
  0x00062700, //GFXMMU_LUT420H
  0x003B0001, //GFXMMU_LUT421L
  0x00062AC0, //GFXMMU_LUT421H
  0x003B0001, //GFXMMU_LUT422L
  0x00062E80, //GFXMMU_LUT422H
  0x003B0001, //GFXMMU_LUT423L
  0x00063240, //GFXMMU_LUT423H
  0x003B0001, //GFXMMU_LUT424L
  0x00063600, //GFXMMU_LUT424H
  0x003B0001, //GFXMMU_LUT425L
  0x000639C0, //GFXMMU_LUT425H
  0x003B0001, //GFXMMU_LUT426L
  0x00063D80, //GFXMMU_LUT426H
  0x003B0001, //GFXMMU_LUT427L
  0x00064140, //GFXMMU_LUT427H
  0x003B0001, //GFXMMU_LUT428L
  0x00064500, //GFXMMU_LUT428H
  0x003B0001, //GFXMMU_LUT429L
  0x000648C0, //GFXMMU_LUT429H
  0x003B0001, //GFXMMU_LUT430L
  0x00064C80, //GFXMMU_LUT430H
  0x003B0001, //GFXMMU_LUT431L
  0x00065040, //GFXMMU_LUT431H
  0x003B0001, //GFXMMU_LUT432L
  0x00065400, //GFXMMU_LUT432H
  0x003B0001, //GFXMMU_LUT433L
  0x000657C0, //GFXMMU_LUT433H
  0x003B0001, //GFXMMU_LUT434L
  0x00065B80, //GFXMMU_LUT434H
  0x003B0001, //GFXMMU_LUT435L
  0x00065F40, //GFXMMU_LUT435H
  0x003B0001, //GFXMMU_LUT436L
  0x00066300, //GFXMMU_LUT436H
  0x003B0001, //GFXMMU_LUT437L
  0x000666C0, //GFXMMU_LUT437H
  0x003B0001, //GFXMMU_LUT438L
  0x00066A80, //GFXMMU_LUT438H
  0x003B0001, //GFXMMU_LUT439L
  0x00066E40, //GFXMMU_LUT439H
  0x003B0001, //GFXMMU_LUT440L
  0x00067200, //GFXMMU_LUT440H
  0x003B0001, //GFXMMU_LUT441L
  0x000675C0, //GFXMMU_LUT441H
  0x003B0001, //GFXMMU_LUT442L
  0x00067980, //GFXMMU_LUT442H
  0x003B0001, //GFXMMU_LUT443L
  0x00067D40, //GFXMMU_LUT443H
  0x003B0001, //GFXMMU_LUT444L
  0x00068100, //GFXMMU_LUT444H
  0x003B0001, //GFXMMU_LUT445L
  0x000684C0, //GFXMMU_LUT445H
  0x003B0001, //GFXMMU_LUT446L
  0x00068880, //GFXMMU_LUT446H
  0x003B0001, //GFXMMU_LUT447L
  0x00068C40, //GFXMMU_LUT447H
  0x003B0001, //GFXMMU_LUT448L
  0x00069000, //GFXMMU_LUT448H
  0x003B0001, //GFXMMU_LUT449L
  0x000693C0, //GFXMMU_LUT449H
  0x003B0001, //GFXMMU_LUT450L
  0x00069780, //GFXMMU_LUT450H
  0x003B0001, //GFXMMU_LUT451L
  0x00069B40, //GFXMMU_LUT451H
  0x003B0001, //GFXMMU_LUT452L
  0x00069F00, //GFXMMU_LUT452H
  0x003B0001, //GFXMMU_LUT453L
  0x0006A2C0, //GFXMMU_LUT453H
  0x003B0001, //GFXMMU_LUT454L
  0x0006A680, //GFXMMU_LUT454H
  0x003B0001, //GFXMMU_LUT455L
  0x0006AA40, //GFXMMU_LUT455H
  0x003B0001, //GFXMMU_LUT456L
  0x0006AE00, //GFXMMU_LUT456H
  0x003B0001, //GFXMMU_LUT457L
  0x0006B1C0, //GFXMMU_LUT457H
  0x003B0001, //GFXMMU_LUT458L
  0x0006B580, //GFXMMU_LUT458H
  0x003B0001, //GFXMMU_LUT459L
  0x0006B940, //GFXMMU_LUT459H
  0x003B0001, //GFXMMU_LUT460L
  0x0006BD00, //GFXMMU_LUT460H
  0x003B0001, //GFXMMU_LUT461L
  0x0006C0C0, //GFXMMU_LUT461H
  0x003B0001, //GFXMMU_LUT462L
  0x0006C480, //GFXMMU_LUT462H
  0x003B0001, //GFXMMU_LUT463L
  0x0006C840, //GFXMMU_LUT463H
  0x003B0001, //GFXMMU_LUT464L
  0x0006CC00, //GFXMMU_LUT464H
  0x003B0001, //GFXMMU_LUT465L
  0x0006CFC0, //GFXMMU_LUT465H
  0x003B0001, //GFXMMU_LUT466L
  0x0006D380, //GFXMMU_LUT466H
  0x003B0001, //GFXMMU_LUT467L
  0x0006D740, //GFXMMU_LUT467H
  0x003B0001, //GFXMMU_LUT468L
  0x0006DB00, //GFXMMU_LUT468H
  0x003B0001, //GFXMMU_LUT469L
  0x0006DEC0, //GFXMMU_LUT469H
  0x003B0001, //GFXMMU_LUT470L
  0x0006E280, //GFXMMU_LUT470H
  0x003B0001, //GFXMMU_LUT471L
  0x0006E640, //GFXMMU_LUT471H
  0x003B0001, //GFXMMU_LUT472L
  0x0006EA00, //GFXMMU_LUT472H
  0x003B0001, //GFXMMU_LUT473L
  0x0006EDC0, //GFXMMU_LUT473H
  0x003B0001, //GFXMMU_LUT474L
  0x0006F180, //GFXMMU_LUT474H
  0x003B0001, //GFXMMU_LUT475L
  0x0006F540, //GFXMMU_LUT475H
  0x003B0001, //GFXMMU_LUT476L
  0x0006F900, //GFXMMU_LUT476H
  0x003B0001, //GFXMMU_LUT477L
  0x0006FCC0, //GFXMMU_LUT477H
  0x003B0001, //GFXMMU_LUT478L
  0x00070080, //GFXMMU_LUT478H
  0x003B0001, //GFXMMU_LUT479L
  0x00070440 //GFXMMU_LUT479H
};

#ifdef __cplusplus
}
#endif
#endif /*__ gfxmmu_lut_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
