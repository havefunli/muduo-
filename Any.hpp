#pragma once
#include <typeinfo>
#include <memory>

class Any
{
public:
	Any(){}
	
	template <class T>
	Any(const T& val)
		: _ptr(std::make_unique<holder<T>>(val))
	{}

	Any(const Any& other)
		: _ptr(other._ptr ? other._ptr->clone() : nullptr)
	{}

	// Any& 

private:
	class placeholder
	{
	public:
		virtual const std::type_info& type() = 0;
		virtual placeholder* clone() = 0;
	};

	template <class T>
	class holder : public placeholder
	{
	public:
		holder(const T& val)
			: _val(val)
		{}

		const std::type_info& type() override
		{
			return typeid(T);
		}
		
		placeholder* clone() override
		{
			return new holder(_val);
		}

	private:
		T _val;
	};

private:
	std::unique_ptr<placeholder> _ptr;
};



