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
void DebugThreadCheckRead(const void* group, const void* object);
void DebugThreadCheckWrite(const void* group, const void* object);
void DebugThreadCheckSync(const void* group);

// Macros that can be used to help fix multithreading issues
#define DEBUG_THREAD_CHECK_READ(group, object) DebugThreadCheckRead(static_cast<const void*>(group), static_cast<const void*>(object))
#define DEBUG_THREAD_CHECK_WRITE(group, object) DebugThreadCheckWrite(static_cast<const void*>(group), static_cast<const void*>(object))
#define DEBUG_THREAD_CHECK_SYNC(group) DebugThreadCheckSync(static_cast<const void*>(group))
#else
#define DEBUG_THREAD_CHECK_READ(group, object)
#define DEBUG_THREAD_CHECK_WRITE(group, object)
#define DEBUG_THREAD_CHECK_SYNC(group)
#endif // DEBUG_THREAD_CHECK