#ifndef SDYNAMIC
#define SDYNAMIC
#include <initializer_list>
#include <stdexcept>
#include <cstdint>
#include <utility>
#include <sallocator.hpp>
#include <ranges>
#include <limits>
#include <compare>

#undef min
#undef max

namespace sere {
	template <typename _Container>
	class _dynamic_iterator;

	template <typename _Container>
	class _dynamic_const_iterator {
		using iterator = _dynamic_const_iterator;
	public:
		using value_type = typename _Container::value_type;
		using pointer = const value_type*;
		using reference = const value_type&;
		using size_type = typename _Container::size_type;
		using difference_type = typename _Container::difference_type;

		pointer _Ptr;

		constexpr explicit _dynamic_const_iterator(pointer ptr) noexcept
			: _Ptr{ ptr }
		{
		}

		constexpr explicit _dynamic_const_iterator(_dynamic_iterator<_Container> it) noexcept
			: _Ptr{ it._Ptr }
		{
		}

		[[nodiscard]] constexpr reference operator*() const noexcept {
			return *_Ptr;
		}

		[[nodiscard]] constexpr pointer operator->() const noexcept {
			return _Ptr;
		}

		constexpr iterator& operator++() noexcept {
			++_Ptr;
			return *this;
		}

		constexpr iterator operator++(int) noexcept {
			iterator tmp{ *this };
			++(*this);
			return tmp;
		}

		constexpr iterator& operator--() noexcept {
			--_Ptr;
			return *this;
		}

		constexpr iterator operator--(int) noexcept {
			iterator tmp{ *this };
			--(*this);
			return tmp;
		}

		constexpr iterator& operator+=(const difference_type offset) noexcept {
			_Ptr += offset;
			return *this;
		}

		constexpr iterator& operator-=(const difference_type offset) noexcept {
			_Ptr -= offset;
			return *this;
		}

		[[nodiscard]] constexpr iterator operator+(const difference_type offset) const noexcept {
			iterator tmp{ *this };
			tmp += offset;
			return tmp;
		}

		[[nodiscard]] constexpr iterator operator-(const difference_type offset) const noexcept {
			iterator tmp{ *this };
			tmp -= offset;
			return tmp;
		}

		[[nodiscard]] constexpr bool operator==(const iterator&) const noexcept = default;
		[[nodiscard]] constexpr std::strong_ordering operator<=>(const iterator&) const noexcept = default;

		constexpr reference operator[](const size_type index) const noexcept {
			return *(_Ptr + index);
		}
	};

	template <typename _Container>
	class _dynamic_iterator : public _dynamic_const_iterator<_Container> {
		using iterator = _dynamic_iterator;
		using base = _dynamic_const_iterator<_Container>;
		using base::_Ptr;
	public:
		using value_type = base::value_type;
		using pointer = value_type*;
		using reference = value_type&;
		using size_type = base::size_type;
		using difference_type = base::difference_type;


		constexpr explicit _dynamic_iterator(pointer ptr) noexcept
			: base{ ptr }
		{
		}

		[[nodiscard]] constexpr reference operator*() const noexcept {
			return const_cast<reference>(*_Ptr);
		}

		[[nodiscard]] constexpr pointer operator->() const noexcept {
			return const_cast<pointer>(_Ptr);
		}

		constexpr iterator& operator++() noexcept {
			++_Ptr;
			return *this;
		}

		constexpr iterator operator++(int) noexcept {
			iterator tmp{ *this };
			++(*this);
			return tmp;
		}

		constexpr iterator& operator--() noexcept {
			--_Ptr;
			return *this;
		}

		constexpr iterator operator--(int) noexcept {
			iterator tmp{ *this };
			--(*this);
			return tmp;
		}

		constexpr iterator& operator+=(const difference_type offset) noexcept {
			_Ptr += offset;
			return *this;
		}

		constexpr iterator& operator-=(const difference_type offset) noexcept {
			_Ptr -= offset;
			return *this;
		}

		[[nodiscard]] constexpr iterator operator+(const difference_type offset) const noexcept {
			iterator tmp{ *this };
			tmp += offset;
			return tmp;
		}

		[[nodiscard]] constexpr iterator operator-(const difference_type offset) const noexcept {
			iterator tmp{ *this };
			tmp -= offset;
			return tmp;
		}

		[[nodiscard]] constexpr bool operator==(const iterator&) const noexcept = default;
		[[nodiscard]] constexpr std::strong_ordering operator<=>(const iterator&) const noexcept = default;

		constexpr reference operator[](const size_type index) const noexcept {
			return const_cast<reference>(*(_Ptr + index));
		}

		friend class _dynamic_const_iterator<_Container>;
	};

	template <typename _Ty, typename _Alloc = sere::allocator<_Ty>>
	class dynamic {
	public:
		using value_type = _Ty;
		using pointer = _Ty*;
		using const_pointer = const _Ty*;
		using reference = _Ty&;
		using const_reference = const _Ty&;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;
		using allocator_type = _Alloc;
		using alloc_traits = std::allocator_traits<_Alloc>;

		using iterator = _dynamic_iterator<dynamic>;
		using const_iterator = _dynamic_const_iterator<dynamic>;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		static_assert(std::is_object_v<_Ty>, "Elements must be an object type");
		static_assert(!std::is_const_v<_Ty>, "Elements cannot be const");
		static_assert(!std::is_volatile_v<_Ty>, "Elements cannot be volatile");
		static_assert(!std::is_abstract_v<_Ty>, "Elements cannot be abstract types");
	private:
		pointer _data;
		size_type _size;
		size_type _capacity;
		[[no_unique_address]] allocator_type _alloc;

		constexpr void destroy_data() {
			if (_data)
				for (size_type i{}; i < _size; ++i)
					alloc_traits::destroy(_alloc, &_data[i]);
		}

		constexpr void _deallocate() {
			if (_data)
				alloc_traits::deallocate(_alloc, _data, _capacity);

			_data = {};
			_capacity = {};
		}

		constexpr void reallocate(size_type new_cap) {
			if (new_cap < _size) return;

			pointer tmp_data{ alloc_traits::allocate(_alloc, new_cap) };

			size_type i{};
			try {
				for (; i < _size; ++i)
					alloc_traits::construct(_alloc, &tmp_data[i], std::move_if_noexcept(_data[i]));
			}
			catch (...) {
				for (size_type j{}; j < i; ++j)
					alloc_traits::destroy(_alloc, &tmp_data[j]);

				alloc_traits::deallocate(_alloc, tmp_data, new_cap);
				tmp_data = {};

				throw;
			}

			destroy_data();
			_deallocate();

			_capacity = new_cap;
			_data = tmp_data;
		}
	public:
		constexpr dynamic() noexcept
			: _data{}, _size{}, _capacity{}
		{
		}

		constexpr explicit dynamic(const size_type capacity)
			: _size{ capacity }, _capacity{ capacity }, _data{}
		{
			_data = alloc_traits::allocate(_alloc, capacity);

			size_type i{};
			try {
				for (; i < _size; ++i)
					alloc_traits::construct(_alloc, &_data[i], value_type{});
			}
			catch (...) {
				for (size_type j{}; j < i; ++j)
					alloc_traits::destroy(_alloc, &_data[j]);

				_deallocate();

				_capacity = {};

				throw;
			}
		}

		constexpr dynamic(const std::initializer_list<value_type> init)
			: _size{ init.size() }, _capacity{ init.size() }, _data{}
		{
			_data = alloc_traits::allocate(_alloc, _size);

			size_type i{};
			try {
				for (; i < _size; ++i)
					alloc_traits::construct(_alloc, &_data[i], init.data()[i]);
			}
			catch (...) {
				for (size_type j{}; j < i; ++j)
					alloc_traits::destroy(_alloc, &_data[j]);

				_deallocate();

				_capacity = {};

				throw;
			}
		}

		template <typename _Rng>
			requires std::ranges::range<_Rng>
		constexpr explicit dynamic(_Rng&& range)
			: _size{ range.size() }, _capacity{ range.size() }, _data{}
		{
			_data = alloc_traits::allocate(_alloc, _size);

			size_type i{};
			try {
				for (auto begin{ range.begin() }; begin != range.end(); ++begin) {
					alloc_traits::construct(_alloc, &_data[i], *begin);
					++i;
				}
			}
			catch (...) {
				for (size_type j{}; j < i; ++j)
					alloc_traits::destroy(_alloc, &_data[j]);

				destroy_data();
				_deallocate();

				_capacity = {};

				throw;
			}
		}

		constexpr dynamic(const dynamic& other)
			: _size{}, _capacity{}, _data{}
		{
			_data = alloc_traits::allocate(_alloc, other._size);

			size_type i{};
			try {
				for (; i < other._size; ++i)
					alloc_traits::construct(_alloc, &_data[i], other._data[i]);
			}
			catch (...) {
				for (size_type j{}; j < i; ++j)
					alloc_traits::destroy(_alloc, &_data[j]);

				destroy_data();
				_deallocate();

				_capacity = {};

				throw;
			}
		}

		constexpr dynamic(dynamic&& other) noexcept
			: _size{ other._size }, _capacity{ other._capacity }, _data{ other._data }, _alloc{ other._alloc }
		{
			other._size = {};
			other._capacity = {};
			other._data = {};
			other._alloc = {};
		}

		constexpr dynamic& operator=(const dynamic& other) {
			if (this == &other) return *this;

			destroy_data();

			if (_capacity >= other._size) {
				size_type i{};
				try {
					for (; i < other._size; ++i)
						alloc_traits::construct(_alloc, &_data[i], other._data[i]);
				}
				catch (...) {
					for (size_type j{}; j < i; ++j)
						alloc_traits::destroy(_alloc, &_data[j]);

					throw;
				}
			}
			else {
				pointer tmp_data{ alloc_traits::allocate(_alloc, other._size) };

				size_type i{};
				try {
					for (; i < other._size; ++i)
						alloc_traits::construct(_alloc, &tmp_data[i], other._data[i]);
				}
				catch (...) {
					for (size_type j{}; j < i; ++j)
						alloc_traits::destroy(_alloc, &tmp_data[j]);

					alloc_traits::deallocate(_alloc, tmp_data, other._size);

					throw;
				}

				_deallocate();
				_data = tmp_data;
			}

			_size, _capacity = other._size;

			return *this;
		}

		constexpr dynamic& operator=(dynamic&& other) noexcept {
			if (this == &other) return *this;

			destroy_data();
			_deallocate();

			_data = other._data;
			_size = other._size;
			_capacity = other._capacity;
			_alloc = other._alloc;

			other._size = {};
			other._capacity = {};
			other._data = {};
			other._alloc = {};

			return *this;
		}

		[[nodiscard]] constexpr const_reference operator[](const size_type index) const noexcept {
			return _data[index];
		}

		[[nodiscard]] constexpr reference operator[](const size_type index) noexcept {
			return _data[index];
		}

		[[nodiscard]] constexpr const_reference at(const size_type index) const {
			if (index >= _size) throw std::out_of_range{ "Given index is out of range." };
			return _data[index];
		}

		[[nodiscard]] constexpr reference at(const size_type index) {
			return const_cast<reference>(std::as_const(*this).at(index));
		}

		[[nodiscard]] constexpr const_reference back() const noexcept {
			return _data[_size - 1];
		}

		[[nodiscard]] constexpr reference back() noexcept {
			return _data[_size - 1];
		}

		[[nodiscard]] constexpr const_reference front() const noexcept {
			return _data[0];
		}

		[[nodiscard]] constexpr reference front() noexcept {
			return _data[0];
		}

		[[nodiscard]] constexpr allocator_type get_allocator() const noexcept {
			return _alloc;
		}

		[[nodiscard]] constexpr size_type max_size() const noexcept {
			return std::numeric_limits<size_type>::max();
		}

		constexpr void push_back(const_reference value) {
			if (_size == _capacity) reallocate(!_capacity ? 1 : _capacity * 2);

			try {
				alloc_traits::construct(_alloc, &_data[_size], value);
			}
			catch (...) {
				throw;
			}

			++_size;
		}

		template <typename... Args>
		constexpr reference emplace_back(Args&&... args) {
			if (_size == _capacity) reallocate(!_capacity ? 1 : _capacity * 2);

			try {
				alloc_traits::construct(_alloc, &_data[_size], std::forward<Args>(args)...);
			}
			catch (...) {
				throw;
			}

			++_size;

			return _data[_size - 1];
		}

		template <typename _Rng>
			requires std::ranges::range<_Rng>
		constexpr void append_range(_Rng&& range) {
			if (range == *this) return;

			if (_size + range.size() > _capacity) reallocate(_size + range.size());

			size_t old_size{ _size };

			for (auto begin{ std::begin(range) }; begin != std::end(range); ++begin) {
				try {
					alloc_traits::construct(_alloc, &_data[_size], *begin);
					++_size;
				}
				catch (...) {
					while (_size > old_size) {
						--_size;
						alloc_traits::destroy(_alloc, &_data[_size]);
					}

					throw;
				}
			}
		}

		constexpr void pop_back() noexcept {
			alloc_traits::destroy(_alloc, &_data[_size - 1]);
			--_size;
		}

		constexpr void clear() noexcept {
			destroy_data();
		}

		constexpr void shrink_to_fit() {
			if (!_size) {
				destroy_data();
				_deallocate();
			}
			else if (_capacity > _size)
				reallocate(_size);
		}

		constexpr void reserve(const size_type new_cap) {
			if (new_cap > max_size())
				throw std::length_error{ "capacity is too large" };

			if (new_cap > _capacity)
				reallocate(new_cap);
		}

		constexpr void resize(const size_type new_size) {
			if (new_size == _size) return;

			if (new_size > max_size())
				throw std::length_error{ "size is too large" };

			if (new_size > _size) {
				if (_capacity < new_size)
					reallocate(new_size);

				size_type old_size{ _size };

				try {
					while (_size < new_size) {
						alloc_traits::construct(_alloc, &_data[_size], value_type{});
						++_size;
					}
				}
				catch (...) {
					while (_size > old_size) {
						--_size;
						alloc_traits::destroy(_alloc, &_data[_size]);
					}

					throw;
				}
			}
			else {
				while (_size > new_size)
					pop_back();
			}
		}

		constexpr void assign(const size_type new_size, const_reference val) {
			resize(new_size);

			if (!new_size) return;

			size_type i{};
			try {
				for (; i < _size; ++i)
					alloc_traits::construct(_alloc, &_data[i], val);
			}
			catch (...) {
				for (size_type j{}; j < i; ++i)
					alloc_traits::destroy(_alloc, &_data[j]);

				_size = {};

				throw;
			}
		}

		constexpr void assign(std::initializer_list<value_type> init) {
			dynamic tmp{ init };
			*this = std::move(tmp);
		}

		template <typename _Rng>
			requires std::ranges::range<_Rng>
		constexpr void assign_range(_Rng&& range) {
			if (range == *this) return;

			dynamic tmp{ range };
			*this = std::move(tmp);
		}

		constexpr void swap(dynamic& other) {
			dynamic tmp{ *this };

			_data = other._data;
			_capacity = other._capacity;
			_size = other._size;

			other._data = tmp._data;
			other._capacity = tmp._capacity;
			other._size = tmp._size;
		}

		[[nodiscard]] constexpr size_type size() const noexcept {
			return _size;
		}

		[[nodiscard]] constexpr size_type capacity() const noexcept {
			return _capacity;
		}

		[[nodiscard]] constexpr pointer data() noexcept {
			return _data;
		}

		[[nodiscard]] constexpr const_pointer data() const noexcept {
			return _data;
		}

		[[nodiscard]] constexpr bool empty() const noexcept {
			return _size;
		}

		[[nodiscard]] constexpr bool operator==(const dynamic& other) const noexcept {
			return _size == other._size;
		}

		[[nodiscard]] constexpr bool operator!=(const dynamic& other) const noexcept {
			return _size != other._size;
		}

		[[nodiscard]] constexpr bool operator>=(const dynamic& other) const noexcept {
			return _size >= other._size;
		}

		[[nodiscard]] constexpr bool operator<=(const dynamic& other) const noexcept {
			return _size <= other._size;
		}

		[[nodiscard]] constexpr bool operator>(const dynamic& other) const noexcept {
			return _size > other._size;
		}

		[[nodiscard]] constexpr bool operator<(const dynamic& other) const noexcept {
			return _size < other._size;
		}

		[[nodiscard]] constexpr std::strong_ordering operator<=>(const dynamic& other) const noexcept {
			return _size <=> other._size;
		}

		[[nodiscard]] constexpr const_iterator begin() const noexcept {
			return const_iterator{ &_data[0] };
		}

		[[nodiscard]] constexpr iterator begin() noexcept {
			return iterator{ &_data[0] };
		}

		[[nodiscard]] constexpr const_iterator end() const noexcept {
			return const_iterator{ &_data[_size] };
		}

		[[nodiscard]] constexpr iterator end() noexcept {
			return iterator{ &_data[_size] };
		}

		[[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept {
			return const_reverse_iterator{ end() };
		}

		[[nodiscard]] constexpr reverse_iterator rbegin() noexcept {
			return reverse_iterator{ end() };
		}

		[[nodiscard]] constexpr const_reverse_iterator rend() const noexcept {
			return const_reverse_iterator{ begin() };
		}

		[[nodiscard]] constexpr reverse_iterator rend() noexcept {
			return reverse_iterator{ begin() };
		}

		[[nodiscard]] constexpr const_iterator cbegin() const noexcept {
			return const_iterator{ &_data[0] };
		}

		[[nodiscard]] constexpr const_iterator cend() const noexcept {
			return const_iterator{ &_data[_size] };
		}

		[[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept {
			return const_reverse_iterator{ cend() };
		}

		[[nodiscard]] constexpr const_reverse_iterator crend() const noexcept {
			return const_reverse_iterator{ cbegin() };
		}

		~dynamic() { destroy_data(); _deallocate(); };
	};

	template <typename _Ty>
	void swap(dynamic<_Ty>& lhs, const dynamic<_Ty>& rhs) {
		lhs.swap(rhs);
	}
}
#endif
