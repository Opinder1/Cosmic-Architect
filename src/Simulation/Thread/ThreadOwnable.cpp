#include "ThreadOwnable.h"

#include "Util/Debug.h"

namespace sim
{
    ThreadOwnable::ThreadOwnable()
    {}

    ThreadOwnable::~ThreadOwnable()
    {
        DEBUG_ASSERT(!ObjectOwned(), "This object should not be owned when destroying it");
    }

    void ThreadOwnable::ThreadClaimObject()
    {
        DEBUG_ASSERT(!ObjectOwned(), "This object should not be owned when claiming it");

        m_owner_id = std::this_thread::get_id(); // This thread now owns 
    }

    void ThreadOwnable::ThreadReleaseObject()
    {
        DEBUG_ASSERT(ThreadOwnsObject(), "This object should be owned when releasing it");

        m_owner_id = std::thread::id{};
    }

    void ThreadOwnable::ThreadTransferObject(std::thread::id thread)
    {
        DEBUG_ASSERT(ThreadOwnsObject(), "This object should be owned when releasing it");

        m_owner_id = thread;
    }

    bool ThreadOwnable::ObjectOwned() const
    {
        return m_owner_id != std::thread::id();
    }

    bool ThreadOwnable::ThreadOwnsObject() const
    {
        if (ObjectOwned())
        {
            return m_owner_id == std::this_thread::get_id();
        }
        else
        {
            return true;
        }
    }

    std::thread::id ThreadOwnable::GetOwnerID() const
    {
        return m_owner_id;
    }
}