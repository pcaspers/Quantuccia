/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file errors.hpp
    \brief Classes and functions for error handling.
*/

#ifndef quantlib_errors_hpp
#define quantlib_errors_hpp

#include <ql/qldefines.hpp>
#include <boost/assert.hpp>
#include <boost/current_function.hpp>
#include <boost/shared_ptr.hpp>
#include <exception>
#include <sstream>
#include <string>

#include <stdexcept>

namespace QuantLib {

    //! Base error class
    class Error : public std::exception {
      public:
        /*! The explicit use of this constructor is not advised.
            Use the QL_FAIL macro instead.
        */
        Error(const std::string& file,
              long line,
              const std::string& functionName,
              const std::string& message = "");
        /*! the automatically generated destructor would
            not have the throw specifier.
        */
        ~Error() throw() {}
        //! returns the error message.
        const char* what() const throw ();
      private:
        boost::shared_ptr<std::string> message_;
    };

}

/* Fix C4127: conditional expression is constant when wrapping macros
   with do { ... } while(false); on MSVC
*/
#define MULTILINE_MACRO_BEGIN do {

#if defined(BOOST_MSVC) && BOOST_MSVC >= 1500
    /* __pragma is available from VC++9 */
    #define MULTILINE_MACRO_END \
        __pragma(warning(push)) \
        __pragma(warning(disable:4127)) \
        } while(false) \
        __pragma(warning(pop))
#else
    #define MULTILINE_MACRO_END } while(false)
#endif


/*! \def QL_FAIL
    \brief throw an error (possibly with file and line information)
*/
#define QL_FAIL(message) \
MULTILINE_MACRO_BEGIN \
    std::ostringstream _ql_msg_stream; \
    _ql_msg_stream << message; \
    throw QuantLib::Error(__FILE__,__LINE__, \
                          BOOST_CURRENT_FUNCTION,_ql_msg_stream.str()); \
MULTILINE_MACRO_END


/*! \def QL_ASSERT
    \brief throw an error if the given condition is not verified
*/
#define QL_ASSERT(condition,message) \
if (!(condition)) { \
    std::ostringstream _ql_msg_stream; \
    _ql_msg_stream << message; \
    throw QuantLib::Error(__FILE__,__LINE__, \
                          BOOST_CURRENT_FUNCTION,_ql_msg_stream.str()); \
 } else 


/*! \def QL_REQUIRE
    \brief throw an error if the given pre-condition is not verified
*/
#define QL_REQUIRE(condition,message) \
if (!(condition)) { \
    std::ostringstream _ql_msg_stream; \
    _ql_msg_stream << message; \
    throw QuantLib::Error(__FILE__,__LINE__, \
                          BOOST_CURRENT_FUNCTION,_ql_msg_stream.str()); \
 } else 


/*! \def QL_ENSURE
    \brief throw an error if the given post-condition is not verified
*/
#define QL_ENSURE(condition,message) \
if (!(condition)) { \
    std::ostringstream _ql_msg_stream; \
    _ql_msg_stream << message; \
    throw QuantLib::Error(__FILE__,__LINE__, \
                          BOOST_CURRENT_FUNCTION,_ql_msg_stream.str()); \
 } else

// implementation

namespace {

    #if defined(_MSC_VER) || defined(__BORLANDC__)
    // allow Visual Studio integration
    inline std::string format(
                       #ifdef QL_ERROR_LINES
                       const std::string& file, long line,
                       #else
                       const std::string&, long,
                       #endif
                       #ifdef QL_ERROR_FUNCTIONS
                       const std::string& function,
                       #else
                       const std::string&,
                       #endif
                       const std::string& message) {
        std::ostringstream msg;
        #ifdef QL_ERROR_FUNCTIONS
        if (function != "(unknown)")
            msg << function << ": ";
        #endif
        #ifdef QL_ERROR_LINES
        msg << "\n  " << file << "(" << line << "): \n";
        #endif
        msg << message;
        return msg.str();
    }
    #else
    // use gcc format (e.g. for integration with Emacs)
    inline std::string format(const std::string& file, long line,
                       const std::string& function,
                       const std::string& message) {
        std::ostringstream msg;
        #ifdef QL_ERROR_LINES
        msg << "\n" << file << ":" << line << ": ";
        #endif
        #ifdef QL_ERROR_FUNCTIONS
        if (function != "(unknown)")
            msg << "In function `" << function << "': \n";
        #endif
        msg << message;
        return msg.str();
    }
    #endif

}

namespace boost {

    // must be defined by the user
    inline void assertion_failed(char const * expr, char const * function,
                          char const * file, long line) {
        throw std::runtime_error(format(file, line, function,
                                        "Boost assertion failed: " +
                                        std::string(expr)));
    }

    inline void assertion_failed_msg(char const * expr, char const * msg,
                              char const * function, char const * file,
                              long line) {
        throw std::runtime_error(format(file, line, function,
                                        "Boost assertion failed: " +
                                        std::string(expr) + ": " +
                                        std::string(msg)));
    }

}

namespace QuantLib {

    inline Error::Error(const std::string& file, long line,
                 const std::string& function,
                 const std::string& message) {
        message_ = boost::shared_ptr<std::string>(new std::string(
                                      format(file, line, function, message)));
    }

    inline const char* Error::what() const throw () {
        return message_->c_str();
    }

}
    
#endif

