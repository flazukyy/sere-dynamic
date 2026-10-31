#ifndef SALLOCATOR
#define SALLOCATOR
#include <type_traits>
#include <new>
#include <cstddef>

namespace sere {
	template <typename _Ty>
	struct allocator {
		using value_type = _Ty;
		using pointer = _Ty*;
		using const_pointer = const _Ty*;
		using reference = _Ty&;
		using const_reference = const _Ty&;
		using size_type = size_t;
		using difference_type = std::ptrdiff_t;

		static_assert(std::is_object_v<_Ty>, "Elements must be an object type");
		static_assert(!std::is_const_v<_Ty>, "Elements cannot be const");
		static_assert(!std::is_volatile_v<_Ty>, "Elements cannot be volatile");
		static_assert(!std::is_abstract_v<_Ty>, "Elements cannot be abstract types");

		allocator() = default;

		[[nodiscard]] pointer allocate(const size_type size) {
			static_assert(!sizeof(_Ty) == 0, "type must be complete to use for allocation");
			return static_cast<pointer>(::operator new(sizeof(value_type) * size));
		}

		void deallocate(pointer block, size_t count) {
			::operator delete(block, count);
		}

		void deallocate(pointer block) {
			::operator delete(block);
		}
	};
}
#endif