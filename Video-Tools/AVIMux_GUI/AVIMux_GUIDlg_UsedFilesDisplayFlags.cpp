#include "stdafx.h"
#include "AVIMux_GUIDlg.h"

void CAVIMux_GUIDlg::ResetAllUsedFilesDisplayFlag()
{
	SetUsedFilesDisplayFlag(NULL);

	HTREEITEM hItem = m_StreamTree.GetRootItem();
	while (hItem) {
		SetUsedFilesDisplayFlag(m_StreamTree.GetItemInfo(hItem)->pMSI);
		hItem = m_StreamTree.GetNextSiblingItem(hItem);
	}

}

void CAVIMux_GUIDlg::SetUsedFilesDisplayFlag(MULTIMEDIA_STREAM_INFO* msi)
{
	bool bLowLight = !!settings->GetInt("gui/main_window/source_files/lowlight");

	if (!bLowLight) {
		m_SourceFiles.SetFlag(0, 0, FILEINFO_FLAG0_DEEMPH, false, true);
		return;
	}

	if (!msi)
		return;

	for (DWORD j = 1; j <= msi->lpdwFiles[0]; j++) {
		m_SourceFiles.SetFlag(m_SourceFiles.FileID2Index(msi->lpdwFiles[j]), 0, 
			FILEINFO_FLAG0_DEEMPH, true, false); 
	}
}