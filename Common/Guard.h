#ifndef I_MEMORY_GUARDS
#define I_MEMORY_GUARDS

typedef void (*DeleteProc)(void*);

/* this class wraps a pointer so that it is deleted on destruction of this object */

/** \brief This class wraps a pointer so that it is deleted
 * when the wrapper is destructed
 *
 * \param T Type of the pointer to wrap
 * \param V Type of the pointer to cast to when destructing
 * \param W Return type of the cleaup function
 * \param W (_stdcall *U)(V*) function to call on destruction
 */
template<class T, class V, class W, W (_stdcall* U)(V)>
class Guard
{
private:
	T* m_Value;
public:
	Guard(T* Stuff) {
		m_Value = Stuff;
	}

	T* Value() const {
		return m_Value;
	}

	virtual ~Guard() {
		U((V)*m_Value);
	}
};

template<class T, class V, class W, W (_stdcall* U)(V)>
class GuardDirect
{
private:
	T m_Value;
public:
	GuardDirect(T Stuff) {
		m_Value = Stuff;
	}

	T Value() const {
		return m_Value;
	}

	virtual ~GuardDirect() {
		U((V)m_Value);
	}
};

class CriticalSectionGuard : public GuardDirect<LPCRITICAL_SECTION, LPCRITICAL_SECTION, void, LeaveCriticalSection>
{
public:
	CriticalSectionGuard(LPCRITICAL_SECTION pcs) 
		: GuardDirect<LPCRITICAL_SECTION, LPCRITICAL_SECTION, void, LeaveCriticalSection>(pcs)
	{
		EnterCriticalSection(pcs);
	}
};

#endif