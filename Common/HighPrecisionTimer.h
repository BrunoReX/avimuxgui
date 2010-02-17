/** \file HighPrecisionTimer.h This file declares a high precision timer.
 *
 * See CHighPrecisionTimer for more details.
 */

#ifndef I_HIGHPRECTIMER
#define I_HIGHPRECTIMER

#include "windows.h"

/** \brief This class provides a high precision timer.
 *
 * A high precision timer is using the Windows API functions 
 * QueryPerformanceFrequency and QueryPerformanceCounter. As a consequence,
 * the maximum precision of such a timer is hardware dependend.
 *
 * Usually, the high precision timer is used the following way:
 * \code
 * char* cDuration;
 * CHighPrecisionTimer timer1(true);
 * ...
 * timer1.Stop();
 * cDuration = strdup(timer1);
 * \endcode
 */
class CHighPrecisionTimer
{
private:
	LARGE_INTEGER Frequency;
	LARGE_INTEGER StartTime;
	LARGE_INTEGER StopTime;
	bool bStarted;
	bool bStopped;
	mutable char string[32];
public:
	/** \brief Initialize a high precision timer.
	 *
	 * This constructor initializes a high precision timer, but does not start
	 * it. A subsequent call to Start() is required.
	 */
	CHighPrecisionTimer();

	/** \brief Initialize a high precision timer and start it.
	 *
	 * \param DoStart When true, the high precision timer is immediately started.
	 */
	CHighPrecisionTimer(bool DoStart);

	/** \brief Start the high precision timer.
	 */
	void Start();

	/** \brief Stops the high precision timer.
	 *
	 * The timer must be stopped before the time that has been passed
	 * since it was started can be rendered into a string.
	 * 
     * \remarks This function has no effect if the timer has never been started.
	 */
	void Stop();

	/** \brief Create a string from the time that has passed.
	 *
	 * This method creates a string from the time that has passed between starting
	 * and stopping the high precision timer.
	 * \param pDest Pointer to a buffer to receive the string
	 * \param max_len size of the buffer to receive the string
	 */	 
	void Format(char* pDest, int max_len) const;

	/** \brief Implicit conversion to char*
	 *
	 * This operator calls Format(char*,int), stores the result in an internal
	 * buffer and returns a pointer to this buffer. It stays valid as long as 
	 * the object is alive.
	 */
	operator char*() const; 
};

#endif
