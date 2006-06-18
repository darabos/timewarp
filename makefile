##############################################################################
# Compiling TimeWarp: make {win32=1} {debug=1}                               #
#                                                                            #
# Define win32=1    when compiling with Mingw32 gcc compiler for windows     #
# Define debug=1    when you want to build debug version of TimeWarp         #
# Define NO_JGMOD=1 when you have not jgmod optional library                 #
# Define NO_NET=1   when you does not need network                           # 
#                                                                            #
# Running just make builds the release version of TimeWarp for *nix          #
# (Linux, FreeBSD, ...)                                                      #
#                                                                            #
# The game depends on Allegro (4.0.x) and Lua (5.0.x) libraries, so you need #
# to install them before running make                                        #
#                                                                            #
##############################################################################

CXX = g++
CC = gcc
LD = g++
CFLAGS = -fsigned-char -Wall -Wno-deprecated-declarations
OBJDIR = obj
NAME = TimeWarp

VPATH = source source/ais source/games source/games/triggers source/melee \
        source/newships source/other source/ppiships source/sc1ships source/sc2ships \
        source/sc3ships source/twgui source/util source/gamex \
        source/gamex/edit source/gamex/general source/gamex/stuff \
        source/gamex/dialogs source/jpgalleg source/jgmod source/menu \
        source/alfont 

FILELIST= ${shell find source -type f "(" -name "*.c" -o -name "*.cpp" ")"}
BASE_NAMES = $(basename $(notdir $(FILELIST)))

POBJS = $(addsuffix .o,$(BASE_NAMES))
PDEPS = $(addsuffix .d,$(BASE_NAMES))

ifndef win32
	ARCH := $(shell echo `arch`)
	ifeq ($(ARCH),ppc)
		ARCH    = powerpc
	endif
	ifeq ($(ARCH),x86_64)
		ARCH	= k8
	endif
else
	ARCH := i586
endif

#FILELIST = source /s.cpp

ifdef debug
	CFLAGS += -pg -g -DDEBUGMODE
	OBJDIR := ${addsuffix -debug,$(OBJDIR)}
	NAME := ${addsuffix -debug,$(NAME)}
else
	CFLAGS += -O -march=$(ARCH) -s
endif

ifdef NO_NET
        CFLAGS += -DNETWORK_NONE
endif

ifdef win32
	OBJDIR := ${addsuffix -win32,$(OBJDIR)}
	NAME := ${addsuffix .exe,$(NAME)}
	CFLAGS += -DWIN32 
	LIBS += -lalleg -lws2_32 -lwinmm
else
	CFLAGS += -DLINUX
	INCLUDES = ${shell allegro-config --cflags} ${shell freetype-config --cflags}
	CFLAGS += $(INCLUDES)
	LIBS += ${shell allegro-config --libs} ${shell freetype-config --libs}
endif

CFLAGS += -I./source -I./source/jgmod -I./source/alfont

#CFLAGS += ${addprefix -I./, $(VPATH)}

LIBS += -llualib -llua
 
OBJS = $(addprefix $(OBJDIR)/,$(POBJS))
DEPS = $(addprefix $(OBJDIR)/,$(PDEPS))

ifdef win32
ifndef debug
SUBSYSTEM=-Wl,--subsystem=windows
endif
endif
ifdef win32
OBJS += $(OBJDIR)/winicon.o
endif

##############################################################################

all: $(OBJDIR) $(OBJS) $(NAME)


$(NAME): $(OBJDIR) $(OBJS)

$(OBJDIR):
	mkdir $(OBJDIR)

$(OBJDIR)/%.o: %.cpp
	$(CXX) -MMD $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: %.c
	$(CC) -MMD $(CFLAGS) -c $< -o $@

$(OBJDIR)/winicon.o: scpwin.rc scpwin.ico
	windres scpwin.rc -o $(OBJDIR)/winicon.o

$(NAME):
	$(LD) $(CFLAGS) $(SUBSYSTEM) -o $@ $(OBJDIR)/*.o $(LIBS) 

clean:
	$(RM) $(OBJDIR)/*.o
	$(RM) $(OBJDIR)/*.d
	$(RM) $(NAME)

-include $(DEPS)
