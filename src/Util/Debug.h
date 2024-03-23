#pragma once

#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#if defined(DEBUG_ENABLED) || defined(TOOLS_ENABLED)

#define DEBUG 1

#define DEBUG_CRASH() CRASH_NOW()
#define DEBUG_ASSERT(m_cond, m_msg) CRASH_COND_MSG(!(m_cond), m_msg)
#define DEBUG_ONLY(m_cond) m_cond

#define DEBUG_PRINT_INFO(m_msg) godot::UtilityFunctions::print(m_msg)
#define DEBUG_PRINT_WARN(m_msg) WARN_PRINT_ED(m_msg)
#define DEBUG_PRINT_ERROR(m_msg) ERR_PRINT_ED(m_msg)

#define RUNTIME_PRINT_INFO(m_msg) DEBUG_PRINT_INFO(m_msg)
#define RUNTIME_PRINT_WARN(m_msg) DEBUG_PRINT_WARN(m_msg)
#define RUNTIME_PRINT_ERROR(m_msg) DEBUG_PRINT_ERROR(m_msg)

#else // Runtime

#define DEBUG 0

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

#endif