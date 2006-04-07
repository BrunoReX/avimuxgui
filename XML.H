#ifndef I_XML
#define I_XML

const int XMLERR_BADTAGNAME = -1;
const int XMLERR_TAGEXPECTED= -2;
const int XMLERR_OK         = 1;

typedef struct
{
	char*	cNodeName;  // name of node
	char*	cValue;     // value of node
	void*	pNext;      // next sibiling node on the same level
	void*   pChild;     // child node
} XMLNODE;

XMLNODE* xmlAddChild(XMLNODE* pParent, char* cName, char* cValue);
XMLNODE* xmlAddSibling(XMLNODE** pNode, char* cName, char* cValue);
XMLNODE* xmlAddSibling(XMLNODE* pNode, char* cName, char* cValue);
int xmlTreeToString(XMLNODE* pNode, char* pDest, int iMaxLen);
void xmlDeleteNode(XMLNODE** ppNode);

int xmlBuildTree(XMLNODE** pDest, char* cSourceText);

#endif