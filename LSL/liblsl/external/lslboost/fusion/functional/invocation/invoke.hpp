/*=============================================================================
    Copyright (c) 2005-2006 Joao Abecasis
    Copyright (c) 2006-2007 Tobias Schwinger

    Use modification and distribution are subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.lslboost.org/LICENSE_1_0.txt).
==============================================================================*/

#if !defined(BOOST_FUSION_FUNCTIONAL_INVOCATION_INVOKE_HPP_INCLUDED)
#if !defined(BOOST_PP_IS_ITERATING)

#include <lslboost/preprocessor/cat.hpp>
#include <lslboost/preprocessor/iteration/iterate.hpp>
#include <lslboost/preprocessor/arithmetic/dec.hpp>
#include <lslboost/preprocessor/repetition/repeat_from_to.hpp>
#include <lslboost/preprocessor/repetition/enum.hpp>
#include <lslboost/preprocessor/repetition/enum_shifted.hpp>
#include <lslboost/preprocessor/repetition/enum_params.hpp>
#include <lslboost/preprocessor/repetition/enum_shifted_params.hpp>

#include <lslboost/mpl/if.hpp>
#include <lslboost/mpl/eval_if.hpp>
#include <lslboost/mpl/or.hpp>
#include <lslboost/mpl/front.hpp>
#include <lslboost/mpl/identity.hpp>

#include <lslboost/type_traits/add_const.hpp>
#include <lslboost/type_traits/remove_cv.hpp>
#include <lslboost/type_traits/add_reference.hpp>
#include <lslboost/type_traits/remove_reference.hpp>
#include <lslboost/type_traits/is_convertible.hpp>

#include <lslboost/function_types/is_function.hpp>
#include <lslboost/function_types/is_callable_builtin.hpp>
#include <lslboost/function_types/is_member_pointer.hpp>
#include <lslboost/function_types/is_member_function_pointer.hpp>
#include <lslboost/function_types/result_type.hpp>
#include <lslboost/function_types/parameter_types.hpp>

#include <lslboost/utility/result_of.hpp>

#include <lslboost/fusion/support/category_of.hpp>
#include <lslboost/fusion/support/detail/enabler.hpp>
#include <lslboost/fusion/sequence/intrinsic/at.hpp>
#include <lslboost/fusion/sequence/intrinsic/size.hpp>
#include <lslboost/fusion/sequence/intrinsic/front.hpp>
#include <lslboost/fusion/sequence/intrinsic/begin.hpp>
#include <lslboost/fusion/iterator/next.hpp>
#include <lslboost/fusion/iterator/deref.hpp>
#include <lslboost/fusion/functional/invocation/limits.hpp>
#include <lslboost/fusion/functional/invocation/detail/that_ptr.hpp>

namespace lslboost { namespace fusion
{
    namespace detail
    {
        namespace ft = function_types;

        template<
            typename Function, class Sequence,
            int N = result_of::size<Sequence>::value,
            bool CBI = ft::is_callable_builtin<Function>::value,
            bool RandomAccess = traits::is_random_access<Sequence>::value,
            typename Enable = void
            >
        struct invoke_impl;

        template <class Sequence, int N>
        struct invoke_param_types;

        template <typename T, class Sequence>
        struct invoke_data_member;

        template <typename Function, class Sequence, int N, bool RandomAccess>
        struct invoke_fn_ptr;

        template <typename Function, class Sequence, int N, bool RandomAccess>
        struct invoke_mem_fn;

        #define  BOOST_PP_FILENAME_1 <lslboost/fusion/functional/invocation/invoke.hpp>
        #define  BOOST_PP_ITERATION_LIMITS (0, BOOST_FUSION_INVOKE_MAX_ARITY)
        #include BOOST_PP_ITERATE()

        template <typename F, class Sequence, int N, bool RandomAccess>
        struct invoke_nonmember_builtin
        // use same implementation as for function objects but...
            : invoke_fn_ptr< // ...work around lslboost::result_of bugs
                typename mpl::eval_if< ft::is_function<F>,
                    lslboost::add_reference<F>, lslboost::remove_cv<F> >::type,
                Sequence, N, RandomAccess >
        { };

        template <typename Function, class Sequence, int N, bool RandomAccess, typename Enable>
        struct invoke_impl<Function,Sequence,N,true,RandomAccess,Enable>
            : mpl::if_< ft::is_member_function_pointer<Function>,
                invoke_mem_fn<Function,Sequence,N,RandomAccess>,
                invoke_nonmember_builtin<Function,Sequence,N,RandomAccess>
            >::type
        { };

        template <typename Function, class Sequence, bool RandomAccess, typename Enable>
        struct invoke_impl<Function,Sequence,1,true,RandomAccess,Enable>
            : mpl::eval_if< ft::is_member_pointer<Function>,
                mpl::if_< ft::is_member_function_pointer<Function>,
                    invoke_mem_fn<Function,Sequence,1,RandomAccess>,
                    invoke_data_member<Function, Sequence> >,
                mpl::identity< invoke_nonmember_builtin<
                    Function,Sequence,1,RandomAccess> >
            >::type
        { };

        template <typename T, class C, class Sequence>
        struct invoke_data_member< T C::*, Sequence >
        {
        private:

            typedef typename result_of::front<Sequence>::type that;

            typedef mpl::or_< lslboost::is_convertible<that,C*>,
                              lslboost::is_convertible<that,C&>,
                              non_const_pointee<that> > non_const_cond;

            typedef typename mpl::eval_if< non_const_cond,
                mpl::identity<C>, add_const<C> >::type qualified_class;

            typedef typename mpl::eval_if< non_const_cond,
                mpl::identity<T>, add_const<T> >::type qualified_type;

        public:

            typedef typename lslboost::add_reference<qualified_type>::type
                result_type;

            BOOST_CXX14_CONSTEXPR BOOST_FUSION_GPU_ENABLED
            static inline result_type call(T C::* f, Sequence & s)
            {
                typename result_of::front<Sequence>::type c = fusion::front(s);
                return that_ptr<qualified_class>::get(c)->*f;
            }
        };
    }

    namespace result_of
    {
        template <typename Function, class Sequence, typename = void>
        struct invoke;

        template <typename Function, class Sequence>
        struct invoke<Function, Sequence,
                      typename detail::enabler<
                          typename detail::invoke_impl<
                              typename lslboost::remove_reference<Function>::type, Sequence
                          >::result_type
                      >::type>
        {
            typedef typename detail::invoke_impl<
                typename lslboost::remove_reference<Function>::type, Sequence
            >::result_type type;
        };
    }

    template <typename Function, class Sequence>
    BOOST_CONSTEXPR BOOST_FUSION_GPU_ENABLED
    inline typename result_of::invoke<Function,Sequence>::type
    invoke(Function f, Sequence & s)
    {
        return detail::invoke_impl<
                typename lslboost::remove_reference<Function>::type,Sequence
            >::call(f,s);
    }

    template <typename Function, class Sequence>
    BOOST_CONSTEXPR BOOST_FUSION_GPU_ENABLED
    inline typename result_of::invoke<Function,Sequence const>::type
    invoke(Function f, Sequence const & s)
    {
        return detail::invoke_impl<
                typename lslboost::remove_reference<Function>::type,Sequence const
            >::call(f,s);
    }

}}

#define BOOST_FUSION_FUNCTIONAL_INVOCATION_INVOKE_HPP_INCLUDED
#else // defined(BOOST_PP_IS_ITERATING)
///////////////////////////////////////////////////////////////////////////////
//
//  Preprocessor vertical repetition code
//
///////////////////////////////////////////////////////////////////////////////
#define N BOOST_PP_ITERATION()

#define M(z,j,data) typename result_of::at_c<Sequence,j>::type

        template <typename Function, class Sequence>
        struct invoke_impl<Function,Sequence,N,false,true,
            typename enabler<
                typename lslboost::result_of<Function(BOOST_PP_ENUM(N,M,~)) >::type
              >::type>
        {
        public:

            typedef typename lslboost::result_of<
                    Function(BOOST_PP_ENUM(N,M,~)) >::type result_type;
#undef M

#if N > 0

            template <typename F>
            BOOST_CONSTEXPR BOOST_FUSION_GPU_ENABLED
            static inline result_type
            call(F & f, Sequence & s)
            {
#define M(z,j,data) fusion::at_c<j>(s)
                return f( BOOST_PP_ENUM(N,M,~) );
            }

#else
            template <typename F>
            BOOST_CONSTEXPR BOOST_FUSION_GPU_ENABLED
            static inline result_type
            call(F & f, Sequence & /*s*/)
            {
                return f();
            }

#endif

        };

        template <typename Function, class Sequence>
        struct invoke_fn_ptr<Function,Sequence,N,true>
        {
        public:

            typedef typename ft::result_type<Function>::type result_type;

#if N > 0

            template <typename F>
            BOOST_CONSTEXPR BOOST_FUSION_GPU_ENABLED
            static inline result_type
            call(F & f, Sequence & s)
            {
#define M(z,j,data) fusion::at_c<j>(s)
                return f( BOOST_PP_ENUM(N,M,~) );
            }

#else
            template <typename F>
            BOOST_CONSTEXPR BOOST_FUSION_GPU_ENABLED
            static inline result_type
            call(F & f, Sequence & /*s*/)
            {
                return f();
            }

#endif

        };


#if N > 0
        template <typename Function, class Sequence>
        struct invoke_mem_fn<Function,Sequence,N,true>
        {
        public:

            typedef typename ft::result_type<Function>::type result_type;

            template <typename F>
            BOOST_CONSTEXPR BOOST_FUSION_GPU_ENABLED
            static inline result_type
            call(F & f, Sequence & s)
            {
                return (that_ptr<typename mpl::front<
                                ft::parameter_types<Function> >::type
                    >::get(fusion::at_c<0>(s))->*f)(BOOST_PP_ENUM_SHIFTED(N,M,~));
            }
        };
#endif

#undef M

#define M(z,j,data)                                                             \
            typename seq::I##j i##j =                                          \
                fusion::next(BOOST_PP_CAT(i,BOOST_PP_DEC(j)));

        template <typename Function, class Sequence>
        struct invoke_impl<Function,Sequence,N,false,false,
            typename enabler<
#define L(z,j,data) typename invoke_param_types<Sequence,N>::BOOST_PP_CAT(T, j)
                typename lslboost::result_of<Function(BOOST_PP_ENUM(N,L,~))>::type
              >::type>
#undef L
        {
        private:
            typedef invoke_param_types<Sequence,N> seq;
        public:

            typedef typename lslboost::result_of<
                Function(BOOST_PP_ENUM_PARAMS(N,typename seq::T))
                >::type result_type;

#if N > 0

            template <typename F>
            BOOST_CXX14_CONSTEXPR BOOST_FUSION_GPU_ENABLED
            static inline result_type
            call(F & f, Sequence & s)
            {
                typename seq::I0 i0 = fusion::begin(s);
                BOOST_PP_REPEAT_FROM_TO(1,N,M,~)
                return f( BOOST_PP_ENUM_PARAMS(N,*i) );
            }

#else

            template <typename F>
            BOOST_CONSTEXPR BOOST_FUSION_GPU_ENABLED
            static inline result_type
            call(F & f, Sequence & /*s*/)
            {
                return f();
            }

#endif

        };

        template <typename Function, class Sequence>
        struct invoke_fn_ptr<Function,Sequence,N,false>
        {
        private:
            typedef invoke_param_types<Sequence,N> seq;
        public:

            typedef typename ft::result_type<Function>::type result_type;

#if N > 0

            template <typename F>
            BOOST_CXX14_CONSTEXPR BOOST_FUSION_GPU_ENABLED
            static inline result_type
            call(F & f, Sequence & s)
            {
                typename seq::I0 i0 = fusion::begin(s);
                BOOST_PP_REPEAT_FROM_TO(1,N,M,~)
                return f( BOOST_PP_ENUM_PARAMS(N,*i) );
            }

#else

            template <typename F>
            BOOST_CONSTEXPR BOOST_FUSION_GPU_ENABLED
            static inline result_type
            call(F & f, Sequence & /*s*/)
            {
                return f();
            }

#endif

        };

#if N > 0
        template <typename Function, class Sequence>
        struct invoke_mem_fn<Function,Sequence,N,false>
        {
        private:
            typedef invoke_param_types<Sequence,N> seq;
        public:

            typedef typename ft::result_type<Function>::type result_type;

            template <typename F>
            BOOST_CXX14_CONSTEXPR BOOST_FUSION_GPU_ENABLED
            static inline result_type
            call(F & f, Sequence & s)
            {
                typename seq::I0 i0 = fusion::begin(s);
                BOOST_PP_REPEAT_FROM_TO(1,N,M,~)

                return (that_ptr< typename mpl::front<
                                      ft::parameter_types<Function> >::type
                    >::get(*i0)->*f)(BOOST_PP_ENUM_SHIFTED_PARAMS(N,*i));
            }
        };
#endif

#undef M

        template <class Sequence> struct invoke_param_types<Sequence,N>
        {
#if N > 0
            typedef typename result_of::begin<Sequence>::type I0;
            typedef typename result_of::deref<I0>::type T0;

#define M(z,i,data)                                                             \
            typedef typename result_of::next<                                  \
                BOOST_PP_CAT(I,BOOST_PP_DEC(i))>::type I##i;                   \
            typedef typename result_of::deref<I##i>::type T##i;

            BOOST_PP_REPEAT_FROM_TO(1,N,M,~)
#undef M
#endif
        };


#undef N
#endif // defined(BOOST_PP_IS_ITERATING)
#endif
