#pragma once

#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <thread>

#if defined(DEBUG_ENABLED) || defined(TOOLS_ENABLED)

#define DEBUG_CRASH() CRASH_NOW()
#define DEBUG_ASSERT(m_cond, m_msg) CRASH_COND_MSG(!(m_cond), m_msg)
#define DEBUG_ONLY(m_cond) m_cond

#define DEBUG_PRINT_INFO(m_msg) godot::UtilityFunctions::print(m_msg)
#define DEBUG_PRINT_WARN(m_msg) WARN_PRINT_ED(m_msg)
#define DEBUG_PRINT_ERROR(m_msg) ERR_PRINT_ED(m_msg)

#define RUNTIME_PRINT_INFO(m_msg) DEBUG_PRINT_INFO(m_msg)
#define RUNTIME_PRINT_WARN(m_msg) DEBUG_PRINT_WARN(m_msg)
#define RUNTIME_PRINT_ERROR(m_msg) DEBUG_PRINT_ERROR(m_msg)

#define DEBUG_THREAD_CHECK

#else // DEBUG

#define DEBUG_CRASH()
#define DEBUG_ASSERT(m_cond, m_msg)
#define DEBUG_ONLY(m_cond)

#define DEBUG_PRINT_INFO(m_msg)
#define DEBUG_PRINT_WARN(m_msg)
#define DEBUG_PRINT_ERROR(m_msg)
#define DEBUG_PRINT_CRASH(m_msg)

#define RUNTIME_PRINT_INFO(m_msg) godot::UtilityFunctions::print(m_msg)
#define RUNTIME_PRINT_WARN(m_msg) WARN_PRINT(m_msg)
#define RUNTIME_PRINT_ERROR(m_msg) ERR_PRINT(m_msg)

#endif // DEBUG

#if defined(DEBUG_THREAD_CHECK)
struct DebugThreadChecker
{
public:
	DebugThreadChecker(const void* object, bool write);
	~DebugThreadChecker();

private:
	const void* m_object;
	bool m_write;
};

#define CONCAT_INNER(a, b) a ## b
#define CONCAT(a, b) CONCAT_INNER(a, b)

// Macros that can be used to help fix multithreading issues
#define DEBUG_THREAD_CHECK_READ(object) DebugThreadChecker CONCAT(threadcheck, __LINE__)(static_cast<const void*>(object), false)
#define DEBUG_THREAD_CHECK_WRITE(object) DebugThreadChecker CONCAT(threadcheck, __LINE__)(static_cast<const void*>(object), true)
#else
#define DEBUG_THREAD_CHECK_READ(object)
#define DEBUG_THREAD_CHECK_WRITE(object)
#define DEBUG_THREAD_CHECK_SYNC(group)
#endif // DEBUG_THREAD_CHECK