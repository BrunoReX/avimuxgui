#ifndef I_TAGS
#define I_TAGS

#include <vector>

typedef struct
{
	int		iTargetType;
	__int32 iTarget;
} TAG_TARGET;

typedef struct
{
	int			iCount;
	TAG_TARGET*	pTargets;
} TAG_TARGET_LIST;

typedef struct
{
	char*				cName;
	char*				cLanguage;
	int					iType;
	int					iRef;
	TAG_TARGET_LIST*	pTargets;
	int					iTargetType;
	union {
		char*	cData;
		void*	pData;
	};
} TAG;

typedef struct
{
	int		iCount;
	TAG**	pTags;
} TAG_LIST;

void DeleteTagList(TAG_LIST** tag_list);

typedef std::vector<int> TAG_INDEX_LIST;

/*typedef struct
{
	int		iCount;
	int*	pIndex;
} TAG_INDEX_LIST;
*/

#endif