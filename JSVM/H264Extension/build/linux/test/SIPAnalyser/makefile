# the SOURCE definiton lets you move your makefile to another position
CONFIG 				= CONSOLE

# set directories to your wanted values
SRC_DIR				= ../../../../src/test/SIPAnalyser
INC_DIR				= ../../../../include
LIB_DIR				= ../../../../../../lib
BIN_DIR				= ../../../../../../bin

SRC_DIR1		=
SRC_DIR2		=
SRC_DIR3		=
SRC_DIR4		=

USER_INC_DIRS	= -I$(SRC_DIR)
USER_LIB_DIRS	= -L/usr/X11R6/lib

# intermediate directory for object files
OBJ_DIR				= ./objects

# set executable name
PRJ_NAME			= SIPAnalyser

# defines to set
DEFS				= -DMSYS_LINUX -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -DMSYS_UNIX_LARGEFILE

# set objects
OBJS          		= 	\
					$(OBJ_DIR)/SIPAnalyser.o \
					$(OBJ_DIR)/SIPParameters.o \
					$(OBJ_DIR)/SIP.o \

# set libs to link with
#LIBS				= -ldl

DEBUG_LIBS			=
RELEASE_LIBS		=

STAT_LIBS			= -lpthread -lX11
DYN_LIBS			=


DYN_DEBUG_LIBS		= -lH264AVCCommonLibd
DYN_DEBUG_PREREQS		= $(LIB_DIR)/libH264AVCCommonLibd.a
STAT_DEBUG_LIBS		= -lH264AVCCommonLibStaticd
STAT_DEBUG_PREREQS		= $(LIB_DIR)/libH264AVCCommonLibStaticd.a

DYN_RELEASE_LIBS	= -lH264AVCCommonLib
DYN_RELEASE_PREREQS	= $(LIB_DIR)/libH264AVCCommonLib.a
STAT_RELEASE_LIBS	= -lH264AVCCommonLibStatic
STAT_RELEASE_PREREQS	= $(LIB_DIR)/libH264AVCCommonLibStatic.a


# name of the base makefile
MAKE_FILE_NAME		= ../../common/makefile.base

# include the base makefile
include $(MAKE_FILE_NAME)