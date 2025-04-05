#pragma once

#include "../Include/Emulator_C.h"

#ifndef FREESTANDING

#if _DEBUG
#include <map>
#include <stdexcept>
#include <memory>
#endif

#include <algorithm>
#include <utility>

#include <string.h>
#include <cmath>

#define memcpy_y memcpy
#define memset_y memset
#define strlen_y strlen
#define pow_y pow
#define abs_y abs

#else

#include <stdint.h>

#define memcpy_y __builtin_memcpy
#define memset_y __builtin_memset
#define strlen_y __builtin_strlen
#define abs_y __builtin_abs

inline uint32_t pow_y( uint32_t base, uint32_t exponent )
{
	uint32_t result = 1;
	for( uint32_t i = 0; i < exponent; ++i )
	{
		result *= base;
	}
	return result;
}

inline void* operator new(size_t /* count */, void* ptr) noexcept
{
	return ptr;
}

inline void operator delete(void* ptr, size_t /* count */) noexcept
{
	// We never allocate memory through new/delete/malloc/free, so this can be empty
}

inline void operator delete(void* ptr) noexcept
{
	// We never allocate memory through new/delete/malloc/free, so this can be empty
}

extern "C" inline void __cxa_pure_virtual()
{
	while (1);
}

#endif

namespace y
{
#ifndef FREESTANDING
	template <typename T>
	constexpr T&& forward(std::remove_reference_t<T>& t) noexcept
	{
		return std::forward<T>(t);
	}

	template <typename T>
	constexpr T&& forward(std::remove_reference_t<T>&& t) noexcept
	{
		return std::forward<T>(t);
	}

	template <class T>
	struct is_trivially_destructible
		: std::is_trivially_destructible<T> {};

	template <bool B, typename T = void>
	using enable_if_t = std::enable_if_t<B, T>;

	template <typename T>
	constexpr T min(T first, T second) {
		return std::min(first, second);
	}

	template <typename T>
	constexpr T max(T first, T second) {
		return std::max(first, second);
	}

#else

	template <class _Tp, _Tp __v>
	struct integral_constant {
		static constexpr const _Tp value = __v;
		typedef _Tp value_type;
		typedef integral_constant type;
		constexpr operator value_type() const noexcept { return value; }
		constexpr value_type operator()() const noexcept { return value; }
	};

	template <class _Tp, _Tp __v>
	constexpr const _Tp integral_constant<_Tp, __v>::value;

	typedef integral_constant<bool, true> true_type;
	typedef integral_constant<bool, false> false_type;

	template <bool _Val>
	using _BoolConstant = integral_constant<bool, _Val>;

	template <class _Tp>
	struct is_lvalue_reference : _BoolConstant<__is_lvalue_reference(_Tp)> {};

	template <class T>
	struct is_trivially_destructible
		: integral_constant<bool, __is_trivially_destructible(T)> {};


	template <typename T>
	constexpr T&& forward(__remove_reference_t(T)& t) noexcept
	{
		return static_cast<T&&>(t);
	}

	template <typename T>
	constexpr T&& forward(__remove_reference_t(T) && t) noexcept
	{
		static_assert(!is_lvalue_reference<T>::value, "cannot forward an rvalue as an lvalue");
		return static_cast<T&&>(t);
	}

	template <bool, class _Tp = void>
	struct enable_if {};
	template <class _Tp>
	struct enable_if<true, _Tp> {
		typedef _Tp type;
	};

	template <bool _Bp, class _Tp = void>
	using __enable_if_t = typename enable_if<_Bp, _Tp>::type;

	template <bool _Bp, class _Tp = void>
	using enable_if_t = typename enable_if<_Bp, _Tp>::type;

	template <typename T>
	constexpr T min(const T a, const T b) {
		return a < b ? a : b;
	}

	template <typename T>
	constexpr T max(const T a, const T b) {
		return a > b ? a : b;
	}

	template <>
	constexpr float min<float>(const float a, const float b) {
		return __builtin_fminf(a, b);
	}

	template <>
	constexpr float max<float>(const float a, const float b) {
		return __builtin_fmaxf(a, b);
	}

#endif
}