##############################################################################
# Compiling TimeWarp: make {win32=1} {debug=1} {NO_JGMOD=1}                  #
#                                                                            #
# Define win32=1    when compiling with Mingw32 gcc compiler for windows     #
# Define debug=1    when you want to build debug version of TimeWarp         #
# Define NO_JGMOD=1 when you have not jgmod optional library                 #
#                                                                            #
# Running just make builds the release version of TimeWarp for *nix          #
# (Linux, FreeBSD, ...)                                                      #
#                                                                            #
# The game depends on Allegro (4.0.x) library, so you need to install it     #
# before running make                                                        #
#                                                                            #
##############################################################################

CC = g++
LD = g++
CFLAGS = -fsigned-char -Wall
OBJDIR = obj
NAME = timewarp

VPATH = source source/ais source/games source/games/triggers source/melee \
        source/newships source/other source/sc1ships source/sc2ships \
        source/sc3ships source/twgui source/util

SRCS = ${shell ls -R -w 15000 -C  source/*/triggers/*.cpp source/*/*.c source/*/*.cpp source/*.cpp | sed s/source[/]//g | sed s/ais[/]//g | sed s/games[/]//g | sed s/triggers[/]//g | sed s/melee[/]//g | sed s/newships[/]//g | sed s/other[/]//g | sed s/sc1ships[/]//g | sed s/sc2ships[/]//g | sed s/sc3ships[/]//g | sed s/twgui[/]//g | sed s/util[/]//g } 

#SRCS = c_input.cpp c_other.cpp c_wussie.cpp frame.cpp gamehierarchy.cpp \
#       gastroid.cpp gdebugonly.cpp gflmelee.cpp ggob.cpp ghyper.cpp     \
#       gleague.cpp gmissions.cpp gmissions_objects.cpp	gplanets.cpp    \
#       gplexplr.cpp gplhuge.cpp	gsamp2.cpp  gsample.cpp gsarena.cpp     \
#       gsolar.cpp  gtrug.cpp  vanguard.cpp  vgGenSys.cpp  gui.cpp       \
#       input.cpp  manim.cpp  mcbodies.cpp  mcontrol.cpp  mfleet.cpp     \
#       mframe.cpp  mgame.cpp  mhelpers.cpp  mitems.cpp  mlog.cpp        \
#       mmain.cpp mmath.cpp mnet1.cpp moptions.cpp mship.cpp mshot.cpp   \
#       mshpdata.cpp  mshppan.cpp  msprite.cpp  mview.cpp  shpalabc.cpp  \
#       shpalckr.cpp shpalhdr.cpp shparitr.cpp shparkpi.cpp shpartem.cpp \
#       shpayrbs.cpp shpbatde.cpp shpbipka.cpp shpbubbo.cpp shpbubex.cpp \
#       shpchoex.cpp shpclofl.cpp shpconca.cpp shpconho.cpp shpcrapl.cpp \
#       shpcresu.cpp shpdajem.cpp shpdjila.cpp shpdragr.cpp shpducla.cpp \
#       shpdyzha.cpp shpearc2.cpp shpearc3.cpp shpestgu.cpp shpfiear.cpp \
#       shpfopsl.cpp shpforsh.cpp shpfresc.cpp shpgahmo.cpp shpgarty.cpp \
#       shpgerhe.cpp shpglacr.cpp shpglads.cpp shphotsp.cpp shphubde.cpp \
#       shphydcr.cpp shpiceco.cpp shpilwsp.cpp shpimpka.cpp shpjnkla.cpp \
#       shpjygst.cpp shpkabwe.cpp shpkahbo.cpp shpkatas.cpp shpkatpo.cpp \
#       shpklidr.cpp shpkoaja.cpp shpkoapa.cpp shpkolfl.cpp shpktesa.cpp \
#       shpleimu.cpp shplyrwa.cpp shpmekpi.cpp shpmoisp.cpp shpmontr.cpp \
#       shpnarlu.cpp shpneccr.cpp shpneodr.cpp shpnisha.cpp shpoliha.cpp \
#       shpostdi.cpp shppanav.cpp shpphepa.cpp shpplane.cpp shpqlore.cpp \
#       shpquasi.cpp shpquawr.cpp shpraame.cpp shpradfi.cpp shprekas.cpp \
#       shprogsq.cpp shpsamat.cpp shpsclfr.cpp shpsefna.cpp shpstaba.cpp \
#       shpstrsc.cpp shptauar.cpp shptaubo.cpp shptauda.cpp shptauem.cpp \
#       shptaugl.cpp shptauhu.cpp shptaule.cpp shptaume.cpp shptaust.cpp \
#       shptautu.cpp shptelno.cpp shptougr.cpp shptrige.cpp shptulra.cpp \
#       shpulzin.cpp shpuosli.cpp shpuxjba.cpp shpvelcr.cpp shpvenke.cpp \
#       shpvezba.cpp shpvioge.cpp shpvirli.cpp shpwassu.cpp shpwistr.cpp \
#       shpxaaar.cpp shpxilcr.cpp shpxxxas.cpp shpxxxma.cpp shpyevme.cpp \
#       shpyurpa.cpp shpyusra.cpp shpzeksh.cpp gup.cpp nullphas.cpp      \
#       objanim.cpp  orbit.cpp  planet3d.cpp  radar.cpp  shippart.cpp    \
#       vbodies.cpp  vtarget.cpp  shpandgu.cpp  shparisk.cpp             \
#       shpchebr.cpp shpearcr.cpp shpilwav.cpp shpkzedr.cpp shpmmrxf.cpp	\
#       shpmycpo.cpp shpshosc.cpp shpspael.cpp shpsyrpe.cpp shpumgdr.cpp \
#       shpvuxin.cpp shpyehte.cpp shpchmav.cpp shpdruma.cpp shpkohma.cpp \
#       shpmeltr.cpp shporzne.cpp shppkufu.cpp shpslypr.cpp shpsupbl.cpp \
#       shpthrto.cpp shputwju.cpp shpzfpst.cpp shpclapi.cpp shpdakvi.cpp	\
#       shpdooco.cpp shpexqen.cpp shpharra.cpp shpherex.cpp shpktacr.cpp \
#       shplk_sa.cpp shpowavo.cpp shpplopl.cpp shpvyrin.cpp shpxchex.cpp \
#       scp.cpp area.cpp gametest.cpp twgui.cpp utils.cpp aarot.c aastr.c \
#       aautil.c base.cpp endian.cpp errors.cpp errors_c.c get_time.c    \
#       history.cpp net_tcp.cpp pmask.c random.cpp round.c sintable.cpp  \
#       sounds.cpp  types.cpp  vector2.cpp

ifdef debug
	CFLAGS += -g -DDEBUGMODE
	OBJDIR := ${addsuffix -debug,$(OBJDIR)}
	NAME := ${addsuffix -debug,$(NAME)}
else
	CFLAGS += -O -mcpu=i686 -s
endif

ifdef NO_JGMOD
	CFLAGS += -DNO_JGMOD
else
	LIBS = -ljgmod
endif

ifdef win32
	OBJDIR := ${addsuffix -win32,$(OBJDIR)}
	NAME := ${addsuffix .exe,$(NAME)}
	CFLAGS += -DWIN32 
	LIBS += -lalleg -lws2_32 -lwinmm
else
	CFLAGS += -DLINUX
	INCLUDES = ${shell allegro-config --cflags}
	CFLAGS += $(INCLUDES)
	LIBS += ${shell allegro-config --libs}
endif

PREP = $(SRCS:.c=.cpp))
OBJS = $(addprefix $(OBJDIR)/,$(PREP:.cpp=.o))
DEPS = $(addprefix $(OBJDIR)/,$(PREP:.cpp=.d))

ifdef win32
ifndef debug
SUBSYSTEM=-Wl,--subsystem=windows
endif
endif
ifdef win32
OBJS += $(OBJDIR)/winicon.o
endif

##############################################################################

all: $(OBJDIR) $(OBJS) PRELINK $(NAME)


PRELINK:
	$(RM) $(NAME)

$(OBJDIR):
	mkdir $(OBJDIR)

$(OBJDIR)/%.o: %.cpp
	$(CC) -MMD $(CFLAGS) -c $< -o $@

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
