#pragma once

#include "..\basestreams.h"
// CHexViewListBox

#define HWLB_MODE_EBML 0x01
#define HWLB_MODE_RIFF 0x02

class CHexViewListBox : public CListBox
{
	DECLARE_DYNAMIC(CHexViewListBox)
private:
	int		mode;
	STREAM* stream;
	bool	bisrifflist;
	__int64	first_pos;
	__int64	bytes_per_line;

	__int64	red_begin;
	__int64 red_end;
	__int64 blue_begin;
	__int64 blue_end;
	__int64 green_begin;
	__int64 green_end;

	BYTE	data[16384];
	int		data_present;
	__int64 data_begin;

public:
	CHexViewListBox();
	virtual ~CHexViewListBox();
	virtual void OnFinalRelease();
	void SetDataSource(STREAM* _stream);
	void SetNewStartPos(__int64 new_pos);
	void SetRange(__int64 range);
	void SetMode(int _mode);

protected:
	DECLARE_MESSAGE_MAP()
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
public:
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);
};


