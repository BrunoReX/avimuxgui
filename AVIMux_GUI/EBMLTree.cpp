// EBMLTree.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "AVIMux_GUI.h"
#include "EBMLTree.h"
#include "..\EBML.h"
#include "formattext.h"
#include "..\utf-8.h"
#include "..\matroska.h"
#include "..\integers.h"
#include "UnicodeTreeCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEBMLTree

CEBMLTree::CEBMLTree()
{
	EnableAutomation();
}

CEBMLTree::~CEBMLTree()
{
}

void CEBMLTree::OnFinalRelease()
{
	// Nachdem die letzte Referenz auf ein Automatisierungsobjekt freigegeben wurde,
	// wird OnFinalRelease aufgerufen. Die Basisklasse löscht das Objekt
	// automatisch. Fügen Sie zusätzlichen Bereinigungscode für Ihr Objekt
	// hinzu, bevor Sie die Basisklasse aufrufen.

	CUnicodeTreeCtrl::OnFinalRelease();
}


BEGIN_MESSAGE_MAP(CEBMLTree, CUnicodeTreeCtrl)
	//{{AFX_MSG_MAP(CEBMLTree)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CEBMLTree, CUnicodeTreeCtrl)
	//{{AFX_DISPATCH_MAP(CEBMLTree)
		// HINWEIS - Der Klassen-Assistent fügt hier Zuordnungsmakros ein und entfernt diese.
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// Hinweis: Wir stellen Unterstützung für IID_IEBMLTree zur Verfügung, um typsicheres Binden
//  von VBA zu ermöglichen. Diese IID muss mit der GUID übereinstimmen, die in der
//  Disp-Schnittstelle in der .ODL-Datei angegeben ist.

// {37E48F2D-8DC7-4F40-9F03-10AE5F15E1A0}
static const IID IID_IEBMLTree =
{ 0x37e48f2d, 0x8dc7, 0x4f40, { 0x9f, 0x3, 0x10, 0xae, 0x5f, 0x15, 0xe1, 0xa0 } };

BEGIN_INTERFACE_MAP(CEBMLTree, CUnicodeTreeCtrl)
	INTERFACE_PART(CEBMLTree, IID_IEBMLTree, Dispatch)
END_INTERFACE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CEBMLTree 


void CEBMLTree::SetMode(int _iMode)
{
	iMode = _iMode;
	InvalidateRect(NULL);
}

void CEBMLTree::GetTextCallback(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	char*	c = pTVDispInfo->item.pszText;
	int i,j;

	if (pTVDispInfo->item.mask & TVIF_TEXT) {

		HTREEITEM hItem = pTVDispInfo->item.hItem;
		EBMLITEM_DESCRIPTOR* d = (EBMLITEM_DESCRIPTOR*)GetItemData(hItem);
		*c = 0;
		int ilen;
		if (d && d->pElement) {
			switch (d->pElement->GetType()) {
				case EBMLTYPE_UNKNOWN:
					c[0]=0;
					char x;
					strcat(c, "unknown ID ");
					d->pElement->SeekStream(-d->pElement->GetHeaderSize());
					d->pElement->GetSource()->Read(&x,1);
					ilen = VSizeInt_Len[unsigned char(x)];
					do {
						char cDigit[4];
						cDigit[0] = 0;
						sprintf(cDigit, "%02X ", (__int8)x & 0xFF);
						strcat(c, cDigit);
						d->pElement->GetSource()->Read(&x,1);
					} while (--ilen);
					break;
				default:
					strcpy(c,d->pElement->GetTypeString());
			}
			strcat(c," (");
			char b[256]; 
			ZeroMemory(b,sizeof(b));
			
			EBMLElement* e = d->pElement;
			QW2Str(d->iHeaderSize, b, 1);
			strcat(c, "header: ");
			strcat(c, b);
			strcat(c, " bytes, data: ");
			
			if (e->IsLengthUndefined())
				strcpy(b, "<undefined>");
			else
				QW2Str(e->GetLength(),b,1);
			
			strcat(c,b);
			strcat(c," bytes, pos.: ");
			QW2Str((!iMode)?d->iRelPosition:d->iItemPosition, b, 1);
			strcat(c,b);
			strcat(c,"): ");
			ZeroMemory(b,sizeof(b));

			switch (d->pElement->GetDataType()) {
				case EBMLDATATYPE_MASTER:
					strcpy(b,"master");
					break;
				case EBMLDATATYPE_BIN:
					strcpy(b,"binary");
					break;
				case EBMLDATATYPE_INT:
					QW2Str(e->GetData()->AsBSWInt(),b,1);
					break;
				case EBMLDATATYPE_SINT:
					QW2Str(FSSInt2Int(e->GetData()),b,1);
					break;
				case EBMLDATATYPE_FLOAT:
					sprintf(b,"%7.2f",e->AsFloat());
					break;
				case EBMLDATATYPE_ASCII:
					strcpy(b,e->GetData()->AsString());
					break;
				case EBMLDATATYPE_UTF8:
					strcpy(b,e->GetData()->AsString());
					break;
				case EBMLDATATYPE_HEX:
					j=0;
					if (d->pElement->GetType() == IDVALUE(MID_MS_SEEKID)) {
						for (i=0;i<MID_4BYTEDESCR_COUNT;i++) {
							if (Comp_EBMLIDs(e->GetData()->AsString(),
								MID_4BYTEDESCR[i].cID)) {
								sprintf(b+strlen(b)," %s ",MID_4BYTEDESCR[i].cName);
								j=1;								
							}
						}
					} 
					if (!j) {
						for (i=0;i<e->GetLength();i++) {
							sprintf(b+strlen(b),"%02X ",(e->GetData()->AsString()[i])&0xFF);
						}
					}
					

					break;

			}
			strcat(c,b);

			if (e->IsMaster()) {
				if (e->CheckCRC() == EBML_CRC_OK) {
					strcat(c, " (CRC32 passed)");
				} else
				if (e->Verify()) {
					strcat(c, " (parent CRC32 passed)");
				}
				if (e->CheckCRC() == EBML_CRC_FAILED) {
					strcat(c, " (CRC32 failed!)");
				}
			}			
		}
	}

	
	*pResult = 0;
}
