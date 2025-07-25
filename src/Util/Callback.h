#pragma once

#include <utility>

namespace cb
{
	// A callback to a function pointer that can change at runtime
	template<class Ret, class... Args>
	static Ret PtrCallback(void* data, Args... args)
	{
		return reinterpret_cast<Ret(*)(Args...)>(data)(std::forward<Args>(args)...);
	}

	template<auto Func, class Ret, class... Args>
	Ret FuncCallback(void* data, Args... args)
	{
		return Func(std::forward<Args>(args)...);
	}

	template<auto Func, class Object, class Ret, class... Args>
	Ret FuncArgCallback(void* data, Args... args)
	{
		Object* object = reinterpret_cast<Object*>(data);

		return Func(*object, std::forward<Args>(args)...);
	}

	// A callback to a method of a class
	template<auto Method, class Class, class Ret, class... Args>
	Ret MethodCallback(void* data, Args... args)
	{
		Class* instance = reinterpret_cast<Class*>(data);

		return (instance->*Method)(std::forward<Args>(args)...);
	}

	// A callback to a method of a class
	template<class Class, class Ret, class... Args>
	Ret TypeCallback(void* data, Args... args)
	{
		Class* instance = reinterpret_cast<Class*>(data);

		return (*instance)(std::forward<Args>(args)...);
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

		Callback(const Callback& other) :
			m_callback(other.m_callback),
			m_data(other.m_data)
		{}

		template<class Type>
		Callback(Type&& type)
		{
			if constexpr (!std::is_same_v<std::remove_reference_t<std::remove_cv_t<Type>>, Callback>)
			{
				m_callback = TypeCallback<Type, Ret, Args...>;
				m_data = &type;
			}
			else
			{
				m_callback = type.m_callback;
				m_data = type.m_data;
			}
		}

		// Bind a function pointer that takes data
		explicit Callback(CallbackType callback, void* data) :
			m_callback(callback),
			m_data(data)
		{}

		// Bind a function pointer that doesn't take data
		Callback(PtrType callback) :
			m_callback(PtrCallback<Ret, Args...>),
			m_data(callback)
		{}

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
		CallbackType m_callback;
		void* m_data;
	};

	template<class T>
	Callback(T) -> Callback<typename get_method_decl<decltype(&T::operator())>::type>;

	/* Helpers for the actual functions. These can't be used by themselves */

	template<auto Func, class Ret, class... Args>
	auto BindHelper(Ret(*)(Args...))
	{
		return Callback<Ret(Args...)>(FuncCallback<Func, Ret, Args...>, nullptr);
	}

	template<auto Func, class Object, class Ret, class... Args>
	auto BindArgHelper(Ret(*)(Object&, Args...), Object* object)
	{
		return Callback<Ret(Args...)>(FuncArgCallback<Func, Object, Ret, Args...>, reinterpret_cast<void*>(object));
	}

	template<auto Method, class Class, class Ret, class... Args>
	auto BindMethodHelper(Ret(Class::*)(Args...), Class* instance)
	{
		return Callback<Ret(Args...)>(MethodCallback<Method, Class, Ret, Args...>, reinterpret_cast<void*>(instance));
	}

	template<auto Method, class Class, class Ret, class... Args>
	auto BindMethodHelper(Ret(Class::*)(Args...) const, Class* instance)
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

	// Bind a function
	template<auto Func, class Object>
	auto BindArg(Object& arg)
	{
		return BindArgHelper<Func, std::remove_cv_t<Object>>(Func, const_cast<std::remove_cv_t<Object>*>(&arg));
	}

	// Bind the method of a class
	template<auto Method, class Class>
	auto BindMethod(Class& instance)
	{
		return BindMethodHelper<Method, std::remove_cv_t<Class>>(Method, const_cast<std::remove_cv_t<Class>*>(&instance));
	}
}