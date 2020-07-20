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
ifeq "$(wildcard nbproject/Makefile-local-MLM_USB_Host.mk)" "nbproject/Makefile-local-MLM_USB_Host.mk"
include nbproject/Makefile-local-MLM_USB_Host.mk
endif
endif

# Environment
MKDIR=gnumkdir -p
RM=rm -f 
MV=mv 
CP=cp 

# Macros
CND_CONF=MLM_USB_Host
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IMAGE_TYPE=debug
OUTPUT_SUFFIX=elf
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/mlm_usb_host.x.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/mlm_usb_host.x.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
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
SOURCEFILES_QUOTED_IF_SPACED=../../../../../../bsp/mlm_usb_host/lcd.c ../../../../../../bsp/mlm_usb_host/leds.c ../../../../../../bsp/mlm_usb_host/timer_1ms.c ../../../../../../bsp/mlm_usb_host/uart1.c ../../../../../../bsp/mlm_usb_host/fwupd.c ../../../../../../framework/usb/src/usb_host.c ../../../../../../framework/usb/src/usb_host_hid.c ../../../../../../framework/usb/src/usb_host_hid_parser.c ../demo_src/usb_config.c ../demo_src/app_host_hid_keyboard.c ../demo_src/main.c system.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/_ext/158577828/lcd.o ${OBJECTDIR}/_ext/158577828/leds.o ${OBJECTDIR}/_ext/158577828/timer_1ms.o ${OBJECTDIR}/_ext/158577828/uart1.o ${OBJECTDIR}/_ext/158577828/fwupd.o ${OBJECTDIR}/_ext/838585624/usb_host.o ${OBJECTDIR}/_ext/838585624/usb_host_hid.o ${OBJECTDIR}/_ext/838585624/usb_host_hid_parser.o ${OBJECTDIR}/_ext/300881143/usb_config.o ${OBJECTDIR}/_ext/300881143/app_host_hid_keyboard.o ${OBJECTDIR}/_ext/300881143/main.o ${OBJECTDIR}/system.o
POSSIBLE_DEPFILES=${OBJECTDIR}/_ext/158577828/lcd.o.d ${OBJECTDIR}/_ext/158577828/leds.o.d ${OBJECTDIR}/_ext/158577828/timer_1ms.o.d ${OBJECTDIR}/_ext/158577828/uart1.o.d ${OBJECTDIR}/_ext/158577828/fwupd.o.d ${OBJECTDIR}/_ext/838585624/usb_host.o.d ${OBJECTDIR}/_ext/838585624/usb_host_hid.o.d ${OBJECTDIR}/_ext/838585624/usb_host_hid_parser.o.d ${OBJECTDIR}/_ext/300881143/usb_config.o.d ${OBJECTDIR}/_ext/300881143/app_host_hid_keyboard.o.d ${OBJECTDIR}/_ext/300881143/main.o.d ${OBJECTDIR}/system.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/_ext/158577828/lcd.o ${OBJECTDIR}/_ext/158577828/leds.o ${OBJECTDIR}/_ext/158577828/timer_1ms.o ${OBJECTDIR}/_ext/158577828/uart1.o ${OBJECTDIR}/_ext/158577828/fwupd.o ${OBJECTDIR}/_ext/838585624/usb_host.o ${OBJECTDIR}/_ext/838585624/usb_host_hid.o ${OBJECTDIR}/_ext/838585624/usb_host_hid_parser.o ${OBJECTDIR}/_ext/300881143/usb_config.o ${OBJECTDIR}/_ext/300881143/app_host_hid_keyboard.o ${OBJECTDIR}/_ext/300881143/main.o ${OBJECTDIR}/system.o

# Source Files
SOURCEFILES=../../../../../../bsp/mlm_usb_host/lcd.c ../../../../../../bsp/mlm_usb_host/leds.c ../../../../../../bsp/mlm_usb_host/timer_1ms.c ../../../../../../bsp/mlm_usb_host/uart1.c ../../../../../../bsp/mlm_usb_host/fwupd.c ../../../../../../framework/usb/src/usb_host.c ../../../../../../framework/usb/src/usb_host_hid.c ../../../../../../framework/usb/src/usb_host_hid_parser.c ../demo_src/usb_config.c ../demo_src/app_host_hid_keyboard.c ../demo_src/main.c system.c


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
	${MAKE}  -f nbproject/Makefile-MLM_USB_Host.mk dist/${CND_CONF}/${IMAGE_TYPE}/mlm_usb_host.x.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=32MM0256GPM028
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
${OBJECTDIR}/_ext/158577828/lcd.o: ../../../../../../bsp/mlm_usb_host/lcd.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/158577828" 
	@${RM} ${OBJECTDIR}/_ext/158577828/lcd.o.d 
	@${RM} ${OBJECTDIR}/_ext/158577828/lcd.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/158577828/lcd.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -DPICkit3PlatformTool=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYSTEM_PERIPHERAL_CLOCK=24000000 -I"." -I"../demo_src" -I"../../../../../../framework/usb/inc" -I"../../../../../../bsp/mlm_usb_host" -MMD -MF "${OBJECTDIR}/_ext/158577828/lcd.o.d" -o ${OBJECTDIR}/_ext/158577828/lcd.o ../../../../../../bsp/mlm_usb_host/lcd.c    -DXPRJ_MLM_USB_Host=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/158577828/leds.o: ../../../../../../bsp/mlm_usb_host/leds.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/158577828" 
	@${RM} ${OBJECTDIR}/_ext/158577828/leds.o.d 
	@${RM} ${OBJECTDIR}/_ext/158577828/leds.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/158577828/leds.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -DPICkit3PlatformTool=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYSTEM_PERIPHERAL_CLOCK=24000000 -I"." -I"../demo_src" -I"../../../../../../framework/usb/inc" -I"../../../../../../bsp/mlm_usb_host" -MMD -MF "${OBJECTDIR}/_ext/158577828/leds.o.d" -o ${OBJECTDIR}/_ext/158577828/leds.o ../../../../../../bsp/mlm_usb_host/leds.c    -DXPRJ_MLM_USB_Host=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/158577828/timer_1ms.o: ../../../../../../bsp/mlm_usb_host/timer_1ms.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/158577828" 
	@${RM} ${OBJECTDIR}/_ext/158577828/timer_1ms.o.d 
	@${RM} ${OBJECTDIR}/_ext/158577828/timer_1ms.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/158577828/timer_1ms.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -DPICkit3PlatformTool=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYSTEM_PERIPHERAL_CLOCK=24000000 -I"." -I"../demo_src" -I"../../../../../../framework/usb/inc" -I"../../../../../../bsp/mlm_usb_host" -MMD -MF "${OBJECTDIR}/_ext/158577828/timer_1ms.o.d" -o ${OBJECTDIR}/_ext/158577828/timer_1ms.o ../../../../../../bsp/mlm_usb_host/timer_1ms.c    -DXPRJ_MLM_USB_Host=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/158577828/uart1.o: ../../../../../../bsp/mlm_usb_host/uart1.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/158577828" 
	@${RM} ${OBJECTDIR}/_ext/158577828/uart1.o.d 
	@${RM} ${OBJECTDIR}/_ext/158577828/uart1.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/158577828/uart1.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -DPICkit3PlatformTool=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYSTEM_PERIPHERAL_CLOCK=24000000 -I"." -I"../demo_src" -I"../../../../../../framework/usb/inc" -I"../../../../../../bsp/mlm_usb_host" -MMD -MF "${OBJECTDIR}/_ext/158577828/uart1.o.d" -o ${OBJECTDIR}/_ext/158577828/uart1.o ../../../../../../bsp/mlm_usb_host/uart1.c    -DXPRJ_MLM_USB_Host=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/158577828/fwupd.o: ../../../../../../bsp/mlm_usb_host/fwupd.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/158577828" 
	@${RM} ${OBJECTDIR}/_ext/158577828/fwupd.o.d 
	@${RM} ${OBJECTDIR}/_ext/158577828/fwupd.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/158577828/fwupd.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -DPICkit3PlatformTool=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYSTEM_PERIPHERAL_CLOCK=24000000 -I"." -I"../demo_src" -I"../../../../../../framework/usb/inc" -I"../../../../../../bsp/mlm_usb_host" -MMD -MF "${OBJECTDIR}/_ext/158577828/fwupd.o.d" -o ${OBJECTDIR}/_ext/158577828/fwupd.o ../../../../../../bsp/mlm_usb_host/fwupd.c    -DXPRJ_MLM_USB_Host=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/838585624/usb_host.o: ../../../../../../framework/usb/src/usb_host.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/838585624" 
	@${RM} ${OBJECTDIR}/_ext/838585624/usb_host.o.d 
	@${RM} ${OBJECTDIR}/_ext/838585624/usb_host.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/838585624/usb_host.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -DPICkit3PlatformTool=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYSTEM_PERIPHERAL_CLOCK=24000000 -I"." -I"../demo_src" -I"../../../../../../framework/usb/inc" -I"../../../../../../bsp/mlm_usb_host" -MMD -MF "${OBJECTDIR}/_ext/838585624/usb_host.o.d" -o ${OBJECTDIR}/_ext/838585624/usb_host.o ../../../../../../framework/usb/src/usb_host.c    -DXPRJ_MLM_USB_Host=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/838585624/usb_host_hid.o: ../../../../../../framework/usb/src/usb_host_hid.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/838585624" 
	@${RM} ${OBJECTDIR}/_ext/838585624/usb_host_hid.o.d 
	@${RM} ${OBJECTDIR}/_ext/838585624/usb_host_hid.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/838585624/usb_host_hid.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -DPICkit3PlatformTool=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYSTEM_PERIPHERAL_CLOCK=24000000 -I"." -I"../demo_src" -I"../../../../../../framework/usb/inc" -I"../../../../../../bsp/mlm_usb_host" -MMD -MF "${OBJECTDIR}/_ext/838585624/usb_host_hid.o.d" -o ${OBJECTDIR}/_ext/838585624/usb_host_hid.o ../../../../../../framework/usb/src/usb_host_hid.c    -DXPRJ_MLM_USB_Host=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/838585624/usb_host_hid_parser.o: ../../../../../../framework/usb/src/usb_host_hid_parser.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/838585624" 
	@${RM} ${OBJECTDIR}/_ext/838585624/usb_host_hid_parser.o.d 
	@${RM} ${OBJECTDIR}/_ext/838585624/usb_host_hid_parser.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/838585624/usb_host_hid_parser.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -DPICkit3PlatformTool=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYSTEM_PERIPHERAL_CLOCK=24000000 -I"." -I"../demo_src" -I"../../../../../../framework/usb/inc" -I"../../../../../../bsp/mlm_usb_host" -MMD -MF "${OBJECTDIR}/_ext/838585624/usb_host_hid_parser.o.d" -o ${OBJECTDIR}/_ext/838585624/usb_host_hid_parser.o ../../../../../../framework/usb/src/usb_host_hid_parser.c    -DXPRJ_MLM_USB_Host=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/300881143/usb_config.o: ../demo_src/usb_config.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/300881143" 
	@${RM} ${OBJECTDIR}/_ext/300881143/usb_config.o.d 
	@${RM} ${OBJECTDIR}/_ext/300881143/usb_config.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/300881143/usb_config.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -DPICkit3PlatformTool=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYSTEM_PERIPHERAL_CLOCK=24000000 -I"." -I"../demo_src" -I"../../../../../../framework/usb/inc" -I"../../../../../../bsp/mlm_usb_host" -MMD -MF "${OBJECTDIR}/_ext/300881143/usb_config.o.d" -o ${OBJECTDIR}/_ext/300881143/usb_config.o ../demo_src/usb_config.c    -DXPRJ_MLM_USB_Host=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/300881143/app_host_hid_keyboard.o: ../demo_src/app_host_hid_keyboard.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/300881143" 
	@${RM} ${OBJECTDIR}/_ext/300881143/app_host_hid_keyboard.o.d 
	@${RM} ${OBJECTDIR}/_ext/300881143/app_host_hid_keyboard.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/300881143/app_host_hid_keyboard.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -DPICkit3PlatformTool=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYSTEM_PERIPHERAL_CLOCK=24000000 -I"." -I"../demo_src" -I"../../../../../../framework/usb/inc" -I"../../../../../../bsp/mlm_usb_host" -MMD -MF "${OBJECTDIR}/_ext/300881143/app_host_hid_keyboard.o.d" -o ${OBJECTDIR}/_ext/300881143/app_host_hid_keyboard.o ../demo_src/app_host_hid_keyboard.c    -DXPRJ_MLM_USB_Host=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/300881143/main.o: ../demo_src/main.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/300881143" 
	@${RM} ${OBJECTDIR}/_ext/300881143/main.o.d 
	@${RM} ${OBJECTDIR}/_ext/300881143/main.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/300881143/main.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -DPICkit3PlatformTool=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYSTEM_PERIPHERAL_CLOCK=24000000 -I"." -I"../demo_src" -I"../../../../../../framework/usb/inc" -I"../../../../../../bsp/mlm_usb_host" -MMD -MF "${OBJECTDIR}/_ext/300881143/main.o.d" -o ${OBJECTDIR}/_ext/300881143/main.o ../demo_src/main.c    -DXPRJ_MLM_USB_Host=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/system.o: system.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/system.o.d 
	@${RM} ${OBJECTDIR}/system.o 
	@${FIXDEPS} "${OBJECTDIR}/system.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -DPICkit3PlatformTool=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYSTEM_PERIPHERAL_CLOCK=24000000 -I"." -I"../demo_src" -I"../../../../../../framework/usb/inc" -I"../../../../../../bsp/mlm_usb_host" -MMD -MF "${OBJECTDIR}/system.o.d" -o ${OBJECTDIR}/system.o system.c    -DXPRJ_MLM_USB_Host=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD) 
	
else
${OBJECTDIR}/_ext/158577828/lcd.o: ../../../../../../bsp/mlm_usb_host/lcd.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/158577828" 
	@${RM} ${OBJECTDIR}/_ext/158577828/lcd.o.d 
	@${RM} ${OBJECTDIR}/_ext/158577828/lcd.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/158577828/lcd.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYSTEM_PERIPHERAL_CLOCK=24000000 -I"." -I"../demo_src" -I"../../../../../../framework/usb/inc" -I"../../../../../../bsp/mlm_usb_host" -MMD -MF "${OBJECTDIR}/_ext/158577828/lcd.o.d" -o ${OBJECTDIR}/_ext/158577828/lcd.o ../../../../../../bsp/mlm_usb_host/lcd.c    -DXPRJ_MLM_USB_Host=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/158577828/leds.o: ../../../../../../bsp/mlm_usb_host/leds.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/158577828" 
	@${RM} ${OBJECTDIR}/_ext/158577828/leds.o.d 
	@${RM} ${OBJECTDIR}/_ext/158577828/leds.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/158577828/leds.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYSTEM_PERIPHERAL_CLOCK=24000000 -I"." -I"../demo_src" -I"../../../../../../framework/usb/inc" -I"../../../../../../bsp/mlm_usb_host" -MMD -MF "${OBJECTDIR}/_ext/158577828/leds.o.d" -o ${OBJECTDIR}/_ext/158577828/leds.o ../../../../../../bsp/mlm_usb_host/leds.c    -DXPRJ_MLM_USB_Host=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/158577828/timer_1ms.o: ../../../../../../bsp/mlm_usb_host/timer_1ms.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/158577828" 
	@${RM} ${OBJECTDIR}/_ext/158577828/timer_1ms.o.d 
	@${RM} ${OBJECTDIR}/_ext/158577828/timer_1ms.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/158577828/timer_1ms.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYSTEM_PERIPHERAL_CLOCK=24000000 -I"." -I"../demo_src" -I"../../../../../../framework/usb/inc" -I"../../../../../../bsp/mlm_usb_host" -MMD -MF "${OBJECTDIR}/_ext/158577828/timer_1ms.o.d" -o ${OBJECTDIR}/_ext/158577828/timer_1ms.o ../../../../../../bsp/mlm_usb_host/timer_1ms.c    -DXPRJ_MLM_USB_Host=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/158577828/uart1.o: ../../../../../../bsp/mlm_usb_host/uart1.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/158577828" 
	@${RM} ${OBJECTDIR}/_ext/158577828/uart1.o.d 
	@${RM} ${OBJECTDIR}/_ext/158577828/uart1.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/158577828/uart1.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYSTEM_PERIPHERAL_CLOCK=24000000 -I"." -I"../demo_src" -I"../../../../../../framework/usb/inc" -I"../../../../../../bsp/mlm_usb_host" -MMD -MF "${OBJECTDIR}/_ext/158577828/uart1.o.d" -o ${OBJECTDIR}/_ext/158577828/uart1.o ../../../../../../bsp/mlm_usb_host/uart1.c    -DXPRJ_MLM_USB_Host=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/158577828/fwupd.o: ../../../../../../bsp/mlm_usb_host/fwupd.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/158577828" 
	@${RM} ${OBJECTDIR}/_ext/158577828/fwupd.o.d 
	@${RM} ${OBJECTDIR}/_ext/158577828/fwupd.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/158577828/fwupd.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYSTEM_PERIPHERAL_CLOCK=24000000 -I"." -I"../demo_src" -I"../../../../../../framework/usb/inc" -I"../../../../../../bsp/mlm_usb_host" -MMD -MF "${OBJECTDIR}/_ext/158577828/fwupd.o.d" -o ${OBJECTDIR}/_ext/158577828/fwupd.o ../../../../../../bsp/mlm_usb_host/fwupd.c    -DXPRJ_MLM_USB_Host=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/838585624/usb_host.o: ../../../../../../framework/usb/src/usb_host.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/838585624" 
	@${RM} ${OBJECTDIR}/_ext/838585624/usb_host.o.d 
	@${RM} ${OBJECTDIR}/_ext/838585624/usb_host.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/838585624/usb_host.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYSTEM_PERIPHERAL_CLOCK=24000000 -I"." -I"../demo_src" -I"../../../../../../framework/usb/inc" -I"../../../../../../bsp/mlm_usb_host" -MMD -MF "${OBJECTDIR}/_ext/838585624/usb_host.o.d" -o ${OBJECTDIR}/_ext/838585624/usb_host.o ../../../../../../framework/usb/src/usb_host.c    -DXPRJ_MLM_USB_Host=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/838585624/usb_host_hid.o: ../../../../../../framework/usb/src/usb_host_hid.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/838585624" 
	@${RM} ${OBJECTDIR}/_ext/838585624/usb_host_hid.o.d 
	@${RM} ${OBJECTDIR}/_ext/838585624/usb_host_hid.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/838585624/usb_host_hid.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYSTEM_PERIPHERAL_CLOCK=24000000 -I"." -I"../demo_src" -I"../../../../../../framework/usb/inc" -I"../../../../../../bsp/mlm_usb_host" -MMD -MF "${OBJECTDIR}/_ext/838585624/usb_host_hid.o.d" -o ${OBJECTDIR}/_ext/838585624/usb_host_hid.o ../../../../../../framework/usb/src/usb_host_hid.c    -DXPRJ_MLM_USB_Host=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/838585624/usb_host_hid_parser.o: ../../../../../../framework/usb/src/usb_host_hid_parser.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/838585624" 
	@${RM} ${OBJECTDIR}/_ext/838585624/usb_host_hid_parser.o.d 
	@${RM} ${OBJECTDIR}/_ext/838585624/usb_host_hid_parser.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/838585624/usb_host_hid_parser.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYSTEM_PERIPHERAL_CLOCK=24000000 -I"." -I"../demo_src" -I"../../../../../../framework/usb/inc" -I"../../../../../../bsp/mlm_usb_host" -MMD -MF "${OBJECTDIR}/_ext/838585624/usb_host_hid_parser.o.d" -o ${OBJECTDIR}/_ext/838585624/usb_host_hid_parser.o ../../../../../../framework/usb/src/usb_host_hid_parser.c    -DXPRJ_MLM_USB_Host=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/300881143/usb_config.o: ../demo_src/usb_config.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/300881143" 
	@${RM} ${OBJECTDIR}/_ext/300881143/usb_config.o.d 
	@${RM} ${OBJECTDIR}/_ext/300881143/usb_config.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/300881143/usb_config.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYSTEM_PERIPHERAL_CLOCK=24000000 -I"." -I"../demo_src" -I"../../../../../../framework/usb/inc" -I"../../../../../../bsp/mlm_usb_host" -MMD -MF "${OBJECTDIR}/_ext/300881143/usb_config.o.d" -o ${OBJECTDIR}/_ext/300881143/usb_config.o ../demo_src/usb_config.c    -DXPRJ_MLM_USB_Host=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/300881143/app_host_hid_keyboard.o: ../demo_src/app_host_hid_keyboard.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/300881143" 
	@${RM} ${OBJECTDIR}/_ext/300881143/app_host_hid_keyboard.o.d 
	@${RM} ${OBJECTDIR}/_ext/300881143/app_host_hid_keyboard.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/300881143/app_host_hid_keyboard.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYSTEM_PERIPHERAL_CLOCK=24000000 -I"." -I"../demo_src" -I"../../../../../../framework/usb/inc" -I"../../../../../../bsp/mlm_usb_host" -MMD -MF "${OBJECTDIR}/_ext/300881143/app_host_hid_keyboard.o.d" -o ${OBJECTDIR}/_ext/300881143/app_host_hid_keyboard.o ../demo_src/app_host_hid_keyboard.c    -DXPRJ_MLM_USB_Host=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/_ext/300881143/main.o: ../demo_src/main.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/_ext/300881143" 
	@${RM} ${OBJECTDIR}/_ext/300881143/main.o.d 
	@${RM} ${OBJECTDIR}/_ext/300881143/main.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/300881143/main.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYSTEM_PERIPHERAL_CLOCK=24000000 -I"." -I"../demo_src" -I"../../../../../../framework/usb/inc" -I"../../../../../../bsp/mlm_usb_host" -MMD -MF "${OBJECTDIR}/_ext/300881143/main.o.d" -o ${OBJECTDIR}/_ext/300881143/main.o ../demo_src/main.c    -DXPRJ_MLM_USB_Host=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD) 
	
${OBJECTDIR}/system.o: system.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/system.o.d 
	@${RM} ${OBJECTDIR}/system.o 
	@${FIXDEPS} "${OBJECTDIR}/system.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -DSYSTEM_PERIPHERAL_CLOCK=24000000 -I"." -I"../demo_src" -I"../../../../../../framework/usb/inc" -I"../../../../../../bsp/mlm_usb_host" -MMD -MF "${OBJECTDIR}/system.o.d" -o ${OBJECTDIR}/system.o system.c    -DXPRJ_MLM_USB_Host=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD) 
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: compileCPP
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
dist/${CND_CONF}/${IMAGE_TYPE}/mlm_usb_host.x.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE) -g -mdebugger -DPICkit3PlatformTool=1 -mprocessor=$(MP_PROCESSOR_OPTION)  -o dist/${CND_CONF}/${IMAGE_TYPE}/mlm_usb_host.x.${IMAGE_TYPE}.${OUTPUT_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}          -DXPRJ_MLM_USB_Host=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)   -mreserve=data@0x0:0x1FC -mreserve=boot@0x1FC00490:0x1FC016FF -mreserve=boot@0x1FC00490:0x1FC00BEF  -Wl,--defsym=__MPLAB_BUILD=1$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION),--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,-D=__DEBUG_D,--defsym=PICkit3PlatformTool=1,--defsym=_min_heap_size=3000,--no-code-in-dinit,--no-dinit-in-serial-mem,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--memorysummary,dist/${CND_CONF}/${IMAGE_TYPE}/memoryfile.xml
	
else
dist/${CND_CONF}/${IMAGE_TYPE}/mlm_usb_host.x.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -mprocessor=$(MP_PROCESSOR_OPTION)  -o dist/${CND_CONF}/${IMAGE_TYPE}/mlm_usb_host.x.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}          -DXPRJ_MLM_USB_Host=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -Wl,--defsym=__MPLAB_BUILD=1$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION),--defsym=_min_heap_size=3000,--no-code-in-dinit,--no-dinit-in-serial-mem,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--memorysummary,dist/${CND_CONF}/${IMAGE_TYPE}/memoryfile.xml
	${MP_CC_DIR}\\xc32-bin2hex dist/${CND_CONF}/${IMAGE_TYPE}/mlm_usb_host.x.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} 
endif


# Subprojects
.build-subprojects:


# Subprojects
.clean-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/MLM_USB_Host
	${RM} -r dist/MLM_USB_Host

# Enable dependency checking
.dep.inc: .depcheck-impl

DEPFILES=$(shell mplabwildcard ${POSSIBLE_DEPFILES})
ifneq (${DEPFILES},)
include ${DEPFILES}
endif
