//  Copyright John Maddock 2007.
//  Copyright Paul A. Bristow 2007.

//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.lslboost.org/LICENSE_1_0.txt)

#ifndef BOOST_MATH_POLICY_ERROR_HANDLING_HPP
#define BOOST_MATH_POLICY_ERROR_HANDLING_HPP

#include <stdexcept>
#include <iomanip>
#include <string>
#include <cerrno>
#include <lslboost/config/no_tr1/complex.hpp>
#include <lslboost/config/no_tr1/cmath.hpp>
#include <stdexcept>
#include <lslboost/math/tools/config.hpp>
#include <lslboost/math/policies/policy.hpp>
#include <lslboost/math/tools/precision.hpp>
#include <lslboost/cstdint.hpp>
#ifdef BOOST_MSVC
#  pragma warning(push) // Quiet warnings in lslboost/format.hpp
#  pragma warning(disable: 4996) // _SCL_SECURE_NO_DEPRECATE
#  pragma warning(disable: 4512) // assignment operator could not be generated.
// And warnings in error handling:
#  pragma warning(disable: 4702) // unreachable code.
// Note that this only occurs when the compiler can deduce code is unreachable,
// for example when policy macros are used to ignore errors rather than throw.
#endif
#include <lslboost/format.hpp>

namespace lslboost{ namespace math{

class evaluation_error : public std::runtime_error
{
public:
   evaluation_error(const std::string& s) : std::runtime_error(s){}
};

class rounding_error : public std::runtime_error
{
public:
   rounding_error(const std::string& s) : std::runtime_error(s){}
};

namespace policies{
//
// Forward declarations of user error handlers,
// it's up to the user to provide the definition of these:
//
template <class T>
T user_domain_error(const char* function, const char* message, const T& val);
template <class T>
T user_pole_error(const char* function, const char* message, const T& val);
template <class T>
T user_overflow_error(const char* function, const char* message, const T& val);
template <class T>
T user_underflow_error(const char* function, const char* message, const T& val);
template <class T>
T user_denorm_error(const char* function, const char* message, const T& val);
template <class T>
T user_evaluation_error(const char* function, const char* message, const T& val);
template <class T, class TargetType>
T user_rounding_error(const char* function, const char* message, const T& val, const TargetType& t);
template <class T>
T user_indeterminate_result_error(const char* function, const char* message, const T& val);

namespace detail
{
//
// Helper function to avoid binding rvalue to non-const-reference,
// in other words a warning suppression mechanism:
//
template <class Formatter, class Group>
inline std::string do_format(Formatter f, const Group& g)
{
   return (f % g).str();
}

template <class T>
inline const char* name_of()
{
#ifndef BOOST_NO_RTTI
   return typeid(T).name();
#else
   return "unknown";
#endif
}
template <> inline const char* name_of<float>(){ return "float"; }
template <> inline const char* name_of<double>(){ return "double"; }
template <> inline const char* name_of<long double>(){ return "long double"; }

#ifdef BOOST_MATH_USE_FLOAT128
template <>
inline const char* name_of<BOOST_MATH_FLOAT128_TYPE>()
{
   return "__float128";
}
#endif

template <class E, class T>
void raise_error(const char* function, const char* message)
{
  if(function == 0)
       function = "Unknown function operating on type %1%";
  if(message == 0)
       message = "Cause unknown";

  std::string msg("Error in function ");
#ifndef BOOST_NO_RTTI
  msg += (lslboost::format(function) % lslboost::math::policies::detail::name_of<T>()).str();
#else
  msg += function;
#endif
  msg += ": ";
  msg += message;

  E e(msg);
  lslboost::throw_exception(e);
}

template <class E, class T>
void raise_error(const char* function, const char* message, const T& val)
{
  if(function == 0)
     function = "Unknown function operating on type %1%";
  if(message == 0)
     message = "Cause unknown: error caused by bad argument with value %1%";

  std::string msg("Error in function ");
#ifndef BOOST_NO_RTTI
  msg += (lslboost::format(function) % lslboost::math::policies::detail::name_of<T>()).str();
#else
  msg += function;
#endif
  msg += ": ";
  msg += message;

  int prec = 2 + (lslboost::math::policies::digits<T, lslboost::math::policies::policy<> >() * 30103UL) / 100000UL;
  msg = do_format(lslboost::format(msg), lslboost::io::group(std::setprecision(prec), val));

  E e(msg);
  lslboost::throw_exception(e);
}

template <class T>
inline T raise_domain_error(
           const char* function,
           const char* message,
           const T& val,
           const ::lslboost::math::policies::domain_error< ::lslboost::math::policies::throw_on_error>&)
{
   raise_error<std::domain_error, T>(function, message, val);
   // we never get here:
   return std::numeric_limits<T>::quiet_NaN();
}

template <class T>
inline T raise_domain_error(
           const char* ,
           const char* ,
           const T& ,
           const ::lslboost::math::policies::domain_error< ::lslboost::math::policies::ignore_error>&)
{
   // This may or may not do the right thing, but the user asked for the error
   // to be ignored so here we go anyway:
   return std::numeric_limits<T>::quiet_NaN();
}

template <class T>
inline T raise_domain_error(
           const char* ,
           const char* ,
           const T& ,
           const ::lslboost::math::policies::domain_error< ::lslboost::math::policies::errno_on_error>&)
{
   errno = EDOM;
   // This may or may not do the right thing, but the user asked for the error
   // to be silent so here we go anyway:
   return std::numeric_limits<T>::quiet_NaN();
}

template <class T>
inline T raise_domain_error(
           const char* function,
           const char* message,
           const T& val,
           const  ::lslboost::math::policies::domain_error< ::lslboost::math::policies::user_error>&)
{
   return user_domain_error(function, message, val);
}

template <class T>
inline T raise_pole_error(
           const char* function,
           const char* message,
           const T& val,
           const  ::lslboost::math::policies::pole_error< ::lslboost::math::policies::throw_on_error>&)
{
   return lslboost::math::policies::detail::raise_domain_error(function, message, val,  ::lslboost::math::policies::domain_error< ::lslboost::math::policies::throw_on_error>());
}

template <class T>
inline T raise_pole_error(
           const char* function,
           const char* message,
           const T& val,
           const  ::lslboost::math::policies::pole_error< ::lslboost::math::policies::ignore_error>&)
{
   return  ::lslboost::math::policies::detail::raise_domain_error(function, message, val,  ::lslboost::math::policies::domain_error< ::lslboost::math::policies::ignore_error>());
}

template <class T>
inline T raise_pole_error(
           const char* function,
           const char* message,
           const T& val,
           const  ::lslboost::math::policies::pole_error< ::lslboost::math::policies::errno_on_error>&)
{
   return  ::lslboost::math::policies::detail::raise_domain_error(function, message, val,  ::lslboost::math::policies::domain_error< ::lslboost::math::policies::errno_on_error>());
}

template <class T>
inline T raise_pole_error(
           const char* function,
           const char* message,
           const T& val,
           const  ::lslboost::math::policies::pole_error< ::lslboost::math::policies::user_error>&)
{
   return user_pole_error(function, message, val);
}


template <class T>
inline T raise_overflow_error(
           const char* function,
           const char* message,
           const  ::lslboost::math::policies::overflow_error< ::lslboost::math::policies::throw_on_error>&)
{
   raise_error<std::overflow_error, T>(function, message ? message : "numeric overflow");
   // We should never get here:
   return std::numeric_limits<T>::has_infinity ? std::numeric_limits<T>::infinity() : lslboost::math::tools::max_value<T>();
}

template <class T>
inline T raise_overflow_error(
           const char* function,
           const char* message,
           const T& val,
           const ::lslboost::math::policies::overflow_error< ::lslboost::math::policies::throw_on_error>&)
{
   raise_error<std::overflow_error, T>(function, message ? message : "numeric overflow", val);
   // We should never get here:
   return std::numeric_limits<T>::has_infinity ? std::numeric_limits<T>::infinity() : lslboost::math::tools::max_value<T>();
}

template <class T>
inline T raise_overflow_error(
           const char* ,
           const char* ,
           const  ::lslboost::math::policies::overflow_error< ::lslboost::math::policies::ignore_error>&)
{
   // This may or may not do the right thing, but the user asked for the error
   // to be ignored so here we go anyway:
   return std::numeric_limits<T>::has_infinity ? std::numeric_limits<T>::infinity() : lslboost::math::tools::max_value<T>();
}

template <class T>
inline T raise_overflow_error(
           const char* ,
           const char* ,
           const T&,
           const  ::lslboost::math::policies::overflow_error< ::lslboost::math::policies::ignore_error>&)
{
   // This may or may not do the right thing, but the user asked for the error
   // to be ignored so here we go anyway:
   return std::numeric_limits<T>::has_infinity ? std::numeric_limits<T>::infinity() : lslboost::math::tools::max_value<T>();
}

template <class T>
inline T raise_overflow_error(
           const char* ,
           const char* ,
           const  ::lslboost::math::policies::overflow_error< ::lslboost::math::policies::errno_on_error>&)
{
   errno = ERANGE;
   // This may or may not do the right thing, but the user asked for the error
   // to be silent so here we go anyway:
   return std::numeric_limits<T>::has_infinity ? std::numeric_limits<T>::infinity() : lslboost::math::tools::max_value<T>();
}

template <class T>
inline T raise_overflow_error(
           const char* ,
           const char* ,
           const T&,
           const  ::lslboost::math::policies::overflow_error< ::lslboost::math::policies::errno_on_error>&)
{
   errno = ERANGE;
   // This may or may not do the right thing, but the user asked for the error
   // to be silent so here we go anyway:
   return std::numeric_limits<T>::has_infinity ? std::numeric_limits<T>::infinity() : lslboost::math::tools::max_value<T>();
}

template <class T>
inline T raise_overflow_error(
           const char* function,
           const char* message,
           const  ::lslboost::math::policies::overflow_error< ::lslboost::math::policies::user_error>&)
{
   return user_overflow_error(function, message, std::numeric_limits<T>::infinity());
}

template <class T>
inline T raise_overflow_error(
           const char* function,
           const char* message,
           const T& val,
           const  ::lslboost::math::policies::overflow_error< ::lslboost::math::policies::user_error>&)
{
   std::string fmsg("Error in function ");
#ifndef BOOST_NO_RTTI
   fmsg += (lslboost::format(function) % lslboost::math::policies::detail::name_of<T>()).str();
#else
   fmsg += function;
#endif
   int prec = 2 + (lslboost::math::policies::digits<T, lslboost::math::policies::policy<> >() * 30103UL) / 100000UL;
   std::string msg = do_format(lslboost::format(message), lslboost::io::group(std::setprecision(prec), val));
   return user_overflow_error(fmsg.c_str(), msg.c_str(), std::numeric_limits<T>::infinity());
}

template <class T>
inline T raise_underflow_error(
           const char* function,
           const char* message,
           const  ::lslboost::math::policies::underflow_error< ::lslboost::math::policies::throw_on_error>&)
{
   raise_error<std::underflow_error, T>(function, message ? message : "numeric underflow");
   // We should never get here:
   return 0;
}

template <class T>
inline T raise_underflow_error(
           const char* ,
           const char* ,
           const  ::lslboost::math::policies::underflow_error< ::lslboost::math::policies::ignore_error>&)
{
   // This may or may not do the right thing, but the user asked for the error
   // to be ignored so here we go anyway:
   return T(0);
}

template <class T>
inline T raise_underflow_error(
           const char* /* function */,
           const char* /* message */,
           const  ::lslboost::math::policies::underflow_error< ::lslboost::math::policies::errno_on_error>&)
{
   errno = ERANGE;
   // This may or may not do the right thing, but the user asked for the error
   // to be silent so here we go anyway:
   return T(0);
}

template <class T>
inline T raise_underflow_error(
           const char* function,
           const char* message,
           const  ::lslboost::math::policies::underflow_error< ::lslboost::math::policies::user_error>&)
{
   return user_underflow_error(function, message, T(0));
}

template <class T>
inline T raise_denorm_error(
           const char* function,
           const char* message,
           const T& /* val */,
           const  ::lslboost::math::policies::denorm_error< ::lslboost::math::policies::throw_on_error>&)
{
   raise_error<std::underflow_error, T>(function, message ? message : "denormalised result");
   // we never get here:
   return T(0);
}

template <class T>
inline T raise_denorm_error(
           const char* ,
           const char* ,
           const T&  val,
           const  ::lslboost::math::policies::denorm_error< ::lslboost::math::policies::ignore_error>&)
{
   // This may or may not do the right thing, but the user asked for the error
   // to be ignored so here we go anyway:
   return val;
}

template <class T>
inline T raise_denorm_error(
           const char* ,
           const char* ,
           const T& val,
           const  ::lslboost::math::policies::denorm_error< ::lslboost::math::policies::errno_on_error>&)
{
   errno = ERANGE;
   // This may or may not do the right thing, but the user asked for the error
   // to be silent so here we go anyway:
   return val;
}

template <class T>
inline T raise_denorm_error(
           const char* function,
           const char* message,
           const T& val,
           const  ::lslboost::math::policies::denorm_error< ::lslboost::math::policies::user_error>&)
{
   return user_denorm_error(function, message, val);
}

template <class T>
inline T raise_evaluation_error(
           const char* function,
           const char* message,
           const T& val,
           const  ::lslboost::math::policies::evaluation_error< ::lslboost::math::policies::throw_on_error>&)
{
   raise_error<lslboost::math::evaluation_error, T>(function, message, val);
   // we never get here:
   return T(0);
}

template <class T>
inline T raise_evaluation_error(
           const char* ,
           const char* ,
           const T& val,
           const  ::lslboost::math::policies::evaluation_error< ::lslboost::math::policies::ignore_error>&)
{
   // This may or may not do the right thing, but the user asked for the error
   // to be ignored so here we go anyway:
   return val;
}

template <class T>
inline T raise_evaluation_error(
           const char* ,
           const char* ,
           const T& val,
           const  ::lslboost::math::policies::evaluation_error< ::lslboost::math::policies::errno_on_error>&)
{
   errno = EDOM;
   // This may or may not do the right thing, but the user asked for the error
   // to be silent so here we go anyway:
   return val;
}

template <class T>
inline T raise_evaluation_error(
           const char* function,
           const char* message,
           const T& val,
           const  ::lslboost::math::policies::evaluation_error< ::lslboost::math::policies::user_error>&)
{
   return user_evaluation_error(function, message, val);
}

template <class T, class TargetType>
inline TargetType raise_rounding_error(
           const char* function,
           const char* message,
           const T& val,
           const TargetType&,
           const  ::lslboost::math::policies::rounding_error< ::lslboost::math::policies::throw_on_error>&)
{
   raise_error<lslboost::math::rounding_error, T>(function, message, val);
   // we never get here:
   return TargetType(0);
}

template <class T, class TargetType>
inline TargetType raise_rounding_error(
           const char* ,
           const char* ,
           const T& val,
           const TargetType&,
           const  ::lslboost::math::policies::rounding_error< ::lslboost::math::policies::ignore_error>&)
{
   // This may or may not do the right thing, but the user asked for the error
   // to be ignored so here we go anyway:
   BOOST_STATIC_ASSERT(std::numeric_limits<TargetType>::is_specialized);
   return  val > 0 ? (std::numeric_limits<TargetType>::max)() : (std::numeric_limits<TargetType>::is_integer ? (std::numeric_limits<TargetType>::min)() : -(std::numeric_limits<TargetType>::max)());
}

template <class T, class TargetType>
inline TargetType raise_rounding_error(
           const char* ,
           const char* ,
           const T& val,
           const TargetType&,
           const  ::lslboost::math::policies::rounding_error< ::lslboost::math::policies::errno_on_error>&)
{
   errno = ERANGE;
   // This may or may not do the right thing, but the user asked for the error
   // to be silent so here we go anyway:
   BOOST_STATIC_ASSERT(std::numeric_limits<TargetType>::is_specialized);
   return  val > 0 ? (std::numeric_limits<TargetType>::max)() : (std::numeric_limits<TargetType>::is_integer ? (std::numeric_limits<TargetType>::min)() : -(std::numeric_limits<TargetType>::max)());
}

template <class T, class TargetType>
inline TargetType raise_rounding_error(
           const char* function,
           const char* message,
           const T& val,
           const TargetType& t,
           const  ::lslboost::math::policies::rounding_error< ::lslboost::math::policies::user_error>&)
{
   return user_rounding_error(function, message, val, t);
}

template <class T, class R>
inline T raise_indeterminate_result_error(
           const char* function,
           const char* message,
           const T& val,
           const R& ,
           const ::lslboost::math::policies::indeterminate_result_error< ::lslboost::math::policies::throw_on_error>&)
{
   raise_error<std::domain_error, T>(function, message, val);
   // we never get here:
   return std::numeric_limits<T>::quiet_NaN();
}

template <class T, class R>
inline T raise_indeterminate_result_error(
           const char* ,
           const char* ,
           const T& ,
           const R& result,
           const ::lslboost::math::policies::indeterminate_result_error< ::lslboost::math::policies::ignore_error>&)
{
   // This may or may not do the right thing, but the user asked for the error
   // to be ignored so here we go anyway:
   return result;
}

template <class T, class R>
inline T raise_indeterminate_result_error(
           const char* ,
           const char* ,
           const T& ,
           const R& result,
           const ::lslboost::math::policies::indeterminate_result_error< ::lslboost::math::policies::errno_on_error>&)
{
   errno = EDOM;
   // This may or may not do the right thing, but the user asked for the error
   // to be silent so here we go anyway:
   return result;
}

template <class T, class R>
inline T raise_indeterminate_result_error(
           const char* function,
           const char* message,
           const T& val,
           const R& ,
           const ::lslboost::math::policies::indeterminate_result_error< ::lslboost::math::policies::user_error>&)
{
   return user_indeterminate_result_error(function, message, val);
}

}  // namespace detail

template <class T, class Policy>
inline T raise_domain_error(const char* function, const char* message, const T& val, const Policy&)
{
   typedef typename Policy::domain_error_type policy_type;
   return detail::raise_domain_error(
      function, message ? message : "Domain Error evaluating function at %1%",
      val, policy_type());
}

template <class T, class Policy>
inline T raise_pole_error(const char* function, const char* message, const T& val, const Policy&)
{
   typedef typename Policy::pole_error_type policy_type;
   return detail::raise_pole_error(
      function, message ? message : "Evaluation of function at pole %1%",
      val, policy_type());
}

template <class T, class Policy>
inline T raise_overflow_error(const char* function, const char* message, const Policy&)
{
   typedef typename Policy::overflow_error_type policy_type;
   return detail::raise_overflow_error<T>(
      function, message ? message : "Overflow Error",
      policy_type());
}

template <class T, class Policy>
inline T raise_overflow_error(const char* function, const char* message, const T& val, const Policy&)
{
   typedef typename Policy::overflow_error_type policy_type;
   return detail::raise_overflow_error(
      function, message ? message : "Overflow evaluating function at %1%",
      val, policy_type());
}

template <class T, class Policy>
inline T raise_underflow_error(const char* function, const char* message, const Policy&)
{
   typedef typename Policy::underflow_error_type policy_type;
   return detail::raise_underflow_error<T>(
      function, message ? message : "Underflow Error",
      policy_type());
}

template <class T, class Policy>
inline T raise_denorm_error(const char* function, const char* message, const T& val, const Policy&)
{
   typedef typename Policy::denorm_error_type policy_type;
   return detail::raise_denorm_error<T>(
      function, message ? message : "Denorm Error",
      val,
      policy_type());
}

template <class T, class Policy>
inline T raise_evaluation_error(const char* function, const char* message, const T& val, const Policy&)
{
   typedef typename Policy::evaluation_error_type policy_type;
   return detail::raise_evaluation_error(
      function, message ? message : "Internal Evaluation Error, best value so far was %1%",
      val, policy_type());
}

template <class T, class TargetType, class Policy>
inline TargetType raise_rounding_error(const char* function, const char* message, const T& val, const TargetType& t, const Policy&)
{
   typedef typename Policy::rounding_error_type policy_type;
   return detail::raise_rounding_error(
      function, message ? message : "Value %1% can not be represented in the target integer type.",
      val, t, policy_type());
}

template <class T, class R, class Policy>
inline T raise_indeterminate_result_error(const char* function, const char* message, const T& val, const R& result, const Policy&)
{
   typedef typename Policy::indeterminate_result_error_type policy_type;
   return detail::raise_indeterminate_result_error(
      function, message ? message : "Indeterminate result with value %1%",
      val, result, policy_type());
}

//
// checked_narrowing_cast:
//
namespace detail
{

template <class R, class T, class Policy>
inline bool check_overflow(T val, R* result, const char* function, const Policy& pol)
{
   BOOST_MATH_STD_USING
   if(fabs(val) > tools::max_value<R>())
   {
      lslboost::math::policies::detail::raise_overflow_error<R>(function, 0, pol);
      *result = static_cast<R>(val);
      return true;
   }
   return false;
}
template <class R, class T, class Policy>
inline bool check_overflow(std::complex<T> val, R* result, const char* function, const Policy& pol)
{
   typedef typename R::value_type r_type;
   r_type re, im;
   bool r = check_overflow<r_type>(val.real(), &re, function, pol);
   r = check_overflow<r_type>(val.imag(), &im, function, pol) || r;
   *result = R(re, im);
   return r;
}
template <class R, class T, class Policy>
inline bool check_underflow(T val, R* result, const char* function, const Policy& pol)
{
   if((val != 0) && (static_cast<R>(val) == 0))
   {
      *result = static_cast<R>(lslboost::math::policies::detail::raise_underflow_error<R>(function, 0, pol));
      return true;
   }
   return false;
}
template <class R, class T, class Policy>
inline bool check_underflow(std::complex<T> val, R* result, const char* function, const Policy& pol)
{
   typedef typename R::value_type r_type;
   r_type re, im;
   bool r = check_underflow<r_type>(val.real(), &re, function, pol);
   r = check_underflow<r_type>(val.imag(), &im, function, pol) || r;
   *result = R(re, im);
   return r;
}
template <class R, class T, class Policy>
inline bool check_denorm(T val, R* result, const char* function, const Policy& pol)
{
   BOOST_MATH_STD_USING
   if((fabs(val) < static_cast<T>(tools::min_value<R>())) && (static_cast<R>(val) != 0))
   {
      *result = static_cast<R>(lslboost::math::policies::detail::raise_denorm_error<R>(function, 0, static_cast<R>(val), pol));
      return true;
   }
   return false;
}
template <class R, class T, class Policy>
inline bool check_denorm(std::complex<T> val, R* result, const char* function, const Policy& pol)
{
   typedef typename R::value_type r_type;
   r_type re, im;
   bool r = check_denorm<r_type>(val.real(), &re, function, pol);
   r = check_denorm<r_type>(val.imag(), &im, function, pol) || r;
   *result = R(re, im);
   return r;
}

// Default instantiations with ignore_error policy.
template <class R, class T>
inline bool check_overflow(T /* val */, R* /* result */, const char* /* function */, const overflow_error<ignore_error>&){ return false; }
template <class R, class T>
inline bool check_overflow(std::complex<T> /* val */, R* /* result */, const char* /* function */, const overflow_error<ignore_error>&){ return false; }
template <class R, class T>
inline bool check_underflow(T /* val */, R* /* result */, const char* /* function */, const underflow_error<ignore_error>&){ return false; }
template <class R, class T>
inline bool check_underflow(std::complex<T> /* val */, R* /* result */, const char* /* function */, const underflow_error<ignore_error>&){ return false; }
template <class R, class T>
inline bool check_denorm(T /* val */, R* /* result*/, const char* /* function */, const denorm_error<ignore_error>&){ return false; }
template <class R, class T>
inline bool check_denorm(std::complex<T> /* val */, R* /* result*/, const char* /* function */, const denorm_error<ignore_error>&){ return false; }

} // namespace detail

template <class R, class Policy, class T>
inline R checked_narrowing_cast(T val, const char* function)
{
   typedef typename Policy::overflow_error_type overflow_type;
   typedef typename Policy::underflow_error_type underflow_type;
   typedef typename Policy::denorm_error_type denorm_type;
   //
   // Most of what follows will evaluate to a no-op:
   //
   R result = 0;
   if(detail::check_overflow<R>(val, &result, function, overflow_type()))
      return result;
   if(detail::check_underflow<R>(val, &result, function, underflow_type()))
      return result;
   if(detail::check_denorm<R>(val, &result, function, denorm_type()))
      return result;

   return static_cast<R>(val);
}

template <class T, class Policy>
inline void check_series_iterations(const char* function, lslboost::uintmax_t max_iter, const Policy& pol)
{
   if(max_iter >= policies::get_max_series_iterations<Policy>())
      raise_evaluation_error<T>(
         function,
         "Series evaluation exceeded %1% iterations, giving up now.", static_cast<T>(static_cast<double>(max_iter)), pol);
}

template <class T, class Policy>
inline void check_root_iterations(const char* function, lslboost::uintmax_t max_iter, const Policy& pol)
{
   if(max_iter >= policies::get_max_root_iterations<Policy>())
      raise_evaluation_error<T>(
         function,
         "Root finding evaluation exceeded %1% iterations, giving up now.", static_cast<T>(static_cast<double>(max_iter)), pol);
}

} //namespace policies

namespace detail{

//
// Simple helper function to assist in returning a pair from a single value,
// that value usually comes from one of the error handlers above:
//
template <class T>
std::pair<T, T> pair_from_single(const T& val)
{
   return std::make_pair(val, val);
}

}

#ifdef BOOST_MSVC
#  pragma warning(pop)
#endif

}} // namespaces lslboost/math

#endif // BOOST_MATH_POLICY_ERROR_HANDLING_HPP
