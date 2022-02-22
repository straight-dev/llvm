#ifndef PTR_UTIL_HPP
#define PTR_UTIL_HPP
#include <memory>

// A Proposal for the World's Dumbest Smart Pointer (N3740)
template<class T>
class exempt_ptr {
	T* _ptr;
public:
	explicit constexpr exempt_ptr() noexcept : _ptr( nullptr ) {}
	explicit constexpr exempt_ptr( std::nullptr_t ) noexcept : _ptr( nullptr ) {}
	explicit constexpr exempt_ptr( T* raw_ptr ) noexcept : _ptr( raw_ptr ) {}
	explicit constexpr exempt_ptr( const std::unique_ptr<T>& unique ) noexcept : _ptr( unique.get() ) {}
	explicit constexpr operator bool() const { return _ptr != nullptr; }
	bool operator!() const { return _ptr == nullptr; }
	bool operator==( std::nullptr_t ) const { return _ptr == nullptr; }
	bool operator!=( std::nullptr_t ) const { return _ptr != nullptr; }
	bool operator==( const exempt_ptr<std::remove_const_t<T>>& rhs ) const { return _ptr == rhs.get(); }
	bool operator!=( const exempt_ptr<std::remove_const_t<T>>& rhs ) const { return _ptr != rhs.get(); }
	bool operator==( const exempt_ptr<std::add_const_t<T>>& rhs ) const { return _ptr == rhs.get(); }
	bool operator!=( const exempt_ptr<std::add_const_t<T>>& rhs ) const { return _ptr != rhs.get(); }
	bool operator<( const exempt_ptr<T>& rhs ) const { return _ptr < rhs.get(); }
	T& operator*() const { return *_ptr; }
	T* operator->() const { return _ptr; }
	T* get() const { return _ptr; }
	/*implicit*/ operator exempt_ptr<const T>() const { return exempt_ptr<const T>( _ptr ); }
};

// テンプレートの自動推論による変換ヘルパ関数
template<class T>
exempt_ptr<T> constexpr make_exempt( const std::unique_ptr<T>& unique ) { return exempt_ptr<T>( unique ); }
template<class T>
exempt_ptr<T> constexpr make_exempt( T* raw_ptr ) { return exempt_ptr<T>( raw_ptr ); }

// 必要なものだけ
template<class T, class U, typename std::enable_if_t<std::is_same_v<std::remove_const_t<T>, std::remove_const_t<U>>, std::nullptr_t> = nullptr>
bool operator==( const std::unique_ptr<T>& lhs, const exempt_ptr<U>& rhs ) { return lhs.get() == rhs.get(); }
template<class T, class U, typename std::enable_if_t<std::is_same_v<std::remove_const_t<T>, std::remove_const_t<U>>, std::nullptr_t> = nullptr>
bool operator==( const exempt_ptr<T>& lhs, const std::unique_ptr<U>& rhs ) { return lhs.get() == rhs.get(); }
template<class T, class U, typename std::enable_if_t<std::is_same_v<std::remove_const_t<T>, std::remove_const_t<U>>, std::nullptr_t> = nullptr>
bool operator!=( const exempt_ptr<T>& lhs, const std::unique_ptr<U>& rhs ) { return lhs.get() != rhs.get(); }


template<template<class>class Container, class T>
class exempt_ptr_Range {
	const Container<std::unique_ptr<T>>& range;
	
	using BaseIterator = typename Container<std::unique_ptr<T>>::const_iterator;
	struct const_iterator : BaseIterator {
		exempt_ptr<T> operator*() const { return make_exempt( **static_cast<const BaseIterator*>(this) ); }
		exempt_ptr<T> operator[]( std::ptrdiff_t i ) const { return make_exempt( (*static_cast<const BaseIterator*>(this))[i] ); }
		const_iterator( BaseIterator iter ) : BaseIterator( iter ) {}
	};
public:
	explicit exempt_ptr_Range( const Container<std::unique_ptr<T>>& range ) : range( range ) {}
	auto begin() const { return const_iterator( range.begin() ); }
	auto end() const { return const_iterator( range.end() ); }
	bool empty() const { return range.empty(); }
	std::size_t size() const { return range.size(); }
	exempt_ptr<T> front() const { return make_exempt( range.front() ); }
	exempt_ptr<T> back() const { return make_exempt( range.back() ); }
	template<class Pred>
	exempt_ptr<T> find_if( Pred pred ) const { 
		const auto it = std::find_if( begin(), end(), pred );
		return it == end() ? make_exempt<T>( nullptr ) : *it;
	}
};


#endif // PTR_UTIL_HPP
