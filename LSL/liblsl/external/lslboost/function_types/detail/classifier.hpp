
// (C) Copyright Tobias Schwinger
//
// Use modification and distribution are subject to the lslboost Software License,
// Version 1.0. (See http://www.lslboost.org/LICENSE_1_0.txt).

//------------------------------------------------------------------------------

#ifndef BOOST_FT_DETAIL_CLASSIFIER_HPP_INCLUDED
#define BOOST_FT_DETAIL_CLASSIFIER_HPP_INCLUDED

#include <lslboost/type.hpp>
#include <lslboost/config.hpp>
#include <lslboost/type_traits/is_reference.hpp>
#include <lslboost/type_traits/add_reference.hpp>

#include <lslboost/function_types/config/config.hpp>
#include <lslboost/function_types/property_tags.hpp>

namespace lslboost { namespace function_types { namespace detail {

template<typename T> struct classifier;

template<std::size_t S> struct char_array { typedef char (&type)[S]; };

template<bits_t Flags, bits_t CCID, std::size_t Arity> struct encode_charr
{
  typedef typename char_array<
    ::lslboost::function_types::detail::encode_charr_impl<Flags,CCID,Arity>::value 
  >::type type;
};

#if defined(BOOST_MSVC) || (defined(__BORLANDC__) && !defined(BOOST_DISABLE_WIN32))
#   define BOOST_FT_DECL __cdecl
#else
#   define BOOST_FT_DECL /**/
#endif

char BOOST_FT_DECL classifier_impl(...);

#define BOOST_FT_variations BOOST_FT_function|BOOST_FT_pointer|\
                            BOOST_FT_member_pointer

#define BOOST_FT_type_function(cc,name) BOOST_FT_SYNTAX( \
    R BOOST_PP_EMPTY,BOOST_PP_LPAREN,cc,* BOOST_PP_EMPTY,name,BOOST_PP_RPAREN)

#define BOOST_FT_type_function_pointer(cc,name) BOOST_FT_SYNTAX( \
    R BOOST_PP_EMPTY,BOOST_PP_LPAREN,cc,** BOOST_PP_EMPTY,name,BOOST_PP_RPAREN)

#define BOOST_FT_type_member_function_pointer(cc,name) BOOST_FT_SYNTAX( \
    R BOOST_PP_EMPTY,BOOST_PP_LPAREN,cc,T0::** BOOST_PP_EMPTY,name,BOOST_PP_RPAREN)

#define BOOST_FT_al_path lslboost/function_types/detail/classifier_impl
#include <lslboost/function_types/detail/pp_loop.hpp>

template<typename T> struct classifier_bits
{
  static typename lslboost::add_reference<T>::type tester;

  BOOST_STATIC_CONSTANT(bits_t,value = (bits_t)sizeof(
    lslboost::function_types::detail::classifier_impl(& tester) 
  )-1);
};

template<typename T> struct classifier
{
  typedef detail::constant<
    ::lslboost::function_types::detail::decode_bits<
      ::lslboost::function_types::detail::classifier_bits<T>::value
    >::tag_bits > 
  bits;

  typedef detail::full_mask mask;
 
  typedef detail::constant<
    ::lslboost::function_types::detail::decode_bits<
      ::lslboost::function_types::detail::classifier_bits<T>::value
    >::arity > 
  function_arity;
};



} } } // namespace ::lslboost::function_types::detail

#endif
