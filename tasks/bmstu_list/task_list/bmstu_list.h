#include <cstddef>
#include <iterator>
#include <ostream>
#include "abstract_iterator.h"

namespace bmstu
{
template <typename T>
class list
{
	struct node
	{
		node() = default;

		node(node* prev, const T& value, node* next)
			: next_node_(next), value_(value), prev_node_(prev)
		{
		}

		T value_;
		node* next_node_ = nullptr;
		node* prev_node_ = nullptr;
	};

   public:
	struct iterator
		: public abstract_iterator<iterator, T, std::bidirectional_iterator_tag>
	{
		node* current;
		iterator() : current(nullptr) {}
		iterator(node* node) : current(node) {}
		iterator& operator++() override { 
			current = current->next_node_;
			return *this; 
		}
		iterator& operator--() override { 
			current = current->prev_node_;
			return *this; 
		}
		iterator operator++(int) override { 
			iterator old = *this;
			current = current->next_node_;
			return old; 
		}
		iterator operator--(int) override { 
			iterator old = *this;
			current = current->prev_node_;
			return old; 
		}
		iterator& operator+=(
			const typename abstract_iterator<
				iterator,
				T,
				std::bidirectional_iterator_tag>::difference_type& n) override
		{
			if (n >= 0) {
				for (int i = 0; i < n; ++i) { ++(*this); }
			} else {
				for (int i = 0; i > n; --i) { --(*this); }
			}
			return *this;
		}
		iterator& operator-=(
			const typename abstract_iterator<
				iterator,
				T,
				std::bidirectional_iterator_tag>::difference_type& n) override
		{
			return *this += -n;
		}
		iterator operator+(const typename abstract_iterator<
						   iterator,
						   T,
						   std::bidirectional_iterator_tag>::difference_type& n)
			const override
		{
			iterator tmp = *this;
			return tmp +=n;
		}
		iterator operator-(const typename abstract_iterator<
						   iterator,
						   T,
						   std::bidirectional_iterator_tag>::difference_type& n)
			const override
		{
			iterator tmp = *this;
			return tmp -= n;
		}
		typename abstract_iterator<iterator,
								   T,
								   std::bidirectional_iterator_tag>::reference
		operator*() const override
		{
			return current->value_;
		}
		typename abstract_iterator<iterator,
								   T,
								   std::bidirectional_iterator_tag>::pointer
		operator->() const override
		{
			return &(current->value_);
		}
		bool operator==(const iterator& other) const override
		{
			return current == other.current;
		}
		bool operator!=(const iterator& other) const override
		{
			return current != other.current;
		}
		explicit operator bool() const override { return current != nullptr; }
		typename abstract_iterator<
			iterator,
			T,
			std::bidirectional_iterator_tag>::difference_type
		operator-(const iterator& other) const override
		{
			int count = 0;
			iterator tmp = other;
			while (tmp != *this ) { ++tmp; ++count; }
			return count;
		}
	};
	using const_iterator = iterator;

	list() {
		head_ = new node();
		tail_ = new node();
		head_->next_node_ = tail_;
		tail_->prev_node_ = head_;
	}

	template <typename it>
	list(it begin, it end) : list()
	{
		for (auto i = begin; i != end; ++i) { push_back(*i); }
	}

	list(std::initializer_list<T> values) : list() {
		for (const auto& v : values) { push_back(v); }
	}

	list(const list& other) : list() {
		for (const auto& v : other) { push_back(v); }
	}

	list(list&& other) : list() { swap(other); }

#pragma endregion
#pragma region pushs

	template <typename Type>
	void push_back(const Type& value)
	{
		node* last = tail_->prev_node_;
		node* new_last = new node(tail_->prev_node_, value, tail_);
		tail_->prev_node_ = new_last;
		last->next_node_ = new_last;
		++size_;
	}

	template <typename Type>
	void push_front(const Type& value)
	{
		// адрес реального последнего элемента
		node* first = head_->next_node_;
		node* new_first = new node(head_, value, first);
		head_->next_node_ = new_first;
		first->prev_node_ = new_first;
		++size_;
	}

#pragma endregion

	bool empty() const noexcept { return (size_ == 0u); }

	~list() {
		clear(); 
		delete head_; 
		delete tail_;
	}

	void clear() {
		node* current = head_->next_node_;
		while (current != tail_) {
			node* next = current->next_node_;
			delete current;
			current = next;
		}
		head_->next_node_ = tail_;
		tail_->prev_node_ = head_;
		size_ = 0;
	}

	size_t size() const { return size_; }

	void swap(list& other) noexcept
	{ 
		std::swap(head_, other.head_);
		std::swap(tail_, other.tail_);
		std::swap(size_, other.size_);
	}

	friend void swap(list& l, list& r) { l.swap(r); }

#pragma region iterators

	iterator begin() noexcept
	{
		return iterator{head_->next_node_};
	}

	iterator end()

		noexcept
	{
		return iterator{tail_};
	}

	const_iterator begin() const

		noexcept
	{
		return const_iterator{head_->next_node_};
	}

	const_iterator end() const

		noexcept
	{
		return const_iterator{tail_};
	}

	const_iterator cbegin() const

		noexcept
	{
		return const_iterator{head_->next_node_};
	}

	const_iterator cend() const

		noexcept
	{
		return const_iterator{tail_};
	}

#pragma endregion

	T operator[](size_t pos) const {
		auto it = cbegin();
		for (size_t i = 0; i < pos; ++i) { ++it; }
		return *it;
	}

	T& operator[](size_t pos) { 
		auto it = begin();
		for (size_t i = 0; i < pos; ++i) { ++it; }
		return *it; 
	}

	friend bool operator==(const list& l, const list& r) { 
		if (l.size_ != r.size_) {return false; }
		auto left = l.cbegin(), right = r.cbegin();
		while (left != l.cend()) {
			if (*left != *right) {
				++left, ++right; 
				return false; 
			}
		}
		return true; 
	}


	friend bool operator!=(const list& l, const list& r) { return !(l == r); }

	friend auto operator<=>(const list& lhs, const list& rhs) { return true; }

	friend std::ostream& operator<<(std::ostream& os, const list& other)
	{
		os << "{";
		bool first = true;
		for (const auto& v : other) {
			if (!first) {os << ", "; }
			os << v;
			first = false;
		}
		return os << "}";
	}

	iterator insert(const_iterator pos, const T& value)
	{
		node* next_node = pos.current;
		node* prev_node = next_node->prev_node_;
		node* new_node = new node(prev_node, value, next_node);
		prev_node->next_node_ = new_node;
		next_node->prev_node_ = new_node;
		++size_;
		return iterator{new_node};
	}

   private:
	static bool lexicographical_compare_(const list<T>& l, const list<T>& r)
	{
		auto left = l.cbegin(), right = r.cbegin();
		while (left != l.cend() && right != r.cend()) {
			if (*left < *right) {return true; }
			if (*left > *right) {return false; }
			++left, ++right;
		}
		return l.size_ < r.size_;
	}

	size_t size_ = 0;
	node* tail_ = nullptr;
	node* head_ = nullptr;
};
}  // namespace bmstu