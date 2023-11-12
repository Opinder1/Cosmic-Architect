#pragma once

#include <utility>

namespace cb
{
	// A callback to the () operator of a struct
	template<class Callable, class Ret, class... Args>
	Ret ObjectCallback(void* data, Args... args)
	{
		Callable* obj = reinterpret_cast<Callable*>(data);

		return obj->operator()(std::forward<Args>(args)...);
	}

	// A callback to a function pointer
	template<auto Func, class Ret, class... Args>
	Ret FuncCallback(void* data, Args... args)
	{
		return Func(std::forward<Args>(args)...);
	}

	// A callback to a function pointer
	template<auto Func, class Param, class Ret, class... Args>
	Ret FuncPtrParamCallback(void* data, Args... args)
	{
		Param* param = reinterpret_cast<Param*>(data);

		return Func(param, std::forward<Args>(args)...);
	}

	// A callback to a function pointer
	template<auto Func, class Param, class Ret, class... Args>
	Ret FuncRefParamCallback(void* data, Args... args)
	{
		Param& param = *reinterpret_cast<Param*>(data);

		return Func(param, std::forward<Args>(args)...);
	}

	// A callback to a method of a class
	template<auto Method, class Class, class Ret, class... Args>
	Ret MethodCallback(void* data, Args... args)
	{
		Class* instance = reinterpret_cast<Class*>(data);

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
			m_callback(nullptr),
			m_data(nullptr)
		{}

		Callback(std::nullptr_t) :
			m_callback(nullptr),
			m_data(nullptr)
		{}

		// Bind a function pointer that doesn't take data
		Callback(PtrType callback) :
			m_callback(PtrCallback),
			m_data(callback)
		{}

		// Bind a function pointer that takes data
		explicit Callback(CallbackType callback, void* data) :
			m_callback(callback),
			m_data(data)
		{}

		// Initialize with an object that has operator()
		template<class Object>
		Callback(const Object& object)
		{
			m_callback = ObjectCallback<Object, Ret, Args...>;
			m_data = const_cast<void*>(reinterpret_cast<const void*>(&object));
		}

		bool operator==(const Callback& other) const
		{
			return m_callback == other.m_callback && m_data == other.m_data;
		}

		bool IsValid() const
		{
			return m_callback != nullptr;
		}

		Ret operator()(Args... args) const
		{
			return m_callback(m_data, std::forward<Args>(args)...);
		}

	private:
		static Ret PtrCallback(void* data, Args... args)
		{
			return reinterpret_cast<PtrType>(data)(std::forward<Args>(args)...);
		}

	private:
		CallbackType m_callback;
		void* m_data;
	};

	/* Helpers for the actual functions. These can't be used by themselves */

	template<auto Func, class Ret, class... Args>
	auto BindHelper(Ret(*)(Args...))
	{
		return Callback<Ret(Args...)>(FuncCallback<Func, Ret, Args...>, nullptr);
	}

	template<auto Func, class Param, class Ret, class... Args>
	auto BindHelperParam(Ret(*)(Param*, Args...), Param* param)
	{
		return Callback<Ret(Args...)>(FuncPtrParamCallback<Func, Param, Ret, Args...>, reinterpret_cast<void*>(param));
	}

	template<auto Func, class Param, class Ret, class... Args>
	auto BindHelperParam(Ret(*)(Param&, Args...), Param& param)
	{
		return Callback<Ret(Args...)>(FuncRefParamCallback<Func, Param, Ret, Args...>, reinterpret_cast<void*>(&param));
	}

	template<auto Method, class Class, class Ret, class... Args>
	auto BindHelper(Ret(Class::*)(Args...), Class* instance)
	{
		return Callback<Ret(Args...)>(MethodCallback<Method, Class, Ret, Args...>, reinterpret_cast<void*>(instance));
	}

	template<auto Method, class Class, class Ret, class... Args>
	auto BindHelper(Ret(Class::*)(Args...) const, Class* instance)
	{
		return Callback<Ret(Args...)>(MethodCallback<Method, Class, Ret, Args...>, reinterpret_cast<void*>(instance));
	}

	/* The bind functions */

	// Bind a function
	template<auto Func>
	auto Bind()
	{
		return BindHelper<Func>(Func);
	}

	// Bind the method of a class
	template<auto Method, class Class>
	auto Bind(Class& instance)
	{
		return BindHelper<Method, std::remove_cv_t<Class>>(Method, const_cast<std::remove_cv_t<Class>*>(&instance));
	}

	// Bind the method of a class
	template<auto Method, class Class>
	auto Bind(Class* instance)
	{
		return BindHelper<Method, std::remove_cv_t<Class>>(Method, const_cast<std::remove_cv_t<Class>*>(instance));
	}

	// Bind a function with a parameter
	template<auto Func, class Param>
	auto BindParam(Param* param)
	{
		return BindHelperParam<Func, Param>(Func, param);
	}

	// Bind a function with a parameter
	template<auto Func, class Param>
	auto BindParam(Param& param)
	{
		return BindHelperParam<Func, Param>(Func, param);
	}
}