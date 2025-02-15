#pragma once

#ifndef FREESTANDING

#if _DEBUG
#include <map>
#include <stdexcept>
#include <memory>
#endif
#include <utility>

#include <string.h>
#include <cmath>

#define memcpy_y memcpy
#define memset_y memset
#define strlen_y strlen

#else

// Signed integer types
using int8_t = __INT8_TYPE__;
using int16_t = __INT16_TYPE__;
using int32_t = __INT32_TYPE__;
using int64_t = __INT64_TYPE__;

// Unsigned integer types
using uint8_t = __UINT8_TYPE__;
using uint16_t = __UINT16_TYPE__;
using uint32_t = __UINT32_TYPE__;
using uint64_t = __UINT64_TYPE__;

// Pointer-sized integer types
using intptr_t = __INTPTR_TYPE__;
using uintptr_t = __UINTPTR_TYPE__;

// Maximum-width integer types
using intmax_t = __INTMAX_TYPE__;
using uintmax_t = __UINTMAX_TYPE__;

typedef uint64_t size_t;

#include <stdint.h>

typedef struct {
	long int quot;
	long int rem;
} ldiv_t;

typedef struct {
	long long quot;
	long long rem;
} lldiv_t;

ldiv_t ldiv(long int numer, long int denom) { return { 0,0 }; }
lldiv_t lldiv(long long int numer, long long int denom) { return { 0,0 };  }

#define FP_NAN      0x0100
#define FP_INFINITE 0x0200
#define FP_NORMAL   0x0400
#define FP_SUBNORMAL 0x0800
#define FP_ZERO     0x1000

typedef struct {
	int __state;
} mbstate_t;

#define memcpy_y __builtin_memcpy
#define memset_y __builtin_memset
#define strlen_y __builtin_strlen
#define pow __builtin_pow

inline void* operator new(size_t /* count */, void* ptr) noexcept
{
	return ptr;
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

#endif
}