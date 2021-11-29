///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2014-2019, PALANDesign Hannover, Germany
//
// \license The MIT License (MIT)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// \brief Tiny printf, sprintf and (v)snprintf implementation, optimized for speed on
//        embedded systems with a very limited resources. These routines are thread
//        safe and reentrant!
//        Use this instead of the bloated standard/newlib printf cause these use
//        malloc for printf (and may not be thread safe).
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _TINY_PRINTF_CONF_H_
#define _TINY_PRINTF_CONF_H_

// support for the floating point type (%f)
// default: activated
#define TINY_PRINTF_SUPPORT_FLOAT

// support for exponential floating point notation (%e/%g)
// default: activated
#define TINY_PRINTF_SUPPORT_EXPONENTIAL

// support for the long long types (%llu or %p)
// default: activated
#define TINY_PRINTF_SUPPORT_LONG_LONG

// support for the ptrdiff_t type (%t)
// ptrdiff_t is normally defined in <stddef.h> as long or long long type
// default: activated
#define TINY_PRINTF_SUPPORT_PTRDIFF_T

// 'ntoa' conversion buffer size, this must be big enough to hold one converted
// numeric number including padded zeros (dynamically created on stack)
// default: 32 byte
#define TINY_PRINTF_NTOA_BUFFER_SIZE    32U

// 'ftoa' conversion buffer size, this must be big enough to hold one converted
// float number including padded zeros (dynamically created on stack)
// default: 32 byte
#define TINY_PRINTF_FTOA_BUFFER_SIZE    32U

// define the default floating point precision
// default: 6 digits
#define TINY_PRINTF_DEFAULT_FLOAT_PRECISION  6U

// define the largest float suitable to print with %f
// default: 1e9
#define TINY_PRINTF_MAX_FLOAT  1e9

#endif  // _TINY_PRINTF_CONF_H_
