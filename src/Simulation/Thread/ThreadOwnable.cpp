#include "ThreadOwnable.h"

#include <godot_cpp/core/error_macros.hpp>

namespace sim
{
    ThreadOwnable::ThreadOwnable()
    {}

    ThreadOwnable::~ThreadOwnable()
    {
        ERR_FAIL_COND_MSG(ObjectOwned(), "This object should not be owned when destroying it");
    }

    void ThreadOwnable::ThreadClaimObject()
    {
        ERR_FAIL_COND_MSG(ObjectOwned(), "This object should not be owned when claiming it");

        m_owner_id = std::this_thread::get_id(); // This thread now owns 
    }

    void ThreadOwnable::ThreadReleaseObject()
    {
        ERR_FAIL_COND_MSG(!ThreadOwnsObject(), "This object should be owned when releasing it");

        m_owner_id = std::thread::id{};
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