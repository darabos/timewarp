# Microsoft Developer Studio Project File - Name="twwin" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=twwin - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "twwin.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "twwin.mak" CFG="twwin - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "twwin - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "twwin - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "twwin - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GR /GX /Ox /Ot /Og /Oi /Op /Ob2 /I "./include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "__i386__" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 ./lib/alleg.lib user32.lib winmm.lib wsock32.lib ./lib/libjgmod.lib ./lib/lua.lib ./lib/libfreetype.lib /nologo /subsystem:windows /incremental:yes /machine:I386 /nodefaultlib:"LIBCMT" /out:"twwin.exe"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "twwin - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /Zi /Op /Ob1 /I "./include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "__i386__" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ./lib/libfreetype.lib ./lib/alld.lib user32.lib winmm.lib wsock32.lib ./lib/libjgmod.lib ./lib/luad.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"LIBCMT" /out:"twwin_DEBUG.exe" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "twwin - Win32 Release"
# Name "twwin - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Melee"

# PROP Default_Filter "*.cpp"
# Begin Source File

SOURCE=.\source\melee\manim.cpp
# End Source File
# Begin Source File

SOURCE=.\source\melee\mcbodies.cpp
# End Source File
# Begin Source File

SOURCE=.\source\melee\mcontrol.cpp
# End Source File
# Begin Source File

SOURCE=.\source\melee\mfleet.cpp
# End Source File
# Begin Source File

SOURCE=.\source\melee\mframe.cpp
# End Source File
# Begin Source File

SOURCE=.\source\melee\mgame.cpp
# End Source File
# Begin Source File

SOURCE=.\source\melee\mhelpers.cpp
# End Source File
# Begin Source File

SOURCE=.\source\melee\mitems.cpp
# End Source File
# Begin Source File

SOURCE=.\source\melee\mlog.cpp
# End Source File
# Begin Source File

SOURCE=.\source\melee\mmain.cpp
# End Source File
# Begin Source File

SOURCE=.\source\melee\mmath.cpp
# End Source File
# Begin Source File

SOURCE=.\source\melee\mnet1.cpp
# End Source File
# Begin Source File

SOURCE=.\source\melee\moptions.cpp
# End Source File
# Begin Source File

SOURCE=.\source\melee\mship.cpp
# End Source File
# Begin Source File

SOURCE=.\source\melee\mshot.cpp
# End Source File
# Begin Source File

SOURCE=.\source\melee\mshpdata.cpp
# End Source File
# Begin Source File

SOURCE=.\source\melee\mshppan.cpp
# End Source File
# Begin Source File

SOURCE=.\source\melee\msprite.cpp
# End Source File
# Begin Source File

SOURCE=.\source\melee\mtarget.cpp
# End Source File
# Begin Source File

SOURCE=.\source\melee\mview.cpp
# End Source File
# End Group
# Begin Group "Util"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\source\util\aarot.c
# End Source File
# Begin Source File

SOURCE=.\source\util\aastr.c
# End Source File
# Begin Source File

SOURCE=.\source\util\aautil.c
# End Source File
# Begin Source File

SOURCE=.\source\util\base.cpp
# End Source File
# Begin Source File

SOURCE=.\source\util\endian.cpp
# End Source File
# Begin Source File

SOURCE=.\source\util\errors.cpp
# End Source File
# Begin Source File

SOURCE=.\source\util\errors_c.c
# End Source File
# Begin Source File

SOURCE=.\source\util\get_time.c
# End Source File
# Begin Source File

SOURCE=.\source\util\history.cpp
# End Source File
# Begin Source File

SOURCE=.\source\util\net_tcp.cpp
# End Source File
# Begin Source File

SOURCE=.\source\util\pmask.c
# End Source File
# Begin Source File

SOURCE=.\source\util\profile.cpp
# End Source File
# Begin Source File

SOURCE=.\source\util\random.cpp
# End Source File
# Begin Source File

SOURCE=.\source\util\round.c
# End Source File
# Begin Source File

SOURCE=.\source\util\sounds.cpp
# End Source File
# Begin Source File

SOURCE=.\source\util\types.cpp
# End Source File
# Begin Source File

SOURCE=.\source\util\vector2.cpp
# End Source File
# End Group
# Begin Group "Other"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\source\other\configrw.cpp
# End Source File
# Begin Source File

SOURCE=.\source\other\configrw.h
# End Source File
# Begin Source File

SOURCE=.\source\other\fontmorph.cpp
# End Source File
# Begin Source File

SOURCE=.\source\other\gdialog.cpp
# End Source File
# Begin Source File

SOURCE=.\source\other\gevent.cpp
# End Source File
# Begin Source File

SOURCE=.\source\other\gquest.cpp
# End Source File
# Begin Source File

SOURCE=.\source\other\gup.cpp
# End Source File
# Begin Source File

SOURCE=.\source\other\luaport.cpp
# End Source File
# Begin Source File

SOURCE=.\source\other\luaport.h
# End Source File
# Begin Source File

SOURCE=.\source\other\nullphas.cpp
# End Source File
# Begin Source File

SOURCE=.\source\other\objanim.cpp
# End Source File
# Begin Source File

SOURCE=.\source\other\orbit.cpp
# End Source File
# Begin Source File

SOURCE=.\source\other\planet3d.cpp
# End Source File
# Begin Source File

SOURCE=.\source\other\radar.cpp
# End Source File
# Begin Source File

SOURCE=.\source\other\shippart.cpp
# End Source File
# Begin Source File

SOURCE=.\source\other\ttf.cpp
# End Source File
# Begin Source File

SOURCE=.\source\other\vbodies.cpp
# End Source File
# Begin Source File

SOURCE=.\source\other\vtarget.cpp
# End Source File
# End Group
# Begin Group "AIs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\source\ais\c_input.cpp
# End Source File
# Begin Source File

SOURCE=.\source\ais\c_other.cpp
# End Source File
# Begin Source File

SOURCE=.\source\ais\c_wussie.cpp
# End Source File
# End Group
# Begin Group "Games"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\source\games\dialog.cpp
# End Source File
# Begin Source File

SOURCE=.\source\games\gamehierarchy.cpp
# End Source File
# Begin Source File

SOURCE=.\source\games\gastroid.cpp
# End Source File
# Begin Source File

SOURCE=.\source\games\gdebugonly.cpp
# End Source File
# Begin Source File

SOURCE=.\source\games\gflmelee.cpp
# End Source File
# Begin Source File

SOURCE=.\source\games\ggob.cpp
# End Source File
# Begin Source File

SOURCE=.\source\games\ghyper.cpp
# End Source File
# Begin Source File

SOURCE=.\source\games\gleague.cpp
# End Source File
# Begin Source File

SOURCE=.\source\games\gluagame.cpp
# End Source File
# Begin Source File

SOURCE=.\source\games\gmissions.cpp
# End Source File
# Begin Source File

SOURCE=.\source\games\gmissions_objects.cpp
# End Source File
# Begin Source File

SOURCE=.\source\games\gplanets.cpp
# End Source File
# Begin Source File

SOURCE=.\source\games\gplexplr.cpp
# End Source File
# Begin Source File

SOURCE=.\source\games\gplhuge.cpp
# End Source File
# Begin Source File

SOURCE=.\source\games\gsamp2.cpp
# End Source File
# Begin Source File

SOURCE=.\source\games\gsample.cpp
# End Source File
# Begin Source File

SOURCE=.\source\games\gsarena.cpp
# End Source File
# Begin Source File

SOURCE=.\source\games\gsolar.cpp
# End Source File
# Begin Source File

SOURCE=.\source\games\gtrug.cpp
# End Source File
# Begin Source File

SOURCE=.\source\games\vanguard.cpp
# End Source File
# Begin Source File

SOURCE=.\source\games\vgGenSys.cpp
# End Source File
# End Group
# Begin Group "GameX"

# PROP Default_Filter ""
# Begin Group "edit"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\source\gamex\edit\disk_stuff.cpp
# End Source File
# Begin Source File

SOURCE=.\source\gamex\edit\edit_dialogue.cpp
# End Source File
# End Group
# Begin Group "stuff"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\source\gamex\stuff\backgr_stars.cpp
# End Source File
# Begin Source File

SOURCE=.\source\gamex\stuff\space_body.cpp
# End Source File
# End Group
# Begin Group "general"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\source\gamex\general\sprites.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\source\gamex\ai_fleet.cpp
# End Source File
# Begin Source File

SOURCE=.\source\gamex\ai_race.cpp
# End Source File
# Begin Source File

SOURCE=.\source\gamex\gamedata.cpp
# End Source File
# Begin Source File

SOURCE=.\source\gamex\gamedata_map.cpp
# End Source File
# Begin Source File

SOURCE=.\source\gamex\gamedialogue.cpp
# End Source File
# Begin Source File

SOURCE=.\source\gamex\gamegeneral.cpp
# End Source File
# Begin Source File

SOURCE=.\source\gamex\gamehyper.cpp
# End Source File
# Begin Source File

SOURCE=.\source\gamex\gamemelee.cpp
# End Source File
# Begin Source File

SOURCE=.\source\gamex\gameplanetmission.cpp
# End Source File
# Begin Source File

SOURCE=.\source\gamex\gameplanetscan.cpp
# End Source File
# Begin Source File

SOURCE=.\source\gamex\gameplanetview.cpp
# End Source File
# Begin Source File

SOURCE=.\source\gamex\gameproject.cpp
# End Source File
# Begin Source File

SOURCE=.\source\gamex\gamesolarview.cpp
# End Source File
# Begin Source File

SOURCE=.\source\gamex\gamestarmap.cpp
# End Source File
# Begin Source File

SOURCE=.\source\gamex\projectx.cpp
# End Source File
# End Group
# Begin Group "Ships"

# PROP Default_Filter ""
# Begin Group "SC1_Ships"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\source\sc1ships\shpandgu.cpp
# End Source File
# Begin Source File

SOURCE=.\source\sc1ships\shparisk.cpp
# End Source File
# Begin Source File

SOURCE=.\source\sc1ships\shpchebr.cpp
# End Source File
# Begin Source File

SOURCE=.\source\sc1ships\shpearcr.cpp
# End Source File
# Begin Source File

SOURCE=.\source\sc1ships\shpilwav.cpp
# End Source File
# Begin Source File

SOURCE=.\source\sc1ships\shpkzedr.cpp
# End Source File
# Begin Source File

SOURCE=.\source\sc1ships\shpmmrxf.cpp
# End Source File
# Begin Source File

SOURCE=.\source\sc1ships\shpmycpo.cpp
# End Source File
# Begin Source File

SOURCE=.\source\sc1ships\shpshosc.cpp
# End Source File
# Begin Source File

SOURCE=.\source\sc1ships\shpspael.cpp
# End Source File
# Begin Source File

SOURCE=.\source\sc1ships\shpsyrpe.cpp
# End Source File
# Begin Source File

SOURCE=.\source\sc1ships\shpumgdr.cpp
# End Source File
# Begin Source File

SOURCE=.\source\sc1ships\shpvuxin.cpp
# End Source File
# Begin Source File

SOURCE=.\source\sc1ships\shpyehte.cpp
# End Source File
# End Group
# Begin Group "SC2_Ships"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\source\sc2ships\shpchmav.cpp
# End Source File
# Begin Source File

SOURCE=.\source\sc2ships\shpdruma.cpp
# End Source File
# Begin Source File

SOURCE=.\source\sc2ships\shpkohma.cpp
# End Source File
# Begin Source File

SOURCE=.\source\sc2ships\shpmeltr.cpp
# End Source File
# Begin Source File

SOURCE=.\source\sc2ships\shporzne.cpp
# End Source File
# Begin Source File

SOURCE=.\source\sc2ships\shppkufu.cpp
# End Source File
# Begin Source File

SOURCE=.\source\sc2ships\shpslypr.cpp
# End Source File
# Begin Source File

SOURCE=.\source\sc2ships\shpsupbl.cpp
# End Source File
# Begin Source File

SOURCE=.\source\sc2ships\shpthrto.cpp
# End Source File
# Begin Source File

SOURCE=.\source\sc2ships\shputwju.cpp
# End Source File
# Begin Source File

SOURCE=.\source\sc2ships\shpzfpst.cpp
# End Source File
# End Group
# Begin Group "SC3_Ships"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\source\sc3ships\shpclapi.cpp
# End Source File
# Begin Source File

SOURCE=.\source\sc3ships\shpdakvi.cpp
# End Source File
# Begin Source File

SOURCE=.\source\sc3ships\shpdooco.cpp
# End Source File
# Begin Source File

SOURCE=.\source\sc3ships\shpexqen.cpp
# End Source File
# Begin Source File

SOURCE=.\source\sc3ships\shpharra.cpp
# End Source File
# Begin Source File

SOURCE=.\source\sc3ships\shpherex.cpp
# End Source File
# Begin Source File

SOURCE=.\source\sc3ships\shpktacr.cpp
# End Source File
# Begin Source File

SOURCE=.\source\sc3ships\shplk_sa.cpp
# End Source File
# Begin Source File

SOURCE=.\source\sc3ships\shpowavo.cpp
# End Source File
# Begin Source File

SOURCE=.\source\sc3ships\shpplopl.cpp
# End Source File
# Begin Source File

SOURCE=.\source\sc3ships\shpvyrin.cpp
# End Source File
# Begin Source File

SOURCE=.\source\sc3ships\shpxchex.cpp
# End Source File
# End Group
# Begin Group "TW_Ships"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\source\newships\shpaktgu.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpalabc.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpalckr.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpalhdr.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpalhha.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shparitr.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shparkpi.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpartem.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpastba.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpayrbs.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpbahbu.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpbatde.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpbipka.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpbogce.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpbubbo.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpbubex.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpchoex.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpclofl.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpconca.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpconho.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpcrapl.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpcresu.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpdajem.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpdeees.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpdjila.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpdragr.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpducla.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpdyzha.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpearc2.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpearc3.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpestgu.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpfiear.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpfopsl.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpforsh.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpfresc.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpgahmo.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpgarty.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpgerhe.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpgeror.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpgerra.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpglacr.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpglads.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shphotsp.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shphubde.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shphydcr.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpiceco.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpilwsp.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpilwsp.h
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpimpka.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpjnkla.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpjurcu.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpjygst.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpkabwe.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpkahbo.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpkatas.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpkatpo.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpklidr.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpkoaja.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpkoapa.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpkolfl.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpktesa.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpleimu.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shplyrwa.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpmekpi.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpmoisp.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpmontr.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpnarlu.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpneccr.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpneodr.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpnisha.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpoliha.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpostdi.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shppanav.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpphepa.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpplala.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpplane.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpqlore.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpquasi.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpquawr.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpraame.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpradfi.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shprekas.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shprogsq.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpsamat.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpsclfr.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpsefn2.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpsefna.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpstaba.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpstrsc.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shptauar.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shptaubo.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shptauda.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shptauem.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shptaufi.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shptaugl.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shptauhu.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shptaule.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shptaumc.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shptaume.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shptausl.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shptaust.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shptauto.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shptautu.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shptelno.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpterbi.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shptougr.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shptrige.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shptulra.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpulzin.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpuosli.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpuxjba.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpvelcr.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpvenke.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpvezba.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpvioge.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpvirli.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpvuvji.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpwassu.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpwistr.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpwolmi.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpxaaar.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpxilcr.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpxxxas.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpxxxma.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpyevme.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpyurpa.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpyusra.cpp
# End Source File
# Begin Source File

SOURCE=.\source\newships\shpzeksh.cpp
# End Source File
# End Group
# End Group
# Begin Group "TW_GUI"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\source\twgui\gametest2.cpp
# End Source File
# Begin Source File

SOURCE=.\source\twgui\twbutton.cpp
# End Source File
# Begin Source File

SOURCE=.\source\twgui\twbuttontypes.cpp
# End Source File
# Begin Source File

SOURCE=.\source\twgui\twgui.cpp
# End Source File
# Begin Source File

SOURCE=.\source\twgui\twhelpers.cpp
# End Source File
# Begin Source File

SOURCE=.\source\twgui\twmenuexamples.cpp
# End Source File
# Begin Source File

SOURCE=.\source\twgui\twpopup.cpp
# End Source File
# Begin Source File

SOURCE=.\source\twgui\twwindow.cpp
# End Source File
# Begin Source File

SOURCE=.\source\twgui\utils.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\source\frame.cpp
# End Source File
# Begin Source File

SOURCE=.\source\gui.cpp
# End Source File
# Begin Source File

SOURCE=.\source\input.cpp
# End Source File
# Begin Source File

SOURCE=.\source\scp.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "Melee_h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\source\melee\manim.h
# End Source File
# Begin Source File

SOURCE=.\source\melee\mcbodies.h
# End Source File
# Begin Source File

SOURCE=.\source\melee\mcontrol.h
# End Source File
# Begin Source File

SOURCE=.\source\melee\mfleet.h
# End Source File
# Begin Source File

SOURCE=.\source\melee\mframe.h
# End Source File
# Begin Source File

SOURCE=.\source\melee\mgame.h
# End Source File
# Begin Source File

SOURCE=.\source\melee\mitems.h
# End Source File
# Begin Source File

SOURCE=.\source\melee\mlog.h
# End Source File
# Begin Source File

SOURCE=.\source\melee\mmain.h
# End Source File
# Begin Source File

SOURCE=.\source\melee\mnet1.h
# End Source File
# Begin Source File

SOURCE=.\source\melee\mship.h
# End Source File
# Begin Source File

SOURCE=.\source\melee\mshot.h
# End Source File
# Begin Source File

SOURCE=.\source\melee\mshppan.h
# End Source File
# Begin Source File

SOURCE=.\source\melee\mtarget.h
# End Source File
# Begin Source File

SOURCE=.\source\melee\mview.h
# End Source File
# End Group
# Begin Group "Util_h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\source\util\aastr.h
# End Source File
# Begin Source File

SOURCE=.\source\util\aautil.h
# End Source File
# Begin Source File

SOURCE=.\source\util\base.h
# End Source File
# Begin Source File

SOURCE=.\source\util\endian.h
# End Source File
# Begin Source File

SOURCE=.\source\util\errors.h
# End Source File
# Begin Source File

SOURCE=.\source\util\get_time.h
# End Source File
# Begin Source File

SOURCE=.\source\util\history.h
# End Source File
# Begin Source File

SOURCE=.\source\util\net_tcp.h
# End Source File
# Begin Source File

SOURCE=.\source\util\pmask.h
# End Source File
# Begin Source File

SOURCE=.\source\util\profile.h
# End Source File
# Begin Source File

SOURCE=.\source\util\profile2.h
# End Source File
# Begin Source File

SOURCE=.\source\util\random.h
# End Source File
# Begin Source File

SOURCE=.\source\util\round.h
# End Source File
# Begin Source File

SOURCE=.\source\util\sintable.h
# End Source File
# Begin Source File

SOURCE=.\source\util\sounds.h
# End Source File
# Begin Source File

SOURCE=.\source\util\t_rarray.h
# End Source File
# Begin Source File

SOURCE=.\source\util\types.h
# End Source File
# Begin Source File

SOURCE=.\source\util\vector2.h
# End Source File
# End Group
# Begin Group "Other_h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\source\other\fontmorph.h
# End Source File
# Begin Source File

SOURCE=.\source\other\gdialog.h
# End Source File
# Begin Source File

SOURCE=.\source\other\gevent.h
# End Source File
# Begin Source File

SOURCE=.\source\other\gquest.h
# End Source File
# Begin Source File

SOURCE=.\source\other\gup.h
# End Source File
# Begin Source File

SOURCE=.\source\other\nullphas.h
# End Source File
# Begin Source File

SOURCE=.\source\other\objanim.h
# End Source File
# Begin Source File

SOURCE=.\source\other\orbit.h
# End Source File
# Begin Source File

SOURCE=.\source\other\planet3d.h
# End Source File
# Begin Source File

SOURCE=.\source\other\radar.h
# End Source File
# Begin Source File

SOURCE=.\source\other\shippart.h
# End Source File
# Begin Source File

SOURCE=.\source\other\ttf.h
# End Source File
# Begin Source File

SOURCE=.\source\other\vbodies.h
# End Source File
# Begin Source File

SOURCE=.\source\other\vtarget.h
# End Source File
# End Group
# Begin Group "Games_h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\source\games\dialog.h
# End Source File
# Begin Source File

SOURCE=.\source\games\gamehierarchy.h
# End Source File
# Begin Source File

SOURCE=.\source\games\gflmelee.h
# End Source File
# Begin Source File

SOURCE=.\source\games\ggob.h
# End Source File
# Begin Source File

SOURCE=.\source\games\gmissions_objects.h
# End Source File
# Begin Source File

SOURCE=.\source\games\gplexplr.h
# End Source File
# Begin Source File

SOURCE=.\source\games\gtrug.h
# End Source File
# End Group
# Begin Group "Allegro_headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\include\allegro\3d.h
# End Source File
# Begin Source File

SOURCE=.\include\allegro\3dmaths.h
# End Source File
# Begin Source File

SOURCE=.\include\allegro\inline\3dmaths.inl
# End Source File
# Begin Source File

SOURCE=.\include\allegro\internal\aintern.h
# End Source File
# Begin Source File

SOURCE=.\include\allegro\platform\aintwin.h
# End Source File
# Begin Source File

SOURCE=.\include\allegro\platform\al386vc.h
# End Source File
# Begin Source File

SOURCE=.\include\allegro\alcompat.h
# End Source File
# Begin Source File

SOURCE=.\include\allegro\internal\alconfig.h
# End Source File
# Begin Source File

SOURCE=.\include\allegro.h
# End Source File
# Begin Source File

SOURCE=.\include\allegro\platform\almsvc.h
# End Source File
# Begin Source File

SOURCE=.\include\allegro\platform\alplatf.h
# End Source File
# Begin Source File

SOURCE=.\include\allegro\platform\alwin.h
# End Source File
# Begin Source File

SOURCE=.\include\allegro\inline\asm.inl
# End Source File
# Begin Source File

SOURCE=.\include\allegro\base.h
# End Source File
# Begin Source File

SOURCE=..\..\MSVS\VC98\Include\BASETSD.H
# End Source File
# Begin Source File

SOURCE=.\include\allegro\color.h
# End Source File
# Begin Source File

SOURCE=.\include\allegro\inline\color.inl
# End Source File
# Begin Source File

SOURCE=.\include\allegro\compiled.h
# End Source File
# Begin Source File

SOURCE=.\include\allegro\config.h
# End Source File
# Begin Source File

SOURCE=.\include\allegro\datafile.h
# End Source File
# Begin Source File

SOURCE=.\include\allegro\debug.h
# End Source File
# Begin Source File

SOURCE=.\include\allegro\digi.h
# End Source File
# Begin Source File

SOURCE=.\include\allegro\draw.h
# End Source File
# Begin Source File

SOURCE=.\include\allegro\inline\draw.inl
# End Source File
# Begin Source File

SOURCE=.\include\allegro\file.h
# End Source File
# Begin Source File

SOURCE=.\include\allegro\inline\file.inl
# End Source File
# Begin Source File

SOURCE=.\include\allegro\fix.h
# End Source File
# Begin Source File

SOURCE=.\include\allegro\inline\fix.inl
# End Source File
# Begin Source File

SOURCE=.\include\allegro\fixed.h
# End Source File
# Begin Source File

SOURCE=.\include\allegro\fli.h
# End Source File
# Begin Source File

SOURCE=.\include\allegro\fmaths.h
# End Source File
# Begin Source File

SOURCE=.\include\allegro\inline\fmaths.inl
# End Source File
# Begin Source File

SOURCE=.\include\allegro\gfx.h
# End Source File
# Begin Source File

SOURCE=.\include\allegro\inline\gfx.inl
# End Source File
# Begin Source File

SOURCE=.\include\allegro\gui.h
# End Source File
# Begin Source File

SOURCE=.\include\allegro\inline\gui.inl
# End Source File
# Begin Source File

SOURCE=.\include\allegro\joystick.h
# End Source File
# Begin Source File

SOURCE=.\include\allegro\keyboard.h
# End Source File
# Begin Source File

SOURCE=.\include\lauxlib.h
# End Source File
# Begin Source File

SOURCE=.\include\allegro\matrix.h
# End Source File
# Begin Source File

SOURCE=.\include\allegro\inline\matrix.inl
# End Source File
# Begin Source File

SOURCE=.\include\allegro\midi.h
# End Source File
# Begin Source File

SOURCE=.\include\allegro\mouse.h
# End Source File
# Begin Source File

SOURCE=.\include\allegro\palette.h
# End Source File
# Begin Source File

SOURCE=.\include\allegro\quat.h
# End Source File
# Begin Source File

SOURCE=.\include\allegro\rle.h
# End Source File
# Begin Source File

SOURCE=.\include\allegro\inline\rle.inl
# End Source File
# Begin Source File

SOURCE=.\include\allegro\sound.h
# End Source File
# Begin Source File

SOURCE=.\include\allegro\stream.h
# End Source File
# Begin Source File

SOURCE=.\include\allegro\system.h
# End Source File
# Begin Source File

SOURCE=.\include\allegro\inline\system.inl
# End Source File
# Begin Source File

SOURCE=.\include\allegro\text.h
# End Source File
# Begin Source File

SOURCE=.\include\allegro\timer.h
# End Source File
# Begin Source File

SOURCE=.\include\allegro\unicode.h
# End Source File
# Begin Source File

SOURCE=.\include\winalleg.h
# End Source File
# End Group
# Begin Group "Library_headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\include\jgmod.h
# End Source File
# Begin Source File

SOURCE=.\include\lua.h
# End Source File
# Begin Source File

SOURCE=.\include\lualib.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\source\ais.h
# End Source File
# Begin Source File

SOURCE=.\source\frame.h
# End Source File
# Begin Source File

SOURCE=.\source\gui.h
# End Source File
# Begin Source File

SOURCE=.\source\gup.h
# End Source File
# Begin Source File

SOURCE=.\source\id.h
# End Source File
# Begin Source File

SOURCE=.\source\input.h
# End Source File
# Begin Source File

SOURCE=.\source\libs.h
# End Source File
# Begin Source File

SOURCE=.\source\melee.h
# End Source File
# Begin Source File

SOURCE=.\source\sc1ships.h
# End Source File
# Begin Source File

SOURCE=.\source\sc2ships.h
# End Source File
# Begin Source File

SOURCE=.\source\scp.h
# End Source File
# Begin Source File

SOURCE=.\source\ship.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\scpwin.ico
# End Source File
# End Group
# End Target
# End Project
