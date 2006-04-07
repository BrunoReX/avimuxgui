# Microsoft Developer Studio Project File - Name="AVIMux_GUI" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=AVIMux_GUI - Win32 Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "AVIMux_GUI.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "AVIMux_GUI.mak" CFG="AVIMux_GUI - Win32 Debug"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "AVIMux_GUI - Win32 Release" (basierend auf  "Win32 (x86) Application")
!MESSAGE "AVIMux_GUI - Win32 Debug" (basierend auf  "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "AVIMux_GUI - Win32 Release"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /c
# SUBTRACT CPP /WX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /machine:I386
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "AVIMux_GUI - Win32 Debug"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 vfw32.lib msacm32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "AVIMux_GUI - Win32 Release"
# Name "AVIMux_GUI - Win32 Debug"
# Begin Group "Quellcodedateien"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Audio - Source - Code"

# PROP Default_Filter "cpp"
# Begin Source File

SOURCE=.\AudioSource.cpp
# End Source File
# Begin Source File

SOURCE=.\AudioSource_AAC.cpp
# End Source File
# Begin Source File

SOURCE=.\AudioSource_AC3.cpp
# End Source File
# Begin Source File

SOURCE=.\AudioSource_AVI.cpp
# End Source File
# Begin Source File

SOURCE=.\AudioSource_binary.cpp
# End Source File
# Begin Source File

SOURCE=.\AudioSource_DTS.cpp
# End Source File
# Begin Source File

SOURCE=.\AudioSource_generic.cpp
# End Source File
# Begin Source File

SOURCE=.\AudioSource_List.cpp
# End Source File
# Begin Source File

SOURCE=.\AudioSource_Matroska.cpp
# End Source File
# Begin Source File

SOURCE=.\AudioSource_MP3.cpp
# End Source File
# Begin Source File

SOURCE=.\AudioSource_Vorbis.cpp
# End Source File
# Begin Source File

SOURCE=.\AudioSource_WAV.cpp
# End Source File
# End Group
# Begin Group "Video - Source - Code"

# PROP Default_Filter "cpp"
# Begin Source File

SOURCE=.\VideoSource.cpp
# End Source File
# Begin Source File

SOURCE=.\VideoSource_AVI.cpp
# End Source File
# Begin Source File

SOURCE=.\VideoSource_generic.cpp
# End Source File
# Begin Source File

SOURCE=.\VideoSource_List.cpp
# End Source File
# Begin Source File

SOURCE=.\VideoSource_Matroska.cpp
# End Source File
# End Group
# Begin Group "Edit - Field - Code"

# PROP Default_Filter "cpp"
# Begin Source File

SOURCE=.\AC3FrameCountEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\AddSplitPointDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ChapterSegmentUIDEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\ClusterTimeEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\MKVAC3FrameCountEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\MKVHeaderSizeEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\ResolutionEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\UserDrawEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\VerifyEdit.cpp
# End Source File
# End Group
# Begin Group "AVI - Stuff"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\AVIFile.cpp
# End Source File
# Begin Source File

SOURCE=.\AVIIndices.cpp
# End Source File
# Begin Source File

SOURCE=.\AVIStream.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\AttachedWindows.cpp
# End Source File
# Begin Source File

SOURCE=.\AudioSourceTree.cpp
# End Source File
# Begin Source File

SOURCE=.\AVIMux_GUI.cpp
# End Source File
# Begin Source File

SOURCE=.\AVIMux_GUI.rc
# End Source File
# Begin Source File

SOURCE=.\AVIMux_GUIDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\AVIMux_GUIDlg_AddFileList.cpp
# End Source File
# Begin Source File

SOURCE=.\AVIMux_GUIDlg_OnInitDialog.cpp
# End Source File
# Begin Source File

SOURCE=..\basestreams.cpp
# End Source File
# Begin Source File

SOURCE=..\Buffers.cpp
# End Source File
# Begin Source File

SOURCE=..\Cache.cpp
# End Source File
# Begin Source File

SOURCE=.\ChapterDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ChapterDlgList.cpp
# End Source File
# Begin Source File

SOURCE=.\ChapterDlgTree.cpp
# End Source File
# Begin Source File

SOURCE=..\Chapters.cpp
# End Source File
# Begin Source File

SOURCE=.\ConfigScripts.cpp
# End Source File
# Begin Source File

SOURCE=..\CRC.cpp
# End Source File
# Begin Source File

SOURCE=.\Debug.cpp
# End Source File
# Begin Source File

SOURCE=.\DynArray.cpp
# End Source File
# Begin Source File

SOURCE=.\EBMLTree.cpp
# End Source File
# Begin Source File

SOURCE=.\EBMLTreeDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\edc_ecc.cpp
# End Source File
# Begin Source File

SOURCE=.\EnhancedListBox.cpp
# End Source File
# Begin Source File

SOURCE=.\FileDialogs.cpp
# End Source File
# Begin Source File

SOURCE=.\FillASIs.cpp
# End Source File
# Begin Source File

SOURCE=..\FormatInt64.cpp
# End Source File
# Begin Source File

SOURCE=.\FormatText.cpp
# End Source File
# Begin Source File

SOURCE=..\FormatTime.cpp
# End Source File
# Begin Source File

SOURCE=.\Global.cpp
# End Source File
# Begin Source File

SOURCE=.\Languages.cpp
# End Source File
# Begin Source File

SOURCE=.\MessageLists.cpp
# End Source File
# Begin Source File

SOURCE=.\Mode2Form2Reader.cpp
# End Source File
# Begin Source File

SOURCE=.\multimedia_source.cpp
# End Source File
# Begin Source File

SOURCE=.\Muxing.cpp
# End Source File
# Begin Source File

SOURCE=.\OGGFile.cpp
# End Source File
# Begin Source File

SOURCE=.\ProgressList.cpp
# End Source File
# Begin Source File

SOURCE=.\ProtocolListCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\ResizeableDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\RIFFChunkTreeDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\RIFFFile.cpp
# End Source File
# Begin Source File

SOURCE=.\SetFramerateDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\SetMainAVIHeaderFlagsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\SetStoreFileOptionsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Silence.cpp
# End Source File
# Begin Source File

SOURCE=.\SourceFileListBox.cpp
# End Source File
# Begin Source File

SOURCE=.\SplitPointList.cpp
# End Source File
# Begin Source File

SOURCE=.\SplitPointsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\streams.cpp
# End Source File
# Begin Source File

SOURCE=.\Strings.cpp
# End Source File
# Begin Source File

SOURCE=.\SubTitles.cpp
# End Source File
# Begin Source File

SOURCE=.\TextFiles.cpp
# End Source File
# Begin Source File

SOURCE=.\TransformFilter.cpp
# End Source File
# Begin Source File

SOURCE=.\Trees.cpp
# End Source File
# Begin Source File

SOURCE=.\UnicodeBase.cpp
# End Source File
# Begin Source File

SOURCE=..\UnicodeCalls.cpp
# End Source File
# Begin Source File

SOURCE=.\UnicodeListControl.cpp
# End Source File
# Begin Source File

SOURCE=.\UnicodeTreeCtrl.cpp
# End Source File
# Begin Source File

SOURCE="..\utf-8.cpp"
# End Source File
# Begin Source File

SOURCE=.\UTF8Windows.cpp
# End Source File
# Begin Source File

SOURCE=.\Version.cpp
# End Source File
# Begin Source File

SOURCE=.\VideoInformationDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\VideoInformationDlgListbox.cpp
# End Source File
# Begin Source File

SOURCE=.\VideoSourceListBox.cpp
# End Source File
# Begin Source File

SOURCE=.\WAVFile.cpp
# End Source File
# Begin Source File

SOURCE=..\XML.CPP
# End Source File
# Begin Source File

SOURCE=.\XMLFiles.cpp
# End Source File
# End Group
# Begin Group "Header-Dateien"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "Audio - Source - Headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\AudioSource.h
# End Source File
# Begin Source File

SOURCE=.\AudioSource_AAC.h
# End Source File
# Begin Source File

SOURCE=.\AudioSource_AC3.h
# End Source File
# Begin Source File

SOURCE=.\AudioSource_AVI.h
# End Source File
# Begin Source File

SOURCE=.\AudioSource_binary.h
# End Source File
# Begin Source File

SOURCE=.\AudioSource_DTS.h
# End Source File
# Begin Source File

SOURCE=.\AudioSource_generic.h
# End Source File
# Begin Source File

SOURCE=.\AudioSource_List.h
# End Source File
# Begin Source File

SOURCE=.\AudioSource_Matroska.h
# End Source File
# Begin Source File

SOURCE=.\AudioSource_MP3.h
# End Source File
# Begin Source File

SOURCE=.\AudioSource_Vorbis.h
# End Source File
# Begin Source File

SOURCE=.\AudioSource_WAV.h
# End Source File
# End Group
# Begin Group "Video - Source - Headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\VideoSource.h
# End Source File
# Begin Source File

SOURCE=.\VideoSource_AVI.h
# End Source File
# Begin Source File

SOURCE=.\VideoSource_generic.h
# End Source File
# Begin Source File

SOURCE=.\VideoSource_List.h
# End Source File
# Begin Source File

SOURCE=.\VideoSource_Matroska.h
# End Source File
# End Group
# Begin Group "Edit - Field - Headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\AC3FrameCountEdit.h
# End Source File
# Begin Source File

SOURCE=.\ChapterSegmentUIDEdit.h
# End Source File
# Begin Source File

SOURCE=.\ClusterTimeEdit.h
# End Source File
# Begin Source File

SOURCE=.\MKVAC3FrameCountEdit.h
# End Source File
# Begin Source File

SOURCE=.\MKVHeaderSizeEdit.h
# End Source File
# Begin Source File

SOURCE=.\ResolutionEdit.h
# End Source File
# Begin Source File

SOURCE=.\UserDrawEdit.h
# End Source File
# Begin Source File

SOURCE=.\VerifyEdit.h
# End Source File
# End Group
# Begin Group "AVI - Stuff - Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\AVIFile.h
# End Source File
# Begin Source File

SOURCE=.\AVIIndices.h
# End Source File
# Begin Source File

SOURCE=.\AVIStream.h
# End Source File
# Begin Source File

SOURCE=.\AVIStructs.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\AddSplitPointDlg.h
# End Source File
# Begin Source File

SOURCE=.\AttachedWindows.h
# End Source File
# Begin Source File

SOURCE=.\AudioSourceTree.h
# End Source File
# Begin Source File

SOURCE=.\AVIMux_GUI.h
# End Source File
# Begin Source File

SOURCE=.\AVIMux_GUIDlg.h
# End Source File
# Begin Source File

SOURCE=..\BaseStreams.h
# End Source File
# Begin Source File

SOURCE=..\Buffers.h
# End Source File
# Begin Source File

SOURCE=..\Cache.h
# End Source File
# Begin Source File

SOURCE=.\ChapterDlg.h
# End Source File
# Begin Source File

SOURCE=.\ChapterDlgList.h
# End Source File
# Begin Source File

SOURCE=.\ChapterDlgTree.h
# End Source File
# Begin Source File

SOURCE=..\Chapters.h
# End Source File
# Begin Source File

SOURCE=.\ConfigScripts.h
# End Source File
# Begin Source File

SOURCE=..\CRC.h
# End Source File
# Begin Source File

SOURCE=.\Debug.h
# End Source File
# Begin Source File

SOURCE=.\DynArray.h
# End Source File
# Begin Source File

SOURCE=.\EBMLTree.h
# End Source File
# Begin Source File

SOURCE=.\EBMLTreeDlg.h
# End Source File
# Begin Source File

SOURCE=.\ecc.h
# End Source File
# Begin Source File

SOURCE=.\EnhancedListBox.h
# End Source File
# Begin Source File

SOURCE=.\FILE_INFO.h
# End Source File
# Begin Source File

SOURCE=.\FileDialogs.h
# End Source File
# Begin Source File

SOURCE=.\FillASIs.h
# End Source File
# Begin Source File

SOURCE=..\FormatInt64.h
# End Source File
# Begin Source File

SOURCE=.\FormatText.h
# End Source File
# Begin Source File

SOURCE=..\FormatTime.h
# End Source File
# Begin Source File

SOURCE=.\Global.h
# End Source File
# Begin Source File

SOURCE=.\IncResource.h
# End Source File
# Begin Source File

SOURCE=.\Languages.h
# End Source File
# Begin Source File

SOURCE=.\MessageLists.h
# End Source File
# Begin Source File

SOURCE=.\Mode2Form2Reader.h
# End Source File
# Begin Source File

SOURCE=.\multimedia_source.h
# End Source File
# Begin Source File

SOURCE=.\Muxing.h
# End Source File
# Begin Source File

SOURCE=.\OGGFile.h
# End Source File
# Begin Source File

SOURCE=.\ProgressList.h
# End Source File
# Begin Source File

SOURCE=.\ProtocolListCtrl.h
# End Source File
# Begin Source File

SOURCE=.\ResizeableDialog.h
# End Source File
# Begin Source File

SOURCE=.\RIFFChunkTreeDlg.h
# End Source File
# Begin Source File

SOURCE=.\RIFFFile.h
# End Source File
# Begin Source File

SOURCE=.\SetFramerateDlg.h
# End Source File
# Begin Source File

SOURCE=.\SetMainAVIHeaderFlagsDlg.h
# End Source File
# Begin Source File

SOURCE=.\SetStoreFileOptionsDlg.h
# End Source File
# Begin Source File

SOURCE=.\Silence.h
# End Source File
# Begin Source File

SOURCE=.\SourceFileListBox.h
# End Source File
# Begin Source File

SOURCE=.\SplitPointList.h
# End Source File
# Begin Source File

SOURCE=.\SplitPointsDlg.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\Streams.h
# End Source File
# Begin Source File

SOURCE=.\Strings.h
# End Source File
# Begin Source File

SOURCE=.\SubTitles.h
# End Source File
# Begin Source File

SOURCE=..\Tags.h
# End Source File
# Begin Source File

SOURCE=.\TextFiles.h
# End Source File
# Begin Source File

SOURCE=.\TransformFilter.h
# End Source File
# Begin Source File

SOURCE=.\Trees.h
# End Source File
# Begin Source File

SOURCE=..\Types.h
# End Source File
# Begin Source File

SOURCE=.\UnicodeBase.h
# End Source File
# Begin Source File

SOURCE=..\UnicodeCalls.h
# End Source File
# Begin Source File

SOURCE=.\UnicodeListControl.h
# End Source File
# Begin Source File

SOURCE=.\UnicodeTreeCtrl.h
# End Source File
# Begin Source File

SOURCE="..\utf-8.h"
# End Source File
# Begin Source File

SOURCE=.\UTF8Windows.h
# End Source File
# Begin Source File

SOURCE=.\Version.h
# End Source File
# Begin Source File

SOURCE=.\VideoInformationDlg.h
# End Source File
# Begin Source File

SOURCE=.\VideoInformationDlgListbox.h
# End Source File
# Begin Source File

SOURCE=.\VideoInformationDlgTypes.h
# End Source File
# Begin Source File

SOURCE=.\VideoSourceListBox.h
# End Source File
# Begin Source File

SOURCE=.\WAVFile.h
# End Source File
# Begin Source File

SOURCE=..\XML.H
# End Source File
# Begin Source File

SOURCE=.\XMLFiles.h
# End Source File
# End Group
# Begin Group "Ressourcendateien"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\AVIMux_GUI.ico
# End Source File
# Begin Source File

SOURCE=.\res_deu\res\AVIMux_GUI.ico
# End Source File
# Begin Source File

SOURCE=.\res_eng\res\AVIMux_GUI.ico
# End Source File
# Begin Source File

SOURCE=.\res\bitmap1.bmp
# End Source File
# End Group
# Begin Group "Matroska - Code"

# PROP Default_Filter "cpp"
# Begin Source File

SOURCE=..\EBML.CPP
# End Source File
# Begin Source File

SOURCE=..\EBML_matroska.cpp
# End Source File
# Begin Source File

SOURCE=..\GenerateUIDs.cpp
# End Source File
# Begin Source File

SOURCE=..\Integers.cpp
# End Source File
# Begin Source File

SOURCE=..\Matroska.cpp
# End Source File
# Begin Source File

SOURCE=..\Matroska_Block.cpp
# End Source File
# Begin Source File

SOURCE=..\Matroska_Clusters.cpp
# End Source File
# Begin Source File

SOURCE=..\Matroska_Segment.cpp
# End Source File
# Begin Source File

SOURCE=..\Matroska_Writing.cpp
# End Source File
# Begin Source File

SOURCE=..\Queue.cpp
# End Source File
# Begin Source File

SOURCE=..\Warnings.cpp
# End Source File
# End Group
# Begin Group "Matroska - Header"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=..\EBML.H
# End Source File
# Begin Source File

SOURCE=..\EBML_IDs.h
# End Source File
# Begin Source File

SOURCE=..\EBML_matroska.h
# End Source File
# Begin Source File

SOURCE=..\GenerateUIDs.h
# End Source File
# Begin Source File

SOURCE=..\Integers.h
# End Source File
# Begin Source File

SOURCE=..\Matroska.h
# End Source File
# Begin Source File

SOURCE=..\Matroska_Block.h
# End Source File
# Begin Source File

SOURCE=..\Matroska_Clusters.h
# End Source File
# Begin Source File

SOURCE=..\Matroska_IDs.h
# End Source File
# Begin Source File

SOURCE=..\Matroska_Segment.h
# End Source File
# Begin Source File

SOURCE=..\Matroska_writing.h
# End Source File
# Begin Source File

SOURCE=..\Queue.h
# End Source File
# Begin Source File

SOURCE=..\Warnings.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\fastintops.asm

!IF  "$(CFG)" == "AVIMux_GUI - Win32 Release"

# Begin Custom Build
InputPath=..\fastintops.asm
InputName=fastintops

"$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /Cx /coff /nologo /c ..\$(InputName).asm

# End Custom Build

!ELSEIF  "$(CFG)" == "AVIMux_GUI - Win32 Debug"

# Begin Custom Build
InputPath=..\fastintops.asm
InputName=fastintops

"$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml /Cx /coff /nologo /c ..\$(InputName).asm

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# Begin Source File

SOURCE=.\VideoSource_generic.asp
# End Source File
# End Target
# End Project
