/*
  Copyright (C) 2013 Przemek Mazurkiewicz (Przemek.Mazurkiewicz.2009@gmail.com)

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#ifndef XML_CHARACTERS_READER_HPP__f66b9cdaf20734ef11086d0851a9c563
#define XML_CHARACTERS_READER_HPP__f66b9cdaf20734ef11086d0851a9c563

#include <istream>

/**
	@file CharactersReader.hpp
*/

namespace Xml
{
	/**
		@brief Encoding stuff.
	*/
	namespace Encoding
	{
		/**
			@brief Abstract class for reading characters from XML documents.
		*/
		class CharactersReader
		{
		public:
			/**
				@brief Destructor.
			*/
			virtual ~CharactersReader();

			/**
				@brief Checks if character is allowed in XML document.

				@param codePoint Unicode character.
				@return True if character is allowed in XML document.
				@sa http://www.w3.org/TR/2008/REC-xml-20081126/#NT-Char.
			*/
			static bool IsChar(char32_t codePoint);

			/**
				@brief Checks if character is a white space.

				@param codePoint Unicode character.
				@return True if character is either space, carriage return, line feed or tab.
			*/
			static bool IsWhiteSpace(char32_t codePoint);

			/**
				@brief Checks if character is allowed to be the first character of the XML name.

				@param codePoint Unicode character.
				@return True if character is allowed to be the first character of the XML name.
				@sa http://www.w3.org/TR/2008/REC-xml-20081126/#NT-NameStartChar.
			*/
			static bool IsNameStartChar(char32_t codePoint);

			/**
				@brief Checks if character is allowed to be the one of
					the XML name characters except first.

				@param codePoint Unicode character.
				@return True if character is allowed to be the one of
					the XML name characters except first.
				@sa http://www.w3.org/TR/2008/REC-xml-20081126/#NT-NameChar.
			*/
			static bool IsNameChar(char32_t codePoint);

			/**
				@brief Checks if character is allowed to be the first character of
					the XML encoding declaration name.

				@param codePoint Unicode character.
				@return True if character is allowed to be the first character of
					the XML encoding declaration name.
				@sa http://www.w3.org/TR/2008/REC-xml-20081126/#NT-EncName.
			*/
			static bool IsEncNameStartChar(char32_t codePoint);

			/**
				@brief Checks if character is allowed to be the one of
					the XML encoding declaration name characters except first.

				@param codePoint Unicode character.
				@return True if character is allowed to be the one of
					the XML encoding declaration name characters except first.
				@sa http://www.w3.org/TR/2008/REC-xml-20081126/#NT-EncName.
			*/
			static bool IsEncNameChar(char32_t codePoint);

			/**
				@brief Gets the value represented by hexadecimal character.

				@param codePoint Unicode character.
				@return Value represented by hexadecimal character,
					or -1 if this is not the hexadecimal character.
			*/
			static int GetHexDigitValue(char32_t codePoint);

			/**
				@brief Reads one character from the specifically encoded source
					and translates it to the Unicode character.

				The source could be a stream, buffer or anything
				depending on derived class.

				@param[out] result Variable that will receive the Unicode character.
				@return @b 1 if the character was read successfully.
					@b 0 if there are no more characters to read.
					@b -1 if character is not allowed in XML document.
					@b -2 if there was an error while reading character.
			*/
			virtual int ReadCharacter(char32_t& result) = 0;
		};

		/**
			@brief Abstract UTF-8 characters reader base class.
		*/
		class Utf8ReaderBase : public CharactersReader
		{
		protected:
			/**
				@brief Gets the bytes count of UTF-8 encoded character.

				@param leadingByte First byte of character sequence.
				@return Bytes count of UTF-8 encoded character or 0 if
					character is invalid.
			*/
			static unsigned int GetBytesCount(unsigned char leadingByte);
		public:
			/**
				@brief Destructor.
			*/
			virtual ~Utf8ReaderBase();
		};

		/**
			@brief UTF-8 characters reader from input stream.
		*/
		class Utf8StreamReader : public Utf8ReaderBase
		{
		private:
			std::istream* in;
		public:
			/**
				@brief Constructor.

				@param[in,out] inputStream Input stream from which
					a Unicode characters will be extracted.
			*/
			Utf8StreamReader(std::istream* inputStream);

			/**
				@brief Destructor.
			*/
			virtual ~Utf8StreamReader();

			/**
				@brief Reads one Unicode character.

				Depending on the character there could be from 1 to 4
				bytes needed to read from the UTF-8 encoded stream.

				@param[out] result Variable that will receive the Unicode character.
				@return @b 1 if the character was read successfully.
					@b 0 if there are no more characters to read.
					@b -1 if character is not allowed in XML document.
					@b -2 if there was an error while reading character.
			*/
			virtual int ReadCharacter(char32_t& result);

			/**
				@brief Gets the pointer to the input stream.
			*/
			std::istream* GetInputStream() const;

			/**
				@brief Replaces the pointer to the input stream.
			*/
			void ResetInputStream(std::istream* inputStream);
		};

		/**
			@brief UTF-8 characters reader from iterators.

			Purpose of this class is an ability to parsing XML documents
			from iterators with a fixed encoding (UTF-8) without
			checking BOM and character set in XML declaration.
			If you are not sure how document is encoded, use Xml::Inspector
			constructor or Xml::Inspector::Reset method with specified
			iterators instead of this reader.

			You can also use the reader object without intermediate
			Xml::Inspector class:
			@code{.cpp}
            #include "CharactersReader.hpp"
            #include &lt;iostream&gt;
            #include &lt;cstdlib&gt;
            #include &lt;string&gt;

            int main()
            {
                // We want to convert UTF-8 string to UTF-32 string.
                std::string source =
                    u8"<root>Greek small letter pi: \U000003C0.</root>";
                std::u32string destination;
                std::u32string desired =
                    U"<root>Greek small letter pi: \U000003C0.</root>";

                Xml::Encoding::Utf8IteratorsReader<std::string::iterator> reader(
                    source.begin(), source.end());

                char32_t c;
                while (reader.ReadCharacter(c) == 1)
                    destination.push_back(c);

                if (destination == desired)
                    std::cout << "OK!\n";
                else
                    std::cout << "Not OK :(\n";

                return EXIT_SUCCESS;
            }
			@endcode
			Result:
			@verbatim
			OK!
			@endverbatim
		*/
		template <typename TInputIterator>
		class Utf8IteratorsReader : public Utf8ReaderBase
		{
		private:
			TInputIterator iterBegin;
			TInputIterator iterEnd;
		public:
			/**
				@brief Constructor.

				@param first,last Input iterators to the initial
					and final positions in a sequence of bytes. The range used
					is [first,last), which contains all the bytes
					between first and last, including the element pointed
					by first but not the element pointed by last.
			*/
			Utf8IteratorsReader(TInputIterator first, TInputIterator last);

			/**
				@brief Destructor.
			*/
			virtual ~Utf8IteratorsReader();

			/**
				@brief Reads one Unicode character.

				Depending on the character there could be from 1 to 4
				bytes per one Unicode character.

				@param[out] result Variable that will receive the Unicode character.
				@return @b 1 if the character was read successfully.
					@b 0 if there are no more characters to read.
					@b -1 if character is not allowed in XML document.
					@b -2 if there was an error while reading character.
			*/
			virtual int ReadCharacter(char32_t& result);

			/**
				@brief Gets the first iterator.
			*/
			TInputIterator GetFirstIterator() const;
			
			/**
				@brief Gets the last iterator.
			*/
			TInputIterator GetLastIterator() const;

			/**
				@brief Replaces the iterators.

				@param first,last Input iterators to the initial
					and final positions in a sequence of bytes. The range used
					is [first,last), which contains all the bytes
					between first and last, including the element pointed
					by first but not the element pointed by last.
			*/
			void ResetIterators(TInputIterator first, TInputIterator last);
		};

		/**
			@brief UTF-16 (big endian) characters reader from input stream.
		*/
		class Utf16BEStreamReader : public CharactersReader
		{
		private:
			std::istream* in;
		public:
			/**
				@brief Constructor.

				@param[in,out] inputStream Input stream from which
					a Unicode characters will be extracted.
			*/
			Utf16BEStreamReader(std::istream* inputStream);

			/**
				@brief Destructor.
			*/
			virtual ~Utf16BEStreamReader();

			/**
				@brief Reads one Unicode character.

				Depending on the character there could be a 2 or 4
				bytes needed to read from the UTF-16 (big endian) encoded stream.

				@param[out] result Variable that will receive the Unicode character.
				@return @b 1 if the character was read successfully.
					@b 0 if there are no more characters to read.
					@b -1 if character is not allowed in XML document.
					@b -2 if there was an error while reading character.
			*/
			virtual int ReadCharacter(char32_t& result);

			/**
				@brief Gets the pointer to the input stream.
			*/
			std::istream* GetInputStream() const;

			/**
				@brief Replaces the pointer to the input stream.
			*/
			void ResetInputStream(std::istream* inputStream);
		};

		/**
			@brief UTF-16 (little endian) characters reader from the input stream.
		*/
		class Utf16LEStreamReader : public CharactersReader
		{
		private:
			std::istream* in;
		public:
			/**
				@brief Constructor.

				@param[in,out] inputStream Input stream from which
					a Unicode characters will be extracted.
			*/
			Utf16LEStreamReader(std::istream* inputStream);

			/**
				@brief Destructor.
			*/
			virtual ~Utf16LEStreamReader();

			/**
				@brief Reads one Unicode character.

				Depending on the character there could be a 2 or 4
				bytes needed to read from the UTF-16 (little endian) encoded stream.

				@param[out] result Variable that will receive the Unicode character.
				@return @b 1 if the character was read successfully.
					@b 0 if there are no more characters to read.
					@b -1 if character is not allowed in XML document.
					@b -2 if there was an error while reading character.
			*/
			virtual int ReadCharacter(char32_t& result);

			/**
				@brief Gets the pointer to the input stream.
			*/
			std::istream* GetInputStream() const;

			/**
				@brief Replaces the pointer to the input stream.
			*/
			void ResetInputStream(std::istream* inputStream);
		};

		/**
			@brief UTF-32 (big endian) characters reader from the input stream.
		*/
		class Utf32BEStreamReader : public CharactersReader
		{
		private:
			std::istream* in;
		public:
			/**
				@brief Constructor.

				@param[in,out] inputStream Input stream from which
					a Unicode characters will be extracted.
			*/
			Utf32BEStreamReader(std::istream* inputStream);

			/**
				@brief Destructor.
			*/
			virtual ~Utf32BEStreamReader();

			/**
				@brief Reads one Unicode character.

				There are 4 bytes needed to read per one character
				from the UTF-32 (big endian) encoded stream.

				@param[out] result Variable that will receive the Unicode character.
				@return @b 1 if the character was read successfully.
					@b 0 if there are no more characters to read.
					@b -1 if character is not allowed in XML document.
					@b -2 if there was an error while reading character.
			*/
			virtual int ReadCharacter(char32_t& result);

			/**
				@brief Gets the pointer to the input stream.
			*/
			std::istream* GetInputStream() const;

			/**
				@brief Replaces the pointer to the input stream.
			*/
			void ResetInputStream(std::istream* inputStream);
		};

		/**
			@brief UTF-32 (little endian) characters reader from the input stream.
		*/
		class Utf32LEStreamReader : public CharactersReader
		{
		private:
			std::istream* in;
		public:
			/**
				@brief Constructor.

				@param[in,out] inputStream Input stream from which
					a Unicode characters will be extracted.
			*/
			Utf32LEStreamReader(std::istream* inputStream);

			/**
				@brief Destructor.
			*/
			virtual ~Utf32LEStreamReader();

			/**
				@brief Reads one Unicode character.

				There are 4 bytes needed to read per one character
				from the UTF-32 (little endian) encoded stream.

				@param[out] result Variable that will receive the Unicode character.
				@return @b 1 if the character was read successfully.
					@b 0 if there are no more characters to read.
					@b -1 if character is not allowed in XML document.
					@b -2 if there was an error while reading character.
			*/
			virtual int ReadCharacter(char32_t& result);

			/**
				@brief Gets the pointer to the input stream.
			*/
			std::istream* GetInputStream() const;

			/**
				@brief Replaces the pointer to the input stream.
			*/
			void ResetInputStream(std::istream* inputStream);
		};

		/**
			@brief ISO-8859-1 characters reader from the input stream.
		*/
		class ISO_8859_1_StreamReader : public CharactersReader
		{
		private:
			std::istream* in;
		public:
			/**
				@brief Constructor.

				@param[in,out] inputStream Input stream from which
					an ISO-8859-1 characters will be extracted.
			*/
			ISO_8859_1_StreamReader(std::istream* inputStream);

			/**
				@brief Destructor.
			*/
			virtual ~ISO_8859_1_StreamReader();

			/**
				@brief Reads one ISO-8859-1 character.

				@param[out] result Variable that will receive the Unicode character.
				@return @b 1 if the character was read successfully.
					@b 0 if there are no more characters to read.
					@b -1 if character is not allowed in XML document.
					@b -2 if there was an error while reading character.
			*/
			virtual int ReadCharacter(char32_t& result);

			/**
				@brief Gets the pointer to the input stream.
			*/
			std::istream* GetInputStream() const;

			/**
				@brief Replaces the pointer to the input stream.
			*/
			void ResetInputStream(std::istream* inputStream);
		};

		/**
			@brief ISO-8859-2 characters reader from the input stream.
		*/
		class ISO_8859_2_StreamReader : public CharactersReader
		{
		private:
			std::istream* in;
		public:
			/**
				@brief Constructor.

				@param[in,out] inputStream Input stream from which
					an ISO-8859-2 characters will be extracted.
			*/
			ISO_8859_2_StreamReader(std::istream* inputStream);

			/**
				@brief Destructor.
			*/
			virtual ~ISO_8859_2_StreamReader();

			/**
				@brief Reads one ISO-8859-2 character.

				@param[out] result Variable that will receive the Unicode character.
				@return @b 1 if the character was read successfully.
					@b 0 if there are no more characters to read.
					@b -1 if character is not allowed in XML document.
					@b -2 if there was an error while reading character.
			*/
			virtual int ReadCharacter(char32_t& result);

			/**
				@brief Gets the pointer to the input stream.
			*/
			std::istream* GetInputStream() const;

			/**
				@brief Replaces the pointer to the input stream.
			*/
			void ResetInputStream(std::istream* inputStream);
		};

		/**
			@brief ISO-8859-3 characters reader from the input stream.
		*/
		class ISO_8859_3_StreamReader : public CharactersReader
		{
		private:
			std::istream* in;
		public:
			/**
				@brief Constructor.

				@param[in,out] inputStream Input stream from which
					an ISO-8859-3 characters will be extracted.
			*/
			ISO_8859_3_StreamReader(std::istream* inputStream);

			/**
				@brief Destructor.
			*/
			virtual ~ISO_8859_3_StreamReader();

			/**
				@brief Reads one ISO-8859-3 character.

				@param[out] result Variable that will receive the Unicode character.
				@return @b 1 if the character was read successfully.
					@b 0 if there are no more characters to read.
					@b -1 if character is not allowed in XML document.
					@b -2 if there was an error while reading character.
			*/
			virtual int ReadCharacter(char32_t& result);

			/**
				@brief Gets the pointer to the input stream.
			*/
			std::istream* GetInputStream() const;

			/**
				@brief Replaces the pointer to the input stream.
			*/
			void ResetInputStream(std::istream* inputStream);
		};

		/**
			@brief ISO-8859-4 characters reader from the input stream.
		*/
		class ISO_8859_4_StreamReader : public CharactersReader
		{
		private:
			std::istream* in;
		public:
			/**
				@brief Constructor.

				@param[in,out] inputStream Input stream from which
					an ISO-8859-4 characters will be extracted.
			*/
			ISO_8859_4_StreamReader(std::istream* inputStream);

			/**
				@brief Destructor.
			*/
			virtual ~ISO_8859_4_StreamReader();

			/**
				@brief Reads one ISO-8859-4 character.

				@param[out] result Variable that will receive the Unicode character.
				@return @b 1 if the character was read successfully.
					@b 0 if there are no more characters to read.
					@b -1 if character is not allowed in XML document.
					@b -2 if there was an error while reading character.
			*/
			virtual int ReadCharacter(char32_t& result);

			/**
				@brief Gets the pointer to the input stream.
			*/
			std::istream* GetInputStream() const;

			/**
				@brief Replaces the pointer to the input stream.
			*/
			void ResetInputStream(std::istream* inputStream);
		};

		/**
			@brief ISO-8859-5 characters reader from the input stream.
		*/
		class ISO_8859_5_StreamReader : public CharactersReader
		{
		private:
			std::istream* in;
		public:
			/**
				@brief Constructor.

				@param[in,out] inputStream Input stream from which
					an ISO-8859-5 characters will be extracted.
			*/
			ISO_8859_5_StreamReader(std::istream* inputStream);

			/**
				@brief Destructor.
			*/
			virtual ~ISO_8859_5_StreamReader();

			/**
				@brief Reads one ISO-8859-5 character.

				@param[out] result Variable that will receive the Unicode character.
				@return @b 1 if the character was read successfully.
					@b 0 if there are no more characters to read.
					@b -1 if character is not allowed in XML document.
					@b -2 if there was an error while reading character.
			*/
			virtual int ReadCharacter(char32_t& result);

			/**
				@brief Gets the pointer to the input stream.
			*/
			std::istream* GetInputStream() const;

			/**
				@brief Replaces the pointer to the input stream.
			*/
			void ResetInputStream(std::istream* inputStream);
		};

		/**
			@brief ISO-8859-6 characters reader from the input stream.
		*/
		class ISO_8859_6_StreamReader : public CharactersReader
		{
		private:
			std::istream* in;
		public:
			/**
				@brief Constructor.

				@param[in,out] inputStream Input stream from which
					an ISO-8859-6 characters will be extracted.
			*/
			ISO_8859_6_StreamReader(std::istream* inputStream);

			/**
				@brief Destructor.
			*/
			virtual ~ISO_8859_6_StreamReader();

			/**
				@brief Reads one ISO-8859-6 character.

				@param[out] result Variable that will receive the Unicode character.
				@return @b 1 if the character was read successfully.
					@b 0 if there are no more characters to read.
					@b -1 if character is not allowed in XML document.
					@b -2 if there was an error while reading character.
			*/
			virtual int ReadCharacter(char32_t& result);

			/**
				@brief Gets the pointer to the input stream.
			*/
			std::istream* GetInputStream() const;

			/**
				@brief Replaces the pointer to the input stream.
			*/
			void ResetInputStream(std::istream* inputStream);
		};

		/**
			@brief ISO-8859-7 characters reader from the input stream.
		*/
		class ISO_8859_7_StreamReader : public CharactersReader
		{
		private:
			std::istream* in;
		public:
			/**
				@brief Constructor.

				@param[in,out] inputStream Input stream from which
					an ISO-8859-7 characters will be extracted.
			*/
			ISO_8859_7_StreamReader(std::istream* inputStream);

			/**
				@brief Destructor.
			*/
			virtual ~ISO_8859_7_StreamReader();

			/**
				@brief Reads one ISO-8859-7 character.

				@param[out] result Variable that will receive the Unicode character.
				@return @b 1 if the character was read successfully.
					@b 0 if there are no more characters to read.
					@b -1 if character is not allowed in XML document.
					@b -2 if there was an error while reading character.
			*/
			virtual int ReadCharacter(char32_t& result);

			/**
				@brief Gets the pointer to the input stream.
			*/
			std::istream* GetInputStream() const;

			/**
				@brief Replaces the pointer to the input stream.
			*/
			void ResetInputStream(std::istream* inputStream);
		};

		/**
			@brief ISO-8859-8 characters reader from the input stream.
		*/
		class ISO_8859_8_StreamReader : public CharactersReader
		{
		private:
			std::istream* in;
		public:
			/**
				@brief Constructor.

				@param[in,out] inputStream Input stream from which
					an ISO-8859-8 characters will be extracted.
			*/
			ISO_8859_8_StreamReader(std::istream* inputStream);

			/**
				@brief Destructor.
			*/
			virtual ~ISO_8859_8_StreamReader();

			/**
				@brief Reads one ISO-8859-8 character.

				@param[out] result Variable that will receive the Unicode character.
				@return @b 1 if the character was read successfully.
					@b 0 if there are no more characters to read.
					@b -1 if character is not allowed in XML document.
					@b -2 if there was an error while reading character.
			*/
			virtual int ReadCharacter(char32_t& result);

			/**
				@brief Gets the pointer to the input stream.
			*/
			std::istream* GetInputStream() const;

			/**
				@brief Replaces the pointer to the input stream.
			*/
			void ResetInputStream(std::istream* inputStream);
		};

		/**
			@brief ISO-8859-9 characters reader from the input stream.
		*/
		class ISO_8859_9_StreamReader : public CharactersReader
		{
		private:
			std::istream* in;
		public:
			/**
				@brief Constructor.

				@param[in,out] inputStream Input stream from which
					an ISO-8859-9 characters will be extracted.
			*/
			ISO_8859_9_StreamReader(std::istream* inputStream);

			/**
				@brief Destructor.
			*/
			virtual ~ISO_8859_9_StreamReader();

			/**
				@brief Reads one ISO-8859-9 character.

				@param[out] result Variable that will receive the Unicode character.
				@return @b 1 if the character was read successfully.
					@b 0 if there are no more characters to read.
					@b -1 if character is not allowed in XML document.
					@b -2 if there was an error while reading character.
			*/
			virtual int ReadCharacter(char32_t& result);

			/**
				@brief Gets the pointer to the input stream.
			*/
			std::istream* GetInputStream() const;

			/**
				@brief Replaces the pointer to the input stream.
			*/
			void ResetInputStream(std::istream* inputStream);
		};

		/**
			@brief ISO-8859-10 characters reader from the input stream.
		*/
		class ISO_8859_10_StreamReader : public CharactersReader
		{
		private:
			std::istream* in;
		public:
			/**
				@brief Constructor.

				@param[in,out] inputStream Input stream from which
					an ISO-8859-10 characters will be extracted.
			*/
			ISO_8859_10_StreamReader(std::istream* inputStream);

			/**
				@brief Destructor.
			*/
			virtual ~ISO_8859_10_StreamReader();

			/**
				@brief Reads one ISO-8859-10 character.

				@param[out] result Variable that will receive the Unicode character.
				@return @b 1 if the character was read successfully.
					@b 0 if there are no more characters to read.
					@b -1 if character is not allowed in XML document.
					@b -2 if there was an error while reading character.
			*/
			virtual int ReadCharacter(char32_t& result);

			/**
				@brief Gets the pointer to the input stream.
			*/
			std::istream* GetInputStream() const;

			/**
				@brief Replaces the pointer to the input stream.
			*/
			void ResetInputStream(std::istream* inputStream);
		};

		/**
			@brief ISO-8859-13 characters reader from the input stream.
		*/
		class ISO_8859_13_StreamReader : public CharactersReader
		{
		private:
			std::istream* in;
		public:
			/**
				@brief Constructor.

				@param[in,out] inputStream Input stream from which
					an ISO-8859-13 characters will be extracted.
			*/
			ISO_8859_13_StreamReader(std::istream* inputStream);

			/**
				@brief Destructor.
			*/
			virtual ~ISO_8859_13_StreamReader();

			/**
				@brief Reads one ISO-8859-13 character.

				@param[out] result Variable that will receive the Unicode character.
				@return @b 1 if the character was read successfully.
					@b 0 if there are no more characters to read.
					@b -1 if character is not allowed in XML document.
					@b -2 if there was an error while reading character.
			*/
			virtual int ReadCharacter(char32_t& result);

			/**
				@brief Gets the pointer to the input stream.
			*/
			std::istream* GetInputStream() const;

			/**
				@brief Replaces the pointer to the input stream.
			*/
			void ResetInputStream(std::istream* inputStream);
		};

		/**
			@brief ISO-8859-14 characters reader from the input stream.
		*/
		class ISO_8859_14_StreamReader : public CharactersReader
		{
		private:
			std::istream* in;
		public:
			/**
				@brief Constructor.

				@param[in,out] inputStream Input stream from which
					an ISO-8859-14 characters will be extracted.
			*/
			ISO_8859_14_StreamReader(std::istream* inputStream);

			/**
				@brief Destructor.
			*/
			virtual ~ISO_8859_14_StreamReader();

			/**
				@brief Reads one ISO-8859-14 character.

				@param[out] result Variable that will receive the Unicode character.
				@return @b 1 if the character was read successfully.
					@b 0 if there are no more characters to read.
					@b -1 if character is not allowed in XML document.
					@b -2 if there was an error while reading character.
			*/
			virtual int ReadCharacter(char32_t& result);

			/**
				@brief Gets the pointer to the input stream.
			*/
			std::istream* GetInputStream() const;

			/**
				@brief Replaces the pointer to the input stream.
			*/
			void ResetInputStream(std::istream* inputStream);
		};

		/**
			@brief ISO-8859-15 characters reader from the input stream.
		*/
		class ISO_8859_15_StreamReader : public CharactersReader
		{
		private:
			std::istream* in;
		public:
			/**
				@brief Constructor.

				@param[in,out] inputStream Input stream from which
					an ISO-8859-15 characters will be extracted.
			*/
			ISO_8859_15_StreamReader(std::istream* inputStream);

			/**
				@brief Destructor.
			*/
			virtual ~ISO_8859_15_StreamReader();

			/**
				@brief Reads one ISO-8859-15 character.

				@param[out] result Variable that will receive the Unicode character.
				@return @b 1 if the character was read successfully.
					@b 0 if there are no more characters to read.
					@b -1 if character is not allowed in XML document.
					@b -2 if there was an error while reading character.
			*/
			virtual int ReadCharacter(char32_t& result);

			/**
				@brief Gets the pointer to the input stream.
			*/
			std::istream* GetInputStream() const;

			/**
				@brief Replaces the pointer to the input stream.
			*/
			void ResetInputStream(std::istream* inputStream);
		};

		/**
			@brief ISO-8859-16 characters reader from the input stream.
		*/
		class ISO_8859_16_StreamReader : public CharactersReader
		{
		private:
			std::istream* in;
		public:
			/**
				@brief Constructor.

				@param[in,out] inputStream Input stream from which
					an ISO-8859-16 characters will be extracted.
			*/
			ISO_8859_16_StreamReader(std::istream* inputStream);

			/**
				@brief Destructor.
			*/
			virtual ~ISO_8859_16_StreamReader();

			/**
				@brief Reads one ISO-8859-16 character.

				@param[out] result Variable that will receive the Unicode character.
				@return @b 1 if the character was read successfully.
					@b 0 if there are no more characters to read.
					@b -1 if character is not allowed in XML document.
					@b -2 if there was an error while reading character.
			*/
			virtual int ReadCharacter(char32_t& result);

			/**
				@brief Gets the pointer to the input stream.
			*/
			std::istream* GetInputStream() const;

			/**
				@brief Replaces the pointer to the input stream.
			*/
			void ResetInputStream(std::istream* inputStream);
		};

		/**
			@brief TIS-620 characters reader from the input stream.
		*/
		class TIS620StreamReader : public CharactersReader
		{
		private:
			std::istream* in;
		public:
			/**
				@brief Constructor.

				@param[in,out] inputStream Input stream from which
					a TIS-620 characters will be extracted.
			*/
			TIS620StreamReader(std::istream* inputStream);

			/**
				@brief Destructor.
			*/
			virtual ~TIS620StreamReader();

			/**
				@brief Reads one TIS-620 character.

				@param[out] result Variable that will receive the Unicode character.
				@return @b 1 if the character was read successfully.
					@b 0 if there are no more characters to read.
					@b -1 if character is not allowed in XML document.
					@b -2 if there was an error while reading character.
			*/
			virtual int ReadCharacter(char32_t& result);

			/**
				@brief Gets the pointer to the input stream.
			*/
			std::istream* GetInputStream() const;

			/**
				@brief Replaces the pointer to the input stream.
			*/
			void ResetInputStream(std::istream* inputStream);
		};

		/**
			@brief windows-874 characters reader from the input stream.
		*/
		class Windows874StreamReader : public CharactersReader
		{
		private:
			std::istream* in;
		public:
			/**
				@brief Constructor.

				@param[in,out] inputStream Input stream from which
					an windows-874 characters will be extracted.
			*/
			Windows874StreamReader(std::istream* inputStream);

			/**
				@brief Destructor.
			*/
			virtual ~Windows874StreamReader();

			/**
				@brief Reads one windows-874 character.

				@param[out] result Variable that will receive the Unicode character.
				@return @b 1 if the character was read successfully.
					@b 0 if there are no more characters to read.
					@b -1 if character is not allowed in XML document.
					@b -2 if there was an error while reading character.
			*/
			virtual int ReadCharacter(char32_t& result);

			/**
				@brief Gets the pointer to the input stream.
			*/
			std::istream* GetInputStream() const;

			/**
				@brief Replaces the pointer to the input stream.
			*/
			void ResetInputStream(std::istream* inputStream);
		};

		/**
			@brief windows-1250 characters reader from the input stream.
		*/
		class Windows1250StreamReader : public CharactersReader
		{
		private:
			std::istream* in;
		public:
			/**
				@brief Constructor.

				@param[in,out] inputStream Input stream from which
					an windows-1250 characters will be extracted.
			*/
			Windows1250StreamReader(std::istream* inputStream);

			/**
				@brief Destructor.
			*/
			virtual ~Windows1250StreamReader();

			/**
				@brief Reads one windows-1250 character.

				@param[out] result Variable that will receive the Unicode character.
				@return @b 1 if the character was read successfully.
					@b 0 if there are no more characters to read.
					@b -1 if character is not allowed in XML document.
					@b -2 if there was an error while reading character.
			*/
			virtual int ReadCharacter(char32_t& result);

			/**
				@brief Gets the pointer to the input stream.
			*/
			std::istream* GetInputStream() const;

			/**
				@brief Replaces the pointer to the input stream.
			*/
			void ResetInputStream(std::istream* inputStream);
		};

		/**
			@brief windows-1251 characters reader from the input stream.
		*/
		class Windows1251StreamReader : public CharactersReader
		{
		private:
			std::istream* in;
		public:
			/**
				@brief Constructor.

				@param[in,out] inputStream Input stream from which
					an windows-1251 characters will be extracted.
			*/
			Windows1251StreamReader(std::istream* inputStream);

			/**
				@brief Destructor.
			*/
			virtual ~Windows1251StreamReader();

			/**
				@brief Reads one windows-1251 character.

				@param[out] result Variable that will receive the Unicode character.
				@return @b 1 if the character was read successfully.
					@b 0 if there are no more characters to read.
					@b -1 if character is not allowed in XML document.
					@b -2 if there was an error while reading character.
			*/
			virtual int ReadCharacter(char32_t& result);

			/**
				@brief Gets the pointer to the input stream.
			*/
			std::istream* GetInputStream() const;

			/**
				@brief Replaces the pointer to the input stream.
			*/
			void ResetInputStream(std::istream* inputStream);
		};

		/**
			@brief windows-1252 characters reader from the input stream.
		*/
		class Windows1252StreamReader : public CharactersReader
		{
		private:
			std::istream* in;
		public:
			/**
				@brief Constructor.

				@param[in,out] inputStream Input stream from which
					an windows-1252 characters will be extracted.
			*/
			Windows1252StreamReader(std::istream* inputStream);

			/**
				@brief Destructor.
			*/
			virtual ~Windows1252StreamReader();

			/**
				@brief Reads one windows-1252 character.

				@param[out] result Variable that will receive the Unicode character.
				@return @b 1 if the character was read successfully.
					@b 0 if there are no more characters to read.
					@b -1 if character is not allowed in XML document.
					@b -2 if there was an error while reading character.
			*/
			virtual int ReadCharacter(char32_t& result);

			/**
				@brief Gets the pointer to the input stream.
			*/
			std::istream* GetInputStream() const;

			/**
				@brief Replaces the pointer to the input stream.
			*/
			void ResetInputStream(std::istream* inputStream);
		};

		/**
			@brief windows-1253 characters reader from the input stream.
		*/
		class Windows1253StreamReader : public CharactersReader
		{
		private:
			std::istream* in;
		public:
			/**
				@brief Constructor.

				@param[in,out] inputStream Input stream from which
					an windows-1253 characters will be extracted.
			*/
			Windows1253StreamReader(std::istream* inputStream);

			/**
				@brief Destructor.
			*/
			virtual ~Windows1253StreamReader();

			/**
				@brief Reads one windows-1253 character.

				@param[out] result Variable that will receive the Unicode character.
				@return @b 1 if the character was read successfully.
					@b 0 if there are no more characters to read.
					@b -1 if character is not allowed in XML document.
					@b -2 if there was an error while reading character.
			*/
			virtual int ReadCharacter(char32_t& result);

			/**
				@brief Gets the pointer to the input stream.
			*/
			std::istream* GetInputStream() const;

			/**
				@brief Replaces the pointer to the input stream.
			*/
			void ResetInputStream(std::istream* inputStream);
		};

		/**
			@brief windows-1254 characters reader from the input stream.
		*/
		class Windows1254StreamReader : public CharactersReader
		{
		private:
			std::istream* in;
		public:
			/**
				@brief Constructor.

				@param[in,out] inputStream Input stream from which
					an windows-1254 characters will be extracted.
			*/
			Windows1254StreamReader(std::istream* inputStream);

			/**
				@brief Destructor.
			*/
			virtual ~Windows1254StreamReader();

			/**
				@brief Reads one windows-1254 character.

				@param[out] result Variable that will receive the Unicode character.
				@return @b 1 if the character was read successfully.
					@b 0 if there are no more characters to read.
					@b -1 if character is not allowed in XML document.
					@b -2 if there was an error while reading character.
			*/
			virtual int ReadCharacter(char32_t& result);

			/**
				@brief Gets the pointer to the input stream.
			*/
			std::istream* GetInputStream() const;

			/**
				@brief Replaces the pointer to the input stream.
			*/
			void ResetInputStream(std::istream* inputStream);
		};

		/**
			@brief windows-1255 characters reader from the input stream.
		*/
		class Windows1255StreamReader : public CharactersReader
		{
		private:
			std::istream* in;
		public:
			/**
				@brief Constructor.

				@param[in,out] inputStream Input stream from which
					an windows-1255 characters will be extracted.
			*/
			Windows1255StreamReader(std::istream* inputStream);

			/**
				@brief Destructor.
			*/
			virtual ~Windows1255StreamReader();

			/**
				@brief Reads one windows-1255 character.

				@param[out] result Variable that will receive the Unicode character.
				@return @b 1 if the character was read successfully.
					@b 0 if there are no more characters to read.
					@b -1 if character is not allowed in XML document.
					@b -2 if there was an error while reading character.
			*/
			virtual int ReadCharacter(char32_t& result);

			/**
				@brief Gets the pointer to the input stream.
			*/
			std::istream* GetInputStream() const;

			/**
				@brief Replaces the pointer to the input stream.
			*/
			void ResetInputStream(std::istream* inputStream);
		};

		/**
			@brief windows-1256 characters reader from the input stream.
		*/
		class Windows1256StreamReader : public CharactersReader
		{
		private:
			std::istream* in;
		public:
			/**
				@brief Constructor.

				@param[in,out] inputStream Input stream from which
					an windows-1256 characters will be extracted.
			*/
			Windows1256StreamReader(std::istream* inputStream);

			/**
				@brief Destructor.
			*/
			virtual ~Windows1256StreamReader();

			/**
				@brief Reads one windows-1256 character.

				@param[out] result Variable that will receive the Unicode character.
				@return @b 1 if the character was read successfully.
					@b 0 if there are no more characters to read.
					@b -1 if character is not allowed in XML document.
					@b -2 if there was an error while reading character.
			*/
			virtual int ReadCharacter(char32_t& result);

			/**
				@brief Gets the pointer to the input stream.
			*/
			std::istream* GetInputStream() const;

			/**
				@brief Replaces the pointer to the input stream.
			*/
			void ResetInputStream(std::istream* inputStream);
		};

		/**
			@brief windows-1257 characters reader from the input stream.
		*/
		class Windows1257StreamReader : public CharactersReader
		{
		private:
			std::istream* in;
		public:
			/**
				@brief Constructor.

				@param[in,out] inputStream Input stream from which
					an windows-1257 characters will be extracted.
			*/
			Windows1257StreamReader(std::istream* inputStream);

			/**
				@brief Destructor.
			*/
			virtual ~Windows1257StreamReader();

			/**
				@brief Reads one windows-1257 character.

				@param[out] result Variable that will receive the Unicode character.
				@return @b 1 if the character was read successfully.
					@b 0 if there are no more characters to read.
					@b -1 if character is not allowed in XML document.
					@b -2 if there was an error while reading character.
			*/
			virtual int ReadCharacter(char32_t& result);

			/**
				@brief Gets the pointer to the input stream.
			*/
			std::istream* GetInputStream() const;

			/**
				@brief Replaces the pointer to the input stream.
			*/
			void ResetInputStream(std::istream* inputStream);
		};

		/**
			@brief windows-1258 characters reader from the input stream.
		*/
		class Windows1258StreamReader : public CharactersReader
		{
		private:
			std::istream* in;
		public:
			/**
				@brief Constructor.

				@param[in,out] inputStream Input stream from which
					an windows-1258 characters will be extracted.
			*/
			Windows1258StreamReader(std::istream* inputStream);

			/**
				@brief Destructor.
			*/
			virtual ~Windows1258StreamReader();

			/**
				@brief Reads one windows-1258 character.

				@param[out] result Variable that will receive the Unicode character.
				@return @b 1 if the character was read successfully.
					@b 0 if there are no more characters to read.
					@b -1 if character is not allowed in XML document.
					@b -2 if there was an error while reading character.
			*/
			virtual int ReadCharacter(char32_t& result);

			/**
				@brief Gets the pointer to the input stream.
			*/
			std::istream* GetInputStream() const;

			/**
				@brief Replaces the pointer to the input stream.
			*/
			void ResetInputStream(std::istream* inputStream);
		};

		//
		// CharactersReader implementation.
		//
		
		inline CharactersReader::~CharactersReader()
		{
		
		}

		inline bool CharactersReader::IsChar(char32_t codePoint)
		{
			if (codePoint < 0xD800)
			{
				return (codePoint >= 0x20 ||
					codePoint == 0x09 ||
					codePoint == 0x0A ||
					codePoint == 0x0D);
			}

			return ((codePoint >= 0xE000 && codePoint <= 0xFFFD) ||
				(codePoint >= 0x10000 && codePoint <= 0x10FFFF));
		}

		inline bool CharactersReader::IsWhiteSpace(char32_t codePoint)
		{
			return (codePoint == 0x20 || codePoint == 0x0A ||
				codePoint == 0x09 || codePoint == 0x0D);
		}

		inline bool CharactersReader::IsNameStartChar(char32_t codePoint)
		{
			static const char Allowed1Byte[256] =
			{
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
				0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
				1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1,
				0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
				1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
				1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1,
				1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
				1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1
			};
			if (codePoint <= 0xFF)
				return (Allowed1Byte[static_cast<unsigned char>(codePoint)] != 0);

			return (
				(codePoint <= 0x2FF) ||
				(codePoint >= 0x370 && codePoint <= 0x37D) ||
				(codePoint >= 0x37F && codePoint <= 0x1FFF) ||
				(codePoint >= 0x200C && codePoint <= 0x200D) ||
				(codePoint >= 0x2070 && codePoint <= 0x218F) ||
				(codePoint >= 0x2C00 && codePoint <= 0x2FEF) ||
				(codePoint >= 0x3001 && codePoint <= 0xD7FF) ||
				(codePoint >= 0xF900 && codePoint <= 0xFDCF) ||
				(codePoint >= 0xFDF0 && codePoint <= 0xFFFD) ||
				(codePoint >= 0x10000 && codePoint <= 0xEFFFF));
		}

		inline bool CharactersReader::IsNameChar(char32_t codePoint)
		{
			static const char Allowed1Byte[256] =
			{
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0,
				1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
				0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
				1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1,
				0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
				1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
				1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
				1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1,
				1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
				1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1
			};
			if (codePoint <= 0xFF)
				return (Allowed1Byte[static_cast<unsigned char>(codePoint)] != 0);

			return (
				(codePoint <= 0x2FF) ||
				(codePoint >= 0x300 && codePoint <= 0x37D) ||
				(codePoint >= 0x37F && codePoint <= 0x1FFF) ||
				(codePoint >= 0x200C && codePoint <= 0x200D) ||
				(codePoint >= 0x203F && codePoint <= 0x2040) ||
				(codePoint >= 0x2070 && codePoint <= 0x218F) ||
				(codePoint >= 0x2C00 && codePoint <= 0x2FEF) ||
				(codePoint >= 0x3001 && codePoint <= 0xD7FF) ||
				(codePoint >= 0xF900 && codePoint <= 0xFDCF) ||
				(codePoint >= 0xFDF0 && codePoint <= 0xFFFD) ||
				(codePoint >= 0x10000 && codePoint <= 0xEFFFF));
		}

		inline bool CharactersReader::IsEncNameStartChar(char32_t codePoint)
		{
			static const char Allowed[256] =
			{
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
				1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
				0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
				1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
			};
			return (codePoint <= 0xFF &&
				Allowed[static_cast<unsigned char>(codePoint)] != 0);
		}

		inline bool CharactersReader::IsEncNameChar(char32_t codePoint)
		{
			static const char Allowed[256] =
			{
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0,
				1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
				0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
				1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1,
				0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
				1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
			};
			return (codePoint <= 0xFF &&
				Allowed[static_cast<unsigned char>(codePoint)] != 0);
		}

		inline int CharactersReader::GetHexDigitValue(char32_t codePoint)
		{
			static const signed char HexValue[256] =
			{
				-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
				-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
				-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
				 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1,
				-1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
				-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
				-1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
				-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
				-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
				-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
				-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
				-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
				-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
				-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
				-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
				-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 
			};
			if (codePoint <= 0xFF)
				return static_cast<int>(HexValue[static_cast<unsigned char>(codePoint)]);
			return -1;
		}

		//
		// Utf8ReaderBase implementation.
		//
		
		inline unsigned int Utf8ReaderBase::GetBytesCount(unsigned char leadingByte)
		{
			// 0 means invalid leading byte.
			static const unsigned char BytesCount[256] =
			{
				0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
				1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
				1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
				1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
				1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
				1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
				2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
				3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
				4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
			};
			return BytesCount[leadingByte];
		}

		inline Utf8ReaderBase::~Utf8ReaderBase()
		{
		
		}

		//
		// Utf8StreamReader implementation.
		//

		inline Utf8StreamReader::Utf8StreamReader(std::istream* inputStream)
			: in(inputStream)
		{
		
		}

		inline Utf8StreamReader::~Utf8StreamReader()
		{
		
		}

		inline int Utf8StreamReader::ReadCharacter(char32_t& result)
		{
			if (in != nullptr)
			{
				// Try to read the first code unit (leading byte).
				int leadingByte = in->get();

				// If not failbit.
				if (leadingByte != std::char_traits<char>::eof())
				{
					unsigned int bytesCount = GetBytesCount(static_cast<unsigned char>(leadingByte));

					if (bytesCount == 1)
					{
						result = static_cast<char32_t>(static_cast<unsigned char>(leadingByte));
						return 1; // The character was read successfully.
					}

					if (bytesCount != 0) // 2, 3 or 4.
					{
						result = 0;
						char32_t continuationByte = 0;
						int codeUnit;
						for (unsigned int i = 1; i < bytesCount; ++i)
						{
							codeUnit = in->get();

							// If failbit...
							if (codeUnit == std::char_traits<char>::eof())
							{
								// Because of the end of the stream.
								if ((in->rdstate() & std::istream::eofbit) != 0)
								{
									// End of the stream, but the leading byte told us
									// there should be the continuation byte, so we treat
									// this like invalid character.
									result = static_cast<char32_t>(static_cast<unsigned char>(leadingByte));
									return -1; // Invalid character.
								}
								return -2; // Something terrible with the stream.
							}

							if ((codeUnit & 0xC0) != 0x80)
							{
								// Two most significant bits in continuation byte should be 1 followed by 0.
								result = static_cast<char32_t>(static_cast<unsigned char>(leadingByte));
								return -1; // Invalid character.
							}

							continuationByte = static_cast<char32_t>(static_cast<unsigned char>(codeUnit));
							continuationByte &= 0x3F; // Hide 2 most significant bits.
							continuationByte <<= ((bytesCount - i - 1) * 6);
							result |= continuationByte;
						}

						if (bytesCount == 2)
						{
							continuationByte = static_cast<char32_t>(static_cast<unsigned char>(leadingByte));
							continuationByte = (continuationByte & 0x1F) << 6;
							result |= continuationByte;
							if (result > 0x7F)
								return 1;
							return -1;
						}

						if (bytesCount == 3)
						{
							continuationByte = static_cast<char32_t>(static_cast<unsigned char>(leadingByte));
							continuationByte = (continuationByte & 0x0F) << 12;
							result |= continuationByte;
							if ((result > 0x07FF && result < 0xD800) ||
								(result > 0xDFFF && result <= 0xFFFD))
								return 1;
							return -1;
						}

						// bytesCount == 4.
						continuationByte = static_cast<char32_t>(static_cast<unsigned char>(leadingByte));
						continuationByte = (continuationByte & 0x07) << 18;
						result |= continuationByte;
						if (result > 0xFFFF && result <= 0x10FFFF)
							return 1;
						return -1;
					}

					result = static_cast<char32_t>(static_cast<unsigned char>(leadingByte));
					return -1; // Invalid character.
				}
				else if ((in->rdstate() & std::istream::eofbit) != 0)
				{
					return 0; // End of the stream.
				}
			}
			return -2; // Something terrible with the stream.
		}

		inline std::istream* Utf8StreamReader::GetInputStream() const
		{
			return in;
		}

		inline void Utf8StreamReader::ResetInputStream(std::istream* inputStream)
		{
			in = inputStream;
		}

		//
		// Utf8IteratorsReader implementation.
		//
		template <typename TInputIterator>
		inline Utf8IteratorsReader<TInputIterator>::Utf8IteratorsReader(
			TInputIterator first, TInputIterator last)
		{
			iterBegin = first;
			iterEnd = last;
		}

		template <typename TInputIterator>
		inline Utf8IteratorsReader<TInputIterator>::~Utf8IteratorsReader()
		{
		
		}

		template <typename TInputIterator>
		inline int Utf8IteratorsReader<TInputIterator>::ReadCharacter(char32_t& result)
		{
			if (iterBegin != iterEnd)
			{
				unsigned char leadingByte = static_cast<unsigned char>(*iterBegin);
				unsigned int bytesCount = GetBytesCount(leadingByte);

				++iterBegin;
				if (bytesCount == 1)
				{
					result = static_cast<char32_t>(leadingByte);
					return 1; // The character was read successfully.
				}

				if (bytesCount != 0) // 2, 3 or 4.
				{
					result = 0;
					char32_t continuationByte = 0;
					unsigned char codeUnit;
					for (unsigned int i = 1; i < bytesCount; ++i, ++iterBegin)
					{
						if (iterBegin == iterEnd)
						{
							// End of the sequence, but the leading byte told us
							// there should be the continuation byte, so we treat
							// this like invalid character.
							result = static_cast<char32_t>(leadingByte);
							return -1; // Invalid character.
						}
						codeUnit = static_cast<unsigned char>(*iterBegin);

						if ((codeUnit & 0xC0) != 0x80)
						{
							// Two most significant bits in continuation byte should be 1 followed by 0.
							result = static_cast<char32_t>(leadingByte);
							return -1; // Invalid character.
						}

						continuationByte = static_cast<char32_t>(codeUnit);
						continuationByte &= 0x3F; // Hide 2 most significant bits.
						continuationByte <<= ((bytesCount - i - 1) * 6);
						result |= continuationByte;
					}

					if (bytesCount == 2)
					{
						continuationByte = static_cast<char32_t>(leadingByte);
						continuationByte = (continuationByte & 0x1F) << 6;
						result |= continuationByte;
						if (result > 0x7F)
							return 1;
						return -1;
					}

					if (bytesCount == 3)
					{
						continuationByte = static_cast<char32_t>(leadingByte);
						continuationByte = (continuationByte & 0x0F) << 12;
						result |= continuationByte;
						if ((result > 0x07FF && result < 0xD800) ||
							(result > 0xDFFF && result <= 0xFFFD))
							return 1;
						return -1;
					}

					// bytesCount == 4.
					continuationByte = static_cast<char32_t>(leadingByte);
					continuationByte = (continuationByte & 0x07) << 18;
					result |= continuationByte;
					if (result > 0xFFFF && result <= 0x10FFFF)
						return 1;
					return -1;
				}

				result = static_cast<char32_t>(leadingByte);
				return -1; // Invalid character.
			}
			return 0;
		}

		template <typename TInputIterator>
		inline TInputIterator Utf8IteratorsReader<TInputIterator>::GetFirstIterator() const
		{
			return iterBegin;
		}

		template <typename TInputIterator>
		inline TInputIterator Utf8IteratorsReader<TInputIterator>::GetLastIterator() const
		{
			return iterEnd;
		}

		template <typename TInputIterator>
		inline void Utf8IteratorsReader<TInputIterator>::ResetIterators(
			TInputIterator first, TInputIterator last)
		{
			iterBegin = first;
			iterEnd = last;
		}

		//
		// Utf16BEStreamReader implementation.
		//

		inline Utf16BEStreamReader::Utf16BEStreamReader(std::istream* inputStream)
			: in(inputStream)
		{
		
		}

		inline Utf16BEStreamReader::~Utf16BEStreamReader()
		{
		
		}

		inline int Utf16BEStreamReader::ReadCharacter(char32_t& result)
		{
			if (in != nullptr)
			{
				// First byte.
				int oneByte = in->get();

				// If not failbit.
				if (oneByte != std::char_traits<char>::eof())
				{
					result = static_cast<char32_t>(static_cast<unsigned char>(oneByte));

					// Second byte.
					oneByte = in->get();

					if (oneByte == std::char_traits<char>::eof())
					{
						if ((in->rdstate() & std::istream::eofbit) != 0)
							return -1; // Invalid character.
						return -2; // Something terrible with the stream.
					}

					result <<= 8;
					result |= static_cast<char32_t>(static_cast<unsigned char>(oneByte));

					if (result < 0xD800)
					{
						if (result >= 0x20 ||
							result == 0x09 ||
							result == 0x0A ||
							result == 0x0D)
							return 1;
						return -1;
					}
					if (result > 0xDFFF)
					{
						if (result <= 0xFFFD)
							return 1;
						return -1;
					}

					// We have lead surrogate, so next code unit should be a trail surrogate.

					// Third byte (first of trail surrogate).
					oneByte = in->get();

					if (oneByte == std::char_traits<char>::eof())
					{
						if ((in->rdstate() & std::istream::eofbit) != 0)
							return -1; // Invalid character.
						return -2; // Something terrible with the stream.
					}

					char32_t trailSurrogate =
						static_cast<char32_t>(static_cast<unsigned char>(oneByte));

					// Fourth byte (second of trail surrogate).
					oneByte = in->get();

					if (oneByte == std::char_traits<char>::eof())
					{
						if ((in->rdstate() & std::istream::eofbit) != 0)
							return -1; // Invalid character.
						return -2; // Something terrible with the stream.
					}

					trailSurrogate <<= 8;
					trailSurrogate |= static_cast<char32_t>(static_cast<unsigned char>(oneByte));

					if (trailSurrogate >= 0xDC00 && trailSurrogate <= 0xDFFF)
					{
						result -= 0xD800;
						result <<= 10;
						result |= (trailSurrogate - 0xDC00);
						result += 0x10000;
						return 1;
					}
					return -1;
				}
				else if ((in->rdstate() & std::istream::eofbit) != 0)
				{
					return 0; // End of the stream.
				}
			}
			return -2; // Something terrible with the stream.
		}

		inline std::istream* Utf16BEStreamReader::GetInputStream() const
		{
			return in;
		}

		inline void Utf16BEStreamReader::ResetInputStream(std::istream* inputStream)
		{
			in = inputStream;
		}

		//
		// Utf16LEStreamReader implementation.
		//

		inline Utf16LEStreamReader::Utf16LEStreamReader(std::istream* inputStream)
			: in(inputStream)
		{
		
		}

		inline Utf16LEStreamReader::~Utf16LEStreamReader()
		{
		
		}

		inline int Utf16LEStreamReader::ReadCharacter(char32_t& result)
		{
			if (in != nullptr)
			{
				// First byte.
				int oneByte = in->get();

				// If not failbit.
				if (oneByte != std::char_traits<char>::eof())
				{
					result = static_cast<char32_t>(static_cast<unsigned char>(oneByte));

					// Second byte.
					oneByte = in->get();

					if (oneByte == std::char_traits<char>::eof())
					{
						if ((in->rdstate() & std::istream::eofbit) != 0)
							return -1; // Invalid character.
						return -2; // Something terrible with the stream.
					}

					result |= (static_cast<char32_t>(static_cast<unsigned char>(oneByte)) << 8);

					if (result < 0xD800)
					{
						if (result >= 0x20 ||
							result == 0x09 ||
							result == 0x0A ||
							result == 0x0D)
							return 1;
						return -1;
					}
					if (result > 0xDFFF)
					{
						if (result <= 0xFFFD)
							return 1;
						return -1;
					}

					// We have lead surrogate, so next code unit should be a trail surrogate.

					// Third byte (second of trail surrogate in little endian).
					oneByte = in->get();

					if (oneByte == std::char_traits<char>::eof())
					{
						if ((in->rdstate() & std::istream::eofbit) != 0)
							return -1; // Invalid character.
						return -2; // Something terrible with the stream.
					}

					char32_t trailSurrogate =
						static_cast<char32_t>(static_cast<unsigned char>(oneByte));

					// Fourth byte (second of trail surrogate).
					oneByte = in->get();

					if (oneByte == std::char_traits<char>::eof())
					{
						if ((in->rdstate() & std::istream::eofbit) != 0)
							return -1; // Invalid character.
						return -2; // Something terrible with the stream.
					}

					trailSurrogate |= (static_cast<char32_t>(static_cast<unsigned char>(oneByte)) << 8);

					if (trailSurrogate >= 0xDC00 && trailSurrogate <= 0xDFFF)
					{
						result -= 0xD800;
						result <<= 10;
						result |= (trailSurrogate - 0xDC00);
						result += 0x10000;
						return 1;
					}
					return -1;
				}
				else if ((in->rdstate() & std::istream::eofbit) != 0)
				{
					return 0; // End of the stream.
				}
			}
			return -2; // Something terrible with the stream.
		}

		inline std::istream* Utf16LEStreamReader::GetInputStream() const
		{
			return in;
		}

		inline void Utf16LEStreamReader::ResetInputStream(std::istream* inputStream)
		{
			in = inputStream;
		}

		//
		// Utf32BEStreamReader implementation.
		//

		inline Utf32BEStreamReader::Utf32BEStreamReader(std::istream* inputStream)
			: in(inputStream)
		{
		
		}

		inline Utf32BEStreamReader::~Utf32BEStreamReader()
		{
		
		}

		inline int Utf32BEStreamReader::ReadCharacter(char32_t& result)
		{
			if (in != nullptr)
			{
				char32_t codePoint, temp;

				// First byte.
				int oneByte = in->get();

				if (oneByte == std::char_traits<char>::eof())
				{
					if ((in->rdstate() & std::istream::eofbit) != 0)
						return 0; // End of the stream.
					return -2;
				}

				codePoint = static_cast<char32_t>(static_cast<unsigned char>(oneByte));
				codePoint <<= 24;

				// Second byte.
				oneByte = in->get();

				if (oneByte == std::char_traits<char>::eof())
				{
					if ((in->rdstate() & std::istream::eofbit) != 0)
						return -1;
					return -2;
				}

				temp = static_cast<char32_t>(static_cast<unsigned char>(oneByte));
				temp <<= 16;
				codePoint |= temp;

				// Third byte.
				oneByte = in->get();

				if (oneByte == std::char_traits<char>::eof())
				{
					if ((in->rdstate() & std::istream::eofbit) != 0)
						return -1;
					return -2;
				}

				temp = static_cast<char32_t>(static_cast<unsigned char>(oneByte));
				temp <<= 8;
				codePoint |= temp;

				// Fourth byte.
				oneByte = in->get();

				if (oneByte == std::char_traits<char>::eof())
				{
					if ((in->rdstate() & std::istream::eofbit) != 0)
						return -1;
					return -2;
				}

				codePoint |= static_cast<char32_t>(static_cast<unsigned char>(oneByte));
				result = codePoint;
				if (IsChar(result))
					return 1;
				return -1;
			}
			return -2; // Something terrible with the stream.
		}

		inline std::istream* Utf32BEStreamReader::GetInputStream() const
		{
			return in;
		}

		inline void Utf32BEStreamReader::ResetInputStream(std::istream* inputStream)
		{
			in = inputStream;
		}

		//
		// Utf32LEStreamReader implementation.
		//

		inline Utf32LEStreamReader::Utf32LEStreamReader(std::istream* inputStream)
			: in(inputStream)
		{
		
		}

		inline Utf32LEStreamReader::~Utf32LEStreamReader()
		{
		
		}

		inline int Utf32LEStreamReader::ReadCharacter(char32_t& result)
		{
			if (in != nullptr)
			{
				char32_t codePoint, temp;

				// First byte.
				int oneByte = in->get();

				if (oneByte == std::char_traits<char>::eof())
				{
					if ((in->rdstate() & std::istream::eofbit) != 0)
						return 0; // End of the stream.
					return -2;
				}

				codePoint = static_cast<char32_t>(static_cast<unsigned char>(oneByte));

				// Second byte.
				oneByte = in->get();

				if (oneByte == std::char_traits<char>::eof())
				{
					if ((in->rdstate() & std::istream::eofbit) != 0)
						return -1;
					return -2;
				}

				temp = static_cast<char32_t>(static_cast<unsigned char>(oneByte));
				temp <<= 8;
				codePoint |= temp;

				// Third byte.
				oneByte = in->get();

				if (oneByte == std::char_traits<char>::eof())
				{
					if ((in->rdstate() & std::istream::eofbit) != 0)
						return -1;
					return -2;
				}

				temp = static_cast<char32_t>(static_cast<unsigned char>(oneByte));
				temp <<= 16;
				codePoint |= temp;

				// Fourth byte.
				oneByte = in->get();

				if (oneByte == std::char_traits<char>::eof())
				{
					if ((in->rdstate() & std::istream::eofbit) != 0)
						return -1;
					return -2;
				}

				codePoint |= (static_cast<char32_t>(static_cast<unsigned char>(oneByte)) << 24);
				result = codePoint;
				if (IsChar(result))
					return 1;
				return -1;
			}
			return -2; // Something terrible with the stream.
		}

		inline std::istream* Utf32LEStreamReader::GetInputStream() const
		{
			return in;
		}

		inline void Utf32LEStreamReader::ResetInputStream(std::istream* inputStream)
		{
			in = inputStream;
		}

		//
		// ISO_8859_1_StreamReader implementation.
		//

		inline ISO_8859_1_StreamReader::ISO_8859_1_StreamReader(std::istream* inputStream)
			: in(inputStream)
		{
		
		}

		inline ISO_8859_1_StreamReader::~ISO_8859_1_StreamReader()
		{
		
		}

		inline int ISO_8859_1_StreamReader::ReadCharacter(char32_t& result)
		{
			static const unsigned char map[256] =
			{
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x0A, 0x00, 0x00, 0x0D, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
				0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
				0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
				0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
				0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
				0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
				0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
				0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
				0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
				0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
				0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
				0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
				0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
				0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF
			};

			if (in != nullptr)
			{
				int oneByte = in->get();

				if (oneByte == std::char_traits<char>::eof())
				{
					if ((in->rdstate() & std::istream::eofbit) != 0)
						return 0; // End of the stream.
					return -2;
				}

				result = static_cast<char32_t>(map[static_cast<unsigned char>(oneByte)]);
				if (result > 0)
					return 1;
				else
					return -1;
			}
			return -2;
		}

		inline std::istream* ISO_8859_1_StreamReader::GetInputStream() const
		{
			return in;
		}

		inline void ISO_8859_1_StreamReader::ResetInputStream(std::istream* inputStream)
		{
			in = inputStream;
		}

		//
		// ISO_8859_2_StreamReader implementation.
		//

		inline ISO_8859_2_StreamReader::ISO_8859_2_StreamReader(std::istream* inputStream)
			: in(inputStream)
		{
		
		}

		inline ISO_8859_2_StreamReader::~ISO_8859_2_StreamReader()
		{
		
		}

		inline int ISO_8859_2_StreamReader::ReadCharacter(char32_t& result)
		{
			static const char16_t map[256] =
			{
				0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0009, 0x000A, 0x0000, 0x0000, 0x000D, 0x0000, 0x0000,
				0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
				0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F,
				0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,
				0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
				0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F,
				0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
				0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x007E, 0x007F,
				0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008A, 0x008B, 0x008C, 0x008D, 0x008E, 0x008F,
				0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0098, 0x0099, 0x009A, 0x009B, 0x009C, 0x009D, 0x009E, 0x009F,
				0x00A0, 0x0104, 0x02D8, 0x0141, 0x00A4, 0x013D, 0x015A, 0x00A7, 0x00A8, 0x0160, 0x015E, 0x0164, 0x0179, 0x00AD, 0x017D, 0x017B,
				0x00B0, 0x0105, 0x02DB, 0x0142, 0x00B4, 0x013E, 0x015B, 0x02C7, 0x00B8, 0x0161, 0x015F, 0x0165, 0x017A, 0x02DD, 0x017E, 0x017C,
				0x0154, 0x00C1, 0x00C2, 0x0102, 0x00C4, 0x0139, 0x0106, 0x00C7, 0x010C, 0x00C9, 0x0118, 0x00CB, 0x011A, 0x00CD, 0x00CE, 0x010E,
				0x0110, 0x0143, 0x0147, 0x00D3, 0x00D4, 0x0150, 0x00D6, 0x00D7, 0x0158, 0x016E, 0x00DA, 0x0170, 0x00DC, 0x00DD, 0x0162, 0x00DF,
				0x0155, 0x00E1, 0x00E2, 0x0103, 0x00E4, 0x013A, 0x0107, 0x00E7, 0x010D, 0x00E9, 0x0119, 0x00EB, 0x011B, 0x00ED, 0x00EE, 0x010F,
				0x0111, 0x0144, 0x0148, 0x00F3, 0x00F4, 0x0151, 0x00F6, 0x00F7, 0x0159, 0x016F, 0x00FA, 0x0171, 0x00FC, 0x00FD, 0x0163, 0x02D9
			};

			if (in != nullptr)
			{
				int oneByte = in->get();

				if (oneByte == std::char_traits<char>::eof())
				{
					if ((in->rdstate() & std::istream::eofbit) != 0)
						return 0; // End of the stream.
					return -2;
				}

				result = static_cast<char32_t>(map[static_cast<unsigned char>(oneByte)]);
				if (result > 0)
					return 1;
				else
					return -1;
			}
			return -2;
		}

		inline std::istream* ISO_8859_2_StreamReader::GetInputStream() const
		{
			return in;
		}

		inline void ISO_8859_2_StreamReader::ResetInputStream(std::istream* inputStream)
		{
			in = inputStream;
		}

		//
		// ISO_8859_3_StreamReader implementation.
		//

		inline ISO_8859_3_StreamReader::ISO_8859_3_StreamReader(std::istream* inputStream)
			: in(inputStream)
		{
		
		}

		inline ISO_8859_3_StreamReader::~ISO_8859_3_StreamReader()
		{
		
		}

		inline int ISO_8859_3_StreamReader::ReadCharacter(char32_t& result)
		{
			static const char16_t map[256] =
			{
				0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0009, 0x000A, 0x0000, 0x0000, 0x000D, 0x0000, 0x0000,
				0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
				0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F,
				0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,
				0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
				0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F,
				0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
				0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x007E, 0x007F,
				0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008A, 0x008B, 0x008C, 0x008D, 0x008E, 0x008F,
				0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0098, 0x0099, 0x009A, 0x009B, 0x009C, 0x009D, 0x009E, 0x009F,
				0x00A0, 0x0126, 0x02D8, 0x00A3, 0x00A4, 0x0000, 0x0124, 0x00A7, 0x00A8, 0x0130, 0x015E, 0x011E, 0x0134, 0x00AD, 0x0000, 0x017B,
				0x00B0, 0x0127, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x0125, 0x00B7, 0x00B8, 0x0131, 0x015F, 0x011F, 0x0135, 0x00BD, 0x0000, 0x017C,
				0x00C0, 0x00C1, 0x00C2, 0x0000, 0x00C4, 0x010A, 0x0108, 0x00C7, 0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
				0x0000, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x0120, 0x00D6, 0x00D7, 0x011C, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x016C, 0x015C, 0x00DF,
				0x00E0, 0x00E1, 0x00E2, 0x0000, 0x00E4, 0x010B, 0x0109, 0x00E7, 0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
				0x0000, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x0121, 0x00F6, 0x00F7, 0x011D, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x016D, 0x015D, 0x02D9,
			};

			if (in != nullptr)
			{
				int oneByte = in->get();

				if (oneByte == std::char_traits<char>::eof())
				{
					if ((in->rdstate() & std::istream::eofbit) != 0)
						return 0; // End of the stream.
					return -2;
				}

				result = static_cast<char32_t>(map[static_cast<unsigned char>(oneByte)]);
				if (result > 0)
					return 1;
				else
					return -1;
			}
			return -2;
		}

		inline std::istream* ISO_8859_3_StreamReader::GetInputStream() const
		{
			return in;
		}

		inline void ISO_8859_3_StreamReader::ResetInputStream(std::istream* inputStream)
		{
			in = inputStream;
		}

		//
		// ISO_8859_4_StreamReader implementation.
		//

		inline ISO_8859_4_StreamReader::ISO_8859_4_StreamReader(std::istream* inputStream)
			: in(inputStream)
		{
		
		}

		inline ISO_8859_4_StreamReader::~ISO_8859_4_StreamReader()
		{
		
		}

		inline int ISO_8859_4_StreamReader::ReadCharacter(char32_t& result)
		{
			static const char16_t map[256] =
			{
				0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0009, 0x000A, 0x0000, 0x0000, 0x000D, 0x0000, 0x0000,
				0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
				0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F,
				0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,
				0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
				0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F,
				0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
				0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x007E, 0x007F,
				0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008A, 0x008B, 0x008C, 0x008D, 0x008E, 0x008F,
				0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0098, 0x0099, 0x009A, 0x009B, 0x009C, 0x009D, 0x009E, 0x009F,
				0x00A0, 0x0104, 0x0138, 0x0156, 0x00A4, 0x0128, 0x013B, 0x00A7, 0x00A8, 0x0160, 0x0112, 0x0122, 0x0166, 0x00AD, 0x017D, 0x00AF,
				0x00B0, 0x0105, 0x02DB, 0x0157, 0x00B4, 0x0129, 0x013C, 0x02C7, 0x00B8, 0x0161, 0x0113, 0x0123, 0x0167, 0x014A, 0x017E, 0x014B,
				0x0100, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x012E, 0x010C, 0x00C9, 0x0118, 0x00CB, 0x0116, 0x00CD, 0x00CE, 0x012A,
				0x0110, 0x0145, 0x014C, 0x0136, 0x00D4, 0x00D5, 0x00D6, 0x00D7, 0x00D8, 0x0172, 0x00DA, 0x00DB, 0x00DC, 0x0168, 0x016A, 0x00DF,
				0x0101, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x012F, 0x010D, 0x00E9, 0x0119, 0x00EB, 0x0117, 0x00ED, 0x00EE, 0x012B,
				0x0111, 0x0146, 0x014D, 0x0137, 0x00F4, 0x00F5, 0x00F6, 0x00F7, 0x00F8, 0x0173, 0x00FA, 0x00FB, 0x00FC, 0x0169, 0x016B, 0x02D9,
			};

			if (in != nullptr)
			{
				int oneByte = in->get();

				if (oneByte == std::char_traits<char>::eof())
				{
					if ((in->rdstate() & std::istream::eofbit) != 0)
						return 0; // End of the stream.
					return -2;
				}

				result = static_cast<char32_t>(map[static_cast<unsigned char>(oneByte)]);
				if (result > 0)
					return 1;
				else
					return -1;
			}
			return -2;
		}

		inline std::istream* ISO_8859_4_StreamReader::GetInputStream() const
		{
			return in;
		}

		inline void ISO_8859_4_StreamReader::ResetInputStream(std::istream* inputStream)
		{
			in = inputStream;
		}

		//
		// ISO_8859_5_StreamReader implementation.
		//

		inline ISO_8859_5_StreamReader::ISO_8859_5_StreamReader(std::istream* inputStream)
			: in(inputStream)
		{
		
		}

		inline ISO_8859_5_StreamReader::~ISO_8859_5_StreamReader()
		{
		
		}

		inline int ISO_8859_5_StreamReader::ReadCharacter(char32_t& result)
		{
			static const char16_t map[256] =
			{
				0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0009, 0x000A, 0x0000, 0x0000, 0x000D, 0x0000, 0x0000,
				0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
				0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F,
				0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,
				0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
				0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F,
				0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
				0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x007E, 0x007F,
				0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008A, 0x008B, 0x008C, 0x008D, 0x008E, 0x008F,
				0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0098, 0x0099, 0x009A, 0x009B, 0x009C, 0x009D, 0x009E, 0x009F,
				0x00A0, 0x0401, 0x0402, 0x0403, 0x0404, 0x0405, 0x0406, 0x0407, 0x0408, 0x0409, 0x040A, 0x040B, 0x040C, 0x00AD, 0x040E, 0x040F,
				0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0416, 0x0417, 0x0418, 0x0419, 0x041A, 0x041B, 0x041C, 0x041D, 0x041E, 0x041F,
				0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427, 0x0428, 0x0429, 0x042A, 0x042B, 0x042C, 0x042D, 0x042E, 0x042F,
				0x0430, 0x0431, 0x0432, 0x0433, 0x0434, 0x0435, 0x0436, 0x0437, 0x0438, 0x0439, 0x043A, 0x043B, 0x043C, 0x043D, 0x043E, 0x043F,
				0x0440, 0x0441, 0x0442, 0x0443, 0x0444, 0x0445, 0x0446, 0x0447, 0x0448, 0x0449, 0x044A, 0x044B, 0x044C, 0x044D, 0x044E, 0x044F,
				0x2116, 0x0451, 0x0452, 0x0453, 0x0454, 0x0455, 0x0456, 0x0457, 0x0458, 0x0459, 0x045A, 0x045B, 0x045C, 0x00A7, 0x045E, 0x045F,
			};

			if (in != nullptr)
			{
				int oneByte = in->get();

				if (oneByte == std::char_traits<char>::eof())
				{
					if ((in->rdstate() & std::istream::eofbit) != 0)
						return 0; // End of the stream.
					return -2;
				}

				result = static_cast<char32_t>(map[static_cast<unsigned char>(oneByte)]);
				if (result > 0)
					return 1;
				else
					return -1;
			}
			return -2;
		}

		inline std::istream* ISO_8859_5_StreamReader::GetInputStream() const
		{
			return in;
		}

		inline void ISO_8859_5_StreamReader::ResetInputStream(std::istream* inputStream)
		{
			in = inputStream;
		}

		//
		// ISO_8859_6_StreamReader implementation.
		//

		inline ISO_8859_6_StreamReader::ISO_8859_6_StreamReader(std::istream* inputStream)
			: in(inputStream)
		{
		
		}

		inline ISO_8859_6_StreamReader::~ISO_8859_6_StreamReader()
		{
		
		}

		inline int ISO_8859_6_StreamReader::ReadCharacter(char32_t& result)
		{
			static const char16_t map[256] =
			{
				0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0009, 0x000A, 0x0000, 0x0000, 0x000D, 0x0000, 0x0000,
				0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
				0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F,
				0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,
				0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
				0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F,
				0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
				0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x007E, 0x007F,
				0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008A, 0x008B, 0x008C, 0x008D, 0x008E, 0x008F,
				0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0098, 0x0099, 0x009A, 0x009B, 0x009C, 0x009D, 0x009E, 0x009F,
				0x00A0, 0x0000, 0x0000, 0x0000, 0x00A4, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x060C, 0x00AD, 0x0000, 0x0000,
				0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x061B, 0x0000, 0x0000, 0x0000, 0x061F,
				0x0000, 0x0621, 0x0622, 0x0623, 0x0624, 0x0625, 0x0626, 0x0627, 0x0628, 0x0629, 0x062A, 0x062B, 0x062C, 0x062D, 0x062E, 0x062F,
				0x0630, 0x0631, 0x0632, 0x0633, 0x0634, 0x0635, 0x0636, 0x0637, 0x0638, 0x0639, 0x063A, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
				0x0640, 0x0641, 0x0642, 0x0643, 0x0644, 0x0645, 0x0646, 0x0647, 0x0648, 0x0649, 0x064A, 0x064B, 0x064C, 0x064D, 0x064E, 0x064F,
				0x0650, 0x0651, 0x0652, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			};

			if (in != nullptr)
			{
				int oneByte = in->get();

				if (oneByte == std::char_traits<char>::eof())
				{
					if ((in->rdstate() & std::istream::eofbit) != 0)
						return 0; // End of the stream.
					return -2;
				}

				result = static_cast<char32_t>(map[static_cast<unsigned char>(oneByte)]);
				if (result > 0)
					return 1;
				else
					return -1;
			}
			return -2;
		}

		inline std::istream* ISO_8859_6_StreamReader::GetInputStream() const
		{
			return in;
		}

		inline void ISO_8859_6_StreamReader::ResetInputStream(std::istream* inputStream)
		{
			in = inputStream;
		}

		//
		// ISO_8859_7_StreamReader implementation.
		//

		inline ISO_8859_7_StreamReader::ISO_8859_7_StreamReader(std::istream* inputStream)
			: in(inputStream)
		{
		
		}

		inline ISO_8859_7_StreamReader::~ISO_8859_7_StreamReader()
		{
		
		}

		inline int ISO_8859_7_StreamReader::ReadCharacter(char32_t& result)
		{
			static const char16_t map[256] =
			{
				0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0009, 0x000A, 0x0000, 0x0000, 0x000D, 0x0000, 0x0000,
				0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
				0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F,
				0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,
				0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
				0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F,
				0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
				0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x007E, 0x007F,
				0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008A, 0x008B, 0x008C, 0x008D, 0x008E, 0x008F,
				0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0098, 0x0099, 0x009A, 0x009B, 0x009C, 0x009D, 0x009E, 0x009F,
				0x00A0, 0x2018, 0x2019, 0x00A3, 0x20AC, 0x20AF, 0x00A6, 0x00A7, 0x00A8, 0x00A9, 0x037A, 0x00AB, 0x00AC, 0x00AD, 0x0000, 0x2015,
				0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x0384, 0x0385, 0x0386, 0x00B7, 0x0388, 0x0389, 0x038A, 0x00BB, 0x038C, 0x00BD, 0x038E, 0x038F,
				0x0390, 0x0391, 0x0392, 0x0393, 0x0394, 0x0395, 0x0396, 0x0397, 0x0398, 0x0399, 0x039A, 0x039B, 0x039C, 0x039D, 0x039E, 0x039F,
				0x03A0, 0x03A1, 0x0000, 0x03A3, 0x03A4, 0x03A5, 0x03A6, 0x03A7, 0x03A8, 0x03A9, 0x03AA, 0x03AB, 0x03AC, 0x03AD, 0x03AE, 0x03AF,
				0x03B0, 0x03B1, 0x03B2, 0x03B3, 0x03B4, 0x03B5, 0x03B6, 0x03B7, 0x03B8, 0x03B9, 0x03BA, 0x03BB, 0x03BC, 0x03BD, 0x03BE, 0x03BF,
				0x03C0, 0x03C1, 0x03C2, 0x03C3, 0x03C4, 0x03C5, 0x03C6, 0x03C7, 0x03C8, 0x03C9, 0x03CA, 0x03CB, 0x03CC, 0x03CD, 0x03CE, 0x0000,
			};

			if (in != nullptr)
			{
				int oneByte = in->get();

				if (oneByte == std::char_traits<char>::eof())
				{
					if ((in->rdstate() & std::istream::eofbit) != 0)
						return 0; // End of the stream.
					return -2;
				}

				result = static_cast<char32_t>(map[static_cast<unsigned char>(oneByte)]);
				if (result > 0)
					return 1;
				else
					return -1;
			}
			return -2;
		}

		inline std::istream* ISO_8859_7_StreamReader::GetInputStream() const
		{
			return in;
		}

		inline void ISO_8859_7_StreamReader::ResetInputStream(std::istream* inputStream)
		{
			in = inputStream;
		}

		//
		// ISO_8859_8_StreamReader implementation.
		//

		inline ISO_8859_8_StreamReader::ISO_8859_8_StreamReader(std::istream* inputStream)
			: in(inputStream)
		{
		
		}

		inline ISO_8859_8_StreamReader::~ISO_8859_8_StreamReader()
		{
		
		}

		inline int ISO_8859_8_StreamReader::ReadCharacter(char32_t& result)
		{
			static const char16_t map[256] =
			{
				0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0009, 0x000A, 0x0000, 0x0000, 0x000D, 0x0000, 0x0000,
				0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
				0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F,
				0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,
				0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
				0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F,
				0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
				0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x007E, 0x007F,
				0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008A, 0x008B, 0x008C, 0x008D, 0x008E, 0x008F,
				0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0098, 0x0099, 0x009A, 0x009B, 0x009C, 0x009D, 0x009E, 0x009F,
				0x00A0, 0x0000, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7, 0x00A8, 0x00A9, 0x00D7, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
				0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7, 0x00B8, 0x00B9, 0x00F7, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x0000,
				0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
				0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x2017,
				0x05D0, 0x05D1, 0x05D2, 0x05D3, 0x05D4, 0x05D5, 0x05D6, 0x05D7, 0x05D8, 0x05D9, 0x05DA, 0x05DB, 0x05DC, 0x05DD, 0x05DE, 0x05DF,
				0x05E0, 0x05E1, 0x05E2, 0x05E3, 0x05E4, 0x05E5, 0x05E6, 0x05E7, 0x05E8, 0x05E9, 0x05EA, 0x0000, 0x0000, 0x200E, 0x200F, 0x0000,
			};

			if (in != nullptr)
			{
				int oneByte = in->get();

				if (oneByte == std::char_traits<char>::eof())
				{
					if ((in->rdstate() & std::istream::eofbit) != 0)
						return 0; // End of the stream.
					return -2;
				}

				result = static_cast<char32_t>(map[static_cast<unsigned char>(oneByte)]);
				if (result > 0)
					return 1;
				else
					return -1;
			}
			return -2;
		}

		inline std::istream* ISO_8859_8_StreamReader::GetInputStream() const
		{
			return in;
		}

		inline void ISO_8859_8_StreamReader::ResetInputStream(std::istream* inputStream)
		{
			in = inputStream;
		}

		//
		// ISO_8859_9_StreamReader implementation.
		//

		inline ISO_8859_9_StreamReader::ISO_8859_9_StreamReader(std::istream* inputStream)
			: in(inputStream)
		{
		
		}

		inline ISO_8859_9_StreamReader::~ISO_8859_9_StreamReader()
		{
		
		}

		inline int ISO_8859_9_StreamReader::ReadCharacter(char32_t& result)
		{
			static const char16_t map[256] =
			{
				0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0009, 0x000A, 0x0000, 0x0000, 0x000D, 0x0000, 0x0000,
				0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
				0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F,
				0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,
				0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
				0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F,
				0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
				0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x007E, 0x007F,
				0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008A, 0x008B, 0x008C, 0x008D, 0x008E, 0x008F,
				0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0098, 0x0099, 0x009A, 0x009B, 0x009C, 0x009D, 0x009E, 0x009F,
				0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7, 0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
				0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7, 0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF,
				0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7, 0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
				0x011E, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00D7, 0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x0130, 0x015E, 0x00DF,
				0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7, 0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
				0x011F, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7, 0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x0131, 0x015F, 0x00FF,
			};

			if (in != nullptr)
			{
				int oneByte = in->get();

				if (oneByte == std::char_traits<char>::eof())
				{
					if ((in->rdstate() & std::istream::eofbit) != 0)
						return 0; // End of the stream.
					return -2;
				}

				result = static_cast<char32_t>(map[static_cast<unsigned char>(oneByte)]);
				if (result > 0)
					return 1;
				else
					return -1;
			}
			return -2;
		}

		inline std::istream* ISO_8859_9_StreamReader::GetInputStream() const
		{
			return in;
		}

		inline void ISO_8859_9_StreamReader::ResetInputStream(std::istream* inputStream)
		{
			in = inputStream;
		}

		//
		// ISO_8859_10_StreamReader implementation.
		//

		inline ISO_8859_10_StreamReader::ISO_8859_10_StreamReader(std::istream* inputStream)
			: in(inputStream)
		{
		
		}

		inline ISO_8859_10_StreamReader::~ISO_8859_10_StreamReader()
		{
		
		}

		inline int ISO_8859_10_StreamReader::ReadCharacter(char32_t& result)
		{
			static const char16_t map[256] =
			{
				0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0009, 0x000A, 0x0000, 0x0000, 0x000D, 0x0000, 0x0000,
				0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
				0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F,
				0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,
				0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
				0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F,
				0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
				0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x007E, 0x007F,
				0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008A, 0x008B, 0x008C, 0x008D, 0x008E, 0x008F,
				0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0098, 0x0099, 0x009A, 0x009B, 0x009C, 0x009D, 0x009E, 0x009F,
				0x00A0, 0x0104, 0x0112, 0x0122, 0x012A, 0x0128, 0x0136, 0x00A7, 0x013B, 0x0110, 0x0160, 0x0166, 0x017D, 0x00AD, 0x016A, 0x014A,
				0x00B0, 0x0105, 0x0113, 0x0123, 0x012B, 0x0129, 0x0137, 0x00B7, 0x013C, 0x0111, 0x0161, 0x0167, 0x017E, 0x2015, 0x016B, 0x014B,
				0x0100, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x012E, 0x010C, 0x00C9, 0x0118, 0x00CB, 0x0116, 0x00CD, 0x00CE, 0x00CF,
				0x00D0, 0x0145, 0x014C, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x0168, 0x00D8, 0x0172, 0x00DA, 0x00DB, 0x00DC, 0x00DD, 0x00DE, 0x00DF,
				0x0101, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x012F, 0x010D, 0x00E9, 0x0119, 0x00EB, 0x0117, 0x00ED, 0x00EE, 0x00EF,
				0x00F0, 0x0146, 0x014D, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x0169, 0x00F8, 0x0173, 0x00FA, 0x00FB, 0x00FC, 0x00FD, 0x00FE, 0x0138,
			};

			if (in != nullptr)
			{
				int oneByte = in->get();

				if (oneByte == std::char_traits<char>::eof())
				{
					if ((in->rdstate() & std::istream::eofbit) != 0)
						return 0; // End of the stream.
					return -2;
				}

				result = static_cast<char32_t>(map[static_cast<unsigned char>(oneByte)]);
				if (result > 0)
					return 1;
				else
					return -1;
			}
			return -2;
		}

		inline std::istream* ISO_8859_10_StreamReader::GetInputStream() const
		{
			return in;
		}

		inline void ISO_8859_10_StreamReader::ResetInputStream(std::istream* inputStream)
		{
			in = inputStream;
		}

		//
		// ISO_8859_13_StreamReader implementation.
		//

		inline ISO_8859_13_StreamReader::ISO_8859_13_StreamReader(std::istream* inputStream)
			: in(inputStream)
		{
		
		}

		inline ISO_8859_13_StreamReader::~ISO_8859_13_StreamReader()
		{
		
		}

		inline int ISO_8859_13_StreamReader::ReadCharacter(char32_t& result)
		{
			static const char16_t map[256] =
			{
				0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0009, 0x000A, 0x0000, 0x0000, 0x000D, 0x0000, 0x0000,
				0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
				0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F,
				0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,
				0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
				0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F,
				0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
				0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x007E, 0x007F,
				0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008A, 0x008B, 0x008C, 0x008D, 0x008E, 0x008F,
				0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0098, 0x0099, 0x009A, 0x009B, 0x009C, 0x009D, 0x009E, 0x009F,
				0x00A0, 0x201D, 0x00A2, 0x00A3, 0x00A4, 0x201E, 0x00A6, 0x00A7, 0x00D8, 0x00A9, 0x0156, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00C6,
				0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x201C, 0x00B5, 0x00B6, 0x00B7, 0x00F8, 0x00B9, 0x0157, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00E6,
				0x0104, 0x012E, 0x0100, 0x0106, 0x00C4, 0x00C5, 0x0118, 0x0112, 0x010C, 0x00C9, 0x0179, 0x0116, 0x0122, 0x0136, 0x012A, 0x013B,
				0x0160, 0x0143, 0x0145, 0x00D3, 0x014C, 0x00D5, 0x00D6, 0x00D7, 0x0172, 0x0141, 0x015A, 0x016A, 0x00DC, 0x017B, 0x017D, 0x00DF,
				0x0105, 0x012F, 0x0101, 0x0107, 0x00E4, 0x00E5, 0x0119, 0x0113, 0x010D, 0x00E9, 0x017A, 0x0117, 0x0123, 0x0137, 0x012B, 0x013C,
				0x0161, 0x0144, 0x0146, 0x00F3, 0x014D, 0x00F5, 0x00F6, 0x00F7, 0x0173, 0x0142, 0x015B, 0x016B, 0x00FC, 0x017C, 0x017E, 0x2019,
			};

			if (in != nullptr)
			{
				int oneByte = in->get();

				if (oneByte == std::char_traits<char>::eof())
				{
					if ((in->rdstate() & std::istream::eofbit) != 0)
						return 0; // End of the stream.
					return -2;
				}

				result = static_cast<char32_t>(map[static_cast<unsigned char>(oneByte)]);
				if (result > 0)
					return 1;
				else
					return -1;
			}
			return -2;
		}

		inline std::istream* ISO_8859_13_StreamReader::GetInputStream() const
		{
			return in;
		}

		inline void ISO_8859_13_StreamReader::ResetInputStream(std::istream* inputStream)
		{
			in = inputStream;
		}

		//
		// ISO_8859_14_StreamReader implementation.
		//

		inline ISO_8859_14_StreamReader::ISO_8859_14_StreamReader(std::istream* inputStream)
			: in(inputStream)
		{
		
		}

		inline ISO_8859_14_StreamReader::~ISO_8859_14_StreamReader()
		{
		
		}

		inline int ISO_8859_14_StreamReader::ReadCharacter(char32_t& result)
		{
			static const char16_t map[256] =
			{
				0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0009, 0x000A, 0x0000, 0x0000, 0x000D, 0x0000, 0x0000,
				0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
				0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F,
				0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,
				0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
				0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F,
				0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
				0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x007E, 0x007F,
				0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008A, 0x008B, 0x008C, 0x008D, 0x008E, 0x008F,
				0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0098, 0x0099, 0x009A, 0x009B, 0x009C, 0x009D, 0x009E, 0x009F,
				0x00A0, 0x1E02, 0x1E03, 0x00A3, 0x010A, 0x010B, 0x1E0A, 0x00A7, 0x1E80, 0x00A9, 0x1E82, 0x1E0B, 0x1EF2, 0x00AD, 0x00AE, 0x0178,
				0x1E1E, 0x1E1F, 0x0120, 0x0121, 0x1E40, 0x1E41, 0x00B6, 0x1E56, 0x1E81, 0x1E57, 0x1E83, 0x1E60, 0x1EF3, 0x1E84, 0x1E85, 0x1E61,
				0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7, 0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
				0x0174, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x1E6A, 0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x00DD, 0x0176, 0x00DF,
				0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7, 0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
				0x0175, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x1E6B, 0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x00FD, 0x0177, 0x00FF,
			};

			if (in != nullptr)
			{
				int oneByte = in->get();

				if (oneByte == std::char_traits<char>::eof())
				{
					if ((in->rdstate() & std::istream::eofbit) != 0)
						return 0; // End of the stream.
					return -2;
				}

				result = static_cast<char32_t>(map[static_cast<unsigned char>(oneByte)]);
				if (result > 0)
					return 1;
				else
					return -1;
			}
			return -2;
		}

		inline std::istream* ISO_8859_14_StreamReader::GetInputStream() const
		{
			return in;
		}

		inline void ISO_8859_14_StreamReader::ResetInputStream(std::istream* inputStream)
		{
			in = inputStream;
		}

		//
		// ISO_8859_15_StreamReader implementation.
		//

		inline ISO_8859_15_StreamReader::ISO_8859_15_StreamReader(std::istream* inputStream)
			: in(inputStream)
		{
		
		}

		inline ISO_8859_15_StreamReader::~ISO_8859_15_StreamReader()
		{
		
		}

		inline int ISO_8859_15_StreamReader::ReadCharacter(char32_t& result)
		{
			static const char16_t map[256] =
			{
				0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0009, 0x000A, 0x0000, 0x0000, 0x000D, 0x0000, 0x0000,
				0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
				0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F,
				0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,
				0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
				0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F,
				0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
				0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x007E, 0x007F,
				0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008A, 0x008B, 0x008C, 0x008D, 0x008E, 0x008F,
				0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0098, 0x0099, 0x009A, 0x009B, 0x009C, 0x009D, 0x009E, 0x009F,
				0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x20AC, 0x00A5, 0x0160, 0x00A7, 0x0161, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
				0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x017D, 0x00B5, 0x00B6, 0x00B7, 0x017E, 0x00B9, 0x00BA, 0x00BB, 0x0152, 0x0153, 0x0178, 0x00BF,
				0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7, 0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
				0x00D0, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00D7, 0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x00DD, 0x00DE, 0x00DF,
				0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7, 0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
				0x00F0, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7, 0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x00FD, 0x00FE, 0x00FF,
			};

			if (in != nullptr)
			{
				int oneByte = in->get();

				if (oneByte == std::char_traits<char>::eof())
				{
					if ((in->rdstate() & std::istream::eofbit) != 0)
						return 0; // End of the stream.
					return -2;
				}

				result = static_cast<char32_t>(map[static_cast<unsigned char>(oneByte)]);
				if (result > 0)
					return 1;
				else
					return -1;
			}
			return -2;
		}

		inline std::istream* ISO_8859_15_StreamReader::GetInputStream() const
		{
			return in;
		}

		inline void ISO_8859_15_StreamReader::ResetInputStream(std::istream* inputStream)
		{
			in = inputStream;
		}

		//
		// ISO_8859_16_StreamReader implementation.
		//

		inline ISO_8859_16_StreamReader::ISO_8859_16_StreamReader(std::istream* inputStream)
			: in(inputStream)
		{
		
		}

		inline ISO_8859_16_StreamReader::~ISO_8859_16_StreamReader()
		{
		
		}

		inline int ISO_8859_16_StreamReader::ReadCharacter(char32_t& result)
		{
			static const char16_t map[256] =
			{
				0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0009, 0x000A, 0x0000, 0x0000, 0x000D, 0x0000, 0x0000,
				0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
				0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F,
				0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,
				0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
				0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F,
				0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
				0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x007E, 0x007F,
				0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008A, 0x008B, 0x008C, 0x008D, 0x008E, 0x008F,
				0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0098, 0x0099, 0x009A, 0x009B, 0x009C, 0x009D, 0x009E, 0x009F,
				0x00A0, 0x0104, 0x0105, 0x0141, 0x20AC, 0x201E, 0x0160, 0x00A7, 0x0161, 0x00A9, 0x0218, 0x00AB, 0x0179, 0x00AD, 0x017A, 0x017B,
				0x00B0, 0x00B1, 0x010C, 0x0142, 0x017D, 0x201D, 0x00B6, 0x00B7, 0x017E, 0x010D, 0x0219, 0x00BB, 0x0152, 0x0153, 0x0178, 0x017C,
				0x00C0, 0x00C1, 0x00C2, 0x0102, 0x00C4, 0x0106, 0x00C6, 0x00C7, 0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
				0x0110, 0x0143, 0x00D2, 0x00D3, 0x00D4, 0x0150, 0x00D6, 0x015A, 0x0170, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x0118, 0x021A, 0x00DF,
				0x00E0, 0x00E1, 0x00E2, 0x0103, 0x00E4, 0x0107, 0x00E6, 0x00E7, 0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
				0x0111, 0x0144, 0x00F2, 0x00F3, 0x00F4, 0x0151, 0x00F6, 0x015B, 0x0171, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x0119, 0x021B, 0x00FF,
			};

			if (in != nullptr)
			{
				int oneByte = in->get();

				if (oneByte == std::char_traits<char>::eof())
				{
					if ((in->rdstate() & std::istream::eofbit) != 0)
						return 0; // End of the stream.
					return -2;
				}

				result = static_cast<char32_t>(map[static_cast<unsigned char>(oneByte)]);
				if (result > 0)
					return 1;
				else
					return -1;
			}
			return -2;
		}

		inline std::istream* ISO_8859_16_StreamReader::GetInputStream() const
		{
			return in;
		}

		inline void ISO_8859_16_StreamReader::ResetInputStream(std::istream* inputStream)
		{
			in = inputStream;
		}

		//
		// TIS620StreamReader implementation.
		//

		inline TIS620StreamReader::TIS620StreamReader(std::istream* inputStream)
			: in(inputStream)
		{
		
		}

		inline TIS620StreamReader::~TIS620StreamReader()
		{
		
		}

		inline int TIS620StreamReader::ReadCharacter(char32_t& result)
		{
			static const char16_t map[256] =
			{
				0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0009, 0x000A, 0x0000, 0x0000, 0x000D, 0x0000, 0x0000,
				0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
				0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F,
				0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,
				0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
				0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F,
				0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
				0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x007E, 0x007F,
				0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087, 0x0088, 0x0089, 0x008A, 0x008B, 0x008C, 0x008D, 0x008E, 0x008F,
				0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0098, 0x0099, 0x009A, 0x009B, 0x009C, 0x009D, 0x009E, 0x009F,
				0x0000, 0x0E01, 0x0E02, 0x0E03, 0x0E04, 0x0E05, 0x0E06, 0x0E07, 0x0E08, 0x0E09, 0x0E0A, 0x0E0B, 0x0E0C, 0x0E0D, 0x0E0E, 0x0E0F,
				0x0E10, 0x0E11, 0x0E12, 0x0E13, 0x0E14, 0x0E15, 0x0E16, 0x0E17, 0x0E18, 0x0E19, 0x0E1A, 0x0E1B, 0x0E1C, 0x0E1D, 0x0E1E, 0x0E1F,
				0x0E20, 0x0E21, 0x0E22, 0x0E23, 0x0E24, 0x0E25, 0x0E26, 0x0E27, 0x0E28, 0x0E29, 0x0E2A, 0x0E2B, 0x0E2C, 0x0E2D, 0x0E2E, 0x0E2F,
				0x0E30, 0x0E31, 0x0E32, 0x0E33, 0x0E34, 0x0E35, 0x0E36, 0x0E37, 0x0E38, 0x0E39, 0x0E3A, 0x0000, 0x0000, 0x0000, 0x0000, 0x0E3F,
				0x0E40, 0x0E41, 0x0E42, 0x0E43, 0x0E44, 0x0E45, 0x0E46, 0x0E47, 0x0E48, 0x0E49, 0x0E4A, 0x0E4B, 0x0E4C, 0x0E4D, 0x0E4E, 0x0E4F,
				0x0E50, 0x0E51, 0x0E52, 0x0E53, 0x0E54, 0x0E55, 0x0E56, 0x0E57, 0x0E58, 0x0E59, 0x0E5A, 0x0E5B, 0x0000, 0x0000, 0x0000, 0x0000,
			};

			if (in != nullptr)
			{
				int oneByte = in->get();

				if (oneByte == std::char_traits<char>::eof())
				{
					if ((in->rdstate() & std::istream::eofbit) != 0)
						return 0; // End of the stream.
					return -2;
				}

				result = static_cast<char32_t>(map[static_cast<unsigned char>(oneByte)]);
				if (result > 0)
					return 1;
				else
					return -1;
			}
			return -2;
		}

		inline std::istream* TIS620StreamReader::GetInputStream() const
		{
			return in;
		}

		inline void TIS620StreamReader::ResetInputStream(std::istream* inputStream)
		{
			in = inputStream;
		}

		//
		// Windows874StreamReader implementation.
		//

		inline Windows874StreamReader::Windows874StreamReader(std::istream* inputStream)
			: in(inputStream)
		{
		
		}

		inline Windows874StreamReader::~Windows874StreamReader()
		{
		
		}

		inline int Windows874StreamReader::ReadCharacter(char32_t& result)
		{
			static const char16_t map[256] =
			{
				0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0009, 0x000A, 0x0000, 0x0000, 0x000D, 0x0000, 0x0000,
				0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
				0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F,
				0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,
				0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
				0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F,
				0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
				0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x007E, 0x007F,
				0x20AC, 0x0000, 0x0000, 0x0000, 0x0000, 0x2026, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
				0x0000, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
				0x00A0, 0x0E01, 0x0E02, 0x0E03, 0x0E04, 0x0E05, 0x0E06, 0x0E07, 0x0E08, 0x0E09, 0x0E0A, 0x0E0B, 0x0E0C, 0x0E0D, 0x0E0E, 0x0E0F,
				0x0E10, 0x0E11, 0x0E12, 0x0E13, 0x0E14, 0x0E15, 0x0E16, 0x0E17, 0x0E18, 0x0E19, 0x0E1A, 0x0E1B, 0x0E1C, 0x0E1D, 0x0E1E, 0x0E1F,
				0x0E20, 0x0E21, 0x0E22, 0x0E23, 0x0E24, 0x0E25, 0x0E26, 0x0E27, 0x0E28, 0x0E29, 0x0E2A, 0x0E2B, 0x0E2C, 0x0E2D, 0x0E2E, 0x0E2F,
				0x0E30, 0x0E31, 0x0E32, 0x0E33, 0x0E34, 0x0E35, 0x0E36, 0x0E37, 0x0E38, 0x0E39, 0x0E3A, 0x0000, 0x0000, 0x0000, 0x0000, 0x0E3F,
				0x0E40, 0x0E41, 0x0E42, 0x0E43, 0x0E44, 0x0E45, 0x0E46, 0x0E47, 0x0E48, 0x0E49, 0x0E4A, 0x0E4B, 0x0E4C, 0x0E4D, 0x0E4E, 0x0E4F,
				0x0E50, 0x0E51, 0x0E52, 0x0E53, 0x0E54, 0x0E55, 0x0E56, 0x0E57, 0x0E58, 0x0E59, 0x0E5A, 0x0E5B, 0x0000, 0x0000, 0x0000, 0x0000
			};

			if (in != nullptr)
			{
				int oneByte = in->get();

				if (oneByte == std::char_traits<char>::eof())
				{
					if ((in->rdstate() & std::istream::eofbit) != 0)
						return 0; // End of the stream.
					return -2;
				}

				result = static_cast<char32_t>(map[static_cast<unsigned char>(oneByte)]);
				if (result > 0)
					return 1;
				else
					return -1;
			}
			return -2;
		}

		inline std::istream* Windows874StreamReader::GetInputStream() const
		{
			return in;
		}

		inline void Windows874StreamReader::ResetInputStream(std::istream* inputStream)
		{
			in = inputStream;
		}

		//
		// Windows1250StreamReader implementation.
		//

		inline Windows1250StreamReader::Windows1250StreamReader(std::istream* inputStream)
			: in(inputStream)
		{
		
		}

		inline Windows1250StreamReader::~Windows1250StreamReader()
		{
		
		}

		inline int Windows1250StreamReader::ReadCharacter(char32_t& result)
		{
			static const char16_t map[256] =
			{
				0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0009, 0x000A, 0x0000, 0x0000, 0x000D, 0x0000, 0x0000,
				0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
				0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F,
				0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,
				0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
				0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F,
				0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
				0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x007E, 0x007F,
				0x20AC, 0x0000, 0x201A, 0x0000, 0x201E, 0x2026, 0x2020, 0x2021, 0x0000, 0x2030, 0x0160, 0x2039, 0x015A, 0x0164, 0x017D, 0x0179,
				0x0000, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x0000, 0x2122, 0x0161, 0x203A, 0x015B, 0x0165, 0x017E, 0x017A,
				0x00A0, 0x02C7, 0x02D8, 0x0141, 0x00A4, 0x0104, 0x00A6, 0x00A7, 0x00A8, 0x00A9, 0x015E, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x017B,
				0x00B0, 0x00B1, 0x02DB, 0x0142, 0x00B4, 0x00B5, 0x00B6, 0x00B7, 0x00B8, 0x0105, 0x015F, 0x00BB, 0x013D, 0x02DD, 0x013E, 0x017C,
				0x0154, 0x00C1, 0x00C2, 0x0102, 0x00C4, 0x0139, 0x0106, 0x00C7, 0x010C, 0x00C9, 0x0118, 0x00CB, 0x011A, 0x00CD, 0x00CE, 0x010E,
				0x0110, 0x0143, 0x0147, 0x00D3, 0x00D4, 0x0150, 0x00D6, 0x00D7, 0x0158, 0x016E, 0x00DA, 0x0170, 0x00DC, 0x00DD, 0x0162, 0x00DF,
				0x0155, 0x00E1, 0x00E2, 0x0103, 0x00E4, 0x013A, 0x0107, 0x00E7, 0x010D, 0x00E9, 0x0119, 0x00EB, 0x011B, 0x00ED, 0x00EE, 0x010F,
				0x0111, 0x0144, 0x0148, 0x00F3, 0x00F4, 0x0151, 0x00F6, 0x00F7, 0x0159, 0x016F, 0x00FA, 0x0171, 0x00FC, 0x00FD, 0x0163, 0x02D9
			};

			if (in != nullptr)
			{
				int oneByte = in->get();

				if (oneByte == std::char_traits<char>::eof())
				{
					if ((in->rdstate() & std::istream::eofbit) != 0)
						return 0; // End of the stream.
					return -2;
				}

				result = static_cast<char32_t>(map[static_cast<unsigned char>(oneByte)]);
				if (result > 0)
					return 1;
				else
					return -1;
			}
			return -2;
		}

		inline std::istream* Windows1250StreamReader::GetInputStream() const
		{
			return in;
		}

		inline void Windows1250StreamReader::ResetInputStream(std::istream* inputStream)
		{
			in = inputStream;
		}

		//
		// Windows1251StreamReader implementation.
		//

		inline Windows1251StreamReader::Windows1251StreamReader(std::istream* inputStream)
			: in(inputStream)
		{
		
		}

		inline Windows1251StreamReader::~Windows1251StreamReader()
		{
		
		}

		inline int Windows1251StreamReader::ReadCharacter(char32_t& result)
		{
			static const char16_t map[256] =
			{
				0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0009, 0x000A, 0x0000, 0x0000, 0x000D, 0x0000, 0x0000,
				0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
				0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F,
				0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,
				0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
				0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F,
				0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
				0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x007E, 0x007F,
				0x0402, 0x0403, 0x201A, 0x0453, 0x201E, 0x2026, 0x2020, 0x2021, 0x20AC, 0x2030, 0x0409, 0x2039, 0x040A, 0x040C, 0x040B, 0x040F,
				0x0452, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x0000, 0x2122, 0x0459, 0x203A, 0x045A, 0x045C, 0x045B, 0x045F,
				0x00A0, 0x040E, 0x045E, 0x0408, 0x00A4, 0x0490, 0x00A6, 0x00A7, 0x0401, 0x00A9, 0x0404, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x0407,
				0x00B0, 0x00B1, 0x0406, 0x0456, 0x0491, 0x00B5, 0x00B6, 0x00B7, 0x0451, 0x2116, 0x0454, 0x00BB, 0x0458, 0x0405, 0x0455, 0x0457,
				0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0416, 0x0417, 0x0418, 0x0419, 0x041A, 0x041B, 0x041C, 0x041D, 0x041E, 0x041F,
				0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427, 0x0428, 0x0429, 0x042A, 0x042B, 0x042C, 0x042D, 0x042E, 0x042F,
				0x0430, 0x0431, 0x0432, 0x0433, 0x0434, 0x0435, 0x0436, 0x0437, 0x0438, 0x0439, 0x043A, 0x043B, 0x043C, 0x043D, 0x043E, 0x043F,
				0x0440, 0x0441, 0x0442, 0x0443, 0x0444, 0x0445, 0x0446, 0x0447, 0x0448, 0x0449, 0x044A, 0x044B, 0x044C, 0x044D, 0x044E, 0x044F
			};

			if (in != nullptr)
			{
				int oneByte = in->get();

				if (oneByte == std::char_traits<char>::eof())
				{
					if ((in->rdstate() & std::istream::eofbit) != 0)
						return 0; // End of the stream.
					return -2;
				}

				result = static_cast<char32_t>(map[static_cast<unsigned char>(oneByte)]);
				if (result > 0)
					return 1;
				else
					return -1;
			}
			return -2;
		}

		inline std::istream* Windows1251StreamReader::GetInputStream() const
		{
			return in;
		}

		inline void Windows1251StreamReader::ResetInputStream(std::istream* inputStream)
		{
			in = inputStream;
		}

		//
		// Windows1252StreamReader implementation.
		//

		inline Windows1252StreamReader::Windows1252StreamReader(std::istream* inputStream)
			: in(inputStream)
		{
		
		}

		inline Windows1252StreamReader::~Windows1252StreamReader()
		{
		
		}

		inline int Windows1252StreamReader::ReadCharacter(char32_t& result)
		{
			static const char16_t map[256] =
			{
				0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0009, 0x000A, 0x0000, 0x0000, 0x000D, 0x0000, 0x0000,
				0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
				0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F,
				0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,
				0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
				0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F,
				0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
				0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x007E, 0x007F,
				0x20AC, 0x0000, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x0000, 0x017D, 0x0000,
				0x0000, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x0000, 0x017E, 0x0178,
				0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7, 0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
				0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7, 0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF,
				0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7, 0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
				0x00D0, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00D7, 0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x00DD, 0x00DE, 0x00DF,
				0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7, 0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
				0x00F0, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7, 0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x00FD, 0x00FE, 0x00FF
			};

			if (in != nullptr)
			{
				int oneByte = in->get();

				if (oneByte == std::char_traits<char>::eof())
				{
					if ((in->rdstate() & std::istream::eofbit) != 0)
						return 0; // End of the stream.
					return -2;
				}

				result = static_cast<char32_t>(map[static_cast<unsigned char>(oneByte)]);
				if (result > 0)
					return 1;
				else
					return -1;
			}
			return -2;
		}

		inline std::istream* Windows1252StreamReader::GetInputStream() const
		{
			return in;
		}

		inline void Windows1252StreamReader::ResetInputStream(std::istream* inputStream)
		{
			in = inputStream;
		}

		//
		// Windows1253StreamReader implementation.
		//

		inline Windows1253StreamReader::Windows1253StreamReader(std::istream* inputStream)
			: in(inputStream)
		{
		
		}

		inline Windows1253StreamReader::~Windows1253StreamReader()
		{
		
		}

		inline int Windows1253StreamReader::ReadCharacter(char32_t& result)
		{
			static const char16_t map[256] =
			{
				0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0009, 0x000A, 0x0000, 0x0000, 0x000D, 0x0000, 0x0000,
				0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
				0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F,
				0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,
				0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
				0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F,
				0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
				0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x007E, 0x007F,
				0x20AC, 0x0000, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 0x0000, 0x2030, 0x0000, 0x2039, 0x0000, 0x0000, 0x0000, 0x0000,
				0x0000, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x0000, 0x2122, 0x0000, 0x203A, 0x0000, 0x0000, 0x0000, 0x0000,
				0x00A0, 0x0385, 0x0386, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7, 0x00A8, 0x00A9, 0x0000, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x2015,
				0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x0384, 0x00B5, 0x00B6, 0x00B7, 0x0388, 0x0389, 0x038A, 0x00BB, 0x038C, 0x00BD, 0x038E, 0x038F,
				0x0390, 0x0391, 0x0392, 0x0393, 0x0394, 0x0395, 0x0396, 0x0397, 0x0398, 0x0399, 0x039A, 0x039B, 0x039C, 0x039D, 0x039E, 0x039F,
				0x03A0, 0x03A1, 0x0000, 0x03A3, 0x03A4, 0x03A5, 0x03A6, 0x03A7, 0x03A8, 0x03A9, 0x03AA, 0x03AB, 0x03AC, 0x03AD, 0x03AE, 0x03AF,
				0x03B0, 0x03B1, 0x03B2, 0x03B3, 0x03B4, 0x03B5, 0x03B6, 0x03B7, 0x03B8, 0x03B9, 0x03BA, 0x03BB, 0x03BC, 0x03BD, 0x03BE, 0x03BF,
				0x03C0, 0x03C1, 0x03C2, 0x03C3, 0x03C4, 0x03C5, 0x03C6, 0x03C7, 0x03C8, 0x03C9, 0x03CA, 0x03CB, 0x03CC, 0x03CD, 0x03CE, 0x0000
			};

			if (in != nullptr)
			{
				int oneByte = in->get();

				if (oneByte == std::char_traits<char>::eof())
				{
					if ((in->rdstate() & std::istream::eofbit) != 0)
						return 0; // End of the stream.
					return -2;
				}

				result = static_cast<char32_t>(map[static_cast<unsigned char>(oneByte)]);
				if (result > 0)
					return 1;
				else
					return -1;
			}
			return -2;
		}

		inline std::istream* Windows1253StreamReader::GetInputStream() const
		{
			return in;
		}

		inline void Windows1253StreamReader::ResetInputStream(std::istream* inputStream)
		{
			in = inputStream;
		}

		//
		// Windows1254StreamReader implementation.
		//

		inline Windows1254StreamReader::Windows1254StreamReader(std::istream* inputStream)
			: in(inputStream)
		{
		
		}

		inline Windows1254StreamReader::~Windows1254StreamReader()
		{
		
		}

		inline int Windows1254StreamReader::ReadCharacter(char32_t& result)
		{
			static const char16_t map[256] =
			{
				0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0009, 0x000A, 0x0000, 0x0000, 0x000D, 0x0000, 0x0000,
				0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
				0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F,
				0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,
				0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
				0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F,
				0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
				0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x007E, 0x007F,
				0x20AC, 0x0000, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x0000, 0x0000, 0x0000,
				0x0000, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x0000, 0x0000, 0x0178,
				0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7, 0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
				0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7, 0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF,
				0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7, 0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
				0x011E, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00D7, 0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x0130, 0x015E, 0x00DF,
				0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7, 0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
				0x011F, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F7, 0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x0131, 0x015F, 0x00FF
			};

			if (in != nullptr)
			{
				int oneByte = in->get();

				if (oneByte == std::char_traits<char>::eof())
				{
					if ((in->rdstate() & std::istream::eofbit) != 0)
						return 0; // End of the stream.
					return -2;
				}

				result = static_cast<char32_t>(map[static_cast<unsigned char>(oneByte)]);
				if (result > 0)
					return 1;
				else
					return -1;
			}
			return -2;
		}

		inline std::istream* Windows1254StreamReader::GetInputStream() const
		{
			return in;
		}

		inline void Windows1254StreamReader::ResetInputStream(std::istream* inputStream)
		{
			in = inputStream;
		}

		//
		// Windows1255StreamReader implementation.
		//

		inline Windows1255StreamReader::Windows1255StreamReader(std::istream* inputStream)
			: in(inputStream)
		{
		
		}

		inline Windows1255StreamReader::~Windows1255StreamReader()
		{
		
		}

		inline int Windows1255StreamReader::ReadCharacter(char32_t& result)
		{
			static const char16_t map[256] =
			{
				0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0009, 0x000A, 0x0000, 0x0000, 0x000D, 0x0000, 0x0000,
				0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
				0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F,
				0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,
				0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
				0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F,
				0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
				0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x007E, 0x007F,
				0x20AC, 0x0000, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 0x02C6, 0x2030, 0x0000, 0x2039, 0x0000, 0x0000, 0x0000, 0x0000,
				0x0000, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x02DC, 0x2122, 0x0000, 0x203A, 0x0000, 0x0000, 0x0000, 0x0000,
				0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x20AA, 0x00A5, 0x00A6, 0x00A7, 0x00A8, 0x00A9, 0x00D7, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
				0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7, 0x00B8, 0x00B9, 0x00F7, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF,
				0x05B0, 0x05B1, 0x05B2, 0x05B3, 0x05B4, 0x05B5, 0x05B6, 0x05B7, 0x05B8, 0x05B9, 0x0000, 0x05BB, 0x05BC, 0x05BD, 0x05BE, 0x05BF,
				0x05C0, 0x05C1, 0x05C2, 0x05C3, 0x05F0, 0x05F1, 0x05F2, 0x05F3, 0x05F4, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
				0x05D0, 0x05D1, 0x05D2, 0x05D3, 0x05D4, 0x05D5, 0x05D6, 0x05D7, 0x05D8, 0x05D9, 0x05DA, 0x05DB, 0x05DC, 0x05DD, 0x05DE, 0x05DF,
				0x05E0, 0x05E1, 0x05E2, 0x05E3, 0x05E4, 0x05E5, 0x05E6, 0x05E7, 0x05E8, 0x05E9, 0x05EA, 0x0000, 0x0000, 0x200E, 0x200F, 0x0000
			};

			if (in != nullptr)
			{
				int oneByte = in->get();

				if (oneByte == std::char_traits<char>::eof())
				{
					if ((in->rdstate() & std::istream::eofbit) != 0)
						return 0; // End of the stream.
					return -2;
				}

				result = static_cast<char32_t>(map[static_cast<unsigned char>(oneByte)]);
				if (result > 0)
					return 1;
				else
					return -1;
			}
			return -2;
		}

		inline std::istream* Windows1255StreamReader::GetInputStream() const
		{
			return in;
		}

		inline void Windows1255StreamReader::ResetInputStream(std::istream* inputStream)
		{
			in = inputStream;
		}

		//
		// Windows1256StreamReader implementation.
		//

		inline Windows1256StreamReader::Windows1256StreamReader(std::istream* inputStream)
			: in(inputStream)
		{
		
		}

		inline Windows1256StreamReader::~Windows1256StreamReader()
		{
		
		}

		inline int Windows1256StreamReader::ReadCharacter(char32_t& result)
		{
			static const char16_t map[256] =
			{
				0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0009, 0x000A, 0x0000, 0x0000, 0x000D, 0x0000, 0x0000,
				0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
				0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F,
				0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,
				0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
				0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F,
				0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
				0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x007E, 0x007F,
				0x20AC, 0x067E, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 0x02C6, 0x2030, 0x0679, 0x2039, 0x0152, 0x0686, 0x0698, 0x0688,
				0x06AF, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x06A9, 0x2122, 0x0691, 0x203A, 0x0153, 0x200C, 0x200D, 0x06BA,
				0x00A0, 0x060C, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7, 0x00A8, 0x00A9, 0x06BE, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
				0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7, 0x00B8, 0x00B9, 0x061B, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x061F,
				0x06C1, 0x0621, 0x0622, 0x0623, 0x0624, 0x0625, 0x0626, 0x0627, 0x0628, 0x0629, 0x062A, 0x062B, 0x062C, 0x062D, 0x062E, 0x062F,
				0x0630, 0x0631, 0x0632, 0x0633, 0x0634, 0x0635, 0x0636, 0x00D7, 0x0637, 0x0638, 0x0639, 0x063A, 0x0640, 0x0641, 0x0642, 0x0643,
				0x00E0, 0x0644, 0x00E2, 0x0645, 0x0646, 0x0647, 0x0648, 0x00E7, 0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x0649, 0x064A, 0x00EE, 0x00EF,
				0x064B, 0x064C, 0x064D, 0x064E, 0x00F4, 0x064F, 0x0650, 0x00F7, 0x0651, 0x00F9, 0x0652, 0x00FB, 0x00FC, 0x200E, 0x200F, 0x06D2
			};

			if (in != nullptr)
			{
				int oneByte = in->get();

				if (oneByte == std::char_traits<char>::eof())
				{
					if ((in->rdstate() & std::istream::eofbit) != 0)
						return 0; // End of the stream.
					return -2;
				}

				result = static_cast<char32_t>(map[static_cast<unsigned char>(oneByte)]);
				if (result > 0)
					return 1;
				else
					return -1;
			}
			return -2;
		}

		inline std::istream* Windows1256StreamReader::GetInputStream() const
		{
			return in;
		}

		inline void Windows1256StreamReader::ResetInputStream(std::istream* inputStream)
		{
			in = inputStream;
		}

		//
		// Windows1257StreamReader implementation.
		//

		inline Windows1257StreamReader::Windows1257StreamReader(std::istream* inputStream)
			: in(inputStream)
		{
		
		}

		inline Windows1257StreamReader::~Windows1257StreamReader()
		{
		
		}

		inline int Windows1257StreamReader::ReadCharacter(char32_t& result)
		{
			static const char16_t map[256] =
			{
				0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0009, 0x000A, 0x0000, 0x0000, 0x000D, 0x0000, 0x0000,
				0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
				0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F,
				0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,
				0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
				0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F,
				0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
				0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x007E, 0x007F,
				0x20AC, 0x0000, 0x201A, 0x0000, 0x201E, 0x2026, 0x2020, 0x2021, 0x0000, 0x2030, 0x0000, 0x2039, 0x0000, 0x00A8, 0x02C7, 0x00B8,
				0x0000, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x0000, 0x2122, 0x0000, 0x203A, 0x0000, 0x00AF, 0x02DB, 0x0000,
				0x00A0, 0x0000, 0x00A2, 0x00A3, 0x00A4, 0x0000, 0x00A6, 0x00A7, 0x00D8, 0x00A9, 0x0156, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00C6,
				0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7, 0x00F8, 0x00B9, 0x0157, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00E6,
				0x0104, 0x012E, 0x0100, 0x0106, 0x00C4, 0x00C5, 0x0118, 0x0112, 0x010C, 0x00C9, 0x0179, 0x0116, 0x0122, 0x0136, 0x012A, 0x013B,
				0x0160, 0x0143, 0x0145, 0x00D3, 0x014C, 0x00D5, 0x00D6, 0x00D7, 0x0172, 0x0141, 0x015A, 0x016A, 0x00DC, 0x017B, 0x017D, 0x00DF,
				0x0105, 0x012F, 0x0101, 0x0107, 0x00E4, 0x00E5, 0x0119, 0x0113, 0x010D, 0x00E9, 0x017A, 0x0117, 0x0123, 0x0137, 0x012B, 0x013C,
				0x0161, 0x0144, 0x0146, 0x00F3, 0x014D, 0x00F5, 0x00F6, 0x00F7, 0x0173, 0x0142, 0x015B, 0x016B, 0x00FC, 0x017C, 0x017E, 0x02D9
			};

			if (in != nullptr)
			{
				int oneByte = in->get();

				if (oneByte == std::char_traits<char>::eof())
				{
					if ((in->rdstate() & std::istream::eofbit) != 0)
						return 0; // End of the stream.
					return -2;
				}

				result = static_cast<char32_t>(map[static_cast<unsigned char>(oneByte)]);
				if (result > 0)
					return 1;
				else
					return -1;
			}
			return -2;
		}

		inline std::istream* Windows1257StreamReader::GetInputStream() const
		{
			return in;
		}

		inline void Windows1257StreamReader::ResetInputStream(std::istream* inputStream)
		{
			in = inputStream;
		}

		//
		// Windows1258StreamReader implementation.
		//

		inline Windows1258StreamReader::Windows1258StreamReader(std::istream* inputStream)
			: in(inputStream)
		{
		
		}

		inline Windows1258StreamReader::~Windows1258StreamReader()
		{
		
		}

		inline int Windows1258StreamReader::ReadCharacter(char32_t& result)
		{
			static const char16_t map[256] =
			{
				0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0009, 0x000A, 0x0000, 0x0000, 0x000D, 0x0000, 0x0000,
				0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
				0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F,
				0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,
				0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
				0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F,
				0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
				0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x007E, 0x007F,
				0x20AC, 0x0000, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 0x02C6, 0x2030, 0x0000, 0x2039, 0x0152, 0x0000, 0x0000, 0x0000,
				0x0000, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x02DC, 0x2122, 0x0000, 0x203A, 0x0153, 0x0000, 0x0000, 0x0178,
				0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7, 0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
				0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7, 0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF,
				0x00C0, 0x00C1, 0x00C2, 0x0102, 0x00C4, 0x00C5, 0x00C6, 0x00C7, 0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x0300, 0x00CD, 0x00CE, 0x00CF,
				0x0110, 0x00D1, 0x0309, 0x00D3, 0x00D4, 0x01A0, 0x00D6, 0x00D7, 0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x01AF, 0x0303, 0x00DF,
				0x00E0, 0x00E1, 0x00E2, 0x0103, 0x00E4, 0x00E5, 0x00E6, 0x00E7, 0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x0301, 0x00ED, 0x00EE, 0x00EF,
				0x0111, 0x00F1, 0x0323, 0x00F3, 0x00F4, 0x01A1, 0x00F6, 0x00F7, 0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x01B0, 0x20AB, 0x00FF
			};

			if (in != nullptr)
			{
				int oneByte = in->get();

				if (oneByte == std::char_traits<char>::eof())
				{
					if ((in->rdstate() & std::istream::eofbit) != 0)
						return 0; // End of the stream.
					return -2;
				}

				result = static_cast<char32_t>(map[static_cast<unsigned char>(oneByte)]);
				if (result > 0)
					return 1;
				else
					return -1;
			}
			return -2;
		}

		inline std::istream* Windows1258StreamReader::GetInputStream() const
		{
			return in;
		}

		inline void Windows1258StreamReader::ResetInputStream(std::istream* inputStream)
		{
			in = inputStream;
		}
	}
}

#endif

