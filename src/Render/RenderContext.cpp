#include "RenderContext.h"

namespace voxel_game::rendering
{
    thread_local ThreadContext* thread_context = nullptr;

    void SetContext(ThreadContext& context)
    {
        thread_context = &context;
    }

    ThreadContext& GetContext()
    {
        DEBUG_ASSERT(thread_context != nullptr, "This thread doesn't have a context yet");
        return *thread_context;
    }
}