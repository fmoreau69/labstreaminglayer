//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Pablo Halpern 2009. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.lslboost.org/LICENSE_1_0.txt)
//
//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2011-2013. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.lslboost.org/LICENSE_1_0.txt)
//
// See http://www.lslboost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_CONTAINER_ALLOCATOR_SCOPED_ALLOCATOR_HPP
#define BOOST_CONTAINER_ALLOCATOR_SCOPED_ALLOCATOR_HPP

#if defined (_MSC_VER)
#  pragma once
#endif

#include <lslboost/container/detail/config_begin.hpp>
#include <lslboost/container/detail/workaround.hpp>

#include <lslboost/container/allocator_traits.hpp>
#include <lslboost/container/scoped_allocator_fwd.hpp>

#include <lslboost/container/detail/addressof.hpp>
#include <lslboost/container/detail/mpl.hpp>
#include <lslboost/container/detail/pair.hpp>
#include <lslboost/container/detail/type_traits.hpp>

#include <lslboost/move/adl_move_swap.hpp>
#if defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES)
#include <lslboost/move/detail/fwd_macros.hpp>
#endif
#include <lslboost/move/utility_core.hpp>

#include <lslboost/core/no_exceptions_support.hpp>

namespace lslboost { namespace container {

//! <b>Remark</b>: if a specialization constructible_with_allocator_suffix<X>::value is true, indicates that T may be constructed
//! with an allocator as its last constructor argument.  Ideally, all constructors of T (including the
//! copy and move constructors) should have a variant that accepts a final argument of
//! allocator_type.
//!
//! <b>Requires</b>: if a specialization constructible_with_allocator_suffix<X>::value is true, T must have a nested type,
//! allocator_type and at least one constructor for which allocator_type is the last
//! parameter.  If not all constructors of T can be called with a final allocator_type argument,
//! and if T is used in a context where a container must call such a constructor, then the program is
//! ill-formed.
//!
//! <code>
//!  template <class T, class Allocator = allocator<T> >
//!  class Z {
//!    public:
//!      typedef Allocator allocator_type;
//!
//!    // Default constructor with optional allocator suffix
//!    Z(const allocator_type& a = allocator_type());
//!
//!    // Copy constructor and allocator-extended copy constructor
//!    Z(const Z& zz);
//!    Z(const Z& zz, const allocator_type& a);
//! };
//!
//! // Specialize trait for class template Z
//! template <class T, class Allocator = allocator<T> >
//! struct constructible_with_allocator_suffix<Z<T,Allocator> >
//! { static const bool value = true;  };
//! </code>
//!
//! <b>Note</b>: This trait is a workaround inspired by "N2554: The Scoped A Model (Rev 2)"
//! (Pablo Halpern, 2008-02-29) to backport the scoped allocator model to C++03, as
//! in C++03 there is no mechanism to detect if a type can be constructed from arbitrary arguments.
//! Applications aiming portability with several compilers should always define this trait.
//!
//! In conforming C++11 compilers or compilers supporting SFINAE expressions
//! (when BOOST_NO_SFINAE_EXPR is NOT defined), this trait is ignored and C++11 rules will be used
//! to detect if a type should be constructed with suffix or prefix allocator arguments.
template <class T>
struct constructible_with_allocator_suffix
{  static const bool value = false; };

//! <b>Remark</b>: if a specialization constructible_with_allocator_prefix<X>::value is true, indicates that T may be constructed
//! with allocator_arg and T::allocator_type as its first two constructor arguments.
//! Ideally, all constructors of T (including the copy and move constructors) should have a variant
//! that accepts these two initial arguments.
//!
//! <b>Requires</b>: specialization constructible_with_allocator_prefix<X>::value is true, T must have a nested type,
//! allocator_type and at least one constructor for which allocator_arg_t is the first
//! parameter and allocator_type is the second parameter.  If not all constructors of T can be
//! called with these initial arguments, and if T is used in a context where a container must call such
//! a constructor, then the program is ill-formed.
//!
//! <code>
//! template <class T, class Allocator = allocator<T> >
//! class Y {
//!    public:
//!       typedef Allocator allocator_type;
//!
//!       // Default constructor with and allocator-extended default constructor
//!       Y();
//!       Y(allocator_arg_t, const allocator_type& a);
//!
//!       // Copy constructor and allocator-extended copy constructor
//!       Y(const Y& yy);
//!       Y(allocator_arg_t, const allocator_type& a, const Y& yy);
//!
//!       // Variadic constructor and allocator-extended variadic constructor
//!       template<class ...Args> Y(Args&& args...);
//!       template<class ...Args>
//!       Y(allocator_arg_t, const allocator_type& a, BOOST_FWD_REF(Args)... args);
//! };
//!
//! // Specialize trait for class template Y
//! template <class T, class Allocator = allocator<T> >
//! struct constructible_with_allocator_prefix<Y<T,Allocator> >
//! { static const bool value = true;  };
//!
//! </code>
//!
//! <b>Note</b>: This trait is a workaround inspired by "N2554: The Scoped Allocator Model (Rev 2)"
//! (Pablo Halpern, 2008-02-29) to backport the scoped allocator model to C++03, as
//! in C++03 there is no mechanism to detect if a type can be constructed from arbitrary arguments.
//! Applications aiming portability with several compilers should always define this trait.
//!
//! In conforming C++11 compilers or compilers supporting SFINAE expressions
//! (when BOOST_NO_SFINAE_EXPR is NOT defined), this trait is ignored and C++11 rules will be used
//! to detect if a type should be constructed with suffix or prefix allocator arguments.
template <class T>
struct constructible_with_allocator_prefix
{  static const bool value = false; };

#ifndef BOOST_CONTAINER_DOXYGEN_INVOKED

namespace container_detail {

template<typename T, typename Allocator>
struct uses_allocator_imp
{
   // Use SFINAE (Substitution Failure Is Not An Error) to detect the
   // presence of an 'allocator_type' nested type convertilble from Allocator.
   private:
   typedef char yes_type;
   struct no_type{ char dummy[2]; };

   // Match this function if TypeT::allocator_type exists and is
   // implicitly convertible from Allocator
   template <class U>
   static yes_type test(typename U::allocator_type);

   // Match this function if TypeT::allocator_type does not exist or is
   // not convertible from Allocator.
   template <typename U>
   static no_type test(...);
   static Allocator alloc;  // Declared but not defined

   public:
   static const bool value = sizeof(test<T>(alloc)) == sizeof(yes_type);
};

}  //namespace container_detail {

#endif   //#ifndef BOOST_CONTAINER_DOXYGEN_INVOKED

//! <b>Remark</b>: Automatically detects if T has a nested allocator_type that is convertible from
//! Allocator. Meets the BinaryTypeTrait requirements ([meta.rqmts] 20.4.1). A program may
//! specialize this type to define uses_allocator<X>::value as true for a T of user-defined type if T does not
//! have a nested allocator_type but is nonetheless constructible using the specified Allocator.
//!
//! <b>Result</b>: uses_allocator<T, Allocator>::value== true if Convertible<Allocator,T::allocator_type>,
//! false otherwise.
template <typename T, typename Allocator>
struct uses_allocator
   : container_detail::uses_allocator_imp<T, Allocator>
{};

#ifndef BOOST_CONTAINER_DOXYGEN_INVOKED

namespace container_detail {

template <typename Allocator>
struct is_scoped_allocator_imp
{
   typedef char yes_type;
   struct no_type{ char dummy[2]; };

   template <typename T>
   static yes_type test(typename T::outer_allocator_type*);

   template <typename T>
   static int test(...);

   static const bool value = (sizeof(yes_type) == sizeof(test<Allocator>(0)));
};

template<class MaybeScopedAlloc, bool = is_scoped_allocator_imp<MaybeScopedAlloc>::value >
struct outermost_allocator_type_impl
{
   typedef typename MaybeScopedAlloc::outer_allocator_type outer_type;
   typedef typename outermost_allocator_type_impl<outer_type>::type type;
};

template<class MaybeScopedAlloc>
struct outermost_allocator_type_impl<MaybeScopedAlloc, false>
{
   typedef MaybeScopedAlloc type;
};

template<class MaybeScopedAlloc, bool = is_scoped_allocator_imp<MaybeScopedAlloc>::value >
struct outermost_allocator_imp
{
   typedef MaybeScopedAlloc type;

   static type &get(MaybeScopedAlloc &a)
   {  return a;  }

   static const type &get(const MaybeScopedAlloc &a)
   {  return a;  }
};

template<class MaybeScopedAlloc>
struct outermost_allocator_imp<MaybeScopedAlloc, true>
{
   typedef typename MaybeScopedAlloc::outer_allocator_type outer_type;
   typedef typename outermost_allocator_type_impl<outer_type>::type type;

   static type &get(MaybeScopedAlloc &a)
   {  return outermost_allocator_imp<outer_type>::get(a.outer_allocator());  }

   static const type &get(const MaybeScopedAlloc &a)
   {  return outermost_allocator_imp<outer_type>::get(a.outer_allocator());  }
};

}  //namespace container_detail {

template <typename Allocator>
struct is_scoped_allocator
   : container_detail::is_scoped_allocator_imp<Allocator>
{};

template <typename Allocator>
struct outermost_allocator
   : container_detail::outermost_allocator_imp<Allocator>
{};

template <typename Allocator>
typename container_detail::outermost_allocator_imp<Allocator>::type &
   get_outermost_allocator(Allocator &a)
{  return container_detail::outermost_allocator_imp<Allocator>::get(a);   }

template <typename Allocator>
const typename container_detail::outermost_allocator_imp<Allocator>::type &
   get_outermost_allocator(const Allocator &a)
{  return container_detail::outermost_allocator_imp<Allocator>::get(a);   }

namespace container_detail {

// Check if we can detect is_convertible using advanced SFINAE expressions
#if !defined(BOOST_NO_CXX11_DECLTYPE) && !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES)

   //! Code inspired by Mathias Gaunard's is_convertible.cpp found in the Boost mailing list
   //! http://lslboost.2283326.n4.nabble.com/type-traits-is-constructible-when-decltype-is-supported-td3575452.html
   //! Thanks Mathias!

   //With variadic templates, we need a single class to implement the trait
   template<class T, class ...Args>
   struct is_constructible
   {
      typedef char yes_type;
      struct no_type
      { char padding[2]; };

      template<std::size_t N>
      struct dummy;

      template<class X>
      static decltype(X(lslboost::move_detail::declval<Args>()...), true_type()) test(int);

      template<class X>
      static no_type test(...);

      static const bool value = sizeof(test<T>(0)) == sizeof(yes_type);
   };

   template <class T, class InnerAlloc, class ...Args>
   struct is_constructible_with_allocator_prefix
      : is_constructible<T, allocator_arg_t, InnerAlloc, Args...>
   {};

#else    // #if !defined(BOOST_NO_SFINAE_EXPR) && !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES)

   //Without advanced SFINAE expressions, we can't use is_constructible
   //so backup to constructible_with_allocator_xxx

   #if !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES)

   template <class T, class InnerAlloc, class ...Args>
   struct is_constructible_with_allocator_prefix
      : constructible_with_allocator_prefix<T>
   {};

   template <class T, class InnerAlloc, class ...Args>
   struct is_constructible_with_allocator_suffix
      : constructible_with_allocator_suffix<T>
   {};

   #else    // #if !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES)

   template <class T, class InnerAlloc, BOOST_MOVE_CLASSDFLT9>
   struct is_constructible_with_allocator_prefix
      : constructible_with_allocator_prefix<T>
   {};

   template <class T, class InnerAlloc, BOOST_MOVE_CLASSDFLT9>
   struct is_constructible_with_allocator_suffix
      : constructible_with_allocator_suffix<T>
   {};

   #endif   // #if !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES)

#endif   // #if !defined(BOOST_NO_SFINAE_EXPR)

#if !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES)

// allocator_arg_t
template < typename OutermostAlloc
         , typename InnerAlloc
         , typename T
         , class ...Args
         >
inline void dispatch_allocator_prefix_suffix
   ( true_type use_alloc_prefix, OutermostAlloc& outermost_alloc
   , InnerAlloc& inner_alloc, T* p, BOOST_FWD_REF(Args) ...args)
{
   (void)use_alloc_prefix;
   allocator_traits<OutermostAlloc>::construct
      ( outermost_alloc, p, allocator_arg, inner_alloc, ::lslboost::forward<Args>(args)...);
}

// allocator suffix
template < typename OutermostAlloc
         , typename InnerAlloc
         , typename T
         , class ...Args
         >
inline void dispatch_allocator_prefix_suffix
   ( false_type use_alloc_prefix, OutermostAlloc& outermost_alloc
   , InnerAlloc &inner_alloc, T* p, BOOST_FWD_REF(Args)...args)
{
   (void)use_alloc_prefix;
   allocator_traits<OutermostAlloc>::construct
      (outermost_alloc, p, ::lslboost::forward<Args>(args)..., inner_alloc);
}

template < typename OutermostAlloc
         , typename InnerAlloc
         , typename T
         , class ...Args
         >
inline void dispatch_uses_allocator
   ( true_type uses_allocator, OutermostAlloc& outermost_alloc
   , InnerAlloc& inner_alloc, T* p, BOOST_FWD_REF(Args)...args)
{
   (void)uses_allocator;
   //BOOST_STATIC_ASSERT((is_constructible_with_allocator_prefix<T, InnerAlloc, Args...>::value ||
   //                     is_constructible_with_allocator_suffix<T, InnerAlloc, Args...>::value ));
   dispatch_allocator_prefix_suffix
      ( bool_< is_constructible_with_allocator_prefix<T, InnerAlloc, Args...>::value>()
      , outermost_alloc, inner_alloc, p, ::lslboost::forward<Args>(args)...);
}

template < typename OutermostAlloc
         , typename InnerAlloc
         , typename T
         , class ...Args
         >
inline void dispatch_uses_allocator
   ( false_type uses_allocator, OutermostAlloc & outermost_alloc
   , InnerAlloc & inner_alloc
   ,T* p, BOOST_FWD_REF(Args)...args)
{
   (void)uses_allocator; (void)inner_alloc;
   allocator_traits<OutermostAlloc>::construct
      (outermost_alloc, p, ::lslboost::forward<Args>(args)...);
}

#else    //#if !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES)

#define BOOST_CONTAINER_SCOPED_ALLOCATOR_DISPATCH_USES_ALLOCATOR_CODE(N) \
template < typename OutermostAlloc, typename InnerAlloc, typename T\
         BOOST_MOVE_I##N BOOST_MOVE_CLASS##N > \
inline void dispatch_allocator_prefix_suffix\
   (true_type use_alloc_prefix, OutermostAlloc& outermost_alloc,\
    InnerAlloc& inner_alloc, T* p BOOST_MOVE_I##N BOOST_MOVE_UREF##N)\
{\
   (void)use_alloc_prefix,\
   allocator_traits<OutermostAlloc>::construct\
      (outermost_alloc, p, allocator_arg, inner_alloc BOOST_MOVE_I##N BOOST_MOVE_FWD##N);\
}\
\
template < typename OutermostAlloc, typename InnerAlloc, typename T\
         BOOST_MOVE_I##N BOOST_MOVE_CLASS##N >\
inline void dispatch_allocator_prefix_suffix\
   (false_type use_alloc_prefix, OutermostAlloc& outermost_alloc,\
    InnerAlloc& inner_alloc, T* p BOOST_MOVE_I##N BOOST_MOVE_UREF##N)\
{\
   (void)use_alloc_prefix;\
   allocator_traits<OutermostAlloc>::construct\
      (outermost_alloc, p BOOST_MOVE_I##N BOOST_MOVE_FWD##N, inner_alloc);\
}\
\
template < typename OutermostAlloc, typename InnerAlloc, typename T\
           BOOST_MOVE_I##N BOOST_MOVE_CLASS##N >\
inline void dispatch_uses_allocator\
   (true_type uses_allocator, OutermostAlloc& outermost_alloc,\
    InnerAlloc& inner_alloc, T* p BOOST_MOVE_I##N BOOST_MOVE_UREF##N)\
{\
   (void)uses_allocator;\
   dispatch_allocator_prefix_suffix\
      ( bool_< is_constructible_with_allocator_prefix<T, InnerAlloc BOOST_MOVE_I##N BOOST_MOVE_TARG##N>::value >()\
      , outermost_alloc, inner_alloc, p BOOST_MOVE_I##N BOOST_MOVE_FWD##N);\
}\
\
template < typename OutermostAlloc, typename InnerAlloc, typename T\
           BOOST_MOVE_I##N BOOST_MOVE_CLASS##N >\
inline void dispatch_uses_allocator\
   (false_type uses_allocator, OutermostAlloc &outermost_alloc,\
    InnerAlloc &inner_alloc, T* p BOOST_MOVE_I##N BOOST_MOVE_UREF##N)\
{\
   (void)uses_allocator; (void)inner_alloc;\
   allocator_traits<OutermostAlloc>::construct(outermost_alloc, p BOOST_MOVE_I##N BOOST_MOVE_FWD##N);\
}\
//
BOOST_MOVE_ITERATE_0TO9(BOOST_CONTAINER_SCOPED_ALLOCATOR_DISPATCH_USES_ALLOCATOR_CODE)
#undef BOOST_CONTAINER_SCOPED_ALLOCATOR_DISPATCH_USES_ALLOCATOR_CODE

#endif   //#if !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES)

#if !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES)

template <typename OuterAlloc, class ...InnerAllocs>
class scoped_allocator_adaptor_base
   : public OuterAlloc
{
   typedef allocator_traits<OuterAlloc> outer_traits_type;
   BOOST_COPYABLE_AND_MOVABLE(scoped_allocator_adaptor_base)

   public:
   template <class OuterA2>
   struct rebind_base
   {
      typedef scoped_allocator_adaptor_base<OuterA2, InnerAllocs...> other;
   };

   typedef OuterAlloc outer_allocator_type;
   typedef scoped_allocator_adaptor<InnerAllocs...>   inner_allocator_type;
   typedef allocator_traits<inner_allocator_type>     inner_traits_type;
   typedef scoped_allocator_adaptor
      <OuterAlloc, InnerAllocs...>                    scoped_allocator_type;
   typedef container_detail::bool_<
      outer_traits_type::propagate_on_container_copy_assignment::value ||
      inner_allocator_type::propagate_on_container_copy_assignment::value
      > propagate_on_container_copy_assignment;
   typedef container_detail::bool_<
      outer_traits_type::propagate_on_container_move_assignment::value ||
      inner_allocator_type::propagate_on_container_move_assignment::value
      > propagate_on_container_move_assignment;
   typedef container_detail::bool_<
      outer_traits_type::propagate_on_container_swap::value ||
      inner_allocator_type::propagate_on_container_swap::value
      > propagate_on_container_swap;
   typedef container_detail::bool_<
      outer_traits_type::is_always_equal::value &&
      inner_allocator_type::is_always_equal::value
      > is_always_equal;

   scoped_allocator_adaptor_base()
      {}

   template <class OuterA2>
   scoped_allocator_adaptor_base(BOOST_FWD_REF(OuterA2) outerAlloc, const InnerAllocs &...args)
      : outer_allocator_type(::lslboost::forward<OuterA2>(outerAlloc))
      , m_inner(args...)
      {}

   scoped_allocator_adaptor_base(const scoped_allocator_adaptor_base& other)
      : outer_allocator_type(other.outer_allocator())
      , m_inner(other.inner_allocator())
      {}

   scoped_allocator_adaptor_base(BOOST_RV_REF(scoped_allocator_adaptor_base) other)
      : outer_allocator_type(::lslboost::move(other.outer_allocator()))
      , m_inner(::lslboost::move(other.inner_allocator()))
      {}

   template <class OuterA2>
   scoped_allocator_adaptor_base
      (const scoped_allocator_adaptor_base<OuterA2, InnerAllocs...>& other)
      : outer_allocator_type(other.outer_allocator())
      , m_inner(other.inner_allocator())
      {}

   template <class OuterA2>
   scoped_allocator_adaptor_base
      (BOOST_RV_REF_BEG scoped_allocator_adaptor_base
         <OuterA2, InnerAllocs...> BOOST_RV_REF_END other)
      : outer_allocator_type(other.outer_allocator())
      , m_inner(other.inner_allocator())
      {}

   public:
   struct internal_type_t{};

   template <class OuterA2>
   scoped_allocator_adaptor_base
      ( internal_type_t
      , BOOST_FWD_REF(OuterA2) outerAlloc
      , const inner_allocator_type &inner)
      : outer_allocator_type(::lslboost::forward<OuterA2>(outerAlloc))
      , m_inner(inner)
   {}

   public:

   scoped_allocator_adaptor_base &operator=
      (BOOST_COPY_ASSIGN_REF(scoped_allocator_adaptor_base) other)
   {
      outer_allocator_type::operator=(other.outer_allocator());
      m_inner = other.inner_allocator();
      return *this;
   }

   scoped_allocator_adaptor_base &operator=(BOOST_RV_REF(scoped_allocator_adaptor_base) other)
   {
      outer_allocator_type::operator=(lslboost::move(other.outer_allocator()));
      m_inner = ::lslboost::move(other.inner_allocator());
      return *this;
   }

   void swap(scoped_allocator_adaptor_base &r)
   {
      lslboost::adl_move_swap(this->outer_allocator(), r.outer_allocator());
      lslboost::adl_move_swap(this->m_inner, r.inner_allocator());
   }

   friend void swap(scoped_allocator_adaptor_base &l, scoped_allocator_adaptor_base &r)
   {  l.swap(r);  }

   inner_allocator_type&       inner_allocator() BOOST_NOEXCEPT_OR_NOTHROW
      { return m_inner; }

   inner_allocator_type const& inner_allocator() const BOOST_NOEXCEPT_OR_NOTHROW
      { return m_inner; }

   outer_allocator_type      & outer_allocator() BOOST_NOEXCEPT_OR_NOTHROW
      { return static_cast<outer_allocator_type&>(*this); }

   const outer_allocator_type &outer_allocator() const BOOST_NOEXCEPT_OR_NOTHROW
      { return static_cast<const outer_allocator_type&>(*this); }

   scoped_allocator_type select_on_container_copy_construction() const
   {
      return scoped_allocator_type
         (internal_type_t()
         ,outer_traits_type::select_on_container_copy_construction(this->outer_allocator())
         ,inner_traits_type::select_on_container_copy_construction(this->inner_allocator())
         );
   }

   private:
   inner_allocator_type m_inner;
};

#else //#if !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES)

//Let's add a dummy first template parameter to allow creating
//specializations up to maximum InnerAlloc count
template <typename OuterAlloc, bool Dummy, BOOST_MOVE_CLASSDFLT9>
class scoped_allocator_adaptor_base;

//Specializations for the adaptor with InnerAlloc allocators

#define BOOST_CONTAINER_SCOPED_ALLOCATOR_ADAPTOR_BASE_CODE(N)\
template <typename OuterAlloc BOOST_MOVE_I##N BOOST_MOVE_CLASS##N>\
class scoped_allocator_adaptor_base<OuterAlloc, true, BOOST_MOVE_TARG##N>\
   : public OuterAlloc\
{\
   typedef allocator_traits<OuterAlloc> outer_traits_type;\
   BOOST_COPYABLE_AND_MOVABLE(scoped_allocator_adaptor_base)\
   \
   public:\
   template <class OuterA2>\
   struct rebind_base\
   {\
      typedef scoped_allocator_adaptor_base<OuterA2, true, BOOST_MOVE_TARG##N> other;\
   };\
   \
   typedef OuterAlloc outer_allocator_type;\
   typedef scoped_allocator_adaptor<BOOST_MOVE_TARG##N> inner_allocator_type;\
   typedef scoped_allocator_adaptor<OuterAlloc, BOOST_MOVE_TARG##N> scoped_allocator_type;\
   typedef allocator_traits<inner_allocator_type> inner_traits_type;\
   typedef container_detail::bool_<\
      outer_traits_type::propagate_on_container_copy_assignment::value ||\
      inner_allocator_type::propagate_on_container_copy_assignment::value\
      > propagate_on_container_copy_assignment;\
   typedef container_detail::bool_<\
      outer_traits_type::propagate_on_container_move_assignment::value ||\
      inner_allocator_type::propagate_on_container_move_assignment::value\
      > propagate_on_container_move_assignment;\
   typedef container_detail::bool_<\
      outer_traits_type::propagate_on_container_swap::value ||\
      inner_allocator_type::propagate_on_container_swap::value\
      > propagate_on_container_swap;\
   \
   typedef container_detail::bool_<\
      outer_traits_type::is_always_equal::value &&\
      inner_allocator_type::is_always_equal::value\
      > is_always_equal;\
   \
   scoped_allocator_adaptor_base(){}\
   \
   template <class OuterA2>\
   scoped_allocator_adaptor_base(BOOST_FWD_REF(OuterA2) outerAlloc, BOOST_MOVE_CREF##N)\
      : outer_allocator_type(::lslboost::forward<OuterA2>(outerAlloc))\
      , m_inner(BOOST_MOVE_ARG##N)\
      {}\
   \
   scoped_allocator_adaptor_base(const scoped_allocator_adaptor_base& other)\
      : outer_allocator_type(other.outer_allocator())\
      , m_inner(other.inner_allocator())\
      {}\
   \
   scoped_allocator_adaptor_base(BOOST_RV_REF(scoped_allocator_adaptor_base) other)\
      : outer_allocator_type(::lslboost::move(other.outer_allocator()))\
      , m_inner(::lslboost::move(other.inner_allocator()))\
      {}\
   \
   template <class OuterA2>\
   scoped_allocator_adaptor_base\
      (const scoped_allocator_adaptor_base<OuterA2, true, BOOST_MOVE_TARG##N>& other)\
      : outer_allocator_type(other.outer_allocator())\
      , m_inner(other.inner_allocator())\
      {}\
   \
   template <class OuterA2>\
   scoped_allocator_adaptor_base\
      (BOOST_RV_REF_BEG scoped_allocator_adaptor_base<OuterA2, true, BOOST_MOVE_TARG##N> BOOST_RV_REF_END other)\
      : outer_allocator_type(other.outer_allocator())\
      , m_inner(other.inner_allocator())\
      {}\
   \
   public:\
   struct internal_type_t{};\
   \
   template <class OuterA2>\
   scoped_allocator_adaptor_base\
      ( internal_type_t, BOOST_FWD_REF(OuterA2) outerAlloc, const inner_allocator_type &inner)\
      : outer_allocator_type(::lslboost::forward<OuterA2>(outerAlloc))\
      , m_inner(inner)\
   {}\
   \
   public:\
   scoped_allocator_adaptor_base &operator=\
      (BOOST_COPY_ASSIGN_REF(scoped_allocator_adaptor_base) other)\
   {\
      outer_allocator_type::operator=(other.outer_allocator());\
      m_inner = other.inner_allocator();\
      return *this;\
   }\
   \
   scoped_allocator_adaptor_base &operator=(BOOST_RV_REF(scoped_allocator_adaptor_base) other)\
   {\
      outer_allocator_type::operator=(lslboost::move(other.outer_allocator()));\
      m_inner = ::lslboost::move(other.inner_allocator());\
      return *this;\
   }\
   \
   void swap(scoped_allocator_adaptor_base &r)\
   {\
      lslboost::adl_move_swap(this->outer_allocator(), r.outer_allocator());\
      lslboost::adl_move_swap(this->m_inner, r.inner_allocator());\
   }\
   \
   friend void swap(scoped_allocator_adaptor_base &l, scoped_allocator_adaptor_base &r)\
   {  l.swap(r);  }\
   \
   inner_allocator_type&       inner_allocator()\
      { return m_inner; }\
   \
   inner_allocator_type const& inner_allocator() const\
      { return m_inner; }\
   \
   outer_allocator_type      & outer_allocator()\
      { return static_cast<outer_allocator_type&>(*this); }\
   \
   const outer_allocator_type &outer_allocator() const\
      { return static_cast<const outer_allocator_type&>(*this); }\
   \
   scoped_allocator_type select_on_container_copy_construction() const\
   {\
      return scoped_allocator_type\
         (internal_type_t()\
         ,outer_traits_type::select_on_container_copy_construction(this->outer_allocator())\
         ,inner_traits_type::select_on_container_copy_construction(this->inner_allocator())\
         );\
   }\
   private:\
   inner_allocator_type m_inner;\
};\
//!
BOOST_MOVE_ITERATE_1TO9(BOOST_CONTAINER_SCOPED_ALLOCATOR_ADAPTOR_BASE_CODE)
#undef BOOST_CONTAINER_SCOPED_ALLOCATOR_ADAPTOR_BASE_CODE

#endif   //#if !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES)

#if defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES) && !defined(BOOST_CONTAINER_DOXYGEN_INVOKED)
   #define BOOST_CONTAINER_SCOPEDALLOC_DUMMYTRUE      ,true
   #define BOOST_CONTAINER_SCOPEDALLOC_ALLINNER       BOOST_MOVE_TARG9
   #define BOOST_CONTAINER_SCOPEDALLOC_ALLINNERCLASS  BOOST_MOVE_CLASS9
#else
   #define BOOST_CONTAINER_SCOPEDALLOC_DUMMYTRUE
   #define BOOST_CONTAINER_SCOPEDALLOC_ALLINNER       InnerAllocs...
   #define BOOST_CONTAINER_SCOPEDALLOC_ALLINNERCLASS  typename... InnerAllocs
#endif

//Specialization for adaptor without any InnerAlloc
template <typename OuterAlloc>
class scoped_allocator_adaptor_base< OuterAlloc BOOST_CONTAINER_SCOPEDALLOC_DUMMYTRUE>
   : public OuterAlloc
{
   BOOST_COPYABLE_AND_MOVABLE(scoped_allocator_adaptor_base)
   public:

   template <class U>
   struct rebind_base
   {
      typedef scoped_allocator_adaptor_base
         <typename allocator_traits<OuterAlloc>::template portable_rebind_alloc<U>::type
         BOOST_CONTAINER_SCOPEDALLOC_DUMMYTRUE > other;
   };

   typedef OuterAlloc                           outer_allocator_type;
   typedef allocator_traits<OuterAlloc>         outer_traits_type;
   typedef scoped_allocator_adaptor<OuterAlloc> inner_allocator_type;
   typedef inner_allocator_type                 scoped_allocator_type;
   typedef allocator_traits<inner_allocator_type>   inner_traits_type;
   typedef typename outer_traits_type::
      propagate_on_container_copy_assignment    propagate_on_container_copy_assignment;
   typedef typename outer_traits_type::
      propagate_on_container_move_assignment    propagate_on_container_move_assignment;
   typedef typename outer_traits_type::
      propagate_on_container_swap               propagate_on_container_swap;
   typedef typename outer_traits_type::
      is_always_equal                           is_always_equal;

   scoped_allocator_adaptor_base()
      {}

   template <class OuterA2>
   scoped_allocator_adaptor_base(BOOST_FWD_REF(OuterA2) outerAlloc)
      : outer_allocator_type(::lslboost::forward<OuterA2>(outerAlloc))
      {}

   scoped_allocator_adaptor_base(const scoped_allocator_adaptor_base& other)
      : outer_allocator_type(other.outer_allocator())
      {}

   scoped_allocator_adaptor_base(BOOST_RV_REF(scoped_allocator_adaptor_base) other)
      : outer_allocator_type(::lslboost::move(other.outer_allocator()))
      {}

   template <class OuterA2>
   scoped_allocator_adaptor_base
      (const scoped_allocator_adaptor_base<OuterA2 BOOST_CONTAINER_SCOPEDALLOC_DUMMYTRUE>& other)
      : outer_allocator_type(other.outer_allocator())
      {}

   template <class OuterA2>
   scoped_allocator_adaptor_base
      (BOOST_RV_REF_BEG scoped_allocator_adaptor_base<OuterA2 BOOST_CONTAINER_SCOPEDALLOC_DUMMYTRUE> BOOST_RV_REF_END other)
      : outer_allocator_type(other.outer_allocator())
      {}

   public:
   struct internal_type_t{};

   template <class OuterA2>
   scoped_allocator_adaptor_base(internal_type_t, BOOST_FWD_REF(OuterA2) outerAlloc, const inner_allocator_type &)
      : outer_allocator_type(::lslboost::forward<OuterA2>(outerAlloc))
      {}

   public:
   scoped_allocator_adaptor_base &operator=(BOOST_COPY_ASSIGN_REF(scoped_allocator_adaptor_base) other)
   {
      outer_allocator_type::operator=(other.outer_allocator());
      return *this;
   }

   scoped_allocator_adaptor_base &operator=(BOOST_RV_REF(scoped_allocator_adaptor_base) other)
   {
      outer_allocator_type::operator=(lslboost::move(other.outer_allocator()));
      return *this;
   }

   void swap(scoped_allocator_adaptor_base &r)
   {
      lslboost::adl_move_swap(this->outer_allocator(), r.outer_allocator());
   }

   friend void swap(scoped_allocator_adaptor_base &l, scoped_allocator_adaptor_base &r)
   {  l.swap(r);  }

   inner_allocator_type&       inner_allocator()
      { return static_cast<inner_allocator_type&>(*this); }

   inner_allocator_type const& inner_allocator() const
      { return static_cast<const inner_allocator_type&>(*this); }

   outer_allocator_type      & outer_allocator()
      { return static_cast<outer_allocator_type&>(*this); }

   const outer_allocator_type &outer_allocator() const
      { return static_cast<const outer_allocator_type&>(*this); }

   scoped_allocator_type select_on_container_copy_construction() const
   {
      return scoped_allocator_type
         (internal_type_t()
         ,outer_traits_type::select_on_container_copy_construction(this->outer_allocator())
         //Don't use inner_traits_type::select_on_container_copy_construction(this->inner_allocator())
         //as inner_allocator() is equal to *this and that would trigger an infinite loop
         , this->inner_allocator()
         );
   }
};

}  //namespace container_detail {

#endif   //#ifndef BOOST_CONTAINER_DOXYGEN_INVOKED

//Scoped allocator
#if !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES) || defined(BOOST_CONTAINER_DOXYGEN_INVOKED)

#if !defined(BOOST_CONTAINER_UNIMPLEMENTED_PACK_EXPANSION_TO_FIXED_LIST)

//! This class is a C++03-compatible implementation of std::scoped_allocator_adaptor.
//! The class template scoped_allocator_adaptor is an allocator template that specifies
//! the memory resource (the outer allocator) to be used by a container (as any other
//! allocator does) and also specifies an inner allocator resource to be passed to
//! the constructor of every element within the container.
//!
//! This adaptor is
//! instantiated with one outer and zero or more inner allocator types. If
//! instantiated with only one allocator type, the inner allocator becomes the
//! scoped_allocator_adaptor itself, thus using the same allocator resource for the
//! container and every element within the container and, if the elements themselves
//! are containers, each of their elements recursively. If instantiated with more than
//! one allocator, the first allocator is the outer allocator for use by the container,
//! the second allocator is passed to the constructors of the container's elements,
//! and, if the elements themselves are containers, the third allocator is passed to
//! the elements' elements, and so on. If containers are nested to a depth greater
//! than the number of allocators, the last allocator is used repeatedly, as in the
//! single-allocator case, for any remaining recursions.
//!
//! [<b>Note</b>: The
//! scoped_allocator_adaptor is derived from the outer allocator type so it can be
//! substituted for the outer allocator type in most expressions. -end note]
//!
//! In the construct member functions, <code>OUTERMOST(x)</code> is x if x does not have
//! an <code>outer_allocator()</code> member function and
//! <code>OUTERMOST(x.outer_allocator())</code> otherwise; <code>OUTERMOST_ALLOC_TRAITS(x)</code> is
//! <code>allocator_traits<decltype(OUTERMOST(x))></code>.
//!
//! [<b>Note</b>: <code>OUTERMOST(x)</code> and
//! <code>OUTERMOST_ALLOC_TRAITS(x)</code> are recursive operations. It is incumbent upon
//! the definition of <code>outer_allocator()</code> to ensure that the recursion terminates.
//! It will terminate for all instantiations of scoped_allocator_adaptor. -end note]
template <typename OuterAlloc, typename ...InnerAllocs>
class scoped_allocator_adaptor

#else // #if !defined(BOOST_CONTAINER_UNIMPLEMENTED_PACK_EXPANSION_TO_FIXED_LIST)

template <typename OuterAlloc, typename ...InnerAllocs>
class scoped_allocator_adaptor<OuterAlloc, InnerAllocs...>

#endif   // #if !defined(BOOST_CONTAINER_UNIMPLEMENTED_PACK_EXPANSION_TO_FIXED_LIST)

#else // #if !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES) || defined(BOOST_CONTAINER_DOXYGEN_INVOKED)

template <typename OuterAlloc, BOOST_MOVE_CLASS9>
class scoped_allocator_adaptor
#endif

   : public container_detail::scoped_allocator_adaptor_base
         <OuterAlloc BOOST_CONTAINER_SCOPEDALLOC_DUMMYTRUE, BOOST_CONTAINER_SCOPEDALLOC_ALLINNER>
{
   BOOST_COPYABLE_AND_MOVABLE(scoped_allocator_adaptor)

   public:
   #ifndef BOOST_CONTAINER_DOXYGEN_INVOKED
   typedef container_detail::scoped_allocator_adaptor_base
      <OuterAlloc BOOST_CONTAINER_SCOPEDALLOC_DUMMYTRUE, BOOST_CONTAINER_SCOPEDALLOC_ALLINNER> base_type;
   typedef typename base_type::internal_type_t              internal_type_t;
   #endif   //#ifndef BOOST_CONTAINER_DOXYGEN_INVOKED
   typedef OuterAlloc                                       outer_allocator_type;
   //! Type: For exposition only
   //!
   typedef allocator_traits<OuterAlloc>                     outer_traits_type;
   //! Type: <code>scoped_allocator_adaptor<OuterAlloc></code> if <code>sizeof...(InnerAllocs)</code> is zero; otherwise,
   //! <code>scoped_allocator_adaptor<InnerAllocs...></code>.
   typedef typename base_type::inner_allocator_type         inner_allocator_type;
   typedef allocator_traits<inner_allocator_type>           inner_traits_type;
   typedef typename outer_traits_type::value_type           value_type;
   typedef typename outer_traits_type::size_type            size_type;
   typedef typename outer_traits_type::difference_type      difference_type;
   typedef typename outer_traits_type::pointer              pointer;
   typedef typename outer_traits_type::const_pointer        const_pointer;
   typedef typename outer_traits_type::void_pointer         void_pointer;
   typedef typename outer_traits_type::const_void_pointer   const_void_pointer;
   //! Type: A type with a constant boolean <code>value</code> == true if
   //!`allocator_traits<Allocator>::propagate_on_container_copy_assignment::value` is
   //! true for any <code>Allocator</code> in the set of <code>OuterAlloc</code> and <code>InnerAllocs...</code>, false otherwise.
   typedef typename base_type::
      propagate_on_container_copy_assignment                propagate_on_container_copy_assignment;
   //! Type: A type with a constant boolean <code>value</code> == true if
   //!`allocator_traits<Allocator>::propagate_on_container_move_assignment::value` is
   //! true for any <code>Allocator</code> in the set of <code>OuterAlloc</code> and <code>InnerAllocs...</code>, false otherwise.
   typedef typename base_type::
      propagate_on_container_move_assignment                propagate_on_container_move_assignment;

   //! Type: A type with a constant boolean <code>value</code> == true if
   //! `allocator_traits<Allocator>::propagate_on_container_swap::value` is
   //! true for any <code>Allocator</code> in the set of <code>OuterAlloc</code> and <code>InnerAllocs...</code>, false otherwise.
   typedef typename base_type::
      propagate_on_container_swap                           propagate_on_container_swap;

   //! Type: A type with a constant boolean <code>value</code> == true if
   //!`allocator_traits<Allocator>::is_always_equal::value` is
   //! true for all <code>Allocator</code> in the set of <code>OuterAlloc</code> and <code>InnerAllocs...</code>, false otherwise.
   typedef typename base_type::
      is_always_equal                           is_always_equal;

   //! Type: Rebinds scoped allocator to
   //!    <code>typedef scoped_allocator_adaptor
   //!      < typename outer_traits_type::template portable_rebind_alloc<U>::type
   //!      , InnerAllocs... ></code>
   template <class U>
   struct rebind
   {
      typedef scoped_allocator_adaptor
         < typename outer_traits_type::template portable_rebind_alloc<U>::type
         , BOOST_CONTAINER_SCOPEDALLOC_ALLINNER> other;
   };

   //! <b>Effects</b>: value-initializes the OuterAlloc base class
   //! and the inner allocator object.
   scoped_allocator_adaptor()
      {}

   ~scoped_allocator_adaptor()
      {}

   //! <b>Effects</b>: initializes each allocator within the adaptor with
   //! the corresponding allocator from other.
   scoped_allocator_adaptor(const scoped_allocator_adaptor& other)
      : base_type(other.base())
      {}

   //! <b>Effects</b>: move constructs each allocator within the adaptor with
   //! the corresponding allocator from other.
   scoped_allocator_adaptor(BOOST_RV_REF(scoped_allocator_adaptor) other)
      : base_type(::lslboost::move(other.base()))
      {}

   #if !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES) || defined(BOOST_CONTAINER_DOXYGEN_INVOKED)

   //! <b>Requires</b>: OuterAlloc shall be constructible from OuterA2.
   //!
   //! <b>Effects</b>: initializes the OuterAlloc base class with lslboost::forward<OuterA2>(outerAlloc) and inner
   //! with innerAllocs...(hence recursively initializing each allocator within the adaptor with the
   //! corresponding allocator from the argument list).
   template <class OuterA2>
   scoped_allocator_adaptor(BOOST_FWD_REF(OuterA2) outerAlloc, const InnerAllocs & ...innerAllocs)
      : base_type(::lslboost::forward<OuterA2>(outerAlloc), innerAllocs...)
      {}
   #else // #if !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES) || defined(BOOST_CONTAINER_DOXYGEN_INVOKED)

   #define BOOST_CONTAINER_SCOPED_ALLOCATOR_ADAPTOR_RELATED_ALLOCATOR_CONSTRUCTOR_CODE(N)\
   template <class OuterA2>\
   scoped_allocator_adaptor(BOOST_FWD_REF(OuterA2) outerAlloc BOOST_MOVE_I##N BOOST_MOVE_CREF##N)\
      : base_type(::lslboost::forward<OuterA2>(outerAlloc) BOOST_MOVE_I##N BOOST_MOVE_ARG##N)\
      {}\
   //
   BOOST_MOVE_ITERATE_0TO9(BOOST_CONTAINER_SCOPED_ALLOCATOR_ADAPTOR_RELATED_ALLOCATOR_CONSTRUCTOR_CODE)
   #undef BOOST_CONTAINER_SCOPED_ALLOCATOR_ADAPTOR_RELATED_ALLOCATOR_CONSTRUCTOR_CODE

   #endif   // #if !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES) || defined(BOOST_CONTAINER_DOXYGEN_INVOKED)

   //! <b>Requires</b>: OuterAlloc shall be constructible from OuterA2.
   //!
   //! <b>Effects</b>: initializes each allocator within the adaptor with the corresponding allocator from other.
   template <class OuterA2>
   scoped_allocator_adaptor(const scoped_allocator_adaptor<OuterA2, BOOST_CONTAINER_SCOPEDALLOC_ALLINNER> &other)
      : base_type(other.base())
      {}

   //! <b>Requires</b>: OuterAlloc shall be constructible from OuterA2.
   //!
   //! <b>Effects</b>: initializes each allocator within the adaptor with the corresponding allocator
   //! rvalue from other.
   template <class OuterA2>
   scoped_allocator_adaptor(BOOST_RV_REF_BEG scoped_allocator_adaptor
      <OuterA2, BOOST_CONTAINER_SCOPEDALLOC_ALLINNER> BOOST_RV_REF_END other)
      : base_type(::lslboost::move(other.base()))
      {}

   scoped_allocator_adaptor &operator=(BOOST_COPY_ASSIGN_REF(scoped_allocator_adaptor) other)
   {  return static_cast<scoped_allocator_adaptor&>(base_type::operator=(static_cast<const base_type &>(other))); }

   scoped_allocator_adaptor &operator=(BOOST_RV_REF(scoped_allocator_adaptor) other)
   {  return static_cast<scoped_allocator_adaptor&>(base_type::operator=(lslboost::move(other.base()))); }

   #ifdef BOOST_CONTAINER_DOXYGEN_INVOKED
   //! <b>Effects</b>: swaps *this with r.
   //!
   void swap(scoped_allocator_adaptor &r);

   //! <b>Effects</b>: swaps *this with r.
   //!
   friend void swap(scoped_allocator_adaptor &l, scoped_allocator_adaptor &r);

   //! <b>Returns</b>:
   //!   <code>static_cast<OuterAlloc&>(*this)</code>.
   outer_allocator_type      & outer_allocator() BOOST_NOEXCEPT_OR_NOTHROW;

   //! <b>Returns</b>:
   //!   <code>static_cast<const OuterAlloc&>(*this)</code>.
   const outer_allocator_type &outer_allocator() const BOOST_NOEXCEPT_OR_NOTHROW;

   //! <b>Returns</b>:
   //!   *this if <code>sizeof...(InnerAllocs)</code> is zero; otherwise, inner.
   inner_allocator_type&       inner_allocator() BOOST_NOEXCEPT_OR_NOTHROW;

   //! <b>Returns</b>:
   //!   *this if <code>sizeof...(InnerAllocs)</code> is zero; otherwise, inner.
   inner_allocator_type const& inner_allocator() const BOOST_NOEXCEPT_OR_NOTHROW;

   #endif   //BOOST_CONTAINER_DOXYGEN_INVOKED

   //! <b>Returns</b>:
   //!   <code>allocator_traits<OuterAlloc>::max_size(outer_allocator())</code>.
   size_type max_size() const BOOST_NOEXCEPT_OR_NOTHROW
   {  return outer_traits_type::max_size(this->outer_allocator());   }

   //! <b>Effects</b>:
   //!   calls <code>OUTERMOST_ALLOC_TRAITS(*this)::destroy(OUTERMOST(*this), p)</code>.
   template <class T>
   void destroy(T* p) BOOST_NOEXCEPT_OR_NOTHROW
   {
      allocator_traits<typename outermost_allocator<OuterAlloc>::type>
         ::destroy(get_outermost_allocator(this->outer_allocator()), p);
   }

   //! <b>Returns</b>:
   //! <code>allocator_traits<OuterAlloc>::allocate(outer_allocator(), n)</code>.
   pointer allocate(size_type n)
   {  return outer_traits_type::allocate(this->outer_allocator(), n);   }

   //! <b>Returns</b>:
   //! <code>allocator_traits<OuterAlloc>::allocate(outer_allocator(), n, hint)</code>.
   pointer allocate(size_type n, const_void_pointer hint)
   {  return outer_traits_type::allocate(this->outer_allocator(), n, hint);   }

   //! <b>Effects</b>:
   //! <code>allocator_traits<OuterAlloc>::deallocate(outer_allocator(), p, n)</code>.
   void deallocate(pointer p, size_type n)
   {  outer_traits_type::deallocate(this->outer_allocator(), p, n);  }

   #ifdef BOOST_CONTAINER_DOXYGEN_INVOKED
   //! <b>Returns</b>: A new scoped_allocator_adaptor object where each allocator
   //! Allocator in the adaptor is initialized from the result of calling
   //! <code>allocator_traits<Allocator>::select_on_container_copy_construction()</code> on
   //! the corresponding allocator in *this.
   scoped_allocator_adaptor select_on_container_copy_construction() const;
   #endif   //BOOST_CONTAINER_DOXYGEN_INVOKED

   #ifndef BOOST_CONTAINER_DOXYGEN_INVOKED
   base_type &base()             { return *this; }

   const base_type &base() const { return *this; }
   #endif   //#ifndef BOOST_CONTAINER_DOXYGEN_INVOKED

   #if !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES) || defined(BOOST_CONTAINER_DOXYGEN_INVOKED)

   //! <b>Effects</b>:
   //! 1) If <code>uses_allocator<T, inner_allocator_type>::value</code> is false calls
   //!    <code>OUTERMOST_ALLOC_TRAITS(*this)::construct
   //!       (OUTERMOST(*this), p, std::forward<Args>(args)...)</code>.
   //!
   //! 2) Otherwise, if <code>uses_allocator<T, inner_allocator_type>::value</code> is true and
   //!    <code>is_constructible<T, allocator_arg_t, inner_allocator_type, Args...>::value</code> is true, calls
   //!    <code>OUTERMOST_ALLOC_TRAITS(*this)::construct(OUTERMOST(*this), p, allocator_arg,
   //!    inner_allocator(), std::forward<Args>(args)...)</code>.
   //!
   //! [<b>Note</b>: In compilers without advanced decltype SFINAE support, <code>is_constructible</code> can't
   //! be implemented so that condition will be replaced by
   //! constructible_with_allocator_prefix<T>::value. -end note]
   //!
   //! 3) Otherwise, if uses_allocator<T, inner_allocator_type>::value is true and
   //!    <code>is_constructible<T, Args..., inner_allocator_type>::value</code> is true, calls
   //!    <code>OUTERMOST_ALLOC_TRAITS(*this)::construct(OUTERMOST(*this), p,
   //!    std::forward<Args>(args)..., inner_allocator())</code>.
   //!
   //! [<b>Note</b>: In compilers without advanced decltype SFINAE support, <code>is_constructible</code> can't be
   //! implemented so that condition will be replaced by
   //! <code>constructible_with_allocator_suffix<T>::value</code>. -end note]
   //!
   //! 4) Otherwise, the program is ill-formed.
   //!
   //! [<b>Note</b>: An error will result if <code>uses_allocator</code> evaluates
   //! to true but the specific constructor does not take an allocator. This definition prevents a silent
   //! failure to pass an inner allocator to a contained element. -end note]
   template < typename T, class ...Args>
   #if defined(BOOST_CONTAINER_DOXYGEN_INVOKED)
   void
   #else
   typename container_detail::enable_if<container_detail::is_not_pair<T> >::type
   #endif
   construct(T* p, BOOST_FWD_REF(Args)...args)
   {
      container_detail::dispatch_uses_allocator
         ( container_detail::bool_<uses_allocator<T, inner_allocator_type>::value>()
         , get_outermost_allocator(this->outer_allocator())
         , this->inner_allocator()
         , p, ::lslboost::forward<Args>(args)...);
   }

   #else // #if !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES) || defined(BOOST_CONTAINER_DOXYGEN_INVOKED)

   //Disable this overload if the first argument is pair as some compilers have
   //overload selection problems when the first parameter is a pair.
   #define BOOST_CONTAINER_SCOPED_ALLOCATOR_CONSTRUCT_CODE(N) \
   template < typename T BOOST_MOVE_I##N BOOST_MOVE_CLASSQ##N >\
   typename container_detail::enable_if< container_detail::is_not_pair<T> >::type\
      construct(T* p BOOST_MOVE_I##N BOOST_MOVE_UREFQ##N)\
   {\
      container_detail::dispatch_uses_allocator\
         ( container_detail::bool_<uses_allocator<T, inner_allocator_type>::value>()\
         , get_outermost_allocator(this->outer_allocator())\
         , this->inner_allocator(), p BOOST_MOVE_I##N BOOST_MOVE_FWDQ##N);\
   }\
   //
   BOOST_MOVE_ITERATE_0TO9(BOOST_CONTAINER_SCOPED_ALLOCATOR_CONSTRUCT_CODE)
   #undef BOOST_CONTAINER_SCOPED_ALLOCATOR_CONSTRUCT_CODE

   #endif   // #if !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES) || defined(BOOST_CONTAINER_DOXYGEN_INVOKED)

   template <class T1, class T2>
   void construct(std::pair<T1,T2>* p)
   {  this->construct_pair(p);  }

   template <class T1, class T2>
   void construct(container_detail::pair<T1,T2>* p)
   {  this->construct_pair(p);  }

   template <class T1, class T2, class U, class V>
   void construct(std::pair<T1, T2>* p, BOOST_FWD_REF(U) x, BOOST_FWD_REF(V) y)
   {  this->construct_pair(p, ::lslboost::forward<U>(x), ::lslboost::forward<V>(y));   }

   template <class T1, class T2, class U, class V>
   void construct(container_detail::pair<T1, T2>* p, BOOST_FWD_REF(U) x, BOOST_FWD_REF(V) y)
   {  this->construct_pair(p, ::lslboost::forward<U>(x), ::lslboost::forward<V>(y));   }

   template <class T1, class T2, class U, class V>
   void construct(std::pair<T1, T2>* p, const std::pair<U, V>& x)
   {  this->construct_pair(p, x);   }

   template <class T1, class T2, class U, class V>
   void construct( container_detail::pair<T1, T2>* p
                 , const container_detail::pair<U, V>& x)
   {  this->construct_pair(p, x);   }

   template <class T1, class T2, class U, class V>
   void construct( std::pair<T1, T2>* p
                 , BOOST_RV_REF_BEG std::pair<U, V> BOOST_RV_REF_END x)
   {  this->construct_pair(p, ::lslboost::move(x));   }

   template <class T1, class T2, class U, class V>
   void construct( container_detail::pair<T1, T2>* p
                 , BOOST_RV_REF_BEG container_detail::pair<U, V> BOOST_RV_REF_END x)
   {  this->construct_pair(p, ::lslboost::move(x));   }

   #ifndef BOOST_CONTAINER_DOXYGEN_INVOKED
   private:
   template <class Pair>
   void construct_pair(Pair* p)
   {
      this->construct(container_detail::addressof(p->first));
      BOOST_TRY{
         this->construct(container_detail::addressof(p->second));
      }
      BOOST_CATCH(...){
         this->destroy(container_detail::addressof(p->first));
         BOOST_RETHROW
      }
      BOOST_CATCH_END
   }

   template <class Pair, class U, class V>
   void construct_pair(Pair* p, BOOST_FWD_REF(U) x, BOOST_FWD_REF(V) y)
   {
      this->construct(container_detail::addressof(p->first), ::lslboost::forward<U>(x));
      BOOST_TRY{
         this->construct(container_detail::addressof(p->second), ::lslboost::forward<V>(y));
      }
      BOOST_CATCH(...){
         this->destroy(container_detail::addressof(p->first));
         BOOST_RETHROW
      }
      BOOST_CATCH_END
   }

   template <class Pair, class Pair2>
   void construct_pair(Pair* p, const Pair2& pr)
   {  this->construct_pair(p, pr.first, pr.second);  }

   template <class Pair, class Pair2>
   void construct_pair(Pair* p, BOOST_RV_REF(Pair2) pr)
   {  this->construct_pair(p, ::lslboost::move(pr.first), ::lslboost::move(pr.second));  }

   //template <class T1, class T2, class... Args1, class... Args2>
   //void construct(pair<T1, T2>* p, piecewise_construct_t, tuple<Args1...> x, tuple<Args2...> y);

   public:
   //Internal function
   template <class OuterA2>
   scoped_allocator_adaptor(internal_type_t, BOOST_FWD_REF(OuterA2) outer, const inner_allocator_type& inner)
      : base_type(internal_type_t(), ::lslboost::forward<OuterA2>(outer), inner)
   {}

   #endif   //#ifndef BOOST_CONTAINER_DOXYGEN_INVOKED
};

template<bool ZeroInner>
struct scoped_allocator_operator_equal
{
   //Optimize equal outer allocator types with 
   //allocator_traits::equal which uses is_always_equal
   template<class IA>
   static bool equal_outer(const IA &l, const IA &r)
   {  return allocator_traits<IA>::equal(l, r);  }

   //Otherwise compare it normally
   template<class IA1, class IA2>
   static bool equal_outer(const IA1 &l, const IA2 &r)
   {  return l == r;  }

   //Otherwise compare it normally
   template<class IA>
   static bool equal_inner(const IA &l, const IA &r)
   {  return allocator_traits<IA>::equal(l, r);  }
};

template<>
struct scoped_allocator_operator_equal<true>
   : scoped_allocator_operator_equal<false>
{
   //when inner allocator count is zero,
   //inner_allocator_type is the same as outer_allocator_type
   //so both types can be different in operator==
   template<class IA1, class IA2>
   static bool equal_inner(const IA1 &, const IA2 &)
   {  return true;  }
};


template <typename OuterA1, typename OuterA2, BOOST_CONTAINER_SCOPEDALLOC_ALLINNERCLASS>
inline bool operator==(const scoped_allocator_adaptor<OuterA1, BOOST_CONTAINER_SCOPEDALLOC_ALLINNER>& a
                      ,const scoped_allocator_adaptor<OuterA2, BOOST_CONTAINER_SCOPEDALLOC_ALLINNER>& b)
{
   #if !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES) || defined(BOOST_CONTAINER_DOXYGEN_INVOKED)
   const bool has_zero_inner = sizeof...(InnerAllocs) == 0u;
   #else
   const bool has_zero_inner = lslboost::container::container_detail::is_same<P0, void>::value;
   #endif
   typedef scoped_allocator_operator_equal<has_zero_inner> equal_t;
   return equal_t::equal_outer(a.outer_allocator(), b.outer_allocator()) &&
          equal_t::equal_inner(a.inner_allocator(), b.inner_allocator());
}

template <typename OuterA1, typename OuterA2, BOOST_CONTAINER_SCOPEDALLOC_ALLINNERCLASS>
inline bool operator!=(const scoped_allocator_adaptor<OuterA1, BOOST_CONTAINER_SCOPEDALLOC_ALLINNER>& a
                      ,const scoped_allocator_adaptor<OuterA2, BOOST_CONTAINER_SCOPEDALLOC_ALLINNER>& b)
{  return !(a == b);   }

}} // namespace lslboost { namespace container {

#include <lslboost/container/detail/config_end.hpp>

#endif //  BOOST_CONTAINER_ALLOCATOR_SCOPED_ALLOCATOR_HPP