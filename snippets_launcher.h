#pragma once
#include "beginthread_usage.h"
#include "beginthreadex_demo.h"

#include "create_thread_usage.h"

#include "mutex_snippet.h"
#include "critical_section_usage.h"

#include "w32_events_signaling.h"

#include "condition_variable_queue.h"
#include "conditional_variable_usage.h"

#include "slim_rw_lock.h"
#include "slim_rw_lock_cv.h"

namespace ConcurrencySnippetsLauncher
{
	int main()
	{
		// BeginThreadUsage::main();
		// BeginThreadExUsage::main();
		// ConditionVariableUsage::main();
		// ConditionVariableUsageQueue::main(); // Win32CV with C++ std::queue
		// CreateThreadUsage::main();
		// CriticalSectionUsage::main();
		Win32EventSignaling::main();
		// SlimRWLock::main();
		// SlimRWLockConditionVariable::main();
		// MutexUsage::main();


		return 0;
	}
}
