#pragma once

#include <exception>
#include <iostream>

namespace bmstu
{
template <typename T>
class basic_string;

using string = basic_string<char>;
using wstring = basic_string<wchar_t>;
using u16string = basic_string<char16_t>;
using u32string = basic_string<char32_t>;

template <typename T>
class basic_string
{
   private:
	static constexpr size_t SSO_CAPACITY =
		(sizeof(T*) + sizeof(size_t) + sizeof(size_t)) / sizeof(T) - 1;

	struct LongString
	{
		T* ptr;
		size_t size;
		size_t capacity;
	};

	struct ShortString
	{
		T buffer[SSO_CAPACITY + 1];
		unsigned char size;
	};

	union Data
	{
		LongString long_str;
		ShortString short_str;
	};

	Data data_;
	bool is_long_;

	bool is_long() const { return is_long_; }

	T* get_ptr() { return is_long_ ? data_.long_str.ptr : data_.short_str.buffer; }

	const T* get_ptr() const { return is_long_ ? data_.long_str.ptr : data_.short_str.buffer; }

	size_t get_size() const { return is_long_ ? data_.long_str.size : data_.short_str.size; }

	size_t get_capacity() const { return is_long_ ? data_.long_str.capacity : SSO_CAPACITY; }

   public:
	basic_string()
	{
		is_long_ = false;
		data_.short_str.buffer[0] = '\0';
		data_.short_str.size = 0;
	}

	basic_string(size_t size)
	{
		if (size <= SSO_CAPACITY)
		{
			is_long_ = false;
			data_.short_str.size = size;
			for (size_t i = 0; i < size; i++)
			{
				data_.short_str.buffer[i] = ' ';
			}
			data_.short_str.buffer[size] = '\0';
		} else
		{
			data_.long_str.size = size;
			data_.long_str.capacity = size + 1;
			data_.long_str.ptr = new T[data_.long_str.capacity];
			for (size_t i = 0; i < size; i++)
			{
				data_.long_str.ptr[i] = ' ';
			}
			data_.long_str.ptr[size] = '\0';
			is_long_ = true;
		}
	}

	basic_string(std::initializer_list<T> il)
	{
		if (il.size() <= SSO_CAPACITY)
		{
			is_long_ = false;
			size_t i = 0;
			for (const T& el : il) { data_.short_str.buffer[i++] = el; }
			data_.short_str.buffer[il.size()] = '\0';
			data_.short_str.size = il.size();
		}
		else
		{
			is_long_ = true;
			data_.long_str.size = il.size();
			data_.long_str.capacity = il.size() + 1;
			data_.long_str.ptr = new T[data_.long_str.capacity];
			size_t i = 0;
			for (const T& el : il)
			{
				data_.long_str.ptr[i++] = el;
			}
			data_.long_str.ptr[data_.long_str.size] = '\0';
		}
	}

	basic_string(const T* c_str)
	{
		if (strlen_(c_str) <= SSO_CAPACITY)
		{
			data_.short_str.size = strlen_(c_str);;
			for (size_t i = 0; i < data_.short_str.size; i++) { data_.short_str.buffer[i] = c_str[i]; }
			data_.short_str.buffer[data_.short_str.size] = '\0';
			is_long_ = false;
		} else
		{
			data_.long_str.size = strlen_(c_str);
			data_.long_str.capacity = data_.long_str.size + 1;
			data_.long_str.ptr = new T[data_.long_str.capacity];
			for (size_t i = 0; i<data_.long_str.size; i++) { data_.long_str.ptr[i] = c_str[i]; }
			data_.long_str.ptr[data_.long_str.size] = '\0';
			is_long_ = true;
		}
	}

	basic_string(const basic_string& other)
{
    if (other.is_long_)
    {
        is_long_ = true;
        data_.long_str.size = other.data_.long_str.size;
        data_.long_str.capacity = other.data_.long_str.capacity;
        data_.long_str.ptr = new T[data_.long_str.capacity]; 
        for (size_t i = 0; i < data_.long_str.size; i++) {
            data_.long_str.ptr[i] = other.data_.long_str.ptr[i];
        }
        data_.long_str.ptr[data_.long_str.size] = '\0';
    }
    else
    {
        is_long_ = false;
        data_.short_str.size = other.data_.short_str.size;
        for (size_t i = 0; i < data_.short_str.size; i++) {
            data_.short_str.buffer[i] = other.data_.short_str.buffer[i];
        }
        data_.short_str.buffer[data_.short_str.size] = '\0';
    }
}

	basic_string(basic_string&& dying) noexcept
	{
		if (dying.is_long_)
		{
			data_.long_str = dying.data_.long_str;
			is_long_ = true;
			dying.clean_();
		} else
		{
			data_.short_str.size = dying.data_.short_str.size;
			for (size_t i = 0; i < data_.short_str.size; i++) { data_.short_str.buffer[i] = dying.data_.short_str.buffer[i]; }
			data_.short_str.buffer[data_.short_str.size] = '\0';
			is_long_ = false;
			dying.clean_();
		}
	}

	~basic_string() { clean_(); }

	const T* c_str() const { return get_ptr(); }

	size_t size() const { return get_size(); }	

	bool is_using_sso() const { return !is_long(); }

	size_t capacity() const { return get_capacity(); }

	basic_string& operator=(basic_string&& other) noexcept
	{
    	clean_();
		if (other.is_long()) {
			data_.long_str.size = other.data_.long_str.size;
			data_.long_str.capacity = other.data_.long_str.capacity;
			data_.long_str.ptr = other.data_.long_str.ptr;
			is_long_ = true;
		} else {
			data_.short_str.size = other.data_.short_str.size;
			for (size_t i = 0; i < data_.short_str.size; i++) { data_.short_str.buffer[i] = other.data_.short_str.buffer[i]; }
			data_.short_str.buffer[data_.short_str.size] = '\0';
		}
		other.clean_();
    	return *this;
	}

	basic_string& operator=(const T* c_str)
	{
		clean_();
		if (strlen_(c_str) <= SSO_CAPACITY)
		{
			data_.short_str.size = strlen_(c_str);
			for (size_t i = 0; i<data_.short_str.size; i++) { data_.short_str.buffer[i] = c_str[i]; }
			data_.short_str.buffer[data_.short_str.size] = '\0';
			is_long_ = false;
		} else
		{
			data_.long_str.size = strlen_(c_str);
			data_.long_str.capacity = strlen_(c_str) + 1;
			data_.long_str.ptr = new T[data_.long_str.capacity];
			for (size_t i = 0; i< data_.long_str.size; i++) {data_.long_str.ptr[i] = c_str[i]; }
			data_.long_str.ptr[data_.long_str.size] = '\0';
			is_long_ = true;
		}
		return *this;
	}

	basic_string& operator=(const basic_string& other)
	{
		clean_();
		if (other.is_long_)
		{
			data_.long_str.size = other.data_.long_str.size;
			data_.long_str.capacity = data_.long_str.size + 1;
			data_.long_str.ptr = new T[data_.long_str.capacity];
			for (size_t i = 0; i< data_.long_str.size; i++) {data_.long_str.ptr[i] = other.data_.long_str.ptr[i]; }
			data_.long_str.ptr[data_.long_str.size] = '\0';
			is_long_ = true;
		} else
		{
			data_.short_str.size = other. data_.short_str.size;
			for (size_t i = 0; i<data_.short_str.size; i++) { data_.short_str.buffer[i] = other.data_.short_str.buffer[i]; }
			data_.short_str.buffer[data_.short_str.size] = '\0';
			is_long_ = false;
		}
		return *this;
	}

	friend basic_string<T> operator+(const basic_string<T>& left,
									 const basic_string<T>& right)
	{
		size_t result_size = left.size() + right.size();
		T* result = new T[result_size + 1];
		for (size_t i = 0; i < left.size(); ++i) {result[i] = left.get_ptr()[i];}
		for (size_t i = 0; i < right.size(); ++i) {result[left.size() + i] = right.get_ptr()[i];}
		result[result_size] = T();
		return basic_string(result);
	}

	template <typename S>
	friend S& operator<<(S& os, const basic_string& obj)
	{
		if (obj.get_ptr())
		{
			for (size_t i = 0; i < obj.size(); ++i) {os << obj.get_ptr()[i];}
		}
		return os;
	}

	template <typename S>
	friend S& operator>>(S& is, basic_string& obj)
	{
		T ch;
		while(is.get(ch)){
			obj+=ch;
		}
		return is;
	}

	basic_string& operator+=(const basic_string& other)
	{
		size_t len = size() + other.size();
		if (len <= SSO_CAPACITY)
		{
			data_.short_str.size = len;
			for (size_t i = 0; i<data_.short_str.size; i++) { data_.short_str.buffer[i + size()] = other.data_.short_str.buffer[i]; }
			data_.short_str.buffer[len] = '\0';
		} else
		{
			T* new_ptr = new T[size() + other.size()];
			for ( size_t i = 0; i<size(); i++) { new_ptr[i] = get_ptr()[i]; }
			for (size_t i = 0; i<other.size(); i++) { new_ptr[i + size()] = other.get_ptr()[i]; }
			new_ptr[len] = '\0';
			clean_();
			data_.long_str.ptr = new_ptr;
			data_.long_str.size = len;
			data_.long_str.capacity = len + 1;
			is_long_ = true;
		}
		return *this;
	}

	basic_string& operator+=(T symbol)
	{
		if (size() + 1 < SSO_CAPACITY)
		{
			data_.short_str.buffer[data_.short_str.size] = symbol;
			data_.short_str.size++;
			data_.short_str.buffer[data_.short_str.size] = '\0';
		} else
		{
			size_t len = size();
			T* new_ptr = new T[len + 2];
			for (size_t i = 0; i< size(); i++) { new_ptr[i] = get_ptr()[i]; }
			new_ptr[size()] = symbol;
			new_ptr[size()+1] = '\0';
			clean_();
			data_.long_str.ptr = new_ptr;
			data_.long_str.size = len + 1;
			data_.long_str.capacity = len + 2;
			is_long_ = true;
		}
		return *this;
	}

	T& operator[](size_t index) noexcept { return get_ptr()[index]; }

	T& at(size_t index)
	{
		if (index >= size()) { throw std::out_of_range("Wrong index"); }
		return get_ptr()[index];
	}

	T* data() { return get_ptr(); }

   private:
	static size_t strlen_(const T* str)
	{
		size_t len = 0;
		while (str[len] != '\0') { len++; }
		return len;
	}

	void clean_()
	{
		if (is_long_)
		{
			delete[] data_.long_str.ptr;
			data_.long_str.ptr = nullptr;
			data_.long_str.size = 0;
			data_.long_str.capacity = 0;
			is_long_ = false;
		} else
		{
			data_.short_str.buffer[0] = '\0';
			data_.short_str.size = 0;
			is_long_ = false;
		}
	}
};
}  // namespace bmstu