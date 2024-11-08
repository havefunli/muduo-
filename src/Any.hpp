/*
* CopyRight Limuyou
* Date 24_11_3
* brief 简单的实现了一个 Any, 用于接受各类协议的数据进行处理
*/

#pragma once
#include <typeinfo>
#include <memory>
#include <utility>
#include <assert.h>

class Any
{
private:
	class holder
	{
	public:
		holder()  = default;
		~holder() = default;

		virtual const std::type_info& type() = 0;
		virtual holder* clone() = 0;
	};

	template <class ValueType>
	class placeholder : public holder
	{
	public:
		placeholder()
			: _val(ValueType())
		{}

		placeholder(const ValueType& val)
			: _val(val)
		{}

		const std::type_info& type() override
		{
			return typeid(ValueType);
		}

		holder* clone() override
		{
			return new placeholder(_val);
		}

		ValueType _val;
	};

public:
	Any()
		: _ptr(nullptr)
	{}

	template <class T>
	Any(const T& val)
		: _ptr(std::make_shared<placeholder<T>>(val))
	{}

	template <class T>
	Any(const Any& other)
	{
		_ptr = (other._ptr.get() ? other._ptr->clone() : nullptr);
	}

	void Swap(Any& other)
	{
		std::swap(other._ptr, _ptr);
	}

	Any& operator=(Any other)
	{
		if (this != &other)
			Swap(other);
		return *this;
	}	

	template <class T>
	Any& operator=(const T& val)
	{
		_ptr = std::make_shared<placeholder<T>>(val);
		return *this;
	}

	template <class T>
	T& Get()
	{
		assert(typeid(T) == _ptr->type());

		if (_ptr.get() == nullptr) throw std::bad_cast();;
		placeholder<T>* holder = static_cast<placeholder<T>*>(_ptr.get());
		return holder->_val;
	}

	bool HasVal()
	{
		return _ptr.get() != nullptr;
	}

private:
	std::shared_ptr<holder> _ptr;
};



