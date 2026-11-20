// Original ABI storage for system_error and ios_base::failure in both modes.
#ifndef _RWSTD_SYSTEM_ERROR_BASE_INCLUDED
#define _RWSTD_SYSTEM_ERROR_BASE_INCLUDED
#include <stdexcept>
#include <rw/_error_code.h>
namespace __rw {
class __rw_system_error : public std::runtime_error {
    std::error_code code_;
protected:
    __rw_system_error(const std::string& text,const std::error_code& code)
        : std::runtime_error(text),code_(code) {}
    __rw_system_error(const char* text,const std::error_code& code)
        : std::runtime_error(text),code_(code) {}
public:
    __rw_system_error(const __rw_system_error& rhs) _THROWS(())
        : std::runtime_error(rhs),code_(rhs.code_) {}
    __rw_system_error& operator=(const __rw_system_error& rhs) _THROWS(()) {
        std::runtime_error::operator=(rhs); code_=rhs.code_; return *this;
    }
    virtual ~__rw_system_error() _THROWS(()) {}
    const std::error_code& code() const _THROWS(()) { return code_; }
#ifdef _RWSTD_CXX11
    __rw_system_error(std::error_code,const std::string&);
    __rw_system_error(std::error_code,const char*);
    __rw_system_error(std::error_code);
    __rw_system_error(int,const std::error_category&,const std::string&);
    __rw_system_error(int,const std::error_category&,const char*);
    __rw_system_error(int,const std::error_category&);
#endif
};
}
#ifdef _RWSTD_CXX11
namespace std { typedef __rw::__rw_system_error system_error; }
#endif
#endif
