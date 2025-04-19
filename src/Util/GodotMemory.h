#pragma once

#include "Debug.h"
#include "Nocopy.h"

#include <godot_cpp/classes/wrapped.hpp>

#include <godot_cpp/variant/variant.hpp>

#include <godot_cpp/core/memory.hpp>

// 

namespace godot
{
	// Object container that stores a godot object in place
	template<class T>
	class Obj
	{
	public:
		Obj()
		{
			T* object = (T*)&m_memory;

			_pre_initialize<T>();
			new (object) T();
			_post_initialize<T>(object);
		}

		~Obj()
		{
			T* object = (T*)&m_memory;

			object->~T();
		}

		T* ptr()
		{
			return (T*)&m_memory;
		}

		T& value()
		{
			return *ptr();
		}

		T* operator->() const
		{
			return ptr();
		}

		T& operator*() const
		{
			return *ptr();
		}

		operator Variant() const
		{
			return Variant(ptr());
		}

	private:
		alignas(alignof(T)) std::byte m_memory[sizeof(T)];
	};

	// std::unique_ptr for godot objects that allocates the object on the heap
	template<class T>
	class ObjPtr : Nocopy
	{
	public:
		ObjPtr() : m_ptr(nullptr) {}

		ObjPtr(T* ptr) : m_ptr(ptr) {}

		~ObjPtr()
		{
			reset();
		}

		ObjPtr(ObjPtr&& other)
		{
			m_ptr = other.m_ptr;
			other.m_ptr = nullptr;
		}

		ObjPtr& operator=(ObjPtr&& other)
		{
			reset(other.m_ptr);
			other.m_ptr = nullptr;
		}

		bool is_valid() const
		{
			return !is_null();
		}

		bool is_null() const
		{
			return m_ptr == nullptr;
		}

		T* ptr() const
		{
			return m_ptr;
		}

		T* operator->() const
		{
			return ptr();
		}

		T& operator*() const
		{
			DEBUG_ASSERT(is_valid(), "Trying to get an invalid object");
			return *ptr();
		}

		operator bool() const
		{
			return is_valid();
		}

		operator Variant() const
		{
			return Variant(ptr());
		}

		void instantiate()
		{
			reset(memnew(T));
		}

		void reset(T* ptr = nullptr)
		{
			if (m_ptr != nullptr)
			{
				memdelete<T>(m_ptr);
			}

			m_ptr = ptr;
		}

		T* release()
		{
			T* ptr = m_ptr;
			m_ptr = nullptr;
			return ptr;
		}

	private:
		T* m_ptr;
	};

	// std::optional for godot object that stores object in place
	template<class T>
	class OptObj
	{
	public:
		OptObj() : m_valid(false) {}

		~OptObj()
		{
			reset();
		}

		T* ptr()
		{
			if (is_valid())
			{
				return (T*)&m_memory;
			}
			else
			{
				return nullptr;
			}
		}

		T& value()
		{
			DEBUG_ASSERT(is_valid(), "Trying to get an invalid optional");
			return *(T*)&m_memory;
		}

		bool is_valid() const
		{
			return m_valid;
		}

		bool is_null() const
		{
			return !is_valid();
		}

		T* operator->() const
		{
			DEBUG_ASSERT(is_valid(), "Trying to get an invalid optional");
			return ptr();
		}

		T& operator*() const
		{
			DEBUG_ASSERT(is_valid(), "Trying to get an invalid optional");
			return *ptr();
		}

		operator bool() const
		{
			return is_valid();
		}

		operator Variant() const
		{
			if (is_valid())
			{
				return Variant(ptr());
			}
			else
			{
				return Variant(nullptr);
			}
		}

		void instantiate()
		{
			reset();

			T* object = (T*)&m_memory;

			_pre_initialize<T>();
			new (object) T();
			_post_initialize<T>(object);

			m_valid = true;
		}

		void reset()
		{
			if (is_valid())
			{
				T* object = (T*)&m_memory;

				object->~T();

				m_valid = false;
			}
		}

	private:
		bool m_valid;
		alignas(alignof(T)) std::byte m_memory[sizeof(T)];
	};
}