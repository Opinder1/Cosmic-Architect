#pragma once

#include <utility>

namespace cb
{
	// A callback to the () operator of a struct
	template<class Callable, class Ret, class... Args>
	Ret ObjectCallback(const void* ptr, Args... args)
	{
		const Callable* c = reinterpret_cast<const Callable*>(ptr);

		return c->operator()(std::forward<Args>(args)...);
	}

	// A callback to a function pointer
	template<auto Func, class Ret, class... Args>
	Ret ConstFuncCallback(const void* ptr, Args... args)
	{
		return Func(std::forward<Args>(args)...);
	}

	// A callback to a method of a class
	template<auto Method, class Class, class Ret, class... Args>
	Ret ConstMethodCallback(const void* ptr, Args... args)
	{
		Class* instance = const_cast<Class*>(reinterpret_cast<const Class*>(ptr));

		return (instance->*Method)(std::forward<Args>(args)...);
	}

	// Forward definition required
	template<class Function>
	class TempCallback;

	// A callback that requires a class or function pointer to be initialized and can always be called
	// Intended to be used for callbacks to temporary objects which define operator() like a lambda
	// Should not be stored since it doesn't store its callbacks data
	template<class Ret, class... Args>
	class TempCallback<Ret(Args...)>
	{
	private:
		using PtrType = Ret(*)(Args...);

		using CallbackType = Ret(&)(const void*, Args...);

	public:
		explicit TempCallback(const void* instance, CallbackType callback) :
			m_ptr(instance),
			m_callback(callback)
		{}

		// Bind a function pointer
		TempCallback(PtrType f) :
			m_ptr(reinterpret_cast<const void*>(f)),
			m_callback(PtrCallback)
		{}

		// Bind a struct object that defines operator()
		template<class Callable>
		TempCallback(const Callable& c) :
			TempCallback<Ret(Args...)>(reinterpret_cast<const void*>(&c), ObjectCallback<Callable, Ret, Args...>)
		{}

		Ret operator()(Args... args) const
		{
			return m_callback(m_ptr, std::forward<Args>(args)...);
		}

	private:
		static Ret PtrCallback(const void* ptr, Args... args)
		{
			return reinterpret_cast<PtrType>(ptr)(std::forward<Args>(args)...);
		}

	private:
		const void* m_ptr;
		CallbackType m_callback;
	};

	// A callback to a function pointer
	template<auto Func, class Ret, class... Args>
	Ret FuncCallback(void* ptr, Args... args)
	{
		return Func(std::forward<Args>(args)...);
	}

	// A callback to a method of a class
	template<auto Method, class Class, class Ret, class... Args>
	Ret MethodCallback(void* ptr, Args... args)
	{
		Class* instance = reinterpret_cast<Class*>(ptr);

		return (instance->*Method)(std::forward<Args>(args)...);
	}

	// Forward definition required
	template<class Function>
	class Callback;

	// A callback that can optionally be initialized with no function and IsValid() should be checked before calling
	// Intended to be stored and moved
	template<class Ret, class... Args>
	class Callback<Ret(Args...)>
	{
	private:
		using PtrType = Ret(*)(Args...);

		using CallbackType = Ret(*)(void*, Args...);

	public:
		Callback() :
			m_ptr(nullptr),
			m_callback(nullptr)
		{}

		Callback(std::nullptr_t) :
			m_ptr(nullptr),
			m_callback(nullptr)
		{}

		explicit Callback(void* instance, CallbackType callback) :
			m_ptr(instance),
			m_callback(callback)
		{}

		// Bind a function pointer
		Callback(PtrType f) :
			m_ptr(reinterpret_cast<void*>(f)),
			m_callback(PtrCallback)
		{}

		bool operator==(const Callback& other) const
		{
			return m_ptr == other.m_ptr && m_callback == other.m_callback;
		}

		bool IsValid() const
		{
			return m_callback != nullptr;
		}

		Ret operator()(Args... args) const
		{
			return m_callback(m_ptr, std::forward<Args>(args)...);
		}

	private:
		static Ret PtrCallback(void* ptr, Args... args)
		{
			return reinterpret_cast<PtrType>(ptr)(std::forward<Args>(args)...);
		}

	private:
		void* m_ptr;
		CallbackType m_callback;
	};

	/* Helpers for the actual functions. These can't be used by themselves */
	template<auto Func, class Ret, class... Args>
	auto TempBindHelper(Ret(*)(Args...))
	{
		return TempCallback<Ret(Args...)>(nullptr, ConstFuncCallback<Func, Ret, Args...>);
	}

	template<auto Method, class Class, class Ret, class... Args>
	auto TempBindHelper(Class& c, Ret(Class::*)(Args...))
	{
		return TempCallback<Ret(Args...)>(&c, ConstMethodCallback<Method, Class, Ret, Args...>);
	}

	template<auto Func, class Ret, class... Args>
	auto BindHelper(Ret(*)(Args...))
	{
		return Callback<Ret(Args...)>(nullptr, FuncCallback<Func, Ret, Args...>);
	}

	template<auto Method, class Class, class Ret, class... Args>
	auto BindHelper(Class& c, Ret(Class::*)(Args...))
	{
		return Callback<Ret(Args...)>(reinterpret_cast<void*>(&c), MethodCallback<Method, Class, Ret, Args...>);
	}

	/* The bind functions */

	// Temporarily bind a function
	template<auto Func>
	auto TBind()
	{
		return TempBindHelper<Func>(Func);
	}

	// Temporarily bind a method of a class
	template<auto Method, class Class>
	auto TBind(Class& c)
	{
		return TempBindHelper<Method, Class>(c, Method);
	}

	// Bind a function
	template<auto Func>
	auto Bind()
	{
		return BindHelper<Func>(Func);
	}

	// Bind the method of a class
	template<auto Method, class Class>
	auto Bind(Class& c)
	{
		return BindHelper<Method, Class>(c, Method);
	}
}