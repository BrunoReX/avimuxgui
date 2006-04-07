#include "stdafx.h"
#include "xml.h"
#include "windows.h"

//////////////////////
//   XML.CPP        //
//////////////////////


// init an empty XML tree
void xmlInitTree(XMLNODE** pNode)
{
	*pNode = new XMLNODE;
	ZeroMemory(*pNode, sizeof(**pNode));
}

// init one node
XMLNODE* xmlSetNode(XMLNODE** pNode, char* cName, char* cValue)
{
	XMLNODE** ppTemp = pNode;
	
	xmlInitTree(ppTemp);
	(*ppTemp)->cNodeName = new char[1+strlen(cName)];
	strcpy((*ppTemp)->cNodeName, cName);
	(*ppTemp)->cValue = new char[1+strlen(cValue)];
	strcpy((*ppTemp)->cValue, cValue);

	return (*ppTemp);
}

// add a child to a childless node
XMLNODE* xmlAddFirstChild(XMLNODE* pParent, char* cName, char* cValue)
{
	return (xmlSetNode((XMLNODE**)&pParent->pChild, cName, cValue));
}

// add a child to a node
XMLNODE* xmlAddChild(XMLNODE* pParent, char* cName, char* cValue)
{
	XMLNODE** ppCurrent = (XMLNODE**)&pParent->pChild;
	if (!*ppCurrent) {
		return xmlAddFirstChild(pParent, cName, cValue);
	}  else {
		while ((*ppCurrent)->pNext) ppCurrent = (XMLNODE**)&(*ppCurrent)->pNext;
		return xmlSetNode((XMLNODE**)&(*ppCurrent)->pNext, cName, cValue);
	}
}

// add a sibling node
XMLNODE* xmlAddSibling(XMLNODE* pNode, char* cName, char* cValue)
{
	return xmlAddSibling(&pNode, cName, cValue);
}

XMLNODE* xmlAddSibling(XMLNODE** pNode, char* cName, char* cValue)
{
	XMLNODE** ppCurrent;
	
	if (*pNode) {
		ppCurrent = (XMLNODE**)&(*pNode)->pNext;
		while (*ppCurrent) {
			ppCurrent = (XMLNODE**)&(*ppCurrent)->pNext;
		}
	} else ppCurrent = pNode;

	xmlSetNode(ppCurrent, cName, cValue);

	return (*ppCurrent);
}

// store name of node to char*
int xmlTreeToString_putname(XMLNODE* pNode, char** pDest, int iMaxLen, int open)
{
	char* c = pNode->cNodeName;
	
	*(*pDest)++ = '<';
	if (!open) *(*pDest)++ = '/';

	while (*c) *(*pDest)++ = *c++;
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
		} else *(*pDest)++ = *c++;
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
	return xmlTreeToString_aux(pNode, &pDest, iMaxLen);
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

		delete (*ppNode)->cNodeName;
		delete (*ppNode)->cValue;
		delete *ppNode;
		*ppNode = NULL;
	}
}

char xmlReadChar(char** pcSourceText)
{
	char t[4];
	char c = *(*pcSourceText)++;
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
		while ((c=*(*pcSourceText)++)!='<') pDest[i++]=c;
	
		pDest[i++] = 0;
		*(*pcSourceText)--;
	}

	return 0;
}

int xmlIsTagOpening(char* pSourceText)
{
	while (*pSourceText == ' ') *pSourceText++;
	if (*pSourceText++ == '<' && *pSourceText++ != '/') return 1;
	return 0;
}

int xmlIsTagClosing(char* pSourceText)
{
	if (*pSourceText++ == '<' && *pSourceText++ == '/') return 1;
	return 0;
}

int xmlReadTagName(char** pcSourceText, int* piOpen, char* pName)
{
	while (**pcSourceText == ' ') *(*pcSourceText)++;
	char c = xmlReadChar(pcSourceText);
	
	if (c!='<') {
		*piOpen = -1;
		*(*pcSourceText)--;
		return 0;
	}

	if ((c = xmlReadChar(pcSourceText)) == '/') {
		*piOpen = 0;
	} else *piOpen = 1;

	int i = 0;
	if (*piOpen) pName[i++] = c;

	while ((c=xmlReadChar(pcSourceText)) != '>') {
		pName[i++]=c;
	}

	pName[i++]=0;

	return i;	
}

int xmlBuildTree_aux(XMLNODE** pDest, char** pcSourceText)
{
	char cName1[1024]; char cValue[1024]; char cName2[1024]; int iOpen;
	ZeroMemory(cValue,sizeof(cValue));

	XMLNODE* pChild = NULL;
	XMLNODE** ppChild = &pChild;
	XMLNODE* pFirstChild = pChild;

	if (xmlIsTagOpening(*pcSourceText)) {
		xmlReadTagName(pcSourceText, &iOpen, cName1);
        
		while (xmlIsTagOpening(*pcSourceText)) {
			xmlBuildTree_aux(ppChild, pcSourceText);
			if (!pFirstChild) pFirstChild = pChild;
			ppChild = (XMLNODE**)&(*ppChild)->pNext;
		}

		if (!xmlIsTagClosing(*pcSourceText)) xmlReadValue(pcSourceText, cValue);
		xmlReadTagName(pcSourceText, &iOpen, cName2);
		xmlSetNode(pDest, cName1, cValue);
		(*pDest)->pChild = pFirstChild;

		if (strcmp(cName1, cName2)) {
			return XMLERR_BADTAGNAME;
		}

	} else return XMLERR_TAGEXPECTED;

	return XMLERR_OK;
}  

int xmlBuildTree(XMLNODE** pDest, char* cSourceText)
{
	return xmlBuildTree_aux(pDest, &cSourceText);
}