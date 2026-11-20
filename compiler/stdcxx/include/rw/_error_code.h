// Original error-code storage shared by C++98 runtime and C++11 interfaces.
#ifndef _RWSTD_ERROR_CODE_STORAGE_INCLUDED
#define _RWSTD_ERROR_CODE_STORAGE_INCLUDED
#include <rw/_exception.h>
#ifdef _RWSTD_CXX11
#include <type_traits>
#endif
namespace std {
class error_category;
class error_condition;
#ifdef _RWSTD_CXX11
template<class T> struct is_error_code_enum : false_type {};
template<class T> struct is_error_condition_enum : false_type {};
#endif
class error_code {
    int value_;
    unsigned kind_; // 0: category pointer/default system; 1: runtime stream tag.
    const error_category* category_;
    error_code(int value,unsigned kind) _THROWS(())
        : value_(value),kind_(kind),category_(0) {}
public:
    error_code() _THROWS(()) : value_(0),kind_(0),category_(0) {}
    error_code(int value,const error_category& cat) _THROWS(())
        : value_(value),kind_(0),category_(&cat) {}
    void assign(int value,const error_category& cat) _THROWS(()) {
        value_=value; kind_=0; category_=&cat;
    }
    void clear() _THROWS(()) { value_=0; kind_=0; category_=0; }
    int value() const _THROWS(()) { return value_; }
    static error_code _C_stream() _THROWS(()) { return error_code(1,1U); }
#ifdef _RWSTD_CXX11
    template<class E,typename enable_if<is_error_code_enum<E>::value,int>::type=0>
    error_code(E value) noexcept { *this=make_error_code(value); }
    template<class E> typename enable_if<is_error_code_enum<E>::value,error_code&>::type
    operator=(E value) noexcept { *this=make_error_code(value); return *this; }
    const error_category& category() const noexcept;
    error_condition default_error_condition() const noexcept;
    string message() const;
    explicit operator bool() const noexcept { return value_!=0; }
#endif
};
}
#endif
