/*! \file csv.hpp
    \brief Text input and output archives */
/*
  Copyright (c) 2014, Randolph Voorhies, Shane Grant
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
      * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
      * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
      * Neither the name of cereal nor the
        names of its contributors may be used to endorse or promote products
        derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL RANDOLPH VOORHIES OR SHANE GRANT BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef CEREAL_ARCHIVES_CSV_HPP_
#define CEREAL_ARCHIVES_CSV_HPP_

#include "cereal/cereal.hpp"
#include <charconv>
#include <sstream>
#include <type_traits>


namespace cereal
{
  // ######################################################################
  //! An output archive designed to save data in a csv representation
  /*! This archive outputs data to a stream in a csv, space seperated,
      representation with as little extra metadata as possible.

      When using a csv archive and a file stream, you must use the
      std::ios::out format flag to avoid having your data altered
      inadvertently.

      \ingroup Archives */
  class CSVOutputArchive : public OutputArchive<CSVOutputArchive, AllowEmptyClassElision>
  {
    public:
    //! Construct, outputting to the provided stream
    /*! @param stream The stream to output to.  Can be a stringstream, a file stream, or
                    even cout! */
    CSVOutputArchive(std::ostream & stream) :
    OutputArchive<CSVOutputArchive, AllowEmptyClassElision>(this),
    itsStream(stream)
    { }

    ~CSVOutputArchive() CEREAL_NOEXCEPT = default;

    //! Writes size bytes of data to the output stream
    template<class T>
    void saveCsv( T const & t ) {
        if constexpr ( std::is_arithmetic<T>::value ) {
            char begin [ 64 ] { 0 };
            std::size_t size = 0;
            if constexpr ( std::is_integral<T>::value ) {
                auto [ end, ec ] = std::to_chars ( begin, begin + 64, t, 10 );
                *end++ = ' ';
                size = static_cast<std::size_t> ( end - begin );
            }
            else if constexpr ( std::is_floating_point<T>::value ) {
                auto [ end, ec ] = std::to_chars ( begin, begin + 64, t, std::chars_format::fixed );
                *end++ = ' ';
                size = static_cast<std::size_t> ( end - begin );
            }
            const std::size_t writtenSize = static_cast< std::size_t >( itsStream.rdbuf ( )->sputn ( reinterpret_cast<const char*> ( begin ), size ) );
            if ( writtenSize != size )
                throw Exception ( "Failed to write " + std::to_string ( size ) + " bytes to output stream! Wrote " + std::to_string ( writtenSize ) );
        }
        else if constexpr ( std::is_same<T, std::string>::value ) {
            itsStream << t << ' ';
        }
    }

    private:
      std::ostream & itsStream;
  };

  // ######################################################################
  //! An input archive designed to load data saved using CSVOutputArchive
  /*  This archive does nothing to ensure that the endianness of the saved
      and loaded data is the same.  If you need to have portability over
      architectures with different endianness, use PortableCsvOutputArchive.

      When using a csv archive and a file stream, you must use the
      std::ios::in format flag to avoid having your data altered
      inadvertently.

      \ingroup Archives */
  class CSVInputArchive : public InputArchive<CSVInputArchive, AllowEmptyClassElision>
  {
    public:
      //! Construct, loading from the provided stream
      CSVInputArchive(std::istream & stream) :
        InputArchive<CSVInputArchive, AllowEmptyClassElision>(this),
        itsStream(stream)
      { }

      ~CSVInputArchive() CEREAL_NOEXCEPT = default;

      //! Reads size bytes of data from the input stream
      template<class T>
      void loadCsv( T & t )
      {
        char begin [ 64 ] { 0 };
        char * end = begin;
        while ( true ) {
            *end = static_cast<char> ( itsStream.rdbuf ( )->sbumpc ( ) );
            if ( -1 == *end ) {
                break;
            }
            if ( ' ' == *end ) {
                break;
            }
            ++end;
        }

        if constexpr ( std::is_integral<T>::value ) {
            std::from_chars ( begin, end, t, 10 );
        }
        else if constexpr ( std::is_floating_point<T>::value ) {
            std::from_chars ( begin, end, t, std::chars_format::fixed );
        }
        else if constexpr ( std::is_same<T, std::string>::value ) {
            t = std::string { begin, end };
        }
      }

    private:
      std::istream & itsStream;
  };

  // ######################################################################
  // Common CsvArchive serialization functions

  //! Saving for POD types to csv
  template<class T> inline
  typename std::enable_if<std::disjunction<std::is_arithmetic<T>, std::is_same<T, std::string>>::value, void>::type
  CEREAL_SAVE_FUNCTION_NAME(CSVOutputArchive & ar, T const & t)
  {
    ar.saveCsv(t);
  }

  //! Loading for POD types from csv
  template<class T> inline
  typename std::enable_if<std::disjunction<std::is_arithmetic<T>, std::is_same<T, std::string>>::value, void>::type
  CEREAL_LOAD_FUNCTION_NAME(CSVInputArchive & ar, T & t)
  {
    ar.loadCsv(t);
  }

  //! Serializing NVP types to csv
  template <class Archive, class T> inline
  CEREAL_ARCHIVE_RESTRICT(CSVInputArchive, CSVOutputArchive)
  CEREAL_SERIALIZE_FUNCTION_NAME( Archive & ar, NameValuePair<T> & t )
  {
    ar( t.value );
  }

  //! Serializing SizeTags to csv
  template <class Archive, class T> inline
  CEREAL_ARCHIVE_RESTRICT(CSVInputArchive, CSVOutputArchive)
  CEREAL_SERIALIZE_FUNCTION_NAME( Archive & ar, SizeTag<T> & t )
  {
    ar( t.size );
  }

  /*
  template <class T>
  struct BinaryData
  {
    //! Internally store the pointer as a void *, keeping const if created with
    //! a const pointer
    using PT = typename std::conditional<std::is_const<typename std::remove_pointer<T>::type>::value,
                                         const void *,
                                         void *>::type;

    BinaryData( T && d, uint64_t s ) : data(std::forward<T>(d)), size(s) {}

    PT data;       //!< pointer to beginning of data
    uint64_t size; //!< size in bytes
  };

  */
  /*
  //! Saving binary data
  template <class T> inline
  void CEREAL_SAVE_FUNCTION_NAME(CSVOutputArchive & ar, CsvData<T> const & bd)
  {
    ar.saveCsv( bd.data, static_cast<std::size_t>( bd.size ) );
  }

  //! Loading binary data
  template <class T> inline
  void CEREAL_LOAD_FUNCTION_NAME(CSVInputArchive & ar, CsvData<T> & bd)
  {
    ar.loadCsv(bd.data, static_cast<std::size_t>(bd.size));
  }
  */
} // namespace cereal

// register archives for polymorphic support
CEREAL_REGISTER_ARCHIVE(cereal::CSVOutputArchive)
CEREAL_REGISTER_ARCHIVE(cereal::CSVInputArchive)

// tie input and output archives together
CEREAL_SETUP_ARCHIVE_TRAITS(cereal::CSVInputArchive, cereal::CSVOutputArchive)

#endif // CEREAL_ARCHIVES_CSV_HPP_
