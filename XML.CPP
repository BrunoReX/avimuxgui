#include "stdafx.h"
#include "xml.h"
#include "windows.h"

#ifdef DEBUG_NEW
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

//////////////////////
//   XML.CPP        //
//////////////////////


// init an empty XML tree
void xmlInitTree(XMLNODE** pNode)
{
	*pNode = new XMLNODE;
}

XMLNODE::XMLNODE()
{
	cNodeName = NULL;
	cValue = NULL;
	pNext = NULL;
	pChild = NULL;
	bIsEmpty = true;
}

// init one node
XMLNODE* xmlSetNode(XMLNODE** pNode, char* cName, char* cValue, bool bIsEmpty = false)
{
	XMLNODE** ppTemp = pNode;
	
	xmlInitTree(ppTemp);
	(*ppTemp)->cNodeName = new char[1+strlen(cName)];
	strcpy((*ppTemp)->cNodeName, cName);
	(*ppTemp)->cValue = new char[1+strlen(cValue)];
	strcpy((*ppTemp)->cValue, cValue);
	(*ppTemp)->bIsEmpty = bIsEmpty;

	return (*ppTemp);
}

void xmlAddAttribute(XMLNODE* pNode, char* cName, char* cValue)
{
	if (!pNode)
		return;

	XMLNODEATTRIBUTE a;

	strncpy(a.name, cName, sizeof(a.name));
	strncpy(a.value, cValue, sizeof(a.value));

	pNode->attributes.push_back(a);
}

// add a child to a childless node
XMLNODE* xmlAddFirstChild(XMLNODE* pParent, char* cName, char* cValue, bool bIsEmpty)
{
	return (xmlSetNode((XMLNODE**)&pParent->pChild, cName, cValue, bIsEmpty));
}

// add a child to a node
XMLNODE* xmlAddChild(XMLNODE* pParent, char* cName, char* cValue, bool bIsEmpty)
{
	XMLNODE** ppCurrent = (XMLNODE**)&pParent->pChild;
	if (!*ppCurrent) {
		return xmlAddFirstChild(pParent, cName, cValue, bIsEmpty);
	}  else {
		while ((*ppCurrent)->pNext) ppCurrent = (XMLNODE**)&(*ppCurrent)->pNext;
		return xmlSetNode((XMLNODE**)&(*ppCurrent)->pNext, cName, cValue, bIsEmpty);
	}
}

// add a sibling node
XMLNODE* xmlAddSibling(XMLNODE* pNode, char* cName, char* cValue, bool bIsEmpty)
{
	return xmlAddSibling(&pNode, cName, cValue, bIsEmpty);
}

XMLNODE* xmlAddSibling(XMLNODE** pNode, char* cName, char* cValue, bool bIsEmpty)
{
	XMLNODE** ppCurrent;
	
	if (*pNode) {
		ppCurrent = (XMLNODE**)&(*pNode)->pNext;
		while (*ppCurrent) {
			ppCurrent = (XMLNODE**)&(*ppCurrent)->pNext;
		}
	} else ppCurrent = pNode;

	xmlSetNode(ppCurrent, cName, cValue, bIsEmpty);

	return (*ppCurrent);
}

// store name of node to char*
int xmlTreeToString_putname(XMLNODE* pNode, char** pDest, int iMaxLen, int open)
{
	char* c = pNode->cNodeName;
	
	*(*pDest)++ = '<';
	if (!open) *(*pDest)++ = '/';

	while (*c) *(*pDest)++ = *c++;

	if (open) for (size_t j=0; j<pNode->attributes.size(); j++) {
		XMLNODEATTRIBUTE& a = pNode->attributes[j];

		*(*pDest)++ = ' ';
		strcpy(*pDest, a.name); *pDest += strlen(*pDest);
		*(*pDest)++ = '=';
		*(*pDest)++ = '"';
		strcpy(*pDest, a.value); *pDest += strlen(*pDest);
		*(*pDest)++ = '"';
	}

	*(*pDest)++ = '>';

	return 1;
}

// add a line feed
int xmlTreeToString_putnewline(char **pDest, int iMaxLen)
{
	*(*pDest)++ = 13;
	*(*pDest)++ = 10;

	return 2;
}

// copy raw string to *pDest
int xmlPutRAW(char** pDest, char* source)
{
	while (*source) *(*pDest)++ = *source++; return 0;
}

// store value of node in *pDest. Use escape sequences if necessary
int xmlTreeToString_putvalue(XMLNODE* pNode, char** pDest, int iMaxLen)
{
	int i=0;
	char* c = pNode->cValue;
	while (*c) {
		if (*c == '&') {
			xmlPutRAW(pDest, "&amp;");
			i+=4;
		} else
		if (*c == '<') {
			xmlPutRAW(pDest, "&lt;");
			i+=3;
		} else
		if (*c == '>') {
			xmlPutRAW(pDest, "&gt;");
			i+=3;
		} else 
		if (*c == '"') {
			xmlPutRAW(pDest, "&quot;");
			i+=5;
		} else *(*pDest)++ = *c;
		c++;
		i++;
	}

	return i;
}

// store element in *pDest: <name><value><children></name>
int xmlTreeToString_putelement(XMLNODE* pNode, char** pDest, int iMaxLen)
{
	if (pNode) {
		xmlTreeToString_putname(pNode, pDest, iMaxLen, 1);
		if (pNode->pChild) xmlTreeToString_putnewline(pDest, iMaxLen);
		xmlTreeToString_putvalue(pNode, pDest, iMaxLen);
		xmlTreeToString_putelement((XMLNODE*)pNode->pChild, pDest, iMaxLen);
		xmlTreeToString_putname(pNode, pDest, iMaxLen, 0);
		xmlTreeToString_putnewline(pDest, iMaxLen);
		xmlTreeToString_putelement((XMLNODE*)pNode->pNext, pDest, iMaxLen);
	}

	return 0;
}

// convert XML tree to char* to be stored in a human-readable text file
int xmlTreeToString_aux(XMLNODE* pNode, char** pDest, int iMaxLen)
{
	return xmlTreeToString_putelement(pNode, pDest, iMaxLen);	
}

int xmlTreeToString(XMLNODE* pNode, char* pDest, int iMaxLen)
{
	char* c = pDest;

	int result = xmlTreeToString_aux(pNode, &pDest, iMaxLen);

	int len = strlen(c)-1;
	while (c[len--] == 0x20)
		c[len+1] = 0;

	return result;
}

// delete node
void xmlDeleteNode(XMLNODE** ppNode)
{
	if (*ppNode) {

		if ((*ppNode)->pNext) {
			xmlDeleteNode((XMLNODE**)&(*ppNode)->pNext);
		}

		if ((*ppNode)->pChild) {
			xmlDeleteNode((XMLNODE**)&(*ppNode)->pChild);
		}

		delete[] (*ppNode)->cNodeName;
		
		if ((*ppNode)->cValue)
			delete[] (*ppNode)->cValue;

		delete *ppNode;
		*ppNode = NULL;
	}
}

void xmlDeleteSimpleNode(XMLNODE** ppNode)
{
	if (*ppNode) {
		delete[] (*ppNode)->cNodeName;

		if ((*ppNode)->cValue)
			delete[] (*ppNode)->cValue;
		
		(*ppNode)->cValue = NULL;
		(*ppNode)->cNodeName = NULL;
	}
}

// remove next element
void xmlRemoveNext(XMLNODE* pDest)
{
	void* p;

	if (!pDest)
		return;

	if (!pDest->pNext) 
		return;

	xmlDeleteSimpleNode((XMLNODE**)&pDest->pNext);

	if (!(p=((XMLNODE*)pDest->pNext)->pNext)) {
		pDest->pNext = NULL;
		return;
	}

	pDest->pNext = p;
}


int FindMinumumElement(TOPO_SORT_LIST* ppList)
{
	TOPO_SORT_ENTRY *t;

	if (!ppList)
		return 0;

	if (!ppList->iCount)
		return 0;

	int		iresult = 0;
	char*   cresult = ppList->entries[0]->cName1;
	int     j = 0;

	while (j<ppList->iCount) {
		t = ppList->entries[j];

		if (t->used && !strcmp(cresult, t->cName2)) {
			cresult = t->cName1;
			iresult = j;
		};
		j++;
	}

	return iresult;
}

int DeleteMinimumElements(TOPO_SORT_LIST* ppList, char* cMin)
{
	TOPO_SORT_ENTRY *t;

	if (!ppList)
		return 0;

	if (!ppList->iCount)
		return 0;

	int j=0;

	while (j<ppList->iCount) {
		t = ppList->entries[j];

		if (t->used && !strcmp(t->cName1, cMin)) {
			ppList->iCount--;
			t->used = 0;
		}

		j++;
	}

	return -1;
}

int	xmlTopoSort(XMLNODE* pSource, XMLNODE** pDest, TOPO_SORT_LIST* ppList)
{
	XMLNODE* pTemp = NULL;
	XMLNODE* pLast = NULL;

	if (!ppList)
		return 0;

	char* cMin = NULL;

	while (ppList->iCount) {
		cMin = ppList->entries[FindMinumumElement(ppList)]->cName1;

		XMLNODE* pCurr = pSource;
		XMLNODE* pPrev = NULL;
		while (pCurr) {
			if (!strcmp(pCurr->cNodeName, cMin)) {
				xmlAddSibling(&pTemp, cMin, pCurr->cValue)->pChild = pCurr->pChild;
				if (!pTemp->pNext) 
					pLast = pTemp;
				else
					pLast = (XMLNODE*)pLast->pNext;

				if (!pPrev) 
					pSource = (XMLNODE*)pCurr->pNext;
				else
					xmlRemoveNext(pPrev);

			}
			pPrev = pCurr;
			pCurr = (XMLNODE*)pCurr->pNext;
		}

		DeleteMinimumElements(ppList, cMin);		
	}

	pLast->pNext = pSource;

	*pDest = pTemp;

	return 1;
}

char xmlReadChar(char** pcSourceText)
{
	char t[6];
	int i;
	char c = *(*pcSourceText)++;

/*	if (c == '<' && pSourceText[0] == '!' && pSourceText[1] == '-' && pSourceText[2] == '-') {

	}
*/
	if (c!='&') return c;
	
	t[0]=*(*pcSourceText)++;
	switch (t[0]) {
		case 'a':
			t[1] = *(*pcSourceText)++; 
			if (t[1]=='m') {
				t[2] = *(*pcSourceText)++;
				if (t[2] == 'p') {
					t[3] = *(*pcSourceText)++;
					if (t[3] == ';') {
						return '&';
					}
				}
			}
			return 0;
			break;
		case 'q':
			for (i=0;i<4;t[++i]=*(*pcSourceText)++);
			t[5] = 0;
			if (!strcmp(t, "quot;"))
				return '"';
			return 0;
			break;

		case 'l':
		case 'g':
			t[1] = *(*pcSourceText)++; 
			if (t[1]=='t') {
				t[2] = *(*pcSourceText)++;
				if (t[2] == ';') {
					return (t[0]=='l')?'<':'>';
				}
			}
			return 0;
			break;
		default: return 0;
	}
	
}

char* xmlReadValue(char** pcSourceText, char* pDest)
{
	char c; int i=0;
	if (*pcSourceText) {
		do {
			if ((**pcSourceText) != '<') {
				c = xmlReadChar(pcSourceText);
				pDest[i++] = c;
				c = ' ';
			} else {
				c = '<';
				(*pcSourceText)++;
			}
			//			if (c != '<') pDest[i++]=c;
		} while (c!='<');
		
		//while ((c=*(*pcSourceText)++)!='<') pDest[i++]=c;
	
		pDest[i++] = 0;
		*(*pcSourceText)--;
	}

	return 0;
}

int xmlIsTagOpening(char* pSourceText)
{
	while (*pSourceText <= ' ') *pSourceText++;
	if (*pSourceText++ == '<' && *pSourceText++ != '/') return 1;
	return 0;
}

int xmlIsTagClosing(char* pSourceText)
{
	while (*pSourceText <= ' ') *pSourceText++;
	if (*pSourceText++ == '<' && *pSourceText++ == '/') return 1;
	return 0;
}

int xmlReadTagName(char** pcSourceText, int* piOpen, char* pName, XMLNODEATTRIBUTES& a)
{
	while (**pcSourceText <= ' ') *(*pcSourceText)++;
	char c = xmlReadChar(pcSourceText);
	
	if (c!='<') {
		*piOpen = -1;
		*(*pcSourceText)--;
		return 0;
	}

	if ((c = xmlReadChar(pcSourceText)) == '/') {
		*piOpen = 0;
	} else {
		*piOpen = 1;
		a.clear();
	}

	int i = 0;
	if (*piOpen) pName[i++] = c;

	while ( (c=xmlReadChar(pcSourceText)) != '>' && c != ' ') {
		pName[i++]=c;
	}

	if (c == ' ' && *piOpen) {
		while ( (c=xmlReadChar(pcSourceText)) != '>' );
	}

	pName[i++]=0;

	return i;	
}

int xmlBuildTree_aux(XMLNODE** pDest, char** pcSourceText)
{
	char cName1[1024]; char cValue[1024]; char cName2[1024]; int iOpen;
	ZeroMemory(cValue,sizeof(cValue));
	ZeroMemory(cName1,sizeof(cName1));
	ZeroMemory(cName2,sizeof(cName2));

	XMLNODE* pChild = NULL;
	XMLNODE** ppChild = &pChild;
	XMLNODE* pFirstChild = pChild;

	if (xmlIsTagOpening(*pcSourceText)) {
		int tag_name_len = 0;
		XMLNODEATTRIBUTES	attribs;
		do {
			tag_name_len = xmlReadTagName(pcSourceText, &iOpen, cName1, attribs);
		} while (cName1[0]=='!');
        
		// item is a Flag?
		if (cName1[tag_name_len-2] == '/') {
			cName1[tag_name_len-2] = 0;
			xmlSetNode(pDest, cName1, "1", true);
			(*pDest)->pChild = pFirstChild;
		} else {
			// item is not a Flag
			while (xmlIsTagOpening(*pcSourceText)) {
				int res = xmlBuildTree_aux(ppChild, pcSourceText);
				if (res != XMLERR_OK)
					return res;

				if (!pFirstChild) pFirstChild = pChild;
				ppChild = (XMLNODE**)&(*ppChild)->pNext;
			}
	
			if (!xmlIsTagClosing(*pcSourceText)) xmlReadValue(pcSourceText, cValue);
			xmlReadTagName(pcSourceText, &iOpen, cName2, attribs);
			xmlSetNode(pDest, cName1, cValue);
			(*pDest)->pChild = pFirstChild;

			if (strcmp(cName1, cName2)) {
				return XMLERR_BADTAGNAME;
			}
		}
	} else return XMLERR_TAGEXPECTED;

	return XMLERR_OK;
}  

int xmlBuildTree(XMLNODE** pDest, char* cSourceText)
{
	return xmlBuildTree_aux(pDest, &cSourceText);
}