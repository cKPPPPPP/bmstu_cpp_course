#pragma once

#include <exception>
#include <iostream>
#include <utility>

namespace bmstu
{
template <typename T>
class stack
{
public:
    stack() : data_(nullptr), size_(0) {
        data_ = static_cast<T*>(::operator new(sizeof(T) * 1));
    }

    bool empty() const noexcept { return size_ == 0; }

    size_t size() const noexcept { return size_; }

    ~stack() {
        clear();
        ::operator delete(data_);
    }
    
    template <typename... Args>
    void emplace(Args&&... args){
        T* new_data =  reallocate_move_and_destroy();
        new (new_data + size_) T(std::forward<Args>(args)...);
        operator delete(data_);
        data_ = new_data;
        size_++;
    }
    
    void push(const T& value){
        T* new_data =  reallocate_move_and_destroy();
        new (new_data + size_) T(value);
        operator delete(data_);
        data_ = new_data;
        size_++;
    }
    
    void push(T&& value){
        T* new_data =  reallocate_move_and_destroy();
        new (new_data + size_) T(std::move(value));
        operator delete(data_);
        data_ = new_data;
        size_++;
    }
    

    void pop() {
        if (empty()) {
            throw std::underflow_error("Stack is empty");
        }
        data_[size_ - 1].~T();
        --size_;
    }

    T& top() {
        if (empty()) {
            throw std::underflow_error("Stack is empty");
        }
        return data_[size_ - 1];
    }
    

    const T& top() const {
        if (empty()) {
            throw std::underflow_error("Stack is empty");
        }
        return data_[size_ - 1];
    }

    void clear() noexcept {
        while (!empty()) {
            pop();
        }
    }

private:
    T* reallocate_move_and_destroy() {
        T* new_data = static_cast<T*>(operator new(sizeof(T) * (size_ + 1)));
        for (size_t i = 0; i < size_; ++i) {
            new (new_data + i) T(std::move(data_[i]));
            data_[i].~T();
        }
        return new_data;
    }

    T* data_;
    size_t size_;
};
}  // namespace bmstu