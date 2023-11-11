#pragma once

#include "Util/Nocopy.h"

#include <thread>
#include <atomic>

namespace sim
{
	// A helper to check thread access
	class ThreadOwnable : nomove
	{
	public:
		explicit ThreadOwnable();
		~ThreadOwnable();

		// Call to claim ownership
		void ThreadClaimObject();

		// Call to release ownership
		void ThreadReleaseObject();

		// Call to transfer ownership
		void ThreadTransferObject(std::thread::id thread);

		// Does this object have a thread set as the owner
		bool ObjectOwned() const;

		// Does this thread have ownership of this object. If there is no set owner then all threads are owners
		bool ThreadOwnsObject() const;

		// Get the thread id of the owner
		std::thread::id GetOwnerID() const;

	private:
		std::atomic<std::thread::id> m_owner_id;
	};
}