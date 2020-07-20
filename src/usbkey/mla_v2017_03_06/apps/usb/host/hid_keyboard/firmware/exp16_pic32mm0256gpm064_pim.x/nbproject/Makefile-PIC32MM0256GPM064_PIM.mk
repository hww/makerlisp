#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Include project Makefile
ifeq "${IGNORE_LOCAL}" "TRUE"
# do not include local makefile. User is passing all local related variables already
else
include Makefile
# Include makefile containing local settings
ifeq "$(wildcard nbproject/Makefile-local-PIC32MM0256GPM064_PIM.mk)" "nbproject/Makefile-local-PIC32MM0256GPM064_PIM.mk"
include nbproject/Makefile-local-PIC32MM0256GPM064_PIM.mk
endif
endif

# Environment
MKDIR=gnumkdir -p
RM=rm -f 
MV=mv 
CP=cp 

# Macros
CND_CONF=PIC32MM0256GPM064_PIM
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IMAGE_TYPE=debug
OUTPUT_SUFFIX=elf
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/exp16_pic32mm0256gpm064_pim.x.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/exp16_pic32mm0256gpm064_pim.x.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
endif

ifeq ($(COMPARE_BUILD), true)
COMPARISON_BUILD=-mafrlcsj
else
COMPARISON_BUILD=
endif

ifdef SUB_IMAGE_ADDRESS

else
SUB_IMAGE_ADDRESS_COMMAND=
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Source Files Quoted if spaced
SOURCEFILES_QUOTED_IF_SPACED=../../../../../../bsp/exp16/pic32mm0256gpm064_pim/lcd.c ../../../../../../bsp/exp16/pic32mm0256gpm064_pim/leds.c ../../../../../../bsp/exp16/pic32mm0256gpm064_pim/timer_1ms.c ../../../../../../bsp/exp16/pic32mm0256gpm064_pim/uart1.c ../../../../../../bsp/exp16/pic32mm0256gpm064_pim/fwupd.c ../../../../../../framework/usb/src/usb_host.c ../../../../../../framework/usb/src/usb_host_hid.c ../../../../../../framework/usb/src/usb_host_hid_parser.c ../demo_src/usb_config.c ../demo_src/app_host_hid_keyboard.c ../demo_src/main.c system.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/_ext/70818574/lcd.o ${OBJECTDIR}/_ext/70818574/leds.o ${OBJECTDIR}/_ext/70818574/timer_1ms.o ${OBJECTDIR}/_ext/70818574/uart1.o ${OBJECTDIR}/_ext/70818574/fwupd.o ${OBJECTDIR}/_ext/838585624/usb_host.o ${OBJECTDIR}/_ext/838585624/usb_host_hid.o ${OBJECTDIR}/_ext/838585624/usb_host_hid_parser.o ${OBJECTDIR}/_ext/300881143/usb_config.o ${OBJECTDIR}/_ext/300881143/app_host_hid_keyboard.o ${OBJECTDIR}/_ext/300881143/main.o ${OBJECTDIR}/system.o
POSSIBLE_DEPFILES=${OBJECTDIR}/_ext/70818574/lcd.o.d ${OBJECTDIR}/_ext/70818574/leds.o.d ${OBJECTDIR}/_ext/70818574/timer_1ms.o.d ${OBJECTDIR}/_ext/70818574/uart1.o.d ${OBJECTDIR}/_ext/70818574/fwupd.o.d ${OBJECTDIR}/_ext/838585624/usb_host.o.d ${OBJECTDIR}/_ext/838585624/usb_host_hid.o.d ${OBJECTDIR}/_ext/838585624/usb_host_hid_parser.o.d ${OBJECTDIR}/_ext/300881143/usb_config.o.d ${OBJECTDIR}/_ext/300881143/app_host_hid_keyboard.o.d ${OBJECTDIR}/_ext/300881143/main.o.d ${OBJECTDIR}/system.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/_ext/70818574/lcd.o ${OBJECTDIR}/_ext/70818574/leds.o ${OBJECTDIR}/_ext/70818574/timer_1ms.o ${OBJECTDIR}/_ext/70818574/uart1.o ${OBJECTDIR}/_ext/70818574/fwupd.o ${OBJECTDIR}/_ext/838585624/usb_host.o ${OBJECTDIR}/_ext/838585624/usb_host_hid.o ${OBJECTDIR}/_ext/838585624/usb_host_hid_parser.o ${OBJECTDIR}/_ext/300881143/usb_config.o ${OBJECTDIR}/_ext/300881143/app_host_hid_keyboard.o ${OBJECTDIR}/_ext/300881143/main.o ${OBJECTDIR}/system.o

# Source Files
SOURCEFILES=../../../../../../bsp/exp16/pic32mm0256gpm064_pim/lcd.c ../../../../../../bsp/exp16/pic32mm0256gpm064_pim/leds.c ../../../../../../bsp/exp16/pic32mm0256gpm064_pim/timer_1ms.c ../../../../../../bsp/exp16/pic32mm0256gpm064_pim/uart1.c ../../../../../../bsp/exp16/pic32mm0256gpm064_pim/fwupd.c ../../../../../../framework/usb/src/usb_host.c ../../../../../../framework/usb/src/usb_host_hid.c ../../../../../../framework/usb/src/usb_host_hid_parser.c ../demo_src/usb_config.c ../demo_src/app_host_hid_keyboard.c ../demo_src/main.c system.c


CFLAGS=
ASFLAGS=
LDLIBSOPTIONS=

############# Tool locations ##########################################
# If you copy a project from one host to another, the path where the  #
# compiler is installed may be different.                             #
# If you open this project with MPLAB X in the new host, this         #
# makefile will be regenerated and the paths will be corrected.       #
#######################################################################
# fixDeps replaces a bunch of sed/cat/printf statements that slow down the build
FIXDEPS=fixDeps

.build-conf:  ${BUILD_SUBPROJECTS}
ifneq ($(INFORMATION_MESSAGE), )
	@echo $(INFORMATION_MESSAGE)
endif
	${MAKE}  -f nbproject/Makefile-PIC32MM0256GPM064_PIM.mk dist/${CND_CONF}/${IMAGE_TYPE}/exp16_pic32mm0256gpm064_pim.x.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=32MM0256GPM064
MP_LINKER_FILE_OPTION=
# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assembleWithPreprocess
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/_ext/70818574/lcd.o: ../../../../../../bsp/exp16/pic32mm0256gpm064_pim/lcd.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/70818574" 
	@${RM} ${OBJECTDIR}/_ext/70818574/lcd.o.d 
	@${RM} ${OBJECTDIR}/_ext/70818574/lcd.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/70818574/lcd.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -DPKOBSKDEPlatformTool=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYSTEM_PERIPHERAL_CLOCK=24000000 -I"." -I"../demo_src" -I"../../../../../../framework/usb/inc" -I"../../../../../../bsp/exp16/pic32mm0256gpm064_pim" -MMD -MF "${OBJECTDIR}/_ext/70818574/lcd.o.d" -o ${OBJECTDIR}/_ext/70818574/lcd.o ../../../../../../bsp/exp16/pic32mm0256gpm064_pim/lcd.c    -DXPRJ_PIC32MM0256GPM064_PIM=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/70818574/leds.o: ../../../../../../bsp/exp16/pic32mm0256gpm064_pim/leds.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/70818574" 
	@${RM} ${OBJECTDIR}/_ext/70818574/leds.o.d 
	@${RM} ${OBJECTDIR}/_ext/70818574/leds.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/70818574/leds.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -DPKOBSKDEPlatformTool=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYSTEM_PERIPHERAL_CLOCK=24000000 -I"." -I"../demo_src" -I"../../../../../../framework/usb/inc" -I"../../../../../../bsp/exp16/pic32mm0256gpm064_pim" -MMD -MF "${OBJECTDIR}/_ext/70818574/leds.o.d" -o ${OBJECTDIR}/_ext/70818574/leds.o ../../../../../../bsp/exp16/pic32mm0256gpm064_pim/leds.c    -DXPRJ_PIC32MM0256GPM064_PIM=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/70818574/timer_1ms.o: ../../../../../../bsp/exp16/pic32mm0256gpm064_pim/timer_1ms.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/70818574" 
	@${RM} ${OBJECTDIR}/_ext/70818574/timer_1ms.o.d 
	@${RM} ${OBJECTDIR}/_ext/70818574/timer_1ms.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/70818574/timer_1ms.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -DPKOBSKDEPlatformTool=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYSTEM_PERIPHERAL_CLOCK=24000000 -I"." -I"../demo_src" -I"../../../../../../framework/usb/inc" -I"../../../../../../bsp/exp16/pic32mm0256gpm064_pim" -MMD -MF "${OBJECTDIR}/_ext/70818574/timer_1ms.o.d" -o ${OBJECTDIR}/_ext/70818574/timer_1ms.o ../../../../../../bsp/exp16/pic32mm0256gpm064_pim/timer_1ms.c    -DXPRJ_PIC32MM0256GPM064_PIM=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/70818574/uart1.o: ../../../../../../bsp/exp16/pic32mm0256gpm064_pim/uart1.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/70818574" 
	@${RM} ${OBJECTDIR}/_ext/70818574/uart1.o.d 
	@${RM} ${OBJECTDIR}/_ext/70818574/uart1.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/70818574/uart1.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -DPKOBSKDEPlatformTool=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYSTEM_PERIPHERAL_CLOCK=24000000 -I"." -I"../demo_src" -I"../../../../../../framework/usb/inc" -I"../../../../../../bsp/exp16/pic32mm0256gpm064_pim" -MMD -MF "${OBJECTDIR}/_ext/70818574/uart1.o.d" -o ${OBJECTDIR}/_ext/70818574/uart1.o ../../../../../../bsp/exp16/pic32mm0256gpm064_pim/uart1.c    -DXPRJ_PIC32MM0256GPM064_PIM=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/70818574/fwupd.o: ../../../../../../bsp/exp16/pic32mm0256gpm064_pim/fwupd.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/70818574" 
	@${RM} ${OBJECTDIR}/_ext/70818574/fwupd.o.d 
	@${RM} ${OBJECTDIR}/_ext/70818574/fwupd.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/70818574/fwupd.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -DPKOBSKDEPlatformTool=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYSTEM_PERIPHERAL_CLOCK=24000000 -I"." -I"../demo_src" -I"../../../../../../framework/usb/inc" -I"../../../../../../bsp/exp16/pic32mm0256gpm064_pim" -MMD -MF "${OBJECTDIR}/_ext/70818574/fwupd.o.d" -o ${OBJECTDIR}/_ext/70818574/fwupd.o ../../../../../../bsp/exp16/pic32mm0256gpm064_pim/fwupd.c    -DXPRJ_PIC32MM0256GPM064_PIM=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/838585624/usb_host.o: ../../../../../../framework/usb/src/usb_host.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/838585624" 
	@${RM} ${OBJECTDIR}/_ext/838585624/usb_host.o.d 
	@${RM} ${OBJECTDIR}/_ext/838585624/usb_host.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/838585624/usb_host.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -DPKOBSKDEPlatformTool=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYSTEM_PERIPHERAL_CLOCK=24000000 -I"." -I"../demo_src" -I"../../../../../../framework/usb/inc" -I"../../../../../../bsp/exp16/pic32mm0256gpm064_pim" -MMD -MF "${OBJECTDIR}/_ext/838585624/usb_host.o.d" -o ${OBJECTDIR}/_ext/838585624/usb_host.o ../../../../../../framework/usb/src/usb_host.c    -DXPRJ_PIC32MM0256GPM064_PIM=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/838585624/usb_host_hid.o: ../../../../../../framework/usb/src/usb_host_hid.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/838585624" 
	@${RM} ${OBJECTDIR}/_ext/838585624/usb_host_hid.o.d 
	@${RM} ${OBJECTDIR}/_ext/838585624/usb_host_hid.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/838585624/usb_host_hid.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -DPKOBSKDEPlatformTool=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYSTEM_PERIPHERAL_CLOCK=24000000 -I"." -I"../demo_src" -I"../../../../../../framework/usb/inc" -I"../../../../../../bsp/exp16/pic32mm0256gpm064_pim" -MMD -MF "${OBJECTDIR}/_ext/838585624/usb_host_hid.o.d" -o ${OBJECTDIR}/_ext/838585624/usb_host_hid.o ../../../../../../framework/usb/src/usb_host_hid.c    -DXPRJ_PIC32MM0256GPM064_PIM=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/838585624/usb_host_hid_parser.o: ../../../../../../framework/usb/src/usb_host_hid_parser.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/838585624" 
	@${RM} ${OBJECTDIR}/_ext/838585624/usb_host_hid_parser.o.d 
	@${RM} ${OBJECTDIR}/_ext/838585624/usb_host_hid_parser.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/838585624/usb_host_hid_parser.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -DPKOBSKDEPlatformTool=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYSTEM_PERIPHERAL_CLOCK=24000000 -I"." -I"../demo_src" -I"../../../../../../framework/usb/inc" -I"../../../../../../bsp/exp16/pic32mm0256gpm064_pim" -MMD -MF "${OBJECTDIR}/_ext/838585624/usb_host_hid_parser.o.d" -o ${OBJECTDIR}/_ext/838585624/usb_host_hid_parser.o ../../../../../../framework/usb/src/usb_host_hid_parser.c    -DXPRJ_PIC32MM0256GPM064_PIM=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/300881143/usb_config.o: ../demo_src/usb_config.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/300881143" 
	@${RM} ${OBJECTDIR}/_ext/300881143/usb_config.o.d 
	@${RM} ${OBJECTDIR}/_ext/300881143/usb_config.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/300881143/usb_config.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -DPKOBSKDEPlatformTool=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYSTEM_PERIPHERAL_CLOCK=24000000 -I"." -I"../demo_src" -I"../../../../../../framework/usb/inc" -I"../../../../../../bsp/exp16/pic32mm0256gpm064_pim" -MMD -MF "${OBJECTDIR}/_ext/300881143/usb_config.o.d" -o ${OBJECTDIR}/_ext/300881143/usb_config.o ../demo_src/usb_config.c    -DXPRJ_PIC32MM0256GPM064_PIM=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/300881143/app_host_hid_keyboard.o: ../demo_src/app_host_hid_keyboard.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/300881143" 
	@${RM} ${OBJECTDIR}/_ext/300881143/app_host_hid_keyboard.o.d 
	@${RM} ${OBJECTDIR}/_ext/300881143/app_host_hid_keyboard.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/300881143/app_host_hid_keyboard.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -DPKOBSKDEPlatformTool=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYSTEM_PERIPHERAL_CLOCK=24000000 -I"." -I"../demo_src" -I"../../../../../../framework/usb/inc" -I"../../../../../../bsp/exp16/pic32mm0256gpm064_pim" -MMD -MF "${OBJECTDIR}/_ext/300881143/app_host_hid_keyboard.o.d" -o ${OBJECTDIR}/_ext/300881143/app_host_hid_keyboard.o ../demo_src/app_host_hid_keyboard.c    -DXPRJ_PIC32MM0256GPM064_PIM=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/300881143/main.o: ../demo_src/main.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/300881143" 
	@${RM} ${OBJECTDIR}/_ext/300881143/main.o.d 
	@${RM} ${OBJECTDIR}/_ext/300881143/main.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/300881143/main.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -DPKOBSKDEPlatformTool=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYSTEM_PERIPHERAL_CLOCK=24000000 -I"." -I"../demo_src" -I"../../../../../../framework/usb/inc" -I"../../../../../../bsp/exp16/pic32mm0256gpm064_pim" -MMD -MF "${OBJECTDIR}/_ext/300881143/main.o.d" -o ${OBJECTDIR}/_ext/300881143/main.o ../demo_src/main.c    -DXPRJ_PIC32MM0256GPM064_PIM=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/system.o: system.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/system.o.d 
	@${RM} ${OBJECTDIR}/system.o 
	@${FIXDEPS} "${OBJECTDIR}/system.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -DPKOBSKDEPlatformTool=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYSTEM_PERIPHERAL_CLOCK=24000000 -I"." -I"../demo_src" -I"../../../../../../framework/usb/inc" -I"../../../../../../bsp/exp16/pic32mm0256gpm064_pim" -MMD -MF "${OBJECTDIR}/system.o.d" -o ${OBJECTDIR}/system.o system.c    -DXPRJ_PIC32MM0256GPM064_PIM=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD) 
	
else
${OBJECTDIR}/_ext/70818574/lcd.o: ../../../../../../bsp/exp16/pic32mm0256gpm064_pim/lcd.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/70818574" 
	@${RM} ${OBJECTDIR}/_ext/70818574/lcd.o.d 
	@${RM} ${OBJECTDIR}/_ext/70818574/lcd.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/70818574/lcd.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYSTEM_PERIPHERAL_CLOCK=24000000 -I"." -I"../demo_src" -I"../../../../../../framework/usb/inc" -I"../../../../../../bsp/exp16/pic32mm0256gpm064_pim" -MMD -MF "${OBJECTDIR}/_ext/70818574/lcd.o.d" -o ${OBJECTDIR}/_ext/70818574/lcd.o ../../../../../../bsp/exp16/pic32mm0256gpm064_pim/lcd.c    -DXPRJ_PIC32MM0256GPM064_PIM=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/70818574/leds.o: ../../../../../../bsp/exp16/pic32mm0256gpm064_pim/leds.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/70818574" 
	@${RM} ${OBJECTDIR}/_ext/70818574/leds.o.d 
	@${RM} ${OBJECTDIR}/_ext/70818574/leds.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/70818574/leds.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYSTEM_PERIPHERAL_CLOCK=24000000 -I"." -I"../demo_src" -I"../../../../../../framework/usb/inc" -I"../../../../../../bsp/exp16/pic32mm0256gpm064_pim" -MMD -MF "${OBJECTDIR}/_ext/70818574/leds.o.d" -o ${OBJECTDIR}/_ext/70818574/leds.o ../../../../../../bsp/exp16/pic32mm0256gpm064_pim/leds.c    -DXPRJ_PIC32MM0256GPM064_PIM=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/70818574/timer_1ms.o: ../../../../../../bsp/exp16/pic32mm0256gpm064_pim/timer_1ms.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/70818574" 
	@${RM} ${OBJECTDIR}/_ext/70818574/timer_1ms.o.d 
	@${RM} ${OBJECTDIR}/_ext/70818574/timer_1ms.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/70818574/timer_1ms.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYSTEM_PERIPHERAL_CLOCK=24000000 -I"." -I"../demo_src" -I"../../../../../../framework/usb/inc" -I"../../../../../../bsp/exp16/pic32mm0256gpm064_pim" -MMD -MF "${OBJECTDIR}/_ext/70818574/timer_1ms.o.d" -o ${OBJECTDIR}/_ext/70818574/timer_1ms.o ../../../../../../bsp/exp16/pic32mm0256gpm064_pim/timer_1ms.c    -DXPRJ_PIC32MM0256GPM064_PIM=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/70818574/uart1.o: ../../../../../../bsp/exp16/pic32mm0256gpm064_pim/uart1.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/70818574" 
	@${RM} ${OBJECTDIR}/_ext/70818574/uart1.o.d 
	@${RM} ${OBJECTDIR}/_ext/70818574/uart1.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/70818574/uart1.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYSTEM_PERIPHERAL_CLOCK=24000000 -I"." -I"../demo_src" -I"../../../../../../framework/usb/inc" -I"../../../../../../bsp/exp16/pic32mm0256gpm064_pim" -MMD -MF "${OBJECTDIR}/_ext/70818574/uart1.o.d" -o ${OBJECTDIR}/_ext/70818574/uart1.o ../../../../../../bsp/exp16/pic32mm0256gpm064_pim/uart1.c    -DXPRJ_PIC32MM0256GPM064_PIM=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/70818574/fwupd.o: ../../../../../../bsp/exp16/pic32mm0256gpm064_pim/fwupd.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/70818574" 
	@${RM} ${OBJECTDIR}/_ext/70818574/fwupd.o.d 
	@${RM} ${OBJECTDIR}/_ext/70818574/fwupd.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/70818574/fwupd.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYSTEM_PERIPHERAL_CLOCK=24000000 -I"." -I"../demo_src" -I"../../../../../../framework/usb/inc" -I"../../../../../../bsp/exp16/pic32mm0256gpm064_pim" -MMD -MF "${OBJECTDIR}/_ext/70818574/fwupd.o.d" -o ${OBJECTDIR}/_ext/70818574/fwupd.o ../../../../../../bsp/exp16/pic32mm0256gpm064_pim/fwupd.c    -DXPRJ_PIC32MM0256GPM064_PIM=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/838585624/usb_host.o: ../../../../../../framework/usb/src/usb_host.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/838585624" 
	@${RM} ${OBJECTDIR}/_ext/838585624/usb_host.o.d 
	@${RM} ${OBJECTDIR}/_ext/838585624/usb_host.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/838585624/usb_host.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYSTEM_PERIPHERAL_CLOCK=24000000 -I"." -I"../demo_src" -I"../../../../../../framework/usb/inc" -I"../../../../../../bsp/exp16/pic32mm0256gpm064_pim" -MMD -MF "${OBJECTDIR}/_ext/838585624/usb_host.o.d" -o ${OBJECTDIR}/_ext/838585624/usb_host.o ../../../../../../framework/usb/src/usb_host.c    -DXPRJ_PIC32MM0256GPM064_PIM=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/838585624/usb_host_hid.o: ../../../../../../framework/usb/src/usb_host_hid.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/838585624" 
	@${RM} ${OBJECTDIR}/_ext/838585624/usb_host_hid.o.d 
	@${RM} ${OBJECTDIR}/_ext/838585624/usb_host_hid.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/838585624/usb_host_hid.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYSTEM_PERIPHERAL_CLOCK=24000000 -I"." -I"../demo_src" -I"../../../../../../framework/usb/inc" -I"../../../../../../bsp/exp16/pic32mm0256gpm064_pim" -MMD -MF "${OBJECTDIR}/_ext/838585624/usb_host_hid.o.d" -o ${OBJECTDIR}/_ext/838585624/usb_host_hid.o ../../../../../../framework/usb/src/usb_host_hid.c    -DXPRJ_PIC32MM0256GPM064_PIM=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/838585624/usb_host_hid_parser.o: ../../../../../../framework/usb/src/usb_host_hid_parser.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/838585624" 
	@${RM} ${OBJECTDIR}/_ext/838585624/usb_host_hid_parser.o.d 
	@${RM} ${OBJECTDIR}/_ext/838585624/usb_host_hid_parser.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/838585624/usb_host_hid_parser.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYSTEM_PERIPHERAL_CLOCK=24000000 -I"." -I"../demo_src" -I"../../../../../../framework/usb/inc" -I"../../../../../../bsp/exp16/pic32mm0256gpm064_pim" -MMD -MF "${OBJECTDIR}/_ext/838585624/usb_host_hid_parser.o.d" -o ${OBJECTDIR}/_ext/838585624/usb_host_hid_parser.o ../../../../../../framework/usb/src/usb_host_hid_parser.c    -DXPRJ_PIC32MM0256GPM064_PIM=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/300881143/usb_config.o: ../demo_src/usb_config.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/300881143" 
	@${RM} ${OBJECTDIR}/_ext/300881143/usb_config.o.d 
	@${RM} ${OBJECTDIR}/_ext/300881143/usb_config.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/300881143/usb_config.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYSTEM_PERIPHERAL_CLOCK=24000000 -I"." -I"../demo_src" -I"../../../../../../framework/usb/inc" -I"../../../../../../bsp/exp16/pic32mm0256gpm064_pim" -MMD -MF "${OBJECTDIR}/_ext/300881143/usb_config.o.d" -o ${OBJECTDIR}/_ext/300881143/usb_config.o ../demo_src/usb_config.c    -DXPRJ_PIC32MM0256GPM064_PIM=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/300881143/app_host_hid_keyboard.o: ../demo_src/app_host_hid_keyboard.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/300881143" 
	@${RM} ${OBJECTDIR}/_ext/300881143/app_host_hid_keyboard.o.d 
	@${RM} ${OBJECTDIR}/_ext/300881143/app_host_hid_keyboard.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/300881143/app_host_hid_keyboard.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYSTEM_PERIPHERAL_CLOCK=24000000 -I"." -I"../demo_src" -I"../../../../../../framework/usb/inc" -I"../../../../../../bsp/exp16/pic32mm0256gpm064_pim" -MMD -MF "${OBJECTDIR}/_ext/300881143/app_host_hid_keyboard.o.d" -o ${OBJECTDIR}/_ext/300881143/app_host_hid_keyboard.o ../demo_src/app_host_hid_keyboard.c    -DXPRJ_PIC32MM0256GPM064_PIM=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/300881143/main.o: ../demo_src/main.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/300881143" 
	@${RM} ${OBJECTDIR}/_ext/300881143/main.o.d 
	@${RM} ${OBJECTDIR}/_ext/300881143/main.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/300881143/main.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYSTEM_PERIPHERAL_CLOCK=24000000 -I"." -I"../demo_src" -I"../../../../../../framework/usb/inc" -I"../../../../../../bsp/exp16/pic32mm0256gpm064_pim" -MMD -MF "${OBJECTDIR}/_ext/300881143/main.o.d" -o ${OBJECTDIR}/_ext/300881143/main.o ../demo_src/main.c    -DXPRJ_PIC32MM0256GPM064_PIM=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/system.o: system.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/system.o.d 
	@${RM} ${OBJECTDIR}/system.o 
	@${FIXDEPS} "${OBJECTDIR}/system.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYSTEM_PERIPHERAL_CLOCK=24000000 -I"." -I"../demo_src" -I"../../../../../../framework/usb/inc" -I"../../../../../../bsp/exp16/pic32mm0256gpm064_pim" -MMD -MF "${OBJECTDIR}/system.o.d" -o ${OBJECTDIR}/system.o system.c    -DXPRJ_PIC32MM0256GPM064_PIM=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD) 
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: compileCPP
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
dist/${CND_CONF}/${IMAGE_TYPE}/exp16_pic32mm0256gpm064_pim.x.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE) -g -mdebugger -DPKOBSKDEPlatformTool=1 -mprocessor=$(MP_PROCESSOR_OPTION)  -o dist/${CND_CONF}/${IMAGE_TYPE}/exp16_pic32mm0256gpm064_pim.x.${IMAGE_TYPE}.${OUTPUT_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}          -DXPRJ_PIC32MM0256GPM064_PIM=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)   -mreserve=data@0x0:0x1FC -mreserve=boot@0x1FC00490:0x1FC016FF -mreserve=boot@0x1FC00490:0x1FC00BEF  -Wl,--defsym=__MPLAB_BUILD=1$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION),--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,-D=__DEBUG_D,--defsym=PKOBSKDEPlatformTool=1,--defsym=_min_heap_size=3000,--no-code-in-dinit,--no-dinit-in-serial-mem,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--memorysummary,dist/${CND_CONF}/${IMAGE_TYPE}/memoryfile.xml
	
else
dist/${CND_CONF}/${IMAGE_TYPE}/exp16_pic32mm0256gpm064_pim.x.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -mprocessor=$(MP_PROCESSOR_OPTION)  -o dist/${CND_CONF}/${IMAGE_TYPE}/exp16_pic32mm0256gpm064_pim.x.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}          -DXPRJ_PIC32MM0256GPM064_PIM=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -Wl,--defsym=__MPLAB_BUILD=1$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION),--defsym=_min_heap_size=3000,--no-code-in-dinit,--no-dinit-in-serial-mem,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--memorysummary,dist/${CND_CONF}/${IMAGE_TYPE}/memoryfile.xml
	${MP_CC_DIR}\\xc32-bin2hex dist/${CND_CONF}/${IMAGE_TYPE}/exp16_pic32mm0256gpm064_pim.x.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} 
endif


# Subprojects
.build-subprojects:


# Subprojects
.clean-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/PIC32MM0256GPM064_PIM
	${RM} -r dist/PIC32MM0256GPM064_PIM

# Enable dependency checking
.dep.inc: .depcheck-impl

DEPFILES=$(shell mplabwildcard ${POSSIBLE_DEPFILES})
ifneq (${DEPFILES},)
include ${DEPFILES}
endif
