/*

	not yet used.

  */

#ifndef I_TRANSFORMFILTER
#define I_TRANSFORMFILTER

typedef struct
{
	int					iRefCount;
	void*				source;
} TRANSFORMFILTER_INFO;

class TRANSFORMFILTER
{
	private:
		TRANSFORMFILTER_INFO	info;
	public:
		TRANSFORMFILTER();
		void	IncRefCount();
		int		DecRefCount();
		void	Connect(TRANSFORMFILTER* filter);
		void	Disconnect();
		int		virtual Close();

};




#endif