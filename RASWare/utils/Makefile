#******************************************************************************
#
# Makefile - Rules for building the utils.
#
# Copyright (c) 2005-2009 Luminary Micro, Inc.  All rights reserved.
# Software License Agreement
# 
# Luminary Micro, Inc. (LMI) is supplying this software for use solely and
# exclusively on LMI's microcontroller products.
# 
# The software is owned by LMI and/or its suppliers, and is protected under
# applicable copyright laws.  All rights are reserved.  You may not combine
# this software with "viral" open-source software in order to form a larger
# program.  Any use in violation of the foregoing restrictions may subject
# the user to criminal sanctions under applicable laws, as well as to civil
# liability for the breach of the terms and conditions of this license.
# 
# THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
# OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
# LMI SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
# CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
# 
# This is part of revision 4781 of the Stellaris Peripheral Driver Library.
#
#******************************************************************************

#
# The base directory for DriverLib.
#
ROOT=..

#
# Include the common make definitions.
#
include ${ROOT}/makedefs

#
# Where to find header files that do not live in the source directory.
#
IPATH=..

#
# The name of the target
#
TARGET=${COMPILER}/libutils.a

#
# The default rule, which causes the utils to be built.
#
all: ${COMPILER}
all: ${TARGET}

#
# The rule to clean out all the build products.
#
clean:
	@rm -rf ${COMPILER}/* settings
	@rm -rf ${wildcard *~} ${wildcard *.Bak} ${wildcard *.dep}

#
# The rule to create the target directory.
#
${COMPILER}:
	@mkdir -p ${COMPILER}

#
# Rules for building the utils.
#
${TARGET}: ${COMPILER}/cmdline.o
${TARGET}: ${COMPILER}/cpu_usage.o
${TARGET}: ${COMPILER}/flash_pb.o
${TARGET}: ${COMPILER}/isqrt.o
${TARGET}: ${COMPILER}/ringbuf.o
${TARGET}: ${COMPILER}/sine.o
${TARGET}: ${COMPILER}/uartstdio.o 
${TARGET}: ${COMPILER}/ustdlib.o

#
# Include the automatically generated dependency files.
#
ifneq (${MAKECMDGOALS},clean)
-include ${wildcard ${COMPILER}/*.d} __dummy__
endif
