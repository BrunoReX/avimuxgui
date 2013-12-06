/* Very old code that needs to be done using STL */

#ifndef I_DYNARRAY
#define I_DYNARRAY

#include <vector>

class CDynArray
{
	protected:
		int		iElementSize;
		int		iCount;
		int		iMaxCount;
	public:
		CDynArray();
		~CDynArray();
		int		GetCount();
};


class CDynIntArray: public CDynArray
{
	protected:
		int*	pData;
		
	public:
		CDynIntArray();
		int		At(int iIndex);
		void    Delete(int iIndex);
		void	DeleteAll(void);
		int		Insert(int iItem);
		int		Insert(int* iItems, int iCount);
		void	Set(int iIndex, int iItem);
		int		Find(int iItem);
		CDynIntArray* Duplicate(int count = -2);
		std::vector<int> GetVector()
		{
			std::vector<int> result;
			for (int j=0; j<GetCount(); j++)
			{
				result.push_back(pData[j]);
			}
			return result;
		}
};



#endif