#ifndef I_CHECKPOINT
#define I_CHECKPOINT

#include "Tracefile.h"

/** \brief This class represents a checkpoint for tracing. 
 *
 * The checkpoint mechanism allows to discard trace file entries if no noteworthy
 * even has occured between two specified points.
 *
 * Example: A checkpoint with TRACE_LEVEL_INFO was created. If no trace entry with
 * a higher trace level (like TRACE_LEVEL_WARN) is added before the checkpoint is 
 * passed, any trace entries made after constructing the checkpoint and before passing
 * it are discarded.
 *
 * \remarks 
 * - When a CCheckpoint object is constructed on the stack, its destructor
 * makes sure that the checkpoint is passed when leaving the area in which the object
 * is valid. 
 * - Checkpoints are attached to the thread that created them.
 * - Do not use checkpoints in a way that would lead to two checkpoints becoming
 * invalid at the same time.
 */
class CCheckpoint
{
private:
	bool m_passed;
	//bool m_normalTermination;
public:
	/** \brief Constructor.
	 *
	 * This constructor initializes a checkpoint item with the default trace level.
	 * \param TraceLevel Minimum trace level an even must have so that the trace entries
	 * added after constructing the checkpoint are not discarded.
	 */
	CCheckpoint() {
		GetApplicationTraceFile()->SetCheckpoint(GetApplicationTraceFile()->GetTraceLevel());
		m_passed = false;
//		m_normalTermination = false;
	}

	/** \brief Constructor.
	 *
	 * This constructor initializes a checkpoint item with a given trace level.
	 * \param TraceLevel Minimum trace level an event must have so that the trace entries
	 * added after constructing the checkpoint are not discarded.
	 */
	CCheckpoint(unsigned int TraceLevel) {
		GetApplicationTraceFile()->SetCheckpoint(TraceLevel);
		m_passed = false;
	}

	/*
	void TerminateNormal() {
		m_normalTermination = true;
	}
	*/

	virtual void Pass() {
		if (!m_passed) {
			GetApplicationTraceFile()->PassCheckpoint();
			m_passed = true;
		}
	}
	/** \brief Destructor
	 *
	 * When the destructor is called, the checkpoint is passed.
	 * \remarks Only the last checkpoint that was created can be passed. 
	 */
	virtual ~CCheckpoint() {
		Pass();
	}
};


#endif