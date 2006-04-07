#ifndef I_XML
#define I_XML

#include <vector>

const int XMLERR_BADTAGNAME = -1;
const int XMLERR_TAGEXPECTED= -2;
const int XMLERR_OK         = 1;

typedef struct
{
	char name[1024];
	char value[1024];
} XMLNODEATTRIBUTE;

typedef std::vector<XMLNODEATTRIBUTE> XMLNODEATTRIBUTES;

class XMLNODE 
{
public:
	XMLNODE();
	char*	cNodeName;  // name of node
	char*	cValue;     // value of node
	void*	pNext;      // next sibiling node on the same level
	void*   pChild;     // child node
	bool    bIsEmpty;
	XMLNODEATTRIBUTES	attributes;
};

XMLNODE*	xmlAddChild(XMLNODE* pParent, char* cName, char* cValue, bool bIsEmpty = false);
void		xmlAddAttribute(XMLNODE* pNode, char* cName, char* cValue);
XMLNODE*	xmlAddSibling(XMLNODE** pNode, char* cName, char* cValue, bool bIsEmpty = false);
XMLNODE*	xmlAddSibling(XMLNODE* pNode, char* cName, char* cValue, bool bIsEmpty = false);
int			xmlTreeToString(XMLNODE* pNode, char* pDest, int iMaxLen);
void		xmlDeleteNode(XMLNODE** ppNode);
void		xmlDeleteSimpleNode(XMLNODE** ppNode);

int			xmlBuildTree(XMLNODE** pDest, char* cSourceText);
void		xmlRemoveNext(XMLNODE* pDest);

typedef struct
{
	int		used;
	char*	cName1;
	char*   cName2;
} TOPO_SORT_ENTRY;

typedef struct
{
	int					iCount;
	TOPO_SORT_ENTRY**	entries;
} TOPO_SORT_LIST;

int			xmlTopoSort(XMLNODE* pSource, XMLNODE** pDest, TOPO_SORT_LIST* pTSL);

#endif