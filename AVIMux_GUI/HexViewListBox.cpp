// HexViewListBox.cpp : Implementierungsdatei
//

#include "stdafx.h"
#include "HexViewListBox.h"
#include ".\hexviewlistbox.h"
#include "..\integers.h"
#include "avistructs.h"

// CHexViewListBox

IMPLEMENT_DYNAMIC(CHexViewListBox, CListBox)
CHexViewListBox::CHexViewListBox()
{
	EnableAutomation();

	first_pos = 0;
	bytes_per_line = 16;
	data_begin = 0;
	mode = HWLB_MODE_EBML;
	bisrifflist = false;
}

CHexViewListBox::~CHexViewListBox()
{
}

void CHexViewListBox::OnFinalRelease()
{
	// Wenn der letzte Verweis auf ein Automatisierungsobjekt freigegeben wird,
	// wird OnFinalRelease aufgerufen. Die Basisklasse löscht das Objekt
	// automatisch. Fügen Sie zusätzlichen Bereinigungscode für Ihr Objekt
	// hinzu, bevor Sie die Basisklasse aufrufen.

	CListBox::OnFinalRelease();
}

void CHexViewListBox::SetDataSource(STREAM* _stream)
{
	stream = _stream;
	SetNewStartPos(0);
}

void CHexViewListBox::SetNewStartPos(__int64 new_pos)
{
	first_pos = new_pos;

	__int64 new_data_begin = (new_pos/16)*16;

	if (new_data_begin > 1024)
		new_data_begin -= 1024;
	else 
		new_data_begin = 0;

	if (abs((double)(new_data_begin - data_begin)) > 256.)
		data_begin = new_data_begin;

	stream->Seek(data_begin);
	data_present = stream->Read(data, 16384);

	InvalidateRect(NULL, false);

	int top_index = GetTopIndex();

	int item_height = GetItemHeight(0);
	RECT r;
	GetClientRect(&r);
	__int64 visible_items = (r.bottom - r.top) / item_height;
	__int64 last_visible = top_index + visible_items - 1;

	int must_be_visible = (int)((new_pos - data_begin)/bytes_per_line);
	int new_top_index = max(0, must_be_visible - 2);
	
	if (top_index > must_be_visible || must_be_visible > last_visible - 1)	
		SetTopIndex(new_top_index);
}



BEGIN_MESSAGE_MAP(CHexViewListBox, CListBox)
	ON_WM_DRAWITEM()
	ON_WM_CREATE()
END_MESSAGE_MAP()


BEGIN_DISPATCH_MAP(CHexViewListBox, CListBox)
END_DISPATCH_MAP()

// Hinweis: Wir stellen Unterstützung für IID_IHexViewListBox zur Verfügung, um typsicheres Binden 
//  von VBA zu unterstützen. Diese IID muss mit der GUID übereinstimmen, die bei der 
//  Disp-Schnittstelle in der .IDL-Datei angegeben ist.

// {AF8A4F8E-DC14-467B-8307-8A9915C0AC9E}
static const IID IID_IHexViewListBox =
{ 0xAF8A4F8E, 0xDC14, 0x467B, { 0x83, 0x7, 0x8A, 0x99, 0x15, 0xC0, 0xAC, 0x9E } };

BEGIN_INTERFACE_MAP(CHexViewListBox, CListBox)
	INTERFACE_PART(CHexViewListBox, IID_IHexViewListBox, Dispatch)
END_INTERFACE_MAP()


// CHexViewListBox-Meldungshandler


void CHexViewListBox::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{

}

void CHexViewListBox::SetRange(__int64 range)
{
	range /= 16;

	if (range > 256)
		range = 256;

	if (range < GetCount()) {
		for (int i=GetCount()-1; i>range; i--)
			DeleteString(GetCount()-1);
	} else {
		for (int i=GetCount(); i<range; i++)
			AddString("xxx");
	}
}

int CHexViewListBox::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CListBox::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Fügen Sie Ihren spezialisierten Erstellcode hier ein.

	return 0;
}

void CHexViewListBox::SetMode(int _mode)
{
	mode = _mode;
}

void CHexViewListBox::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	// TODO: Fügen Sie hier Ihren Meldungsbehandlungscode ein, und/oder benutzen Sie den Standard.

	__int64 start_file_pos = data_begin;//((first_pos / 16) * 16);

	if (lpDrawItemStruct->itemID < 0)
		lpDrawItemStruct->itemID = GetCurSel();

	// TODO: Code einfügen, um das angegebene Element zu zeichnen
	CDC	dc;
	LPDRAWITEMSTRUCT d = lpDrawItemStruct;

	char addr[32]; addr[0]=0;
	__int64 i64addr = start_file_pos + bytes_per_line * lpDrawItemStruct->itemID;
	_snprintf(addr, sizeof(addr), "0x %04X %08X  ", (__int32)(i64addr>>32), 
		(__int32)((i64addr) & 0xFFFFFFFF));

	int offset = (int)(bytes_per_line * lpDrawItemStruct->itemID);

    dc.Attach(d->hDC);
	COLORREF crOldBkColor = dc.GetBkColor();
	COLORREF bkColor;

	char* u = NULL;
	CFont* font = GetFont();

	LOGFONT logfont;
	font->GetLogFont(&logfont);

	COLORREF normal_color;
	
	if ((d->itemState & ODS_SELECTED)) {
		if (GetFocus() == this) {
			normal_color = GetSysColor(COLOR_HIGHLIGHTTEXT);
			bkColor = RGB(128,192,255);//(GetSysColor(COLOR_HIGHLIGHT));
		} else {
			normal_color = GetSysColor(COLOR_WINDOWTEXT);
			bkColor = GetSysColor(COLOR_BTNFACE);
		}
	} else {
		normal_color = GetSysColor(COLOR_WINDOWTEXT);
		bkColor = GetSysColor(COLOR_WINDOW);
	}

    dc.SetTextColor(normal_color);
	dc.SetTextAlign(TA_LEFT);
	dc.SetBkMode(TRANSPARENT);

	CFont* used_font = new CFont();
	strcpy(logfont.lfFaceName, "Courier New");
//	logfont.lfWeight = FW_BOLD;

	if (!used_font->CreateFontIndirect(&logfont)) {
		logfont.lfQuality = DEFAULT_QUALITY;
		if (!used_font->CreateFontIndirect(&logfont)) {
			GetParent()->GetFont()->GetLogFont(&logfont);
			used_font->CreateFontIndirect(&logfont);
		}
	}

	dc.SelectObject(used_font);
	used_font->DeleteObject();
	delete used_font;

	LOGBRUSH b;
	b.lbColor = bkColor;
	b.lbStyle = BS_SOLID;
	HBRUSH brush = CreateBrushIndirect(&b);

	LOGPEN p;
	p.lopnColor = 0;
	p.lopnStyle = PS_NULL;
	HPEN pen = CreatePenIndirect(&p);

	dc.SelectObject(brush);
	dc.SelectObject(pen);
	d->rcItem.right++;
	d->rcItem.bottom++;
	dc.Rectangle(&d->rcItem);

	int addr_len = strlen(addr);
	CSize size = dc.GetTextExtent(addr, addr_len);
	int addr_width = size.cx;

	size = dc.GetTextExtent("00 ", 3);
	int byte_width = size.cx;
	dc.TextOut(d->rcItem.left, d->rcItem.top, addr, strlen(addr));

	size = dc.GetTextExtent("0", 1);
	int char_width = size.cx;

	for (int j=0; j<bytes_per_line && j + offset < data_present; j++) {
		if (offset + j + start_file_pos == first_pos) {
			if (mode == HWLB_MODE_EBML) {
				if (red_begin != offset + j) {
					red_begin = offset + j;
					InvalidateRect(NULL, false);
					red_end = red_begin + VSizeInt_Len[data[offset+j]];
					blue_begin = red_end;
					blue_end = blue_begin + VSizeInt_Len[data[blue_begin]];
					green_begin = blue_end;
					__int64 k = 0;
					VSUInt2Int((char*)data + blue_begin, &k);
					green_end = green_begin + k;
				}
			}

			if (mode == HWLB_MODE_RIFF) {
				if (red_begin != offset + j) {
					red_begin = offset + j;
					InvalidateRect(NULL, false);
					red_end = red_begin + 4;
					blue_begin = red_end;
					blue_end = blue_begin + 4;
					green_begin = blue_end;
                    green_end = green_begin + *(DWORD*)((char*)data+blue_begin);
					if (green_end - green_begin >= 4 && (
						*(DWORD*)((char*)data+red_begin) == MKFOURCC('L','I','S','T') ||
						*(DWORD*)((char*)data+red_begin) == MKFOURCC('R','I','F','F')))
						bisrifflist = true;
					else
						bisrifflist = false;
				}
			}
		}

		char bt[8]; bt[0]=0;

		if (offset + j >= red_begin && offset + j < red_end)
			dc.SetTextColor(RGB(192,0,0));
		else 
		if (bisrifflist && offset + j - 8 >= red_begin && offset + j - 8< red_end)
			dc.SetTextColor(RGB(192,0,0));
		else
		if (offset + j >= blue_begin && offset + j < blue_end)
			dc.SetTextColor(RGB(0,0,255));
		else
		if (offset + j >= green_begin && offset + j < green_end)
			dc.SetTextColor(RGB(0,128,0));
		else
			dc.SetTextColor(normal_color);

		_snprintf(bt, sizeof(bt), "%02X ", data[offset+j]);
		dc.TextOut(addr_width + 20 + byte_width * j +
			(2 * byte_width/3 * !!(j>=8)) +
			(byte_width/3 * !!(j>=4)) +
			(byte_width/3 * !!(j>=12))
			, d->rcItem.top, bt, 3);

		if (data[offset+j] >= 32)
			sprintf(bt, "%c", data[offset+j]);
		else
			sprintf(bt, ".");
		
		dc.TextOut(addr_width + 20 + bytes_per_line * byte_width + j * (char_width)
			+ 6 * char_width, d->rcItem.top, bt, 1);
	}
	DeleteObject(brush);
	DeleteObject(pen);
	
	dc.Detach();
	free(u);
}
