/*****************************************************************************
*	GSI a socket interface for Micro-controllers and Controller Area Network *
*   Copyright (C) 2014 Glenn Self                                            *
*                                                                            *
*   This program is free software: you can redistribute it and/or modify     *
*   it under the terms of the GNU General Public License as published by     *
*   the Free Software Foundation, either version 3 of the License, or        *
*   (at your option) any later version.                                      *
*                                                                            *
*    This program is distributed in the hope that it will be useful,         *
*    but WITHOUT ANY WARRANTY; without even the implied warranty of          *
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
*    GNU General Public License for more details.                            *
*                                                                            *
*    You should have received a copy of the GNU General Public License       *
*    along with this program.  If not, see <http://www.gnu.org/licenses/>.   *
*****************************************************************************/
#ifndef _GEXCEPT_INCLUDED 
#define _GEXCEPT_INCLUDED

//commented out 16/2/14 doesn't work in eclipse. ?? needed for codeblocks
//#include <glbldef.h>		doesn't work in eclipse
#include <gerror.h>

#if COMPILER == __GNUC__

#if 0
// Methods for Exception Support for -*- C++ -*-
// Copyright (C) 1994, 1995, 1997 Free Software Foundation

// This file is part of the GNU ANSI C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this library; see the file COPYING.  If not, write to the Free
// Software Foundation, 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

// As a special exception, if you link this library with files
// compiled with a GNU compiler to produce an executable, this does not cause
// the resulting executable to be covered by the GNU General Public License.
// This exception does not however invalidate any other reasons why
// the executable file might be covered by the GNU General Public License.

// Written by Mike Stump based upon the specification in the 20 September 1994
// C++ working paper, ANSI document X3J16/94-0158.

#ifndef __STDEXCEPT__
#define __STDEXCEPT__

#ifdef __GNUG__
#pragma interface "stdexcept"
#endif

#include <exception>
#include <string>

extern "C++" {

namespace std {

class logic_error : public exception {
  string _what;
public:
  logic_error(const string& what_arg): _what (what_arg) { }
  virtual const char* what () const { return _what.c_str (); }
};

class domain_error : public logic_error {
public:
  domain_error (const string& what_arg): logic_error (what_arg) { }
};

class invalid_argument : public logic_error {
public:
  invalid_argument (const string& what_arg): logic_error (what_arg) { }
};

class length_error : public logic_error {
public:
  length_error (const string& what_arg): logic_error (what_arg) { }
};

class out_of_range : public logic_error {
public:
  out_of_range (const string& what_arg): logic_error (what_arg) { }
};

class runtime_error : public exception {
  string _what;
public:
  runtime_error(const string& what_arg): _what (what_arg) { }
  virtual const char* what () const { return _what.c_str (); }
protected:
  runtime_error(): exception () { }
};

class range_error : public runtime_error {
public:
  range_error (const string& what_arg): runtime_error (what_arg) { }
};

class overflow_error : public runtime_error {
public:
  overflow_error (const string& what_arg): runtime_error (what_arg) { }
};

class underflow_error : public runtime_error {
public:
  underflow_error (const string& what_arg): runtime_error (what_arg) { }
};

} // namespace std
 
} // extern "C++"

#endif


#endif
#endif

//modified 16/2/14
//class _Export ErrorClass{
class ErrorClass{

public:
  ErrorClass(void);
  ~ErrorClass(void);
  char *		GetText(errorType error_code);


private:

  struct _ErrS{
    errorType errCode;
    char * str;
  };

  struct _ErrS			*ErrS;
  uint16_t				NumStrs;
};


#endif   //_GEXCEPT_INCLUDED
