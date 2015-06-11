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

#ifndef GSI_EXCEPTION_HPP_INCLUDED
#define GSI_EXCEPTION_HPP_INCLUDED

#include <boost/exception/all.hpp>
#include "gerror.hpp"

using namespace boost;
/*********************************************************************************************
XXXGS
These are standard exceptions but with a 'be_' prefix indicating a boost exception.
Using the boost exc we can typedef structs containing other information. Also, a boost exc
will add filename, line_number information to the exc.
We can define our own exceptions:
eg.
struct gs_easy_populate_error: virtual std::exception, virtual boost::exception{};
And then could define some fields to send back with the exception

typedef boost::error_info<struct tag_file_name,std::string> file_name_info;
typedef boost::error_info<struct tag_errno,int> errno_info;
typedef boost::error_info<struct tag_misc_text,std::string> misc_text_info;
typedef boost::error_info<struct tag_first_int,int> first_int_info;

Note, that when the exc is caught, we can also add information (the structs above) and re throw
catch ( const gs_easy_populate_error& e)
   {
       e<<file_name_info("added in dummy2()");
       throw;
   }
*********************************************************************************************/

  /** Logic errors represent problems in the internal logic of a program;
   *  in theory, these are preventable, and even detectable before the
   *  program runs (e.g., violations of class invariants).
   *  @brief One of two subclasses of exception.
  */
struct be_logic_error: virtual std::exception, virtual boost::exception{};

  /** Thrown by the library, or by you, to report domain errors (domain in
   *  the mathematical sense).  */
struct be_domain_error: virtual std::exception, virtual boost::exception{};

/** Thrown to report invalid arguments to functions.  */
struct be_invalid_argument: virtual std::exception, virtual boost::exception{};

/* Thrown when an object is constructed that would exceed its maximum
   *  permitted size (e.g., a basic_string instance).
*/
//struct be_length_error: virtual std::exception, virtual boost::exception{};

/** This represents an argument whose value is not within the expected
   *  range (e.g., boundary checks in basic_string).  */
struct be_out_of_range: virtual std::exception, virtual boost::exception{};

/** Runtime errors represent problems outside the scope of a program;
   *  they cannot be easily predicted and can generally only be caught as
   *  the program executes.
   *  @brief One of two subclasses of exception.
   */
struct be_runtime_error: virtual std::exception, virtual boost::exception{};

 /** Thrown to indicate range errors in internal computations.  */
struct be_range_error: virtual std::exception, virtual boost::exception{};

 /** Thrown to indicate arithmetic overflow.  */
struct be_overflow_error: virtual std::exception, virtual boost::exception{};

/** Thrown to indicate arithmetic underflow.  */
struct be_underflow_error: virtual std::exception, virtual boost::exception{};


struct gse_file_not_found_error: virtual std::exception, virtual boost::exception{};

typedef boost::error_info<struct tag_file_name,std::string> file_name_info;
typedef boost::error_info<struct tag_errno,int> errno_info;
typedef boost::error_info<struct tag_text,std::string> text_info;
typedef boost::error_info<struct tag_misc_text,std::string> misc_text_info;
typedef boost::error_info<struct tag_first_int,int> first_int_info;



const   int GSExcMaxData=8;

class gsException //: public std::runtime_error
{
public:
    //gsException (errorType e, char const * s=NULL); //: std::runtime_error(str){};
	gsException (errorType e, const wxString &s="",unsigned int data0=0,unsigned int data1=0, unsigned int data2=0);
    virtual ~gsException();

    errorType GetErrorCode(void)
    {
        return(m_err);
    }
	const wxString & GetUserString();
    const wxString & GetString();
    unsigned int    GetData(int index);
    errorType       SetData(int index,unsigned int data);
private:
    errorType m_err;
	wxString m_UserStr;
    wxString m_Str;
    unsigned int    m_data[GSExcMaxData];

};

#endif //GSI_EXCEPTION_HPP_INCLUDED
