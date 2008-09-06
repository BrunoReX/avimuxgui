#include "stdafx.h"
#include "AVIMux_GUIDlg.h"

void CAVIMux_GUIDlg::SetDialogState_Config()
{
	iCurrentView=1;

	// shows source file, video source etc
	m_Protocol.ShowWindow(SW_HIDE);
	m_Protocol_Label.ShowWindow(SW_HIDE);
	m_Progress_Group.ShowWindow(SW_HIDE);
	m_Prg_Dest_File.ShowWindow(SW_HIDE);
	m_Prg_Dest_File_Label.ShowWindow(SW_HIDE);
	m_Prg_Frames.ShowWindow(SW_HIDE);
	m_Prg_Frames_Label.ShowWindow(SW_HIDE);
	m_Prg_Legidx_Label.ShowWindow(SW_HIDE);
	m_Prg_Legidx_Progress.ShowWindow(SW_HIDE);
	m_Prg_Progress.ShowWindow(SW_HIDE);
	m_Prg_Progress_Label.ShowWindow(SW_HIDE);
	m_Progress_List.ShowWindow(SW_HIDE);

	m_OutputResolution.ShowWindow(SW_SHOW);
	m_OutputResolution_Label.ShowWindow(SW_SHOW);
	m_Output_Options_Button.ShowWindow(SW_SHOW);
	m_AudioName.ShowWindow(SW_HIDE);

// restore visibility of m_Audiodelay
	HTREEITEM h = m_StreamTree.GetSelectedItem();
	m_StreamTree.SelectItem(0);
	m_StreamTree.SelectItem(h);
	
	m_Audio_Label.ShowWindow(SW_SHOW);

	m_No_Audio.ShowWindow(SW_SHOW);
	m_All_Audio.ShowWindow(SW_SHOW);
	m_No_Subtitles.ShowWindow(SW_SHOW);
	m_All_Subtitles.ShowWindow(SW_SHOW);

// selecting audio stream by number is hidden
	m_Default_Audio.ShowWindow(SW_HIDE);
	m_Default_Audio_Label.ShowWindow(SW_HIDE);

	
	//	m_AvailableStreams_Header.ShowWindow(SW_SHOW);
	m_SourceFiles.ShowWindow(SW_SHOW);
	m_Add_Video_Source.ShowWindow(SW_SHOW);
	m_Open_Files_Label.ShowWindow(SW_SHOW);

	::ShowWindow(hTitleEdit, SW_SHOW);

	m_Title_Label.ShowWindow(SW_SHOW);
	m_StreamTree.ShowWindow(SW_SHOW);
	m_Subtitles_Label.ShowWindow(SW_SHOW);

	if (sfOptions.bB0rk) {
		m_VideoStretchFactor.ShowWindow(SW_SHOW);
		m_VideoStretchFactor_Label.ShowWindow(SW_SHOW);
	} else {
		m_VideoStretchFactor.ShowWindow(SW_HIDE);
		m_VideoStretchFactor_Label.ShowWindow(SW_HIDE);
	}

	m_Chapter_Editor.ShowWindow(SW_SHOW);
}

void CAVIMux_GUIDlg::SetDialogState_Muxing()
{
	iCurrentView=2;

	// displays protocol and progress when muxing
	m_Protocol.ShowWindow(SW_SHOW);
	m_Protocol_Label.ShowWindow(SW_SHOW);
	m_Progress_Group.ShowWindow(SW_SHOW);
	m_Prg_Dest_File.ShowWindow(SW_SHOW);
	m_Prg_Dest_File_Label.ShowWindow(SW_SHOW);
	m_Prg_Frames.ShowWindow(SW_SHOW);
	m_Prg_Frames_Label.ShowWindow(SW_SHOW);
	m_Prg_Legidx_Label.ShowWindow(SW_SHOW);
	m_Prg_Legidx_Progress.ShowWindow(SW_SHOW);
	m_Prg_Progress.ShowWindow(SW_SHOW);
	m_Prg_Progress_Label.ShowWindow(SW_SHOW);
	m_Progress_List.ShowWindow(SW_SHOW);
	
	m_OutputResolution.ShowWindow(SW_HIDE);
	m_OutputResolution_Label.ShowWindow(SW_HIDE);
	m_Output_Options_Button.ShowWindow(SW_HIDE);
	m_AudioName.ShowWindow(SW_HIDE);
	m_Stream_Lng_Label.ShowWindow(SW_HIDE);
	m_Audiodelay.ShowWindow(SW_HIDE);
	m_Audiodelay_Label.ShowWindow(SW_HIDE);
	m_Audiodelay.ShowWindow(SW_HIDE);

	m_Audio_Label.ShowWindow(SW_HIDE);

	m_No_Audio.ShowWindow(SW_HIDE);
	m_All_Audio.ShowWindow(SW_HIDE);
	m_No_Subtitles.ShowWindow(SW_HIDE);
	m_All_Subtitles.ShowWindow(SW_HIDE);
	m_Default_Audio.ShowWindow(SW_HIDE);
	m_Default_Audio_Label.ShowWindow(SW_HIDE);
	m_SourceFiles.ShowWindow(SW_HIDE);
	m_Add_Video_Source.ShowWindow(SW_HIDE);
	m_Open_Files_Label.ShowWindow(SW_HIDE);

	::ShowWindow(hTitleEdit, SW_HIDE);

	m_Title_Label.ShowWindow(SW_HIDE);
	m_StreamTree.ShowWindow(SW_HIDE);
	m_Stream_Lng.ShowWindow(SW_HIDE);
	m_Subtitles_Label.ShowWindow(SW_HIDE);

	m_VideoStretchFactor.ShowWindow(SW_HIDE);
	m_VideoStretchFactor_Label.ShowWindow(SW_HIDE);

	m_Chapter_Editor.ShowWindow(SW_HIDE);
}
