#ifndef I_MEMORYGUARD
#define I_MEMORYGUARD

template<class T>
void delete_ptr(T* ptr)
{
	delete ptr;
}

template<class T>
void delete_array(T* ptr)
{
	delete[] ptr;
}

template<class T, class V, void U(V*)>
class Finalizer
{
private:
	T* m_ptr;
public:
	Finalizer(T* ptr)
	{
		m_ptr = ptr;
	}

	~Finalizer()
	{
		(*U)((V*)m_ptr);
	}
};

#endif