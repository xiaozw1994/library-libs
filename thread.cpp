// -*- C++ -*-
//===--------------------------- thread -----------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCPP_THREAD
#define _LIBCPP_THREAD

/*

    thread synopsis

#define __STDCPP_THREADS__ __cplusplus

namespace std
{

class thread
{
public:
    class id;
    typedef pthread_t native_handle_type;

    thread() noexcept;
    template <class F, class ...Args> explicit thread(F&& f, Args&&... args);
    ~thread();

    thread(const thread&) = delete;
    thread(thread&& t) noexcept;

    thread& operator=(const thread&) = delete;
    thread& operator=(thread&& t) noexcept;

    void swap(thread& t) noexcept;

    bool joinable() const noexcept;
    void join();
    void detach();
    id get_id() const noexcept;
    native_handle_type native_handle();

    static unsigned hardware_concurrency() noexcept;
};

void swap(thread& x, thread& y) noexcept;

class thread::id
{
public:
    id() noexcept;
};

bool operator==(thread::id x, thread::id y) noexcept;
bool operator!=(thread::id x, thread::id y) noexcept;
bool operator< (thread::id x, thread::id y) noexcept;
bool operator<=(thread::id x, thread::id y) noexcept;
bool operator> (thread::id x, thread::id y) noexcept;
bool operator>=(thread::id x, thread::id y) noexcept;

template<class charT, class traits>
basic_ostream<charT, traits>&
operator<<(basic_ostream<charT, traits>& out, thread::id id);

namespace this_thread
{

thread::id get_id() noexcept;

void yield() noexcept;

template <class Clock, class Duration>
void sleep_until(const chrono::time_point<Clock, Duration>& abs_time);

template <class Rep, class Period>
void sleep_for(const chrono::duration<Rep, Period>& rel_time);

}  // this_thread

}  // std

*/

#include <__config>
#include <iosfwd>
#include <__functional_base>
#include <type_traits>
#include <cstddef>
#include <functional>
#include <memory>
#include <system_error>
#include <chrono>
#include <__mutex_base>
#ifndef _LIBCPP_HAS_NO_VARIADICS
#include <tuple>
#endif
#include <__threading_support>
#include <__debug>

#if !defined(_LIBCPP_HAS_NO_PRAGMA_SYSTEM_HEADER)
#pragma GCC system_header
#endif

#define __STDCPP_THREADS__ __cplusplus

#ifdef _LIBCPP_HAS_NO_THREADS
#error <thread> is not supported on this single threaded system
#else // !_LIBCPP_HAS_NO_THREADS

_LIBCPP_BEGIN_NAMESPACE_STD

template <class _Tp> class __thread_specific_ptr;
class _LIBCPP_TYPE_VIS __thread_struct;
class _LIBCPP_HIDDEN __thread_struct_imp;
class __assoc_sub_state;

_LIBCPP_FUNC_VIS __thread_specific_ptr<__thread_struct>& __thread_local_data();

class _LIBCPP_TYPE_VIS __thread_struct
{
    __thread_struct_imp* __p_;

    __thread_struct(const __thread_struct&);
    __thread_struct& operator=(const __thread_struct&);
public:
    __thread_struct();
    ~__thread_struct();

    void notify_all_at_thread_exit(condition_variable*, mutex*);
    void __make_ready_at_thread_exit(__assoc_sub_state*);
};

template <class _Tp>
class __thread_specific_ptr
{
    __libcpp_tls_key __key_;

     // Only __thread_local_data() may construct a __thread_specific_ptr
     // and only with _Tp == __thread_struct.
    static_assert((is_same<_Tp, __thread_struct>::value), "");
    __thread_specific_ptr();
    friend _LIBCPP_FUNC_VIS __thread_specific_ptr<__thread_struct>& __thread_local_data();

    __thread_specific_ptr(const __thread_specific_ptr&);
    __thread_specific_ptr& operator=(const __thread_specific_ptr&);

    static void _LIBCPP_TLS_DESTRUCTOR_CC __at_thread_exit(void*);

public:
    typedef _Tp* pointer;

    ~__thread_specific_ptr();

    _LIBCPP_INLINE_VISIBILITY
    pointer get() const {return static_cast<_Tp*>(__libcpp_tls_get(__key_));}
    _LIBCPP_INLINE_VISIBILITY
    pointer operator*() const {return *get();}
    _LIBCPP_INLINE_VISIBILITY
    pointer operator->() const {return get();}
    void set_pointer(pointer __p);
};

template <class _Tp>
void _LIBCPP_TLS_DESTRUCTOR_CC
__thread_specific_ptr<_Tp>::__at_thread_exit(void* __p)
{
    delete static_cast<pointer>(__p);
}

template <class _Tp>
__thread_specific_ptr<_Tp>::__thread_specific_ptr()
{
  int __ec =
      __libcpp_tls_create(&__key_, &__thread_specific_ptr::__at_thread_exit);
  if (__ec)
    __throw_system_error(__ec, "__thread_specific_ptr construction failed");
}

template <class _Tp>
__thread_specific_ptr<_Tp>::~__thread_specific_ptr()
{
    // __thread_specific_ptr is only created with a static storage duration
    // so this destructor is only invoked during program termination. Invoking
    // pthread_key_delete(__key_) may prevent other threads from deleting their
    // thread local data. For this reason we leak the key.
}

template <class _Tp>
void
__thread_specific_ptr<_Tp>::set_pointer(pointer __p)
{
    _LIBCPP_ASSERT(get() == nullptr,
                   "Attempting to overwrite thread local data");
    __libcpp_tls_set(__key_, __p);
}

class _LIBCPP_TYPE_VIS thread;
class _LIBCPP_TYPE_VIS __thread_id;

namespace this_thread
{

_LIBCPP_INLINE_VISIBILITY __thread_id get_id() _NOEXCEPT;

}  // this_thread

template<> struct hash<__thread_id>;

class _LIBCPP_TEMPLATE_VIS __thread_id
{
    // FIXME: pthread_t is a pointer on Darwin but a long on Linux.
    // NULL is the no-thread value on Darwin.  Someone needs to check
    // on other platforms.  We assume 0 works everywhere for now.
    __libcpp_thread_id __id_;

public:
    _LIBCPP_INLINE_VISIBILITY
    __thread_id() _NOEXCEPT : __id_(0) {}

    friend _LIBCPP_INLINE_VISIBILITY
        bool operator==(__thread_id __x, __thread_id __y) _NOEXCEPT
        {return __libcpp_thread_id_equal(__x.__id_, __y.__id_);}
    friend _LIBCPP_INLINE_VISIBILITY
        bool operator!=(__thread_id __x, __thread_id __y) _NOEXCEPT
        {return !(__x == __y);}
    friend _LIBCPP_INLINE_VISIBILITY
        bool operator< (__thread_id __x, __thread_id __y) _NOEXCEPT
        {return  __libcpp_thread_id_less(__x.__id_, __y.__id_);}
    friend _LIBCPP_INLINE_VISIBILITY
        bool operator<=(__thread_id __x, __thread_id __y) _NOEXCEPT
        {return !(__y < __x);}
    friend _LIBCPP_INLINE_VISIBILITY
        bool operator> (__thread_id __x, __thread_id __y) _NOEXCEPT
        {return   __y < __x ;}
    friend _LIBCPP_INLINE_VISIBILITY
        bool operator>=(__thread_id __x, __thread_id __y) _NOEXCEPT
        {return !(__x < __y);}

    template<class _CharT, class _Traits>
    friend
    _LIBCPP_INLINE_VISIBILITY
    basic_ostream<_CharT, _Traits>&
    operator<<(basic_ostream<_CharT, _Traits>& __os, __thread_id __id)
        {return __os << __id.__id_;}

private:
    _LIBCPP_INLINE_VISIBILITY
    __thread_id(__libcpp_thread_id __id) : __id_(__id) {}

    friend __thread_id this_thread::get_id() _NOEXCEPT;
    friend class _LIBCPP_TYPE_VIS thread;
    friend struct _LIBCPP_TEMPLATE_VIS hash<__thread_id>;
};

template<>
struct _LIBCPP_TEMPLATE_VIS hash<__thread_id>
    : public unary_function<__thread_id, size_t>
{
    _LIBCPP_INLINE_VISIBILITY
    size_t operator()(__thread_id __v) const
    {
        return hash<__libcpp_thread_id>()(__v.__id_);
    }
};

namespace this_thread
{

inline _LIBCPP_INLINE_VISIBILITY
__thread_id
get_id() _NOEXCEPT
{
    return __libcpp_thread_get_current_id();
}

}  // this_thread

class _LIBCPP_TYPE_VIS thread
{
    __libcpp_thread_t __t_;

    thread(const thread&);
    thread& operator=(const thread&);
public:
    typedef __thread_id id;
    typedef __libcpp_thread_t native_handle_type;

    _LIBCPP_INLINE_VISIBILITY
    thread() _NOEXCEPT : __t_(0) {}
#ifndef _LIBCPP_HAS_NO_VARIADICS
    template <class _Fp, class ..._Args,
              class = typename enable_if
              <
                   !is_same<typename decay<_Fp>::type, thread>::value
              >::type
             >
        explicit thread(_Fp&& __f, _Args&&... __args);
#else  // _LIBCPP_HAS_NO_VARIADICS
    template <class _Fp> explicit thread(_Fp __f);
#endif
    ~thread();

#ifndef _LIBCPP_HAS_NO_RVALUE_REFERENCES
    _LIBCPP_INLINE_VISIBILITY
    thread(thread&& __t) _NOEXCEPT : __t_(__t.__t_) {__t.__t_ = 0;}
    _LIBCPP_INLINE_VISIBILITY
    thread& operator=(thread&& __t) _NOEXCEPT;
#endif  // _LIBCPP_HAS_NO_RVALUE_REFERENCES

    _LIBCPP_INLINE_VISIBILITY
    void swap(thread& __t) _NOEXCEPT {_VSTD::swap(__t_, __t.__t_);}

    _LIBCPP_INLINE_VISIBILITY
    bool joinable() const _NOEXCEPT {return __t_ != 0;}
    void join();
    void detach();
    _LIBCPP_INLINE_VISIBILITY
    id get_id() const _NOEXCEPT {return __libcpp_thread_get_id(&__t_);}
    _LIBCPP_INLINE_VISIBILITY
    native_handle_type native_handle() _NOEXCEPT {return __t_;}

    static unsigned hardware_concurrency() _NOEXCEPT;
};

#ifndef _LIBCPP_HAS_NO_VARIADICS

template <class _TSp, class _Fp, class ..._Args, size_t ..._Indices>
inline _LIBCPP_INLINE_VISIBILITY
void
__thread_execute(tuple<_TSp, _Fp, _Args...>& __t, __tuple_indices<_Indices...>)
{
    __invoke(_VSTD::move(_VSTD::get<1>(__t)), _VSTD::move(_VSTD::get<_Indices>(__t))...);
}

template <class _Fp>
void* __thread_proxy(void* __vp)
{
    // _Fp = std::tuple< unique_ptr<__thread_struct>, Functor, Args...>
    std::unique_ptr<_Fp> __p(static_cast<_Fp*>(__vp));
    __thread_local_data().set_pointer(_VSTD::get<0>(*__p).release());
    typedef typename __make_tuple_indices<tuple_size<_Fp>::value, 2>::type _Index;
    __thread_execute(*__p, _Index());
    return nullptr;
}

template <class _Fp, class ..._Args,
          class
         >
thread::thread(_Fp&& __f, _Args&&... __args)
{
    typedef unique_ptr<__thread_struct> _TSPtr;
    _TSPtr __tsp(new __thread_struct);
    typedef tuple<_TSPtr, typename decay<_Fp>::type, typename decay<_Args>::type...> _Gp;
    _VSTD::unique_ptr<_Gp> __p(
            new _Gp(std::move(__tsp),
                    __decay_copy(_VSTD::forward<_Fp>(__f)),
                    __decay_copy(_VSTD::forward<_Args>(__args))...));
    int __ec = __libcpp_thread_create(&__t_, &__thread_proxy<_Gp>, __p.get());
    if (__ec == 0)
        __p.release();
    else
        __throw_system_error(__ec, "thread constructor failed");
}

#else  // _LIBCPP_HAS_NO_VARIADICS

template <class _Fp>
struct __thread_invoke_pair {
    // This type is used to pass memory for thread local storage and a functor
    // to a newly created thread because std::pair doesn't work with
    // std::unique_ptr in C++03.
    __thread_invoke_pair(_Fp& __f) : __tsp_(new __thread_struct), __fn_(__f) {}
    unique_ptr<__thread_struct> __tsp_;
    _Fp __fn_;
};

template <class _Fp>
void* __thread_proxy_cxx03(void* __vp)
{
    std::unique_ptr<_Fp> __p(static_cast<_Fp*>(__vp));
    __thread_local_data().set_pointer(__p->__tsp_.release());
    (__p->__fn_)();
    return nullptr;
}

template <class _Fp>
thread::thread(_Fp __f)
{

    typedef __thread_invoke_pair<_Fp> _InvokePair;
    typedef std::unique_ptr<_InvokePair> _PairPtr;
    _PairPtr __pp(new _InvokePair(__f));
    int __ec = __libcpp_thread_create(&__t_, &__thread_proxy_cxx03<_InvokePair>, __pp.get());
    if (__ec == 0)
        __pp.release();
    else
        __throw_system_error(__ec, "thread constructor failed");
}

#endif  // _LIBCPP_HAS_NO_VARIADICS

#ifndef _LIBCPP_HAS_NO_RVALUE_REFERENCES

inline
thread&
thread::operator=(thread&& __t) _NOEXCEPT
{
    if (__t_ != 0)
        terminate();
    __t_ = __t.__t_;
    __t.__t_ = 0;
    return *this;
}

#endif  // _LIBCPP_HAS_NO_RVALUE_REFERENCES

inline _LIBCPP_INLINE_VISIBILITY
void swap(thread& __x, thread& __y) _NOEXCEPT {__x.swap(__y);}

namespace this_thread
{

_LIBCPP_FUNC_VIS void sleep_for(const chrono::nanoseconds& ns);

template <class _Rep, class _Period>
void
sleep_for(const chrono::duration<_Rep, _Period>& __d)
{
    using namespace chrono;
    if (__d > duration<_Rep, _Period>::zero())
    {
        _LIBCPP_CONSTEXPR duration<long double> _Max = nanoseconds::max();
        nanoseconds __ns;
        if (__d < _Max)
        {
            __ns = duration_cast<nanoseconds>(__d);
            if (__ns < __d)
                ++__ns;
        }
        else
            __ns = nanoseconds::max();
        sleep_for(__ns);
    }
}

template <class _Clock, class _Duration>
void
sleep_until(const chrono::time_point<_Clock, _Duration>& __t)
{
    using namespace chrono;
    mutex __mut;
    condition_variable __cv;
    unique_lock<mutex> __lk(__mut);
    while (_Clock::now() < __t)
        __cv.wait_until(__lk, __t);
}

template <class _Duration>
inline _LIBCPP_INLINE_VISIBILITY
void
sleep_until(const chrono::time_point<chrono::steady_clock, _Duration>& __t)
{
    using namespace chrono;
    sleep_for(__t - steady_clock::now());
}

inline _LIBCPP_INLINE_VISIBILITY
void yield() _NOEXCEPT {__libcpp_thread_yield();}

}  // this_thread

_LIBCPP_END_NAMESPACE_STD

#endif // !_LIBCPP_HAS_NO_THREADS

#endif  // _LIBCPP_THREAD
// -*- C++ -*-
//===--------------------------- thread -----------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCPP_THREAD
#define _LIBCPP_THREAD

/*

    thread synopsis

#define __STDCPP_THREADS__ __cplusplus

namespace std
{

class thread
{
public:
    class id;
    typedef pthread_t native_handle_type;

    thread() noexcept;
    template <class F, class ...Args> explicit thread(F&& f, Args&&... args);
    ~thread();

    thread(const thread&) = delete;
    thread(thread&& t) noexcept;

    thread& operator=(const thread&) = delete;
    thread& operator=(thread&& t) noexcept;

    void swap(thread& t) noexcept;

    bool joinable() const noexcept;
    void join();
    void detach();
    id get_id() const noexcept;
    native_handle_type native_handle();

    static unsigned hardware_concurrency() noexcept;
};

void swap(thread& x, thread& y) noexcept;

class thread::id
{
public:
    id() noexcept;
};

bool operator==(thread::id x, thread::id y) noexcept;
bool operator!=(thread::id x, thread::id y) noexcept;
bool operator< (thread::id x, thread::id y) noexcept;
bool operator<=(thread::id x, thread::id y) noexcept;
bool operator> (thread::id x, thread::id y) noexcept;
bool operator>=(thread::id x, thread::id y) noexcept;

template<class charT, class traits>
basic_ostream<charT, traits>&
operator<<(basic_ostream<charT, traits>& out, thread::id id);

namespace this_thread
{

thread::id get_id() noexcept;

void yield() noexcept;

template <class Clock, class Duration>
void sleep_until(const chrono::time_point<Clock, Duration>& abs_time);

template <class Rep, class Period>
void sleep_for(const chrono::duration<Rep, Period>& rel_time);

}  // this_thread

}  // std

*/

#include <__config>
#include <iosfwd>
#include <__functional_base>
#include <type_traits>
#include <cstddef>
#include <functional>
#include <memory>
#include <system_error>
#include <chrono>
#include <__mutex_base>
#ifndef _LIBCPP_HAS_NO_VARIADICS
#include <tuple>
#endif
#include <__threading_support>
#include <__debug>

#if !defined(_LIBCPP_HAS_NO_PRAGMA_SYSTEM_HEADER)
#pragma GCC system_header
#endif

#define __STDCPP_THREADS__ __cplusplus

#ifdef _LIBCPP_HAS_NO_THREADS
#error <thread> is not supported on this single threaded system
#else // !_LIBCPP_HAS_NO_THREADS

_LIBCPP_BEGIN_NAMESPACE_STD

template <class _Tp> class __thread_specific_ptr;
class _LIBCPP_TYPE_VIS __thread_struct;
class _LIBCPP_HIDDEN __thread_struct_imp;
class __assoc_sub_state;

_LIBCPP_FUNC_VIS __thread_specific_ptr<__thread_struct>& __thread_local_data();

class _LIBCPP_TYPE_VIS __thread_struct
{
    __thread_struct_imp* __p_;

    __thread_struct(const __thread_struct&);
    __thread_struct& operator=(const __thread_struct&);
public:
    __thread_struct();
    ~__thread_struct();

    void notify_all_at_thread_exit(condition_variable*, mutex*);
    void __make_ready_at_thread_exit(__assoc_sub_state*);
};

template <class _Tp>
class __thread_specific_ptr
{
    __libcpp_tls_key __key_;

     // Only __thread_local_data() may construct a __thread_specific_ptr
     // and only with _Tp == __thread_struct.
    static_assert((is_same<_Tp, __thread_struct>::value), "");
    __thread_specific_ptr();
    friend _LIBCPP_FUNC_VIS __thread_specific_ptr<__thread_struct>& __thread_local_data();

    __thread_specific_ptr(const __thread_specific_ptr&);
    __thread_specific_ptr& operator=(const __thread_specific_ptr&);

    static void _LIBCPP_TLS_DESTRUCTOR_CC __at_thread_exit(void*);

public:
    typedef _Tp* pointer;

    ~__thread_specific_ptr();

    _LIBCPP_INLINE_VISIBILITY
    pointer get() const {return static_cast<_Tp*>(__libcpp_tls_get(__key_));}
    _LIBCPP_INLINE_VISIBILITY
    pointer operator*() const {return *get();}
    _LIBCPP_INLINE_VISIBILITY
    pointer operator->() const {return get();}
    void set_pointer(pointer __p);
};

template <class _Tp>
void _LIBCPP_TLS_DESTRUCTOR_CC
__thread_specific_ptr<_Tp>::__at_thread_exit(void* __p)
{
    delete static_cast<pointer>(__p);
}

template <class _Tp>
__thread_specific_ptr<_Tp>::__thread_specific_ptr()
{
  int __ec =
      __libcpp_tls_create(&__key_, &__thread_specific_ptr::__at_thread_exit);
  if (__ec)
    __throw_system_error(__ec, "__thread_specific_ptr construction failed");
}

template <class _Tp>
__thread_specific_ptr<_Tp>::~__thread_specific_ptr()
{
    // __thread_specific_ptr is only created with a static storage duration
    // so this destructor is only invoked during program termination. Invoking
    // pthread_key_delete(__key_) may prevent other threads from deleting their
    // thread local data. For this reason we leak the key.
}

template <class _Tp>
void
__thread_specific_ptr<_Tp>::set_pointer(pointer __p)
{
    _LIBCPP_ASSERT(get() == nullptr,
                   "Attempting to overwrite thread local data");
    __libcpp_tls_set(__key_, __p);
}

class _LIBCPP_TYPE_VIS thread;
class _LIBCPP_TYPE_VIS __thread_id;

namespace this_thread
{

_LIBCPP_INLINE_VISIBILITY __thread_id get_id() _NOEXCEPT;

}  // this_thread

template<> struct hash<__thread_id>;

class _LIBCPP_TEMPLATE_VIS __thread_id
{
    // FIXME: pthread_t is a pointer on Darwin but a long on Linux.
    // NULL is the no-thread value on Darwin.  Someone needs to check
    // on other platforms.  We assume 0 works everywhere for now.
    __libcpp_thread_id __id_;

public:
    _LIBCPP_INLINE_VISIBILITY
    __thread_id() _NOEXCEPT : __id_(0) {}

    friend _LIBCPP_INLINE_VISIBILITY
        bool operator==(__thread_id __x, __thread_id __y) _NOEXCEPT
        {return __libcpp_thread_id_equal(__x.__id_, __y.__id_);}
    friend _LIBCPP_INLINE_VISIBILITY
        bool operator!=(__thread_id __x, __thread_id __y) _NOEXCEPT
        {return !(__x == __y);}
    friend _LIBCPP_INLINE_VISIBILITY
        bool operator< (__thread_id __x, __thread_id __y) _NOEXCEPT
        {return  __libcpp_thread_id_less(__x.__id_, __y.__id_);}
    friend _LIBCPP_INLINE_VISIBILITY
        bool operator<=(__thread_id __x, __thread_id __y) _NOEXCEPT
        {return !(__y < __x);}
    friend _LIBCPP_INLINE_VISIBILITY
        bool operator> (__thread_id __x, __thread_id __y) _NOEXCEPT
        {return   __y < __x ;}
    friend _LIBCPP_INLINE_VISIBILITY
        bool operator>=(__thread_id __x, __thread_id __y) _NOEXCEPT
        {return !(__x < __y);}

    template<class _CharT, class _Traits>
    friend
    _LIBCPP_INLINE_VISIBILITY
    basic_ostream<_CharT, _Traits>&
    operator<<(basic_ostream<_CharT, _Traits>& __os, __thread_id __id)
        {return __os << __id.__id_;}

private:
    _LIBCPP_INLINE_VISIBILITY
    __thread_id(__libcpp_thread_id __id) : __id_(__id) {}

    friend __thread_id this_thread::get_id() _NOEXCEPT;
    friend class _LIBCPP_TYPE_VIS thread;
    friend struct _LIBCPP_TEMPLATE_VIS hash<__thread_id>;
};

template<>
struct _LIBCPP_TEMPLATE_VIS hash<__thread_id>
    : public unary_function<__thread_id, size_t>
{
    _LIBCPP_INLINE_VISIBILITY
    size_t operator()(__thread_id __v) const
    {
        return hash<__libcpp_thread_id>()(__v.__id_);
    }
};

namespace this_thread
{

inline _LIBCPP_INLINE_VISIBILITY
__thread_id
get_id() _NOEXCEPT
{
    return __libcpp_thread_get_current_id();
}

}  // this_thread

class _LIBCPP_TYPE_VIS thread
{
    __libcpp_thread_t __t_;

    thread(const thread&);
    thread& operator=(const thread&);
public:
    typedef __thread_id id;
    typedef __libcpp_thread_t native_handle_type;

    _LIBCPP_INLINE_VISIBILITY
    thread() _NOEXCEPT : __t_(0) {}
#ifndef _LIBCPP_HAS_NO_VARIADICS
    template <class _Fp, class ..._Args,
              class = typename enable_if
              <
                   !is_same<typename decay<_Fp>::type, thread>::value
              >::type
             >
        explicit thread(_Fp&& __f, _Args&&... __args);
#else  // _LIBCPP_HAS_NO_VARIADICS
    template <class _Fp> explicit thread(_Fp __f);
#endif
    ~thread();

#ifndef _LIBCPP_HAS_NO_RVALUE_REFERENCES
    _LIBCPP_INLINE_VISIBILITY
    thread(thread&& __t) _NOEXCEPT : __t_(__t.__t_) {__t.__t_ = 0;}
    _LIBCPP_INLINE_VISIBILITY
    thread& operator=(thread&& __t) _NOEXCEPT;
#endif  // _LIBCPP_HAS_NO_RVALUE_REFERENCES

    _LIBCPP_INLINE_VISIBILITY
    void swap(thread& __t) _NOEXCEPT {_VSTD::swap(__t_, __t.__t_);}

    _LIBCPP_INLINE_VISIBILITY
    bool joinable() const _NOEXCEPT {return __t_ != 0;}
    void join();
    void detach();
    _LIBCPP_INLINE_VISIBILITY
    id get_id() const _NOEXCEPT {return __libcpp_thread_get_id(&__t_);}
    _LIBCPP_INLINE_VISIBILITY
    native_handle_type native_handle() _NOEXCEPT {return __t_;}

    static unsigned hardware_concurrency() _NOEXCEPT;
};

#ifndef _LIBCPP_HAS_NO_VARIADICS

template <class _TSp, class _Fp, class ..._Args, size_t ..._Indices>
inline _LIBCPP_INLINE_VISIBILITY
void
__thread_execute(tuple<_TSp, _Fp, _Args...>& __t, __tuple_indices<_Indices...>)
{
    __invoke(_VSTD::move(_VSTD::get<1>(__t)), _VSTD::move(_VSTD::get<_Indices>(__t))...);
}

template <class _Fp>
void* __thread_proxy(void* __vp)
{
    // _Fp = std::tuple< unique_ptr<__thread_struct>, Functor, Args...>
    std::unique_ptr<_Fp> __p(static_cast<_Fp*>(__vp));
    __thread_local_data().set_pointer(_VSTD::get<0>(*__p).release());
    typedef typename __make_tuple_indices<tuple_size<_Fp>::value, 2>::type _Index;
    __thread_execute(*__p, _Index());
    return nullptr;
}

template <class _Fp, class ..._Args,
          class
         >
thread::thread(_Fp&& __f, _Args&&... __args)
{
    typedef unique_ptr<__thread_struct> _TSPtr;
    _TSPtr __tsp(new __thread_struct);
    typedef tuple<_TSPtr, typename decay<_Fp>::type, typename decay<_Args>::type...> _Gp;
    _VSTD::unique_ptr<_Gp> __p(
            new _Gp(std::move(__tsp),
                    __decay_copy(_VSTD::forward<_Fp>(__f)),
                    __decay_copy(_VSTD::forward<_Args>(__args))...));
    int __ec = __libcpp_thread_create(&__t_, &__thread_proxy<_Gp>, __p.get());
    if (__ec == 0)
        __p.release();
    else
        __throw_system_error(__ec, "thread constructor failed");
}

#else  // _LIBCPP_HAS_NO_VARIADICS

template <class _Fp>
struct __thread_invoke_pair {
    // This type is used to pass memory for thread local storage and a functor
    // to a newly created thread because std::pair doesn't work with
    // std::unique_ptr in C++03.
    __thread_invoke_pair(_Fp& __f) : __tsp_(new __thread_struct), __fn_(__f) {}
    unique_ptr<__thread_struct> __tsp_;
    _Fp __fn_;
};

template <class _Fp>
void* __thread_proxy_cxx03(void* __vp)
{
    std::unique_ptr<_Fp> __p(static_cast<_Fp*>(__vp));
    __thread_local_data().set_pointer(__p->__tsp_.release());
    (__p->__fn_)();
    return nullptr;
}

template <class _Fp>
thread::thread(_Fp __f)
{

    typedef __thread_invoke_pair<_Fp> _InvokePair;
    typedef std::unique_ptr<_InvokePair> _PairPtr;
    _PairPtr __pp(new _InvokePair(__f));
    int __ec = __libcpp_thread_create(&__t_, &__thread_proxy_cxx03<_InvokePair>, __pp.get());
    if (__ec == 0)
        __pp.release();
    else
        __throw_system_error(__ec, "thread constructor failed");
}

#endif  // _LIBCPP_HAS_NO_VARIADICS

#ifndef _LIBCPP_HAS_NO_RVALUE_REFERENCES

inline
thread&
thread::operator=(thread&& __t) _NOEXCEPT
{
    if (__t_ != 0)
        terminate();
    __t_ = __t.__t_;
    __t.__t_ = 0;
    return *this;
}

#endif  // _LIBCPP_HAS_NO_RVALUE_REFERENCES

inline _LIBCPP_INLINE_VISIBILITY
void swap(thread& __x, thread& __y) _NOEXCEPT {__x.swap(__y);}

namespace this_thread
{

_LIBCPP_FUNC_VIS void sleep_for(const chrono::nanoseconds& ns);

template <class _Rep, class _Period>
void
sleep_for(const chrono::duration<_Rep, _Period>& __d)
{
    using namespace chrono;
    if (__d > duration<_Rep, _Period>::zero())
    {
        _LIBCPP_CONSTEXPR duration<long double> _Max = nanoseconds::max();
        nanoseconds __ns;
        if (__d < _Max)
        {
            __ns = duration_cast<nanoseconds>(__d);
            if (__ns < __d)
                ++__ns;
        }
        else
            __ns = nanoseconds::max();
        sleep_for(__ns);
    }
}

template <class _Clock, class _Duration>
void
sleep_until(const chrono::time_point<_Clock, _Duration>& __t)
{
    using namespace chrono;
    mutex __mut;
    condition_variable __cv;
    unique_lock<mutex> __lk(__mut);
    while (_Clock::now() < __t)
        __cv.wait_until(__lk, __t);
}

template <class _Duration>
inline _LIBCPP_INLINE_VISIBILITY
void
sleep_until(const chrono::time_point<chrono::steady_clock, _Duration>& __t)
{
    using namespace chrono;
    sleep_for(__t - steady_clock::now());
}

inline _LIBCPP_INLINE_VISIBILITY
void yield() _NOEXCEPT {__libcpp_thread_yield();}

}  // this_thread

_LIBCPP_END_NAMESPACE_STD

#endif // !_LIBCPP_HAS_NO_THREADS

#endif  // _LIBCPP_THREAD
