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

#ifndef XML_INSPECTOR_HPP__f66b9cdaf20734ef11086d0851a9c563
#define XML_INSPECTOR_HPP__f66b9cdaf20734ef11086d0851a9c563

#include "CharactersReader.hpp"
#include "CharactersWriter.hpp"
#include <string>
#include <ios>
#include <streambuf>
#include <istream>
#include <fstream>
#include <cstdint>
#include <memory>
#include <deque>
#include <stdexcept>

/**
	@file XmlInspector.hpp
*/

/**
	@brief Main namespace for the XML solutions.
*/
namespace Xml
{
	/**
		@brief An inspected node type.
	*/
	enum class Inspected
	{
		/**
			@brief This is returned by the Inspector if an Inspect method has not been called.
		*/
		None,

		/**
			@brief A start element tag (for example <tt>&lt;mytag&gt;</tt> ).
		*/
		StartTag,

		/**
			@brief An end element tag (for example <tt>&lt;/mytag&gt;</tt> ).
		*/
		EndTag,

		/**
			@brief An empty element tag (for example <tt>&lt;mytag /&gt;</tt> ).
		*/
		EmptyElementTag,

		/**
			@brief A text content of a node.
		*/
		Text,

		/**
			@brief A CDATA section (for example <tt>&lt;![CDATA[don't touch]]&gt;</tt> ).
		*/
		CDATA,

		/**
			@brief The XML declaration (for example <tt>&lt;?xml version='1.0'?&gt;</tt> ).
		*/
		XmlDeclaration,

		/**
			@brief A comment (for example <tt>&lt;!-- my comment --&gt;</tt> ).
		*/
		Comment,

		/**
			@brief A processing instruction (for example <tt>&lt;?mypi ...?&gt;</tt> ).
		*/
		ProcessingInstruction,

		/**
			@brief A reference to an entity (for example <tt>&amp;myref;</tt> ).
		*/
		EntityReference,

		/**
			@brief A document type declaration (for example <tt>&lt;!DOCTYPE...&gt;</tt> ).
		*/
		DocumentType,

		/**
			@brief White space between markup. 
		*/
		Whitespace
	};

	/**
		@brief Error code.
	*/
	enum class ErrorCode
	{
		/**
			@brief There is no error.
		*/
		None,

		/**
			@brief Stream error has occurred.
		*/
		StreamError,

		/**
			@brief Invalid byte sequence. For example
				invalid byte order mark (BOM) or alone
				surrogate halve in the UTF-16 encoding.
		*/
		InvalidByteSequence,

		/**
			@brief Unknown encoding.
		*/
		UnknownEncoding,

		/**
			@brief Encoding confusion. For example UTF-8 from a byte order mark,
				but UTF-32 in an XML declaration encoding.
		*/
		EncodingConfusion,

		/**
			@brief Encoding declaration must precede content
				that is not legal UTF-8 or UTF-16.
		*/
		EncodingDeclarationRequired,

		/**
			@brief Not allowed characters. For example some characters
				outside a root element, where a white spaces are the
				only characters allowed.
		*/
		InvalidSyntax,

		/**
			@brief Check http://www.w3.org/TR/2008/REC-xml-20081126/#NT-XMLDecl.
		*/
		InvalidXmlDeclarationLocation,

		/**
			@brief CDATA section is outside a root element.
				Check http://www.w3.org/TR/2008/REC-xml-20081126/#NT-CDSect.
		*/
		CDataSectionOutside,

		/**
			@brief Element is outside a root element.
		*/
		ElementOutside,

		/**
			@brief Check http://www.w3.org/TR/2008/REC-xml-20081126/#NT-doctypedecl.
		*/
		InvalidDocumentTypeDeclarationLocation,

		/**
			@brief Check http://www.w3.org/TR/2008/REC-xml-20081126/#NT-doctypedecl.
		*/
		DoubleDocumentTypeDeclaration,

		/**
			@brief Check http://www.w3.org/TR/2009/REC-xml-names-20091208/#NT-QName.
		*/
		InvalidTagName,

		/**
			@brief Check http://www.w3.org/TR/2009/REC-xml-names-20091208/#NT-QName.
		*/
		InvalidAttributeName,

		/**
			@brief For example: <tt>&lt;tagname attr[end of document]</tt>.
		*/
		UnclosedToken,

		/**
			@brief Invalid syntax of the Reference.
				Check http://www.w3.org/TR/2008/REC-xml-20081126/#NT-Reference.
		*/
		InvalidReferenceSyntax,

		/**
			@brief Code point in the character reference doesn't match
				the valid character in ISO/IEC 10646 character set.
				Check http://www.w3.org/TR/2008/REC-xml-20081126/#NT-CharRef.
		*/
		InvalidCharacterReference,

		/**
			@brief For example: <tt>&lt;a&gt;text&lt;/b&gt;</tt>. <tt>&lt;/a&gt;</tt> expected, but <tt>&lt;/b&gt;</tt> found.
				Another example: <tt>&lt;/b&gt;</tt>. Found closing tag, but there is no start tag of @c b.
				Both examples are not allowed in the XML files.
		*/
		UnexpectedEndTag,

		/**
			@brief For example: <tt>&lt;a&gt;&lt;b&gt;&lt;c&gt;&lt;/c&gt;&lt;/b&gt;</tt>. Unclosed @c a tag.
		*/
		UnclosedTag,

		/**
			@brief There is no root element in a document.
		*/
		NoElement,

		/**
			@brief More than one attribute name in the same start-tag or empty-element tag.
		*/
		DoubleAttributeName,

		/**
			@brief There is some name prefix which is not bound to any namespace URI.
		*/
		PrefixWithoutAssignedNamespace,

		/**
			@brief Namespace declaration with a prefix cannot have an empty value.
		*/
		PrefixWithEmptyNamespace,

		/**
			@brief Reserved xmlns prefix cannot be declared or set to an empty value.
		*/
		XmlnsDeclared,

		/**
			@brief Prefix is bound to the reserved namespace.
		*/
		PrefixBoundToReservedNamespace,

		/**
			@brief Reserved namespace cannot be declared as a default namespace.
		*/
		ReservedNamespaceAsDefault,

		/**
			@brief Prefix 'xml' is reserved for use by the XML and has a fixed
				namespace URI http://www.w3.org/XML/1998/namespace.
		*/
		InvalidXmlPrefixDeclaration
	};

	/**
		@brief Delimiter for an attribute value.
	*/
	enum class QuotationMark
	{
		/**
			@brief Attribute is delimited by a single-quote characters (for example <tt>&lt;a name='value'&gt;</tt> ).
		*/
		SingleQuote,

		/**
			@brief Attribute is delimited by a double-quote characters (for example <tt>&lt;a name="value"&gt;</tt> ).
		*/
		DoubleQuote
	};

	/**
		@brief Class for storing attribute data like name and value.
	*/
	template <typename TStringType>
	class InspectedAttribute
	{
	public:
		/**
			@brief Alias to the string type provided by the class template parameter.
		*/
		typedef TStringType StringType;

		/**
			@brief Unsigned integer type definition for determining location in the XML document.
				This type should be enough to store any file size or memory buffer size.
		*/
		typedef std::uint_least64_t SizeType;

		/**
			@brief Qualified name of the attribute.
		*/
		StringType Name;

		/**
			@brief Value of the attribute.
		*/
		StringType Value;

		/**
			@brief Local name of the attribute.
		*/
		StringType LocalName;

		/**
			@brief Namespace prefix of the attribute.
		*/
		StringType Prefix;

		/**
			@brief Namespace URI of the attribute.
		*/
		StringType NamespaceUri;

		/**
			@brief Row number of the attribute name.

			Starting value is 1. For example:
			@verbatim
			<root>
			   <a
			       attrName=
				       "value"
			   />
			</root>
			@endverbatim
			Row number of @c attrName is 3.

			@sa Column.
		*/
		SizeType Row;

		/**
			@brief Column number of the attribute name.

			Starting value is 1. For example:
			@verbatim
			<root attrName="value"/>
			@endverbatim
			Column number of @c attrName is 7.

			@warning Carriage return characters (U+000D) are ignored.
			@sa Row.
		*/
		SizeType Column;

		/**
			@brief Delimiter of the attribute value.
		*/
		QuotationMark Delimiter;
	};

	/// @cond DETAILS
	namespace Details
	{
		enum class Bom
		{
			None,
			StreamError,
			Invalid,
			Utf8,
			Utf16BE,
			Utf16LE,
			Utf32BE,
			Utf32LE
		};

		Bom ReadBom(std::istream* inputStream);

		template <typename TInputIterator>
		Bom ReadBom(TInputIterator& first, TInputIterator& last);

		template <
			typename TInputIterator,
			typename TCharacterType,
			typename TTraits = std::char_traits<TCharacterType> >
		class BasicIteratorsBuf
			: public std::basic_streambuf<TCharacterType, TTraits>
		{
		public:
			typedef TInputIterator IteratorType;
			typedef std::basic_streambuf<TCharacterType, TTraits> StreambufType;

			typedef TCharacterType char_type;
			typedef TTraits traits_type;
			typedef typename traits_type::int_type int_type;
			typedef typename traits_type::pos_type pos_type;
			typedef typename traits_type::off_type off_type;
		protected:
			IteratorType curIter;
			IteratorType endIter;

			virtual int_type underflow();

			virtual int_type uflow();

			virtual std::streamsize showmanyc();
		public:
			BasicIteratorsBuf(IteratorType first, IteratorType last)
				: StreambufType(), curIter(first), endIter(last)
			{
			
			}

			virtual ~BasicIteratorsBuf()
			{
			
			}
		};

		template <typename TStringType>
		class NamespaceDeclaration
		{
		public:	
			typedef TStringType StringType;
			typedef std::uint_least64_t SizeType;

			StringType Prefix;
			StringType Uri;
			SizeType TagIndex; // Counting from 0.
		};

		template <typename TStringType>
		class UnclosedTag
		{
		public:	
			typedef TStringType StringType;
			typedef std::uint_least64_t SizeType;

			StringType Name;
			StringType LocalName;
			StringType Prefix;
			StringType NamespaceUri;
			SizeType Row;
			SizeType Column;
		};
	}
	/// @endcond

	/**
		@brief Streaming XML parser class.

		Example:
		@code{.cpp}
        #include "XmlInspector.hpp"
        #include &lt;iostream&gt;
        #include &lt;cstdlib&gt

        int main()
        {
            Xml::Inspector<Xml::Encoding::Utf8Writer> inspector("test.xml");

            while (inspector.Inspect())
            {
                switch (inspector.GetInspected())
                {
                    case Xml::Inspected::StartTag:
                        std::cout << "[StartTag] name(" << inspector.GetName() <<
                            "), value(" << inspector.GetValue() << ").\n";
                        break;
                    case Xml::Inspected::EndTag:
                        std::cout << "[EndTag] name(" << inspector.GetName() <<
                            "), value(" << inspector.GetValue() << ").\n";
                        break;
                    case Xml::Inspected::EmptyElementTag:
                        std::cout << "[EmptyElementTag] name(" << inspector.GetName() <<
                            "), value(" << inspector.GetValue() << ").\n";
                        break;
                    case Xml::Inspected::Text:
                        std::cout << "[Text] value(" << inspector.GetValue() << ").";
                        break;
                    case Xml::Inspected::Whitespace:
                        // Ignore white spaces between markup.
                        break;
                    default:
                        std::cout << "[...] name(" << inspector.GetName() <<
                            "), value(" << inspector.GetValue() << ").\n";
                        break;
                }
            }

            if (inspector.GetErrorCode() != Xml::ErrorCode::None)
            {
                std::cout << "Error: " << inspector.GetErrorMessage() <<
                " At row: " << inspector.GetRow() <<
				", column: " << inspector.GetColumn() << "\n";
            }

            return EXIT_SUCCESS;
        }
		@endcode

		@tparam TCharactersWriter Writer with a specified encoding. You don't need to care how the XML file is encoded.
			You can choose how you want to store the strings between Xml::Encoding::Utf8Writer, Xml::Encoding::Utf16Writer
			and Xml::Encoding::Utf32Writer class from CharactersWriter.hpp file. They respectively store the strings in
			@c std::string, @c std::u16string and @c std::u32string. You can also write your own fancy way of
			storing strings. For example you may want to use @c std::wstring and even other than Unicode encoding.
	*/
	template <typename TCharactersWriter>
	class Inspector
	{
	public:
		/**
			@brief Alias of the characters writer type that is used to write strings.
		*/
		typedef TCharactersWriter CharactersWriterType;

		/**
			@brief String type provided by the CharactersWriterType.
		*/
		typedef typename TCharactersWriter::StringType StringType;

		/**
			@brief Attribute type.
		*/
		typedef InspectedAttribute<StringType> AttributeType;

		/**
			@brief Unsigned integer type definition for determining location in the XML document.
				This type should be enough to store any file size or memory buffer size.
		*/
		typedef std::uint_least64_t SizeType;
	private:
		typedef typename StringType::size_type StringSizeType;
		typedef Details::UnclosedTag<StringType> UnclosedTagType;
		typedef Details::NamespaceDeclaration<StringType> NamespaceDeclarationType;
		typedef typename std::deque<AttributeType>::size_type AttributesSizeType;
		typedef typename std::deque<UnclosedTagType>::size_type UnclosedTagsSizeType;
		typedef typename std::deque<NamespaceDeclarationType>::size_type NamespacesSizeType;

		static const unsigned char Space = 0x20;                  // ' '
		static const unsigned char LineFeed = 0x0A;               // '\n'
		static const unsigned char CarriageReturn = 0x0D;         // '\r'
		static const unsigned char LessThan = 0x3C;               // '<'
		static const unsigned char GreaterThan = 0x3E;            // '>'
		static const unsigned char Equals = 0x3D;                 // '='
		static const unsigned char SingleQuote = 0x27;            // '\''
		static const unsigned char DoubleQuote = 0x22;            // '\"'
		static const unsigned char Slash = 0x2F;                  // '/'
		static const unsigned char Question = 0x3F;               // '?'
		static const unsigned char Exclamation = 0x21;            // '!'
		static const unsigned char Minus = 0x2D;                  // '-'
		static const unsigned char Ampersand = 0x26;              // '&'
		static const unsigned char Hash = 0x23;                   // '#'
		static const unsigned char X = 0x78;                      // 'x'
		static const unsigned char Colon = 0x3A;                  // ':'
		static const unsigned char Semicolon = 0x3B;              // ';'
		static const unsigned char LeftSquareBracket = 0x5B;      // '['
		static const unsigned char RightSquareBracket = 0x5D;     // ']'
		static const unsigned char Dot = 0x2E;                    // '.'
		static const unsigned char LowerXml[3];                   // "xml"
		static const unsigned char UpperXml[3];                   // "XML"
		static const unsigned char Xmlns[5];                      // "xmlns"
		static const unsigned char XmlUri[36];                    // "http://www.w3.org/XML/1998/namespace"
		static const unsigned char XmlnsUri[29];                  // "http://www.w3.org/2000/xmlns/"
		static const unsigned char XmlDeclarationVersion[7];      // "version"
		static const unsigned char XmlDeclarationEncoding[8];     // "encoding"
		static const unsigned char XmlDeclarationStandalone[10];  // "standalone"
		static const unsigned char Yes[3];                        // "yes"
		static const unsigned char No[2];                         // "no"
		static const unsigned char CDATA[5];                      // "CDATA"
		static const unsigned char DOCTYPE[7];                    // "DOCTYPE"
		static const unsigned char LtEntityName[2];               // "lt"
		static const unsigned char GtEntityName[2];               // "gt"
		static const unsigned char AmpEntityName[3];              // "amp"
		static const unsigned char AposEntityName[4];             // "apos"
		static const unsigned char QuotEntityName[4];             // "quot"

		// Use only for 1-byte characters!
		static const unsigned char ToLower[256];

		// Source types.
		static const int SourceNone = 0; // Inspector() constructor.
		static const int SourcePath = 1; // Inspector(const char*) or Inspector(const std::string&) constructor.
		static const int SourceStream = 2; // Inspector(std::istream*) constructor.
		static const int SourceIterators = 3; // Inspector(InputIterator first, InputIterator last) constructor.
		static const int SourceReader = 4; // Inspector(Encoding::CharactersReader*) constructor.
		
		static const StringSizeType NameReserve = 31;
		static const StringSizeType ValueReserve = 63;
		static const StringSizeType LocalNameReserve = 15;
		static const StringSizeType PrefixReserve = 15;
		static const StringSizeType NamespaceUriReserve = 63;

		SizeType row;
		SizeType column;
		SizeType currentRow;
		SizeType currentColumn;
		Inspected node;
		ErrorCode err;
		const char* errMsg;
		std::string fPath;
		std::ifstream fileStream;
		std::istream* inputStreamPtr;
		Encoding::CharactersReader* reader;
		int sourceType;
		bool afterBom;
		Details::Bom bom;
		StringType name;
		StringType value;
		StringType localName;
		StringType prefix;
		StringType namespaceUri;
		StringType entityName;
		std::u32string comparingName;
		SizeType entityNameCharCount;
		char32_t currentCharacter;
		char32_t bufferedCharacter;
		bool foundElement;
		bool foundDOCTYPE;
		bool eof;
		StringType lowerXmlString;
		StringType xmlnsString;
		StringType xmlUriString;
		StringType xmlnsUriString;
		// Instead of removing objects from collection I decrement size.
		// It's a fake size, but I don't want to
		// allocate strings in objects after each element node and each XML document.
		// To clear these collections you can call Inspector::Clear method.
		std::deque<AttributeType> attributes;
		AttributesSizeType attributesSize;
		std::deque<UnclosedTagType> unclosedTags;
		UnclosedTagsSizeType unclosedTagsSize;
		std::deque<NamespaceDeclarationType> namespaces;
		NamespacesSizeType namespacesSize;

		// We don't need to check carriage return
		// while NextCharBad method removes them for us.
		// That's why I use this method instead of
		// Xml::Encoding::CharactersReader::IsWhiteSpace.
		static bool IsWhiteSpace(char32_t codePoint);

		void SetError(ErrorCode errorCode);

		void SavePosition();

		// Extracts the next character and sets the error flag
		// if eof (only if insideTag flag), invalid character or stream error.
		// Returns true if error or eof (insideTag == false) happened.
		bool NextCharBad(bool insideTag);

		void ParseBom();

		bool ParseElement();

		bool ParseAttributes();

		bool ParseEndTag();

		bool ParseText();

		bool ParseQuestion();

		bool ParseXmlDeclaration();

		bool ParseProcessingInstruction();

		bool ParseExclamation();

		bool ParseComment();

		bool ParseCDATA();

		bool ParseDOCTYPE();

		void PrepareNode();

		bool NamespacesStuff();

		// Returns false if error.
		bool ParseCharacterReference(char32_t& result, bool insideTag);

		// Returns 1 if predefined entity, 0 if unknown entity, -1 if error.
		int ParseEntityReference(bool insideTag);

		bool AttributeUniqueness();

		bool ResolveEncoding(const AttributeType& encoding);

		bool IsUtf8Charset();

		bool IsUtf16Charset();

		bool IsUtf16BECharset();

		bool IsUtf16LECharset();

		bool IsUtf32Charset();

		bool IsUtf32BECharset();

		bool IsUtf32LECharset();

		bool IsISO_8859_1_Charset();

		bool IsISO_8859_2_Charset();

		bool IsISO_8859_3_Charset();

		bool IsISO_8859_4_Charset();

		bool IsISO_8859_5_Charset();

		bool IsISO_8859_6_Charset();

		bool IsISO_8859_7_Charset();

		bool IsISO_8859_8_Charset();

		bool IsISO_8859_9_Charset();

		bool IsISO_8859_10_Charset();

		bool IsISO_8859_13_Charset();

		bool IsISO_8859_14_Charset();

		bool IsISO_8859_15_Charset();

		bool IsISO_8859_16_Charset();

		bool IsTIS620Charset();

		bool IsWindows874Charset();

		bool IsWindows1250Charset();

		bool IsWindows1251Charset();

		bool IsWindows1252Charset();

		bool IsWindows1253Charset();

		bool IsWindows1254Charset();

		bool IsWindows1255Charset();

		bool IsWindows1256Charset();

		bool IsWindows1257Charset();

		bool IsWindows1258Charset();

		AttributeType& NewAttribute();

		UnclosedTagType& NewUnclosedTag();

		NamespaceDeclarationType& NewNamespace();

		bool CharsetEqual(const char32_t* charset);

		bool CharsetEqual(const unsigned char* charset, std::size_t len);

		void InitStrings();

		// Copy constructor is inaccessible for this class.
		Inspector(const Inspector&) { };

		// Assignment operator is inaccessible for this class.
		Inspector& operator=(const Inspector&) { return *this; };
	public:
		/**
			@brief Initializes a new instance of the Inspector class.
		*/
		Inspector();

		/**
			@brief Initializes a new instance of the Inspector class
				with the specified file path.
		*/
		Inspector(const char* filePath);

		/**
			@brief Initializes a new instance of the Inspector class
				with the specified file path.
		*/
		Inspector(const std::string& filePath);

		/**
			@brief Initializes a new instance of the Inspector class
				with the specified stream.
		*/
		Inspector(std::istream* inputStream);

		/**
			@brief Initializes a new instance of the Inspector class
				with the specified iterators.

			@param first,last Input iterators to the initial
				and final positions in a sequence of bytes. The range used
				is [first,last), which contains all the bytes
				between first and last, including the byte pointed
				by first but not the byte pointed by last.
		*/
		template <typename TInputIterator>
		Inspector(TInputIterator first, TInputIterator last);

		/**
			@brief Initializes a new instance of the Inspector class
				with the specified characters reader interface.
		*/
		Inspector(Encoding::CharactersReader* reader);

		/**
			@brief Destructor.
		*/
		~Inspector();

		/**
			@brief Inspects the next node from the stream.

			@return True if the next node was inspected successfully.
				False if there are no more nodes to inspect.
		*/
		bool Inspect();

		/**
			@brief Gets the last inspected node.
		*/
		Inspected GetInspected() const;

		/**
			@brief Gets the qualified name of the last inspected node.
		*/
		const StringType& GetName() const;

		/**
			@brief Gets the value of the last inspected node.
		*/
		const StringType& GetValue() const;

		/**
			@brief Gets the local name of the last inspected node.
		*/
		const StringType& GetLocalName() const;

		/**
			@brief Gets the namespace prefix of the last inspected node.
		*/
		const StringType& GetPrefix() const;

		/**
			@brief Gets the namespace URI of the last inspected node.
		*/
		const StringType& GetNamespaceUri() const;

		/**
			@brief Gets a value indicating whether the last inspected node has any attributes.
		*/
		bool HasAttributes() const;

		/**
			@brief Gets the number of attributes on the last inspected node.
		*/
		SizeType GetAttributesCount() const;

		/**
			@brief Returns attribute at the specified index on the last inspected node.

			@param index Index of the attribute.
			@return Constant reference to the chosen attribute.
			@exception std::out_of_range Index starting value is 0,
				and GetAttributesCount() result should be greater than index.
		*/
		const AttributeType& GetAttributeAt(SizeType index) const;

		/**
			@brief Gets the last error message.
		*/
		const char* GetErrorMessage() const;

		/**
			@brief Gets the last error code.
		*/
		ErrorCode GetErrorCode() const;

		/**
			@brief Gets the current row number.

			Starting value is 1. For example:
			@verbatim
			<root>
			   <a>aaa</a>
			   <b>
				   bbb
			   </b>
			</root>
			@endverbatim
			Row number of @c bbb is 4.

			@sa GetColumn() and GetDepth().
		*/
		SizeType GetRow() const;

		/**
			@brief Gets the current column number.

			Starting value is 1. For example:
			@verbatim
			<root>
			abcdef<mytag />
			</root>
			@endverbatim
			Column number of <tt>&lt;mytag /&gt;</tt> is 7.

			@sa GetRow() and GetDepth().
		*/
		SizeType GetColumn() const;

		/**
			@brief Gets the depth of the last inspected node in the XML document.

			Example:
			@verbatim
			<root>
			   <a>aaa</a>
			   <b>
				   bbb
			   </b>
			</root>
			@endverbatim
			Depth of @c aaa is 2, the same as depth of @c bbb.
			Depht of &lt;@c a&gt; tag, the same as closing tag &lt;/@c a&gt; is 1.

			@sa GetRow() and GetColumn().
		*/
		SizeType GetDepth() const;

		/**
			@brief Removes the association with the source and resets
				the state of Inspector object.

			It doesn't clear the helpful containers to reduce the number of
			string allocations in future reading nodes. To completely
			clear those containers you should call the Clear method instead.

			@sa Clear().
		*/
		void Reset();

		/**
			@brief Resets the state of Inspector object and assign
				the source to the specified file path.

			It doesn't clear the helpful containers to reduce the number of
			string allocations in future reading nodes. To completely
			clear those containers you can call the Clear method.

			@sa Clear().
		*/
		void Reset(const char* filePath);

		/**
			@brief Resets the state of Inspector object and assign
				the source to the specified file path.

			It doesn't clear the helpful containers to reduce the number of
			string allocations in future reading nodes. To completely
			clear those containers you can call the Clear method.

			@sa Clear().
		*/
		void Reset(const std::string& filePath);

		/**
			@brief Resets the state of Inspector object and assign
				the source to the specified stream.

			It doesn't clear the helpful containers to reduce the number of
			string allocations in future reading nodes. To completely
			clear those containers you can call the Clear method.

			@sa Clear().
		*/
		void Reset(std::istream* inputStream);

		/**
			@brief Resets the state of Inspector object and assign
				the source to the specified byte sequence.

			@param first,last Input iterators to the initial
				and final positions in a sequence of bytes. The range used
				is [first,last), which contains all the bytes
				between first and last, including the element pointed
				by first but not the element pointed by last.

			It doesn't clear the helpful containers to reduce the number of
			string allocations in future reading nodes. To completely
			clear those containers you can call the Clear method.

			@sa Clear().
		*/
		template <typename TInputIterator>
		void Reset(TInputIterator first, TInputIterator last);

		/**
			@brief Resets the state of Inspector object and assign
				the source to the specified characters reader interface.

			It doesn't clear the helpful containers to reduce the number of
			string allocations in future reading nodes. To completely
			clear those containers you can call the Clear method.

			@sa Clear().
		*/
		void Reset(Encoding::CharactersReader* reader);

		/**
			@brief Removes the association with the source, resets
				the state of Inspector object and clears internal containers.

			This method clears all the helpful containers in addition to
			just reset the state. If you want to use Inspector object
			to parse more XML documents you should consider Reset methods
			instead. It would reduce the number of string allocations.

			@sa Reset().
		*/
		void Clear();
	};

	template <typename TCharactersWriter>
	inline bool Inspector<TCharactersWriter>::IsWhiteSpace(char32_t codePoint)
	{
		return (codePoint == 0x20 || codePoint == 0x0A || codePoint == 0x09);
	}

	template <typename TCharactersWriter>
	const unsigned char Inspector<TCharactersWriter>::LowerXml[3] =
	{
		// "xml"
		0x78, 0x6D, 0x6C
	};

	template <typename TCharactersWriter>
	const unsigned char Inspector<TCharactersWriter>::UpperXml[3] =
	{
		// "XML"
		0x58, 0x4D, 0x4C
	};

	template <typename TCharactersWriter>
	const unsigned char Inspector<TCharactersWriter>::Xmlns[5] =
	{
		// "xmlns"
		0x78, 0x6D, 0x6C, 0x6E, 0x73
	};

	template <typename TCharactersWriter>
	const unsigned char Inspector<TCharactersWriter>::XmlUri[36] =
	{
		// "http://www.w3.org/XML/1998/namespace"
		0x68, 0x74, 0x74, 0x70, 0x3A, 0x2F, 0x2F, 0x77, 0x77, 0x77, 0x2E, 0x77,
		0x33, 0x2E, 0x6F, 0x72, 0x67, 0x2F, 0x58, 0x4D, 0x4C, 0x2F, 0x31, 0x39,
		0x39, 0x38, 0x2F, 0x6E, 0x61, 0x6D, 0x65, 0x73, 0x70, 0x61, 0x63, 0x65
	};

	template <typename TCharactersWriter>
	const unsigned char Inspector<TCharactersWriter>::XmlnsUri[29] =
	{
		// "http://www.w3.org/2000/xmlns/"
		0x68, 0x74, 0x74, 0x70, 0x3A, 0x2F, 0x2F, 0x77, 0x77, 0x77, 0x2E, 0x77, 0x33, 0x2E, 0x6F,
		0x72, 0x67, 0x2F, 0x32, 0x30, 0x30, 0x30, 0x2F, 0x78, 0x6D, 0x6C, 0x6E, 0x73, 0x2F
	};

	template <typename TCharactersWriter>
	const unsigned char Inspector<TCharactersWriter>::XmlDeclarationVersion[7] =
	{
		// "version"
		0x76, 0x65, 0x72, 0x73, 0x69, 0x6F, 0x6E
	};

	template <typename TCharactersWriter>
	const unsigned char Inspector<TCharactersWriter>::XmlDeclarationEncoding[8] =
	{
		// "encoding"
		0x65, 0x6E, 0x63, 0x6F, 0x64, 0x69, 0x6E, 0x67
	};

	template <typename TCharactersWriter>
	const unsigned char Inspector<TCharactersWriter>::XmlDeclarationStandalone[10] =
	{
		// "standalone"
		0x73, 0x74, 0x61, 0x6E, 0x64, 0x61, 0x6C, 0x6F, 0x6E, 0x65
	};

	template <typename TCharactersWriter>
	const unsigned char Inspector<TCharactersWriter>::Yes[3] =
	{
		// "yes"
		0x79, 0x65, 0x73
	};

	template <typename TCharactersWriter>
	const unsigned char Inspector<TCharactersWriter>::No[2] =
	{
		// "no"
		0x6E, 0x6F
	};

	template <typename TCharactersWriter>
	const unsigned char Inspector<TCharactersWriter>::CDATA[5] =
	{
		// "CDATA"
		0x43, 0x44, 0x41, 0x54, 0x41
	};

	template <typename TCharactersWriter>
	const unsigned char Inspector<TCharactersWriter>::DOCTYPE[7] =
	{
		// "DOCTYPE"
		0x44, 0x4F, 0x43, 0x54, 0x59, 0x50, 0x45
	};

	template <typename TCharactersWriter>
	const unsigned char Inspector<TCharactersWriter>::LtEntityName[2] =
	{
		// "lt"
		0x6C, 0x74
	};

	template <typename TCharactersWriter>
	const unsigned char Inspector<TCharactersWriter>::GtEntityName[2] =
	{
		// "gt"
		0x67, 0x74
	};

	template <typename TCharactersWriter>
	const unsigned char Inspector<TCharactersWriter>::AmpEntityName[3] =
	{
		// "amp"
		0x61, 0x6D, 0x70
	};

	template <typename TCharactersWriter>
	const unsigned char Inspector<TCharactersWriter>::AposEntityName[4] =
	{
		// "apos"
		0x61, 0x70, 0x6F, 0x73
	};

	template <typename TCharactersWriter>
	const unsigned char Inspector<TCharactersWriter>::QuotEntityName[4] =
	{
		// "quot"
		0x71, 0x75, 0x6F, 0x74
	};

	template <typename TCharactersWriter>
	const unsigned char Inspector<TCharactersWriter>::ToLower[256] =
	{
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x0A, 0x00, 0x00, 0x0D, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
		0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
		0x40, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
		0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
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

	template <typename TCharactersWriter>
	inline Inspector<TCharactersWriter>::Inspector()
		: row(0),
		column(0),
		currentRow(0),
		currentColumn(0),
		node(Inspected::None),
		err(ErrorCode::None),
		errMsg(nullptr),
		fPath(),
		fileStream(),
		inputStreamPtr(nullptr),
		reader(nullptr),
		sourceType(SourceNone),
		afterBom(false),
		bom(Details::Bom::None),
		name(),
		value(),
		localName(),
		prefix(),
		namespaceUri(),
		entityName(),
		comparingName(),
		entityNameCharCount(0),
		currentCharacter(0),
		bufferedCharacter(0),
		foundElement(false),
		foundDOCTYPE(false),
		eof(false),
		lowerXmlString(),
		xmlnsString(),
		xmlUriString(),
		xmlnsUriString(),
		attributes(),
		attributesSize(0),
		unclosedTags(),
		unclosedTagsSize(0),
		namespaces(),
		namespacesSize(0)
	{
		InitStrings();
	}

	template <typename TCharactersWriter>
	inline Inspector<TCharactersWriter>::Inspector(const char* filePath)
		: row(0),
		column(0),
		currentRow(0),
		currentColumn(0),
		node(Inspected::None),
		err(ErrorCode::None),
		errMsg(nullptr),
		fPath(),
		fileStream(),
		inputStreamPtr(nullptr),
		reader(nullptr),
		sourceType(SourceNone),
		afterBom(false),
		bom(Details::Bom::None),
		name(),
		value(),
		localName(),
		prefix(),
		namespaceUri(),
		entityName(),
		comparingName(),
		entityNameCharCount(0),
		currentCharacter(0),
		bufferedCharacter(0),
		foundElement(false),
		foundDOCTYPE(false),
		eof(false),
		lowerXmlString(),
		xmlnsString(),
		xmlUriString(),
		xmlnsUriString(),
		attributes(),
		attributesSize(0),
		unclosedTags(),
		unclosedTagsSize(0),
		namespaces(),
		namespacesSize(0)
	{
		InitStrings();
		Reset(filePath);
	}

	template <typename TCharactersWriter>
	inline Inspector<TCharactersWriter>::Inspector(const std::string& filePath)
		: row(0),
		column(0),
		currentRow(0),
		currentColumn(0),
		node(Inspected::None),
		err(ErrorCode::None),
		errMsg(nullptr),
		fPath(),
		fileStream(),
		inputStreamPtr(nullptr),
		reader(nullptr),
		sourceType(SourceNone),
		afterBom(false),
		bom(Details::Bom::None),
		name(),
		value(),
		localName(),
		prefix(),
		namespaceUri(),
		entityName(),
		comparingName(),
		entityNameCharCount(0),
		currentCharacter(0),
		bufferedCharacter(0),
		foundElement(false),
		foundDOCTYPE(false),
		eof(false),
		lowerXmlString(),
		xmlnsString(),
		xmlUriString(),
		xmlnsUriString(),
		attributes(),
		attributesSize(0),
		unclosedTags(),
		unclosedTagsSize(0),
		namespaces(),
		namespacesSize(0)
	{
		InitStrings();
		Reset(filePath);
	}

	template <typename TCharactersWriter>
	inline Inspector<TCharactersWriter>::Inspector(std::istream* inputStream)
		: row(0),
		column(0),
		currentRow(0),
		currentColumn(0),
		node(Inspected::None),
		err(ErrorCode::None),
		errMsg(nullptr),
		fPath(),
		fileStream(),
		inputStreamPtr(nullptr),
		reader(nullptr),
		sourceType(SourceNone),
		afterBom(false),
		bom(Details::Bom::None),
		name(),
		value(),
		localName(),
		prefix(),
		namespaceUri(),
		entityName(),
		comparingName(),
		entityNameCharCount(0),
		currentCharacter(0),
		bufferedCharacter(0),
		foundElement(false),
		foundDOCTYPE(false),
		eof(false),
		lowerXmlString(),
		xmlnsString(),
		xmlUriString(),
		xmlnsUriString(),
		attributes(),
		attributesSize(0),
		unclosedTags(),
		unclosedTagsSize(0),
		namespaces(),
		namespacesSize(0)
	{
		InitStrings();
		Reset(inputStream);
	}

	template <typename TCharactersWriter>
	template <typename TInputIterator>
	inline Inspector<TCharactersWriter>::Inspector(
		TInputIterator first, TInputIterator last)
		: row(0),
		column(0),
		currentRow(0),
		currentColumn(0),
		node(Inspected::None),
		err(ErrorCode::None),
		errMsg(nullptr),
		fPath(),
		fileStream(),
		inputStreamPtr(nullptr),
		reader(nullptr),
		sourceType(SourceNone),
		afterBom(false),
		bom(Details::Bom::None),
		name(),
		value(),
		localName(),
		prefix(),
		namespaceUri(),
		entityName(),
		comparingName(),
		entityNameCharCount(0),
		currentCharacter(0),
		bufferedCharacter(0),
		foundElement(false),
		foundDOCTYPE(false),
		eof(false),
		lowerXmlString(),
		xmlnsString(),
		xmlUriString(),
		xmlnsUriString(),
		attributes(),
		attributesSize(0),
		unclosedTags(),
		unclosedTagsSize(0),
		namespaces(),
		namespacesSize(0)
	{
		InitStrings();
		Reset(first, last);
	}

	template <typename TCharactersWriter>
	inline Inspector<TCharactersWriter>::Inspector(Encoding::CharactersReader* reader)
		: row(0),
		column(0),
		currentRow(0),
		currentColumn(0),
		node(Inspected::None),
		err(ErrorCode::None),
		errMsg(nullptr),
		fPath(),
		fileStream(),
		inputStreamPtr(nullptr),
		reader(nullptr),
		sourceType(SourceNone),
		afterBom(false),
		bom(Details::Bom::None),
		name(),
		value(),
		localName(),
		prefix(),
		namespaceUri(),
		entityName(),
		comparingName(),
		entityNameCharCount(0),
		currentCharacter(0),
		bufferedCharacter(0),
		foundElement(false),
		foundDOCTYPE(false),
		eof(false),
		lowerXmlString(),
		xmlnsString(),
		xmlUriString(),
		xmlnsUriString(),
		attributes(),
		attributesSize(0),
		unclosedTags(),
		unclosedTagsSize(0),
		namespaces(),
		namespacesSize(0)
	{
		InitStrings();
		Reset(reader);
	}

	template <typename TCharactersWriter>
	inline Inspector<TCharactersWriter>::~Inspector()
	{
		Reset();
	}

	template <typename TCharactersWriter>
	inline void Inspector<TCharactersWriter>::SetError(ErrorCode errorCode)
	{
		err = errorCode;
		if (errorCode != ErrorCode::None)
		{
			switch (errorCode)
			{
				case ErrorCode::StreamError:
					errMsg = "Stream error has occurred.";
					return;
				case ErrorCode::InvalidByteSequence:
					errMsg = "Invalid byte sequence.";
					return;
				case ErrorCode::UnknownEncoding:
					errMsg = "Unknown encoding.";
					return;
				case ErrorCode::EncodingConfusion:
					errMsg = "Encoding confusion.";
					return;
				case ErrorCode::EncodingDeclarationRequired:
					errMsg = "Encoding declaration must precede content that is not "
						"a legal UTF-8 or UTF-16.";
					return;
				case ErrorCode::InvalidSyntax:
					errMsg = "Invalid syntax.";
					return;
				case ErrorCode::InvalidXmlDeclarationLocation:
					errMsg = "Invalid location of XML declaration.";
					return;
				case ErrorCode::CDataSectionOutside:
					errMsg = "CDATA section is outside a root element.";
					return;
				case ErrorCode::ElementOutside:
					errMsg = "Element is outside a root element.";
					return;
				case ErrorCode::InvalidDocumentTypeDeclarationLocation:
					errMsg = "Invalid location of document type declaration.";
					return;
				case ErrorCode::DoubleDocumentTypeDeclaration:
					errMsg = "There should be exactly one document type declaration.";
					return;
				case ErrorCode::InvalidTagName:
					errMsg = "Invalid tag name.";
					return;
				case ErrorCode::InvalidAttributeName:
					errMsg = "Invalid attribute name.";
					return;
				case ErrorCode::UnclosedToken:
					errMsg = "Unclosed token.";
					return;
				case ErrorCode::InvalidReferenceSyntax:
					errMsg = "Invalid syntax of reference.";
					return;
				case ErrorCode::InvalidCharacterReference:
					errMsg = "Code point in character reference doesn\'t match "
						"the valid character in ISO/IEC 10646 character set.";
					return;
				case ErrorCode::UnexpectedEndTag:
					errMsg = "Unexpected end tag.";
					return;
				case ErrorCode::UnclosedTag:
					errMsg = "Unclosed tag.";
					return;
				case ErrorCode::NoElement:
					errMsg = "Cannot find an element.";
					return;
				case ErrorCode::DoubleAttributeName:
					errMsg = "An attribute name must not appear more than "
						"once in the same start-tag or empty-element tag.";
					return;
				case ErrorCode::PrefixWithoutAssignedNamespace:
					errMsg = "Name prefix must bound to the namespace URI.";
					return;
				case ErrorCode::PrefixWithEmptyNamespace:
					errMsg = "Namespace declaration with prefix cannot have an empty value.";
					return;
				case ErrorCode::XmlnsDeclared:
					errMsg = "Reserved xmlns prefix cannot be declared or set to an empty value.";
					return;
				case ErrorCode::PrefixBoundToReservedNamespace:
					errMsg = "Prefix is bound to reserved namespace.";
					return;
				case ErrorCode::ReservedNamespaceAsDefault:
					errMsg = "Reserved namespace cannot be declared as a default namespace.";
					return;
				case ErrorCode::InvalidXmlPrefixDeclaration:
					errMsg = "Prefix \'xml\' is reserved for use by XML and has a fixed "
						"namespace URI http://www.w3.org/XML/1998/namespace.";
					return;
				default:
					errMsg = "XML error has occurred.";
					return;
			}
		}
		errMsg = nullptr;
	}

	template <typename TCharactersWriter>
	inline void Inspector<TCharactersWriter>::ParseBom()
	{
		if (sourceType == SourcePath)
		{
			fileStream.open(fPath.c_str(), std::ifstream::binary);
			if (!fileStream.is_open())
			{
				fileStream.clear();
				SetError(ErrorCode::StreamError);
				return;
			}

			Details::Bom tempBom = Details::ReadBom(&fileStream);
			if (tempBom == Details::Bom::None || tempBom == Details::Bom::Utf8)
			{
				try
				{
					reader = new Encoding::Utf8StreamReader(&fileStream);
				}
				catch (...)
				{
					fileStream.close();
					fileStream.clear();
					throw;
				}
				err = ErrorCode::None;
				afterBom = true;
				bom = tempBom;
				eof = ((fileStream.rdstate() & std::istream::eofbit) != 0);
				return;
			}

			if (tempBom == Details::Bom::StreamError)
			{
				fileStream.close();
				fileStream.clear();
				SetError(ErrorCode::StreamError);
				return;
			}

			if (tempBom == Details::Bom::Invalid)
			{
				fileStream.close();
				fileStream.clear();
				SetError(ErrorCode::InvalidByteSequence);
				eof = ((fileStream.rdstate() & std::istream::eofbit) != 0);
				return;
			}

			if (tempBom == Details::Bom::Utf16BE)
			{
				try
				{
					reader = new Encoding::Utf16BEStreamReader(&fileStream);
				}
				catch (...)
				{
					fileStream.close();
					fileStream.clear();
					throw;
				}
				err = ErrorCode::None;
				afterBom = true;
				bom = tempBom;
			}
			else if (tempBom == Details::Bom::Utf16LE)
			{
				try
				{
					reader = new Encoding::Utf16LEStreamReader(&fileStream);
				}
				catch (...)
				{
					fileStream.close();
					fileStream.clear();
					throw;
				}
				err = ErrorCode::None;
				afterBom = true;
				bom = tempBom;
			}
			else if (tempBom == Details::Bom::Utf32BE)
			{
				try
				{
					reader = new Encoding::Utf32BEStreamReader(&fileStream);
				}
				catch (...)
				{
					fileStream.close();
					fileStream.clear();
					throw;
				}
				err = ErrorCode::None;
				afterBom = true;
				bom = tempBom;
			}
			else if (tempBom == Details::Bom::Utf32LE)
			{
				try
				{
					reader = new Encoding::Utf32LEStreamReader(&fileStream);
				}
				catch (...)
				{
					fileStream.close();
					fileStream.clear();
					throw;
				}
				err = ErrorCode::None;
				afterBom = true;
				bom = tempBom;
			}
		}
		else if (sourceType == SourceStream || sourceType == SourceIterators)
		{
			Details::Bom tempBom = Details::ReadBom(inputStreamPtr);
			if (tempBom == Details::Bom::None || tempBom == Details::Bom::Utf8)
			{
				reader = new Encoding::Utf8StreamReader(inputStreamPtr);
				err = ErrorCode::None;
				afterBom = true;
				bom = tempBom;
				eof = ((inputStreamPtr->rdstate() & std::istream::eofbit) != 0);
				return;
			}

			if (tempBom == Details::Bom::StreamError)
			{
				SetError(ErrorCode::StreamError);
				return;
			}

			if (tempBom == Details::Bom::Invalid)
			{
				SetError(ErrorCode::InvalidByteSequence);
				eof = ((inputStreamPtr->rdstate() & std::istream::eofbit) != 0);
				return;
			}

			if (tempBom == Details::Bom::Utf16BE)
			{
				reader = new Encoding::Utf16BEStreamReader(inputStreamPtr);
				err = ErrorCode::None;
				afterBom = true;
				bom = tempBom;
			}
			else if (tempBom == Details::Bom::Utf16LE)
			{
				reader = new Encoding::Utf16LEStreamReader(inputStreamPtr);
				err = ErrorCode::None;
				afterBom = true;
				bom = tempBom;
			}
			else if (tempBom == Details::Bom::Utf32BE)
			{
				reader = new Encoding::Utf32BEStreamReader(inputStreamPtr);
				err = ErrorCode::None;
				afterBom = true;
				bom = tempBom;
			}
			else if (tempBom == Details::Bom::Utf32LE)
			{
				reader = new Encoding::Utf32LEStreamReader(inputStreamPtr);
				err = ErrorCode::None;
				afterBom = true;
				bom = tempBom;
			}
		}
		else if (sourceType == SourceReader)
		{
			err = ErrorCode::None;
			afterBom = true;
		}
		else
		{
			SetError(ErrorCode::StreamError);
		}
	}

	template <typename TCharactersWriter>
	inline bool Inspector<TCharactersWriter>::ParseElement()
	{
		// currentCharacter == name start character.
		// and
		// currentCharacter != Colon.
		SizeType tempRow = currentRow;
		SizeType tempColumn = currentColumn;

		if (unclosedTagsSize == 0 && foundElement)
		{
			tempRow = row;
			tempColumn = column;
			Reset();
			SetError(ErrorCode::ElementOutside);
			row = tempRow;
			column = tempColumn;
			return false;
		}

		PrepareNode();

		// Element name.
		do
		{
			CharactersWriterType::WriteCharacter(name, currentCharacter);
			CharactersWriterType::WriteCharacter(localName, currentCharacter);

			if (NextCharBad(true))
				return false;

			if (currentCharacter == Colon)
			{
				// Prefixed name.
				prefix = name;
				localName.clear();
				CharactersWriterType::WriteCharacter(name, currentCharacter);

				if (NextCharBad(true))
					return false;

				if (currentCharacter == Colon ||
					!Encoding::CharactersReader::IsNameStartChar(currentCharacter))
				{
					Reset();
					SetError(ErrorCode::InvalidTagName);
					row = tempRow;
					column = tempColumn;
					return false;
				}

				do
				{
					CharactersWriterType::WriteCharacter(name, currentCharacter);
					CharactersWriterType::WriteCharacter(localName, currentCharacter);

					if (NextCharBad(true))
						return false;

					if (currentCharacter == Colon)
					{
						Reset();
						SetError(ErrorCode::InvalidTagName);
						row = tempRow;
						column = tempColumn;
						return false;
					}
				}
				while (Encoding::CharactersReader::IsNameChar(currentCharacter));
				break;
			}
		}
		while (Encoding::CharactersReader::IsNameChar(currentCharacter));

		if (currentCharacter == GreaterThan)
		{
			node = Inspected::StartTag;
			bool noErrors = NamespacesStuff();
			if (noErrors)
			{
				UnclosedTagType& ref = NewUnclosedTag();
				ref.Name = name;
				ref.LocalName = localName;
				ref.Prefix = prefix;
				ref.NamespaceUri = namespaceUri;
				ref.Row = row;
				ref.Column = column;
				foundElement = true;
				return true;
			}
			return false;
		}

		if (currentCharacter == Slash)
		{
			// <tagName/
			if (NextCharBad(true))
				return false;
			if (currentCharacter != GreaterThan)
			{
				tempRow = currentRow;
				tempColumn = currentColumn;
				Reset();
				SetError(ErrorCode::InvalidSyntax);
				row = tempRow;
				column = tempColumn;
				return false;
			}

			node = Inspected::EmptyElementTag;
			bool noErrors = NamespacesStuff();
			if (noErrors)
			{
				foundElement = true;
				return true;
			}
			return false;
		}

		if (IsWhiteSpace(currentCharacter))
		{
			// Ignore white spaces.
			do
			{
				if (NextCharBad(true))
					return false;
			}
			while (IsWhiteSpace(currentCharacter));

			if (currentCharacter != Colon &&
				Encoding::CharactersReader::IsNameStartChar(currentCharacter))
			{
				// Attributes.
				return ParseAttributes();
			}

			if (currentCharacter == Slash)
			{
				// <tagName /
				if (NextCharBad(true))
					return false;
				if (currentCharacter != GreaterThan)
				{
					tempRow = currentRow;
					tempColumn = currentColumn;
					Reset();
					SetError(ErrorCode::InvalidSyntax);
					row = tempRow;
					column = tempColumn;
					return false;
				}

				node = Inspected::EmptyElementTag;
				bool noErrors = NamespacesStuff();
				if (noErrors)
				{
					foundElement = true;
					return true;
				}
				return false;
			}

			if (currentCharacter == GreaterThan)
			{
				// <tagName >
				node = Inspected::StartTag;
				bool noErrors = NamespacesStuff();
				if (noErrors)
				{
					UnclosedTagType& ref = NewUnclosedTag();
					ref.Name = name;
					ref.LocalName = localName;
					ref.Prefix = prefix;
					ref.NamespaceUri = namespaceUri;
					ref.Row = row;
					ref.Column = column;
					foundElement = true;
					return true;
				}
				return false;
			}

			if (Encoding::CharactersReader::IsNameChar(currentCharacter))
			{
				// For example <tagName 123attr="value">
				// 1 is not allowed as a first character name.
				tempRow = currentRow;
				tempColumn = currentColumn;
				Reset();
				SetError(ErrorCode::InvalidAttributeName);
				row = tempRow;
				column = tempColumn;
				return false;
			}

			// Invalid syntax.
			// For example <tagName !abc...
			// ! is not allowed as a part of the name.
			tempRow = currentRow;
			tempColumn = currentColumn;
			Reset();
			SetError(ErrorCode::InvalidSyntax);
			row = tempRow;
			column = tempColumn;
			return false;
		}

		// Invalid tag name.
		Reset();
		SetError(ErrorCode::InvalidTagName);
		row = tempRow;
		column = tempColumn;
		return false;
	}

	
	template <typename TCharactersWriter>
	inline bool Inspector<TCharactersWriter>::ParseAttributes()
	{
		// IsNameStartChar(currentCharacter) == true
		// and
		// currentCharacter != Colon

		SizeType tempRow;
		SizeType tempColumn;
		bool white = true;

		do // {...} while (Encoding::CharactersReader::IsNameStartChar(currentCharacter));
		{
			AttributeType& attr = NewAttribute();
			attr.Row = currentRow;
			attr.Column = currentColumn;

			// attribute.Name, attribute.LocalName, attribute.Prefix.
			do // {...} while (Encoding::CharactersReader::IsNameChar(currentCharacter));
			{
				CharactersWriterType::WriteCharacter(attr.Name, currentCharacter);
				CharactersWriterType::WriteCharacter(attr.LocalName, currentCharacter);

				if (NextCharBad(true))
					return false;

				if (currentCharacter == Colon)
				{
					// Prefixed name.
					attr.Prefix = attr.Name;
					attr.LocalName.clear();
					CharactersWriterType::WriteCharacter(attr.Name, currentCharacter);

					if (NextCharBad(true))
						return false;

					if (currentCharacter == Colon ||
						!Encoding::CharactersReader::IsNameStartChar(currentCharacter))
					{
						Reset();
						SetError(ErrorCode::InvalidAttributeName);
						row = attr.Row;
						column = attr.Column;
						return false;
					}

					do // {...} while (Encoding::CharactersReader::IsNameChar(currentCharacter));
					{
						CharactersWriterType::WriteCharacter(attr.Name, currentCharacter);
						CharactersWriterType::WriteCharacter(attr.LocalName, currentCharacter);

						if (NextCharBad(true))
							return false;

						if (currentCharacter == Colon)
						{
							Reset();
							SetError(ErrorCode::InvalidAttributeName);
							row = attr.Row;
							column = attr.Column;
							return false;
						}
					}
					while (Encoding::CharactersReader::IsNameChar(currentCharacter));
					break;
				}
			}
			while (Encoding::CharactersReader::IsNameChar(currentCharacter));

			if (IsWhiteSpace(currentCharacter))
			{
				// Ignore white spaces.
				do
				{
					if (NextCharBad(true))
						return false;
				}
				while (IsWhiteSpace(currentCharacter));
				white = true;
			}
			else
			{
				white = false;
			}

			if (currentCharacter != Equals)
			{
				if (white)
				{
					tempRow = currentRow;
					tempColumn = currentColumn;
					Reset();
					SetError(ErrorCode::InvalidSyntax);
					row = tempRow;
					column = tempColumn;
					return false;
				}
				Reset();
				SetError(ErrorCode::InvalidAttributeName);
				row = attr.Row;
				column = attr.Column;
				return false;
			}

			// attrname=
			if (NextCharBad(true))
				return false;

			// Ignore white spaces.
			while (IsWhiteSpace(currentCharacter))
			{
				if (NextCharBad(true))
					return false;
			}

			char32_t quoteChar = currentCharacter;

			if (quoteChar == DoubleQuote)
			{
				// attrname="
				attr.Delimiter = QuotationMark::DoubleQuote;
			}
			else if (quoteChar == SingleQuote)
			{
				// attrname='
				attr.Delimiter = QuotationMark::SingleQuote;
			}
			else
			{
				tempRow = currentRow;
				tempColumn = currentColumn;
				Reset();
				SetError(ErrorCode::InvalidSyntax);
				row = tempRow;
				column = tempColumn;
				return false;
			}

			if (NextCharBad(true))
				return false;

			// attr.Value
			while (currentCharacter != quoteChar)
			{
				if (currentCharacter == LessThan)
				{
					tempRow = currentRow;
					tempColumn = currentColumn;
					Reset();
					SetError(ErrorCode::InvalidSyntax);
					row = tempRow;
					column = tempColumn;
					return false;
				}

				if (currentCharacter == Ampersand)
				{
					if (NextCharBad(true))
						return false;

					if (currentCharacter == Hash)
					{
						// "&#"

						char32_t codePoint;

						if (!ParseCharacterReference(codePoint, true))
							return false;

						// currentCharacter == Semicolon.

						CharactersWriterType::WriteCharacter(attr.Value, codePoint);
						if (NextCharBad(true))
							return false;
						continue; // while (currentCharacter != quoteChar) {...}
					}
					else if (currentCharacter != Colon &&
						Encoding::CharactersReader::IsNameStartChar(currentCharacter))
					{
						int resultParsing = ParseEntityReference(true);
						if (resultParsing == -1)
						{
							// Error.
							return false;
						}
						else if (resultParsing == 0)
						{
							// Unknown entity reference.
							CharactersWriterType::WriteCharacter(attr.Value, Ampersand);
							attr.Value.append(entityName);
							entityName.clear();
							CharactersWriterType::WriteCharacter(attr.Value, Semicolon);
							if (NextCharBad(true))
								return false;
							continue; // while (currentCharacter != quoteChar) {...}
						}
						else // resultParsing == 1.
						{
							// Predefined entity reference.
							CharactersWriterType::WriteCharacter(attr.Value, currentCharacter);
							if (NextCharBad(true))
								return false;
							continue; // while (currentCharacter != quoteChar) {...}
						}
					}
					else
					{
						tempRow = currentRow;
						tempColumn = currentColumn - 1;
						Reset();
						SetError(ErrorCode::InvalidReferenceSyntax);
						row = tempRow;
						column = tempColumn;
						return false;
					}
				}

				if (!IsWhiteSpace(currentCharacter))
					CharactersWriterType::WriteCharacter(attr.Value, currentCharacter);
				else
					CharactersWriterType::WriteCharacter(attr.Value, Space);

				if (NextCharBad(true))
					return false;
			} // while (currentCharacter != quoteChar) {...}

			// attrname="value"

			if (!AttributeUniqueness())
				return false;

			if (NextCharBad(true))
				return false;

			if (IsWhiteSpace(currentCharacter))
			{
				// Ignore white spaces.
				do
				{
					if (NextCharBad(true))
						return false;
				}
				while (IsWhiteSpace(currentCharacter));
				white = true;

				if (currentCharacter == Colon)
				{
					Reset();
					SetError(ErrorCode::InvalidAttributeName);
					row = attr.Row;
					column = attr.Column;
					return false;
				}
			}
			else
			{
				white = false;
				break;
			}
		}
		while (Encoding::CharactersReader::IsNameStartChar(currentCharacter));

		if (currentCharacter == GreaterThan)
		{
			// attrname="value">
			node = Inspected::StartTag;
			bool noErrors = NamespacesStuff();
			if (noErrors)
			{
				UnclosedTagType& ref = NewUnclosedTag();
				ref.Name = name;
				ref.LocalName = localName;
				ref.Prefix = prefix;
				ref.NamespaceUri = namespaceUri;
				ref.Row = row;
				ref.Column = column;
				foundElement = true;
				return true;
			}
			return false;
		}
		else if (currentCharacter == Slash)
		{
			// attrname="value"/
			if (NextCharBad(true))
				return false;
			if (currentCharacter != GreaterThan)
			{
				tempRow = currentRow;
				tempColumn = currentColumn;
				Reset();
				SetError(ErrorCode::InvalidSyntax);
				row = tempRow;
				column = tempColumn;
				return false;
			}

			node = Inspected::EmptyElementTag;
			bool noErrors = NamespacesStuff();
			if (noErrors)
			{
				// Namespaces associated with this tag are no longer needed.
				SizeType indicesToRemove = static_cast<SizeType>(unclosedTagsSize);
				NamespacesSizeType newNamespacesSize = 0;
				while (newNamespacesSize < namespacesSize)
				{
					if (namespaces[newNamespacesSize].TagIndex == indicesToRemove)
						break;
					++newNamespacesSize;
				}
				namespacesSize = newNamespacesSize;
				foundElement = true;
				return true;
			}
			return false;
		}
		else // Some error.
		{
			if (white && Encoding::CharactersReader::IsNameChar(currentCharacter))
			{
				// After white space, cannot be a start character of attribute name,
				// but can be a part of this name. Something like:
				// <tag 123attrName="value">. "1" is not allowed as the start character.
				tempRow = currentRow;
				tempColumn = currentColumn;
				Reset();
				SetError(ErrorCode::InvalidAttributeName);
				row = tempRow;
				column = tempColumn;
				return false;
			}

			tempRow = currentRow;
			tempColumn = currentColumn;
			Reset();
			SetError(ErrorCode::InvalidSyntax);
			row = tempRow;
			column = tempColumn;
			return false;
		}
	}

	template <typename TCharactersWriter>
	inline bool Inspector<TCharactersWriter>::ParseEndTag()
	{
		// currentCharacter == Slash.
		if (NextCharBad(true))
			return false;

		SizeType tempRow = currentRow;
		SizeType tempColumn = currentColumn;

		if (currentCharacter == Colon)
		{
			Reset();
			SetError(ErrorCode::InvalidTagName);
			row = tempRow;
			column = tempColumn;
			return false;
		}
		else if (!Encoding::CharactersReader::IsNameStartChar(currentCharacter))
		{
			if (Encoding::CharactersReader::IsNameChar(currentCharacter))
			{
				// Not allowed as start character of the name,
				// but allowed as a part of this name.
				Reset();
				SetError(ErrorCode::InvalidTagName);
			}
			else
			{
				// Some weird character.
				Reset();
				SetError(ErrorCode::InvalidSyntax);
			}
			row = tempRow;
			column = tempColumn;
			return false;
		}

		PrepareNode();

		// End element name.
		do
		{
			CharactersWriterType::WriteCharacter(name, currentCharacter);
			CharactersWriterType::WriteCharacter(localName, currentCharacter);

			if (NextCharBad(true))
				return false;

			if (currentCharacter == Colon)
			{
				// Prefixed name.
				prefix = name;
				localName.clear();
				CharactersWriterType::WriteCharacter(name, currentCharacter);

				if (NextCharBad(true))
					return false;

				if (currentCharacter == Colon ||
					!Encoding::CharactersReader::IsNameStartChar(currentCharacter))
				{
					Reset();
					SetError(ErrorCode::InvalidTagName);
					row = tempRow;
					column = tempColumn;
					return false;
				}

				do
				{
					CharactersWriterType::WriteCharacter(name, currentCharacter);
					CharactersWriterType::WriteCharacter(localName, currentCharacter);

					if (NextCharBad(true))
						return false;

					if (currentCharacter == Colon)
					{
						Reset();
						SetError(ErrorCode::InvalidTagName);
						row = tempRow;
						column = tempColumn;
						return false;
					}
				}
				while (Encoding::CharactersReader::IsNameChar(currentCharacter));
				break;
			}
		}
		while (Encoding::CharactersReader::IsNameChar(currentCharacter));

		if (IsWhiteSpace(currentCharacter))
		{
			// Ignore white spaces.
			do
			{
				if (NextCharBad(true))
					return false;
			}
			while (IsWhiteSpace(currentCharacter));

			if (currentCharacter != GreaterThan)
			{
				tempRow = currentRow;
				tempColumn = currentColumn;
				Reset();
				SetError(ErrorCode::InvalidSyntax);
				row = tempRow;
				column = tempColumn;
				return false;
			}
		}
		else if (currentCharacter != GreaterThan)
		{
			Reset();
			SetError(ErrorCode::InvalidTagName);
			row = tempRow;
			column = tempColumn;
			return false;
		}

		if (unclosedTagsSize == 0 ||
			unclosedTags[unclosedTagsSize - 1].Name != name)
		{
			tempRow = row;
			tempColumn = column;
			Reset();
			SetError(ErrorCode::UnexpectedEndTag);
			row = tempRow;
			column = tempColumn;
			return false;
		}
		namespaceUri = unclosedTags[unclosedTagsSize - 1].NamespaceUri;

		// Tag is closed.
		--unclosedTagsSize;

		// Namespaces associated with this tag are no longer needed.
		SizeType indicesToRemove = static_cast<SizeType>(unclosedTagsSize);
		NamespacesSizeType newNamespacesSize = 0;
		while (newNamespacesSize < namespacesSize)
		{
			if (namespaces[newNamespacesSize].TagIndex == indicesToRemove)
				break;
			++newNamespacesSize;
		}
		namespacesSize = newNamespacesSize;
		node = Inspected::EndTag;
		return true;
	}

	template <typename TCharactersWriter>
	inline bool Inspector<TCharactersWriter>::ParseText()
	{
		// currentCharacter == first character of text.

		SizeType tempRow;
		SizeType tempColumn;

		PrepareNode();

		if (IsWhiteSpace(currentCharacter))
		{
			do
			{
				CharactersWriterType::WriteCharacter(value, currentCharacter);
				
				if (NextCharBad(false))
				{
					if (eof)
					{
						if (unclosedTagsSize != 0)
						{
							UnclosedTagType& ref = unclosedTags[unclosedTagsSize - 1];
							Reset();
							SetError(ErrorCode::UnclosedTag);
							row = ref.Row;
							column = ref.Column;
							eof = true;
							return false;
						}

						node = Inspected::Whitespace;
						return true;
					}
					return false;
				}
			}
			while (IsWhiteSpace(currentCharacter));

			if (currentCharacter == LessThan)
			{
				node = Inspected::Whitespace;
				return true;
			}
		}

		if (unclosedTagsSize == 0)
		{
			tempRow = currentRow;
			tempColumn = currentColumn;
			Reset();
			SetError(ErrorCode::InvalidSyntax);
			row = tempRow;
			column = tempColumn;
			return false;
		}

		bool onlyWhite = true;
		do
		{
			if (currentCharacter == Ampersand)
			{
				if (NextCharBad(false))
				{
					if (eof)
					{
						UnclosedTagType& ref = unclosedTags[unclosedTagsSize - 1];
						Reset();
						SetError(ErrorCode::UnclosedTag);
						row = ref.Row;
						column = ref.Column;
						eof = true;
					}
					return false;
				}

				if (currentCharacter == Hash)
				{
					// "&#"

					char32_t codePoint;

					if (!ParseCharacterReference(codePoint, false))
						return false;

					// currentCharacter == Semicolon.

					if (!Encoding::CharactersReader::IsWhiteSpace(codePoint))
						onlyWhite = false;
					CharactersWriterType::WriteCharacter(value, codePoint);
					if (NextCharBad(false))
					{
						if (eof)
						{
							UnclosedTagType& ref = unclosedTags[unclosedTagsSize - 1];
							Reset();
							SetError(ErrorCode::UnclosedTag);
							row = ref.Row;
							column = ref.Column;
							eof = true;
						}
						return false;
					}
					continue;
				}
				else if (currentCharacter != Colon &&
					Encoding::CharactersReader::IsNameStartChar(currentCharacter))
				{
					int resultParsing = ParseEntityReference(false);
					if (resultParsing == -1)
					{
						// Error.
						return false;
					}
					else if (resultParsing == 0)
					{
						// Unknown entity reference.
						if (value.empty())
						{
							name = entityName;
							localName = entityName;
							entityName.clear();
							node = Inspected::EntityReference;
							return true;
						}
						else
						{
							// entityName field is set,
							// but first I must return some text.
							if (onlyWhite)
								node = Inspected::Whitespace;
							else
								node = Inspected::Text;
							return true;
						}
					}
					else // resultParsing == 1.
					{
						// Predefined entity reference.
						onlyWhite = false;
						CharactersWriterType::WriteCharacter(value, currentCharacter);
						if (NextCharBad(false))
						{
							if (eof)
							{
								UnclosedTagType& ref = unclosedTags[unclosedTagsSize - 1];
								Reset();
								SetError(ErrorCode::UnclosedTag);
								row = ref.Row;
								column = ref.Column;
								eof = true;
							}
							return false;
						}
						continue;
					}
				}
				else
				{
					tempRow = currentRow;
					tempColumn = currentColumn - 1;
					Reset();
					SetError(ErrorCode::InvalidReferenceSyntax);
					row = tempRow;
					column = tempColumn;
					return false;
				}
			}

			if (currentCharacter == RightSquareBracket)
			{
				// "]]>" is not allowed here.
				SizeType bracketCount = 0;
				onlyWhite = false;
				do
				{
					++bracketCount;
					CharactersWriterType::WriteCharacter(value, currentCharacter);

					if (NextCharBad(false))
					{
						if (eof)
						{
							UnclosedTagType& ref = unclosedTags[unclosedTagsSize - 1];
							Reset();
							SetError(ErrorCode::UnclosedTag);
							row = ref.Row;
							column = ref.Column;
							eof = true;
						}
						return false;
					}
				}
				while (currentCharacter == RightSquareBracket);

				if (currentCharacter == GreaterThan && bracketCount > 1)
				{
					tempRow = currentRow;
					tempColumn = currentColumn - 2;
					Reset();
					SetError(ErrorCode::InvalidSyntax);
					row = tempRow;
					column = tempColumn;
					return false;
				}
				continue;
			}

			if (!IsWhiteSpace(currentCharacter))
				onlyWhite = false;
			CharactersWriterType::WriteCharacter(value, currentCharacter);
			if (NextCharBad(false))
			{
				if (eof)
				{
					UnclosedTagType& ref = unclosedTags[unclosedTagsSize - 1];
					Reset();
					SetError(ErrorCode::UnclosedTag);
					row = ref.Row;
					column = ref.Column;
					eof = true;
				}
				return false;
			}
		}
		while (currentCharacter != LessThan);

		if (!onlyWhite)
			node = Inspected::Text;
		else
			node = Inspected::Whitespace;

		return true;
	}

	template <typename TCharactersWriter>
	inline bool Inspector<TCharactersWriter>::ParseQuestion()
	{
		// currentCharacter == Question.
		// XmlDeclaration or ProcessingInstruction.

		SizeType tempRow;
		SizeType tempColumn;

		PrepareNode();

		// '<?'
		if (NextCharBad(true))
			return false;
		if (currentCharacter == LowerXml[0] ||
			currentCharacter == UpperXml[0])
		{
			// '<?' ('X' | 'x')
			CharactersWriterType::WriteCharacter(name, currentCharacter);
			if (NextCharBad(true))
				return false;
			if (currentCharacter == LowerXml[1] ||
				currentCharacter == UpperXml[1])
			{
				// '<?' ('X' | 'x') ('M' | 'm')
				CharactersWriterType::WriteCharacter(name, currentCharacter);
				if (NextCharBad(true))
					return false;
				if (currentCharacter == LowerXml[2] ||
					currentCharacter == UpperXml[2])
				{
					// '<?' ('X' | 'x') ('M' | 'm') ('L' | 'l')
					CharactersWriterType::WriteCharacter(name, currentCharacter);
					if (NextCharBad(true))
						return false;

					if (name == lowerXmlString)
					{
						// '<?xml' Char
						if (IsWhiteSpace(currentCharacter))
						{
							// '<?xml '
							localName = name;
							return ParseXmlDeclaration();	
						}
					}

					if (currentCharacter == Colon ||
						!Encoding::CharactersReader::IsNameChar(currentCharacter))
					{
						tempRow = currentRow;
						tempColumn = currentColumn;
						Reset();
						SetError(ErrorCode::InvalidSyntax);
						row = tempRow;
						column = tempColumn;
						return false;
					}
				}
			}
		}
		else if (currentCharacter == Colon ||
			!Encoding::CharactersReader::IsNameStartChar(currentCharacter))
		{
			tempRow = currentRow;
			tempColumn = currentColumn;
			Reset();
			SetError(ErrorCode::InvalidSyntax);
			row = tempRow;
			column = tempColumn;
			return false;
		}
		else
		{
			// '<?' (NameStartChar - ':')
			CharactersWriterType::WriteCharacter(name, currentCharacter);
			if (NextCharBad(true))
				return false;
		}

		while (currentCharacter != Colon &&
			Encoding::CharactersReader::IsNameChar(currentCharacter))
		{
			CharactersWriterType::WriteCharacter(name, currentCharacter);
			if (NextCharBad(true))
				return false;
		}

		// '<?' PITarget (':' | (Char - NameChar))

		return ParseProcessingInstruction();
	}

	template <typename TCharactersWriter>
	inline bool Inspector<TCharactersWriter>::ParseXmlDeclaration()
	{
		// '<?xml '

		SizeType tempRow;
		SizeType tempColumn;

		if (row != 1 || column != 1)
		{
			tempRow = row;
			tempColumn = column;
			Reset();
			SetError(ErrorCode::InvalidXmlDeclarationLocation);
			row = tempRow;
			column = tempColumn;
			return false;
		}

		// Ignore white spaces.
		do
		{
			if (NextCharBad(true))
				return false;
		}
		while (IsWhiteSpace(currentCharacter));

		AttributeType& versionAttr = NewAttribute();
		versionAttr.Row = currentRow;
		versionAttr.Column = currentColumn;

		for (std::size_t i = 0; i < 7; ++i)
		{
			if (currentCharacter != XmlDeclarationVersion[i])
			{
				tempRow = currentRow;
				tempColumn = currentColumn;
				Reset();
				SetError(ErrorCode::InvalidSyntax);
				row = tempRow;
				column = tempColumn;
				return false;
			}
			CharactersWriterType::WriteCharacter(versionAttr.Name, currentCharacter);
			CharactersWriterType::WriteCharacter(versionAttr.LocalName, currentCharacter);
			if (NextCharBad(true))
				return false;
		}

		// '<?xml version' Char
		
		while (IsWhiteSpace(currentCharacter))
		{
			if (NextCharBad(true))
				return false;
		}

		if (currentCharacter != Equals)
		{
			tempRow = currentRow;
			tempColumn = currentColumn;
			Reset();
			SetError(ErrorCode::InvalidSyntax);
			row = tempRow;
			column = tempColumn;
			return false;
		}

		// '<?xml version='

		if (NextCharBad(true))
			return false;

		while (IsWhiteSpace(currentCharacter))
		{
			if (NextCharBad(true))
				return false;
		}

		char32_t quoteChar = currentCharacter;

		if (quoteChar == DoubleQuote)
		{
			// <?xml version="
			versionAttr.Delimiter = QuotationMark::DoubleQuote;
		}
		else if (quoteChar == SingleQuote)
		{
			// <?xml version='
			versionAttr.Delimiter = QuotationMark::SingleQuote;
		}
		else
		{
			tempRow = currentRow;
			tempColumn = currentColumn;
			Reset();
			SetError(ErrorCode::InvalidSyntax);
			row = tempRow;
			column = tempColumn;
			return false;
		}

		if (NextCharBad(true))
			return false;

		// Now should be:
		// '1.' [0-9]+

		if (Encoding::CharactersReader::GetHexDigitValue(currentCharacter) != 1)
		{
			tempRow = currentRow;
			tempColumn = currentColumn;
			Reset();
			SetError(ErrorCode::InvalidSyntax);
			row = tempRow;
			column = tempColumn;
			return false;
		}

		// <?xml version="1
		CharactersWriterType::WriteCharacter(versionAttr.Value, currentCharacter);

		if (NextCharBad(true))
			return false;

		if (currentCharacter != Dot)
		{
			tempRow = currentRow;
			tempColumn = currentColumn;
			Reset();
			SetError(ErrorCode::InvalidSyntax);
			row = tempRow;
			column = tempColumn;
			return false;
		}

		// <?xml version="1.
		CharactersWriterType::WriteCharacter(versionAttr.Value, currentCharacter);

		if (NextCharBad(true))
			return false;

		int digit = Encoding::CharactersReader::GetHexDigitValue(currentCharacter);

		if (digit < 0 || digit > 9)
		{
			// After a dot should be at least one digit.
			tempRow = currentRow;
			tempColumn = currentColumn;
			Reset();
			SetError(ErrorCode::InvalidSyntax);
			row = tempRow;
			column = tempColumn;
			return false;
		}

		do
		{
			CharactersWriterType::WriteCharacter(versionAttr.Value, currentCharacter);
			if (NextCharBad(true))
				return false;
			digit = Encoding::CharactersReader::GetHexDigitValue(currentCharacter);
		}
		while (digit >= 0 && digit <= 9);

		if (currentCharacter != quoteChar)
		{
			tempRow = currentRow;
			tempColumn = currentColumn;
			Reset();
			SetError(ErrorCode::InvalidSyntax);
			row = tempRow;
			column = tempColumn;
			return false;
		}

		// <?xml version="1.x"

		if (NextCharBad(true))
			return false;

		if (IsWhiteSpace(currentCharacter))
		{
			do
			{
				if (NextCharBad(true))
					return false;
			}
			while (IsWhiteSpace(currentCharacter));

			if (currentCharacter == Question)
			{
				// <?xml version="1.x"   ?
				if (NextCharBad(true))
					return false;

				if (currentCharacter != GreaterThan)
				{
					tempRow = currentRow;
					tempColumn = currentColumn;
					Reset();
					SetError(ErrorCode::InvalidSyntax);
					row = tempRow;
					column = tempColumn;
					return false;
				}

				// <?xml version="1.x"   ?>
				if (sourceType != SourceReader &&
					bom != Details::Bom::None &&
					bom != Details::Bom::Utf8 &&
					bom != Details::Bom::Utf16BE &&
					bom != Details::Bom::Utf16LE)
				{
					Reset();
					SetError(ErrorCode::EncodingDeclarationRequired);
					row = 1;
					column = 1;
					return false;
				}

				node = Inspected::XmlDeclaration;
				return true;
			}
		}
		else // IsWhiteSpace(currentCharacter) == false
		{
			if (currentCharacter != Question)
			{
				tempRow = currentRow;
				tempColumn = currentColumn;
				Reset();
				SetError(ErrorCode::InvalidSyntax);
				row = tempRow;
				column = tempColumn;
				return false;
			}

			// <?xml version="1.x"?
			if (NextCharBad(true))
				return false;

			if (currentCharacter != GreaterThan)
			{
				tempRow = currentRow;
				tempColumn = currentColumn;
				Reset();
				SetError(ErrorCode::InvalidSyntax);
				row = tempRow;
				column = tempColumn;
				return false;
			}

			// <?xml version="1.x"?>
			if (sourceType != SourceReader &&
				bom != Details::Bom::None &&
				bom != Details::Bom::Utf8 &&
				bom != Details::Bom::Utf16BE &&
				bom != Details::Bom::Utf16LE)
			{
				Reset();
				SetError(ErrorCode::EncodingDeclarationRequired);
				row = 1;
				column = 1;
				return false;
			}

			node = Inspected::XmlDeclaration;
			return true;
		}

		// Now should be an encoding or standalone attribute.
		if (currentCharacter == XmlDeclarationEncoding[0])
		{
			// encoding
			AttributeType& encodingAttr = NewAttribute();
			encodingAttr.Row = currentRow;
			encodingAttr.Column = currentColumn;
			comparingName.clear(); // Could be not empty after call of Reset method.
			comparingName.reserve(NameReserve);

			for (std::size_t i = 0; i < 8; ++i)
			{
				if (currentCharacter != XmlDeclarationEncoding[i])
				{
					tempRow = currentRow;
					tempColumn = currentColumn;
					Reset();
					SetError(ErrorCode::InvalidSyntax);
					row = tempRow;
					column = tempColumn;
					return false;
				}
				CharactersWriterType::WriteCharacter(encodingAttr.Name, currentCharacter);
				CharactersWriterType::WriteCharacter(encodingAttr.LocalName, currentCharacter);
				if (NextCharBad(true))
					return false;
			}

			// '<?xml version="1.x" encoding' Char
			
			while (IsWhiteSpace(currentCharacter))
			{
				if (NextCharBad(true))
					return false;
			}

			if (currentCharacter != Equals)
			{
				tempRow = currentRow;
				tempColumn = currentColumn;
				Reset();
				SetError(ErrorCode::InvalidSyntax);
				row = tempRow;
				column = tempColumn;
				return false;
			}

			// '<?xml version="1.x" encoding='

			if (NextCharBad(true))
				return false;

			while (IsWhiteSpace(currentCharacter))
			{
				if (NextCharBad(true))
					return false;
			}

			quoteChar = currentCharacter;

			if (quoteChar == DoubleQuote)
			{
				// <?xml version="1.x" encoding="
				encodingAttr.Delimiter = QuotationMark::DoubleQuote;
			}
			else if (quoteChar == SingleQuote)
			{
				// <?xml version="1.x" encoding='
				encodingAttr.Delimiter = QuotationMark::SingleQuote;
			}
			else
			{
				tempRow = currentRow;
				tempColumn = currentColumn;
				Reset();
				SetError(ErrorCode::InvalidSyntax);
				row = tempRow;
				column = tempColumn;
				return false;
			}

			if (NextCharBad(true))
				return false;

			if (!Encoding::CharactersReader::IsEncNameStartChar(currentCharacter))
			{
				tempRow = currentRow;
				tempColumn = currentColumn;
				Reset();
				SetError(ErrorCode::InvalidSyntax);
				row = tempRow;
				column = tempColumn;
				return false;
			}

			do
			{
				CharactersWriterType::WriteCharacter(encodingAttr.Value, currentCharacter);
				comparingName.push_back(currentCharacter);
				if (NextCharBad(true))
					return false;
			}
			while (Encoding::CharactersReader::IsEncNameChar(currentCharacter));

			if (currentCharacter != quoteChar)
			{
				tempRow = currentRow;
				tempColumn = currentColumn;
				Reset();
				SetError(ErrorCode::InvalidSyntax);
				row = tempRow;
				column = tempColumn;
				return false;
			}

			// '<?xml' VersionInfo EncodingDecl
			if (sourceType != SourceReader && !ResolveEncoding(encodingAttr))
				return false;
			if (NextCharBad(true))
				return false;
			
			if (IsWhiteSpace(currentCharacter))
			{
				do
				{
					if (NextCharBad(true))
						return false;
				}
				while (IsWhiteSpace(currentCharacter));

				if (currentCharacter == Question)
				{
					// '<?xml' VersionInfo EncodingDecl S '?'
					if (NextCharBad(true))
						return false;

					if (currentCharacter != GreaterThan)
					{
						tempRow = currentRow;
						tempColumn = currentColumn;
						Reset();
						SetError(ErrorCode::InvalidSyntax);
						row = tempRow;
						column = tempColumn;
						return false;
					}

					// '<?xml' VersionInfo EncodingDecl S '?>'
					node = Inspected::XmlDeclaration;
					return true;
				}
			}
			else // IsWhiteSpace(currentCharacter) == false
			{
				if (currentCharacter != Question)
				{
					tempRow = currentRow;
					tempColumn = currentColumn;
					Reset();
					SetError(ErrorCode::InvalidSyntax);
					row = tempRow;
					column = tempColumn;
					return false;
				}

				if (NextCharBad(true))
					return false;

				if (currentCharacter != GreaterThan)
				{
					tempRow = currentRow;
					tempColumn = currentColumn;
					Reset();
					SetError(ErrorCode::InvalidSyntax);
					row = tempRow;
					column = tempColumn;
					return false;
				}

				// '<?xml' VersionInfo EncodingDecl '?>'
				node = Inspected::XmlDeclaration;
				return true;
			}
		}
		else
		{
			if (sourceType != SourceReader &&
				bom != Details::Bom::None &&
				bom != Details::Bom::Utf8 &&
				bom != Details::Bom::Utf16BE &&
				bom != Details::Bom::Utf16LE)
			{
				Reset();
				SetError(ErrorCode::EncodingDeclarationRequired);
				row = 1;
				column = 1;
				return false;
			}
		}

		// Now should be a standalone attribute.
		AttributeType& standaloneAttr = NewAttribute();
		standaloneAttr.Row = currentRow;
		standaloneAttr.Column = currentColumn;

		for (std::size_t i = 0; i < 10; ++i)
		{
			if (currentCharacter != XmlDeclarationStandalone[i])
			{
				tempRow = currentRow;
				tempColumn = currentColumn;
				Reset();
				SetError(ErrorCode::InvalidSyntax);
				row = tempRow;
				column = tempColumn;
				return false;
			}
			CharactersWriterType::WriteCharacter(standaloneAttr.Name, currentCharacter);
			CharactersWriterType::WriteCharacter(standaloneAttr.LocalName, currentCharacter);
			if (NextCharBad(true))
				return false;
		}

		// '<?xml' VersionInfo EncodingDecl S 'standalone' Char
		
		while (IsWhiteSpace(currentCharacter))
		{
			if (NextCharBad(true))
				return false;
		}

		if (currentCharacter != Equals)
		{
			tempRow = currentRow;
			tempColumn = currentColumn;
			Reset();
			SetError(ErrorCode::InvalidSyntax);
			row = tempRow;
			column = tempColumn;
			return false;
		}

		// '<?xml' VersionInfo EncodingDecl S 'standalone' S? '='

		if (NextCharBad(true))
			return false;

		while (IsWhiteSpace(currentCharacter))
		{
			if (NextCharBad(true))
				return false;
		}

		quoteChar = currentCharacter;

		if (quoteChar == DoubleQuote)
		{
			// '<?xml' VersionInfo EncodingDecl S 'standalone' Eq '"'
			standaloneAttr.Delimiter = QuotationMark::DoubleQuote;
		}
		else if (quoteChar == SingleQuote)
		{
			// '<?xml' VersionInfo EncodingDecl S 'standalone' Eq "'"
			standaloneAttr.Delimiter = QuotationMark::SingleQuote;
		}
		else
		{
			tempRow = currentRow;
			tempColumn = currentColumn;
			Reset();
			SetError(ErrorCode::InvalidSyntax);
			row = tempRow;
			column = tempColumn;
			return false;
		}

		if (NextCharBad(true))
			return false;

		if (currentCharacter == Yes[0])
		{
			CharactersWriterType::WriteCharacter(standaloneAttr.Value, currentCharacter);
			if (NextCharBad(true))
				return false;
			if (currentCharacter == Yes[1])
			{
				CharactersWriterType::WriteCharacter(standaloneAttr.Value, currentCharacter);
				if (NextCharBad(true))
					return false;
				if (currentCharacter == Yes[2])
				{
					CharactersWriterType::WriteCharacter(standaloneAttr.Value, currentCharacter);
					if (NextCharBad(true))
						return false;
					if (currentCharacter == quoteChar)
					{
						// '<?xml' VersionInfo EncodingDecl SDDecl
						if (NextCharBad(true))
							return false;
						while (IsWhiteSpace(currentCharacter))
						{
							if (NextCharBad(true))
								return false;
						}
						if (currentCharacter != Question)
						{
							tempRow = currentRow;
							tempColumn = currentColumn;
							Reset();
							SetError(ErrorCode::InvalidSyntax);
							row = tempRow;
							column = tempColumn;
							return false;
						}
						if (NextCharBad(true))
							return false;
						if (currentCharacter != GreaterThan)
						{
							tempRow = currentRow;
							tempColumn = currentColumn;
							Reset();
							SetError(ErrorCode::InvalidSyntax);
							row = tempRow;
							column = tempColumn;
							return false;
						}

						// '<?xml' VersionInfo EncodingDecl SDDecl '?>'
						node = Inspected::XmlDeclaration;
						return true;
					}
				}
			}
		}
		else if (currentCharacter == No[0])
		{
			CharactersWriterType::WriteCharacter(standaloneAttr.Value, currentCharacter);
			if (NextCharBad(true))
				return false;
			if (currentCharacter == No[1])
			{
				CharactersWriterType::WriteCharacter(standaloneAttr.Value, currentCharacter);
				if (NextCharBad(true))
					return false;
				if (currentCharacter == quoteChar)
				{
					// '<?xml' VersionInfo EncodingDecl SDDecl
					if (NextCharBad(true))
						return false;
					while (IsWhiteSpace(currentCharacter))
					{
						if (NextCharBad(true))
							return false;
					}
					if (currentCharacter != Question)
					{
						tempRow = currentRow;
						tempColumn = currentColumn;
						Reset();
						SetError(ErrorCode::InvalidSyntax);
						row = tempRow;
						column = tempColumn;
						return false;
					}
					if (NextCharBad(true))
						return false;
					if (currentCharacter != GreaterThan)
					{
						tempRow = currentRow;
						tempColumn = currentColumn;
						Reset();
						SetError(ErrorCode::InvalidSyntax);
						row = tempRow;
						column = tempColumn;
						return false;
					}

					// '<?xml' VersionInfo EncodingDecl SDDecl '?>'
					node = Inspected::XmlDeclaration;
					return true;
				}
			}
		}

		tempRow = currentRow;
		tempColumn = currentColumn;
		Reset();
		SetError(ErrorCode::InvalidSyntax);
		row = tempRow;
		column = tempColumn;
		return false;
	}

	template <typename TCharactersWriter>
	inline bool Inspector<TCharactersWriter>::ParseProcessingInstruction()
	{
		// '<?' PITarget (':' | (Char - NameChar))
		// name == PITarget
		// localName.empty() == true

		SizeType tempRow;
		SizeType tempColumn;

		if (currentCharacter == Question)
		{
			// '<?' PITarget '?'
			if (NextCharBad(true))
				return false;
			if (currentCharacter != GreaterThan)
			{
				tempRow = currentRow;
				tempColumn = currentColumn;
				Reset();
				SetError(ErrorCode::InvalidSyntax);
				row = tempRow;
				column = tempColumn;
				return false;
			}

			// '<?' PITarget '?>'
			localName = name;
			node = Inspected::ProcessingInstruction;
			return true;
		}

		if (!IsWhiteSpace(currentCharacter))
		{
			tempRow = currentRow;
			tempColumn = currentColumn;
			Reset();
			SetError(ErrorCode::InvalidSyntax);
			row = tempRow;
			column = tempColumn;
			return false;
		}

		localName = name;

		// Ignore white spaces.
		do
		{
			if (NextCharBad(true))
				return false;
		}
		while (IsWhiteSpace(currentCharacter));

		do // {...} while (true);
		{
			if (currentCharacter == Question)
			{
				if (NextCharBad(true))
					return false;
				if (currentCharacter == GreaterThan)
				{
					node = Inspected::ProcessingInstruction;
					return true;
				}
				CharactersWriterType::WriteCharacter(value, Question);
			}
			else
			{
				CharactersWriterType::WriteCharacter(value, currentCharacter);
				if (NextCharBad(true))
					return false;
			}
		}
		while (true);

		// Should never happen.
		return false;
	}

	template <typename TCharactersWriter>
	inline bool Inspector<TCharactersWriter>::ParseExclamation()
	{
		// currentCharacter == Exclamation.
		// Comment, CDATA or DocumentType.

		SizeType tempRow;
		SizeType tempColumn;

		if (NextCharBad(true))
			return false;

		if (currentCharacter == Minus)
		{
			// <!-
			// Looks like a comment.
			if (NextCharBad(true))
				return false;
			if (currentCharacter != Minus)
			{
				tempRow = currentRow;
				tempColumn = currentColumn;
				Reset();
				SetError(ErrorCode::InvalidSyntax);
				row = tempRow;
				column = tempColumn;
				return false;
			}

			return ParseComment();
		}

		if (currentCharacter == LeftSquareBracket)
		{
			// <![
			// Looks like CDATA section.
			for (std::size_t i = 0; i < 5; ++i)
			{
				if (NextCharBad(true))
					return false;
				if (currentCharacter != CDATA[i])
				{
					tempRow = currentRow;
					tempColumn = currentColumn;
					Reset();
					SetError(ErrorCode::InvalidSyntax);
					row = tempRow;
					column = tempColumn;
					return false;
				}
			}
			// <![CDATA
			if (NextCharBad(true))
				return false;
			if (currentCharacter != LeftSquareBracket)
			{
				tempRow = currentRow;
				tempColumn = currentColumn;
				Reset();
				SetError(ErrorCode::InvalidSyntax);
				row = tempRow;
				column = tempColumn;
				return false;
			}

			return ParseCDATA();
		}

		// <!currentCharacter
		// Should be DOCTYPE declaration.
		for (std::size_t i = 0; i < 7; ++i)
		{
			if (currentCharacter != DOCTYPE[i])
			{
				tempRow = currentRow;
				tempColumn = currentColumn;
				Reset();
				SetError(ErrorCode::InvalidSyntax);
				row = tempRow;
				column = tempColumn;
				return false;
			}
			if (NextCharBad(true))
				return false;
		}
		return ParseDOCTYPE();
	}

	template <typename TCharactersWriter>
	inline bool Inspector<TCharactersWriter>::ParseComment()
	{
		// currentCharacter == Minus.

		SizeType tempRow;
		SizeType tempColumn;

		PrepareNode();

		bool doubleMinus = false;
		do
		{
			if (NextCharBad(true))
				return false;
			if (currentCharacter == Minus)
			{
				// <!-- -
				if (NextCharBad(true))
					return false;
				if (currentCharacter == Minus)
				{
					// <!-- --
					if (NextCharBad(true))
						return false;
					doubleMinus = true;
				}
				else
				{
					CharactersWriterType::WriteCharacter(value, Minus);
					CharactersWriterType::WriteCharacter(value, currentCharacter);
				}
			}
			else // Not minus.
			{
				CharactersWriterType::WriteCharacter(value, currentCharacter);
			}
		}
		while (!doubleMinus);

		if (currentCharacter != GreaterThan)
		{
			tempRow = currentRow;
			tempColumn = currentColumn - 2;
			Reset();
			SetError(ErrorCode::InvalidSyntax);
			row = tempRow;
			column = tempColumn;
			return false;
		}

		node = Inspected::Comment;
		return true;
	}

	template <typename TCharactersWriter>
	inline bool Inspector<TCharactersWriter>::ParseCDATA()
	{
		// currentCharacter == LeftSquareBracket.
		// <![CDATA[

		if (unclosedTagsSize == 0)
		{
			SizeType tempRow = row;
			SizeType tempColumn = column;
			Reset();
			SetError(ErrorCode::CDataSectionOutside);
			row = tempRow;
			column = tempColumn;
			return false;
		}

		PrepareNode();

		do
		{
			if (NextCharBad(true))
				return false;
			if (currentCharacter == RightSquareBracket)
			{
				// <![CDATA[ text ]
				SizeType bracketCount = 0;
				do
				{
					++bracketCount;
					if (NextCharBad(true))
						return false;
				}
				while (currentCharacter == RightSquareBracket);

				if (currentCharacter == GreaterThan && bracketCount >= 2)
				{
					// <![CDATA[ text ]]>
					bracketCount -= 2;
					for (SizeType i = 0; i < bracketCount; ++i)
						CharactersWriterType::WriteCharacter(value, RightSquareBracket);
					node = Inspected::CDATA;
					return true;
				}
				else
				{
					for (SizeType i = 0; i < bracketCount; ++i)
						CharactersWriterType::WriteCharacter(value, RightSquareBracket);
					CharactersWriterType::WriteCharacter(value, currentCharacter);
				}
			}
			else
			{
				CharactersWriterType::WriteCharacter(value, currentCharacter);
			}
		}
		while (true);

		// Should never happen.
		return false;
	}

	template <typename TCharactersWriter>
	inline bool Inspector<TCharactersWriter>::ParseDOCTYPE()
	{
		// <!DOCTYPEcurrentCharacter

		SizeType tempRow;
		SizeType tempColumn;

		if (!IsWhiteSpace(currentCharacter))
		{
			tempRow = currentRow;
			tempColumn = currentColumn;
			Reset();
			SetError(ErrorCode::InvalidSyntax);
			row = tempRow;
			column = tempColumn;
			return false;
		}

		if (foundElement)
		{
			tempRow = row;
			tempColumn = column;
			Reset();
			SetError(ErrorCode::InvalidDocumentTypeDeclarationLocation);
			row = tempRow;
			column = tempColumn;
			return false;
		}

		if (foundDOCTYPE)
		{
			tempRow = row;
			tempColumn = column;
			Reset();
			SetError(ErrorCode::DoubleDocumentTypeDeclaration);
			row = tempRow;
			column = tempColumn;
			return false;
		}
		foundDOCTYPE = true;

		PrepareNode();

		// Ignore white spaces.
		do
		{
			if (NextCharBad(true))
				return false;
		}
		while (IsWhiteSpace(currentCharacter));

		// <!DOCTYPE   currentCharacter
		if (currentCharacter == Colon ||
			!Encoding::CharactersReader::IsNameStartChar(currentCharacter))
		{
			tempRow = currentRow;
			tempColumn = currentColumn;
			Reset();
			SetError(ErrorCode::InvalidSyntax);
			row = tempRow;
			column = tempColumn;
			return false;
		}

		// QName
		do
		{
			CharactersWriterType::WriteCharacter(name, currentCharacter);
			CharactersWriterType::WriteCharacter(localName, currentCharacter);

			if (NextCharBad(true))
				return false;

			if (currentCharacter == Colon)
			{
				// Prefixed name.
				prefix = name;
				localName.clear();
				CharactersWriterType::WriteCharacter(name, currentCharacter);

				if (NextCharBad(true))
					return false;

				if (currentCharacter == Colon ||
					!Encoding::CharactersReader::IsNameStartChar(currentCharacter))
				{
					tempRow = currentRow;
					tempColumn = currentColumn;
					Reset();
					SetError(ErrorCode::InvalidSyntax);
					row = tempRow;
					column = tempColumn;
					return false;
				}

				do
				{
					CharactersWriterType::WriteCharacter(name, currentCharacter);
					CharactersWriterType::WriteCharacter(localName, currentCharacter);

					if (NextCharBad(true))
						return false;

					if (currentCharacter == Colon)
					{
						tempRow = currentRow;
						tempColumn = currentColumn;
						Reset();
						SetError(ErrorCode::InvalidSyntax);
						row = tempRow;
						column = tempColumn;
						return false;
					}
				}
				while (Encoding::CharactersReader::IsNameChar(currentCharacter));
				break;
			}
		}
		while (Encoding::CharactersReader::IsNameChar(currentCharacter));

		// <!DOCTYPE QNAMEcurrentCharacter

		if (currentCharacter == GreaterThan)
		{
			// <!DOCTYPE QName>
			node = Inspected::DocumentType;
			return true;
		}
		else if (IsWhiteSpace(currentCharacter))
		{
			// Ignore white spaces.
			do
			{
				if (NextCharBad(true))
					return false;
			}
			while (IsWhiteSpace(currentCharacter));

			if (currentCharacter == GreaterThan)
			{
				// <!DOCTYPE QName  >
				node = Inspected::DocumentType;
				return true;
			}
		}
		else if (currentCharacter != LeftSquareBracket)
		{
			tempRow = currentRow;
			tempColumn = currentColumn;
			Reset();
			SetError(ErrorCode::InvalidSyntax);
			row = tempRow;
			column = tempColumn;
			return false;
		}

		do
		{
			CharactersWriterType::WriteCharacter(value, currentCharacter);
			if (currentCharacter == LeftSquareBracket)
			{
				// <!DOCTYPE QName [
				do
				{
					if (NextCharBad(true))
						return false;
					while (currentCharacter == RightSquareBracket)
					{
						CharactersWriterType::WriteCharacter(value, currentCharacter);
						if (NextCharBad(true))
							return false;
						while (IsWhiteSpace(currentCharacter))
						{
							CharactersWriterType::WriteCharacter(value, currentCharacter);
							if (NextCharBad(true))
								return false;
						}
						if (currentCharacter == GreaterThan)
						{
							// <!DOCTYPE QName [...] >
							node = Inspected::DocumentType;
							return true;
						}
					}
					CharactersWriterType::WriteCharacter(value, currentCharacter);
				}
				while (true);
			}

			if (NextCharBad(true))
				return false;
		}
		while (currentCharacter != GreaterThan);

		node = Inspected::DocumentType;
		return true;
	}

	template <typename TCharactersWriter>
	inline void Inspector<TCharactersWriter>::PrepareNode()
	{
		name.clear();
		value.clear();
		localName.clear();
		prefix.clear();
		namespaceUri.clear();
		attributesSize = 0;
	}

	template <typename TCharactersWriter>
	inline bool Inspector<TCharactersWriter>::NamespacesStuff()
	{
		// Collect namespaces from attributes.
		typedef typename std::deque<AttributeType>::iterator AttrIter;
		AttrIter attrEnd = attributes.begin() + attributesSize;
		for (AttrIter attr = attributes.begin(); attr != attrEnd; ++attr)
		{
			if (attr->Prefix.empty())
			{
				if (attr->LocalName == xmlnsString)
				{
					// Default namespace.
					// <mytag xmlns=...
					if (attr->Value == xmlUriString ||
						attr->Value == xmlnsUriString)
					{
						// <mytag xmlns="http://www.w3.org/XML/1998/namespace"...
						// or
						// <mytag xmlns="http://www.w3.org/2000/xmlns/"...
						Reset();
						SetError(ErrorCode::ReservedNamespaceAsDefault);
						row = attr->Row;
						column = attr->Column;
						return false;
					}

					NamespaceDeclarationType& ref = NewNamespace();
					ref.Uri = attr->Value;
					ref.TagIndex = static_cast<SizeType>(unclosedTagsSize);
				}
			}
			else if (attr->Prefix == xmlnsString)
			{
				if (attr->LocalName == xmlnsString)
				{
					// <mytag xmlns:xmlns=...
					Reset();
					SetError(ErrorCode::XmlnsDeclared);
					row = attr->Row;
					column = attr->Column;
					return false;
				}
				else if (attr->LocalName == lowerXmlString)
				{
					// <mytag xmlns:xml=...

					if (attr->Value != xmlUriString)
					{
						Reset();
						SetError(ErrorCode::InvalidXmlPrefixDeclaration);
						row = attr->Row;
						column = attr->Column;
						return false;
					}
				}
				else if (attr->Value == xmlUriString ||
					attr->Value == xmlnsUriString)
				{
					// <mytag xmlns:newprefix="http://www.w3.org/XML/1998/namespace"...
					// or
					// <mytag xmlns:newprefix="http://www.w3.org/2000/xmlns/"...
					Reset();
					SetError(ErrorCode::PrefixBoundToReservedNamespace);
					row = attr->Row;
					column = attr->Column;
					return false;
				}
				else if (attr->Value.empty())
				{
					// <mytag xmlns:newprefix=""...
					Reset();
					SetError(ErrorCode::PrefixWithEmptyNamespace);
					row = attr->Row;
					column = attr->Column;
					return false;
				}
				else
				{
					NamespaceDeclarationType& ref = NewNamespace();
					ref.Prefix = attr->LocalName;
					ref.Uri = attr->Value;
					ref.TagIndex = static_cast<SizeType>(unclosedTagsSize);
				}
			}
		}

		// Assign URIs to attributes.
		typedef typename std::deque<NamespaceDeclarationType>::const_iterator NamespaceIter;
		NamespaceIter namespaceLast = namespaces.begin();
		if (namespacesSize != 0)
			namespaceLast += (namespacesSize - 1);
		for (AttrIter attr = attributes.begin(); attr != attrEnd; ++attr)
		{
			if (!attr->Prefix.empty())
			{
				if (attr->Prefix == xmlnsString)
				{
					attr->NamespaceUri = xmlnsUriString;
				}
				else if (attr->Prefix == lowerXmlString)
				{
					attr->NamespaceUri = xmlUriString;
				}
				else
				{
					bool found = false;
					NamespaceIter n = namespaceLast;
					for ( ; n != namespaces.begin(); --n)
					{
						if (attr->Prefix == n->Prefix)
						{
							found = true;
							break;
						}
					}
					if (!found && (namespacesSize == 0 || attr->Prefix != n->Prefix))
					{
						Reset();
						SetError(ErrorCode::PrefixWithoutAssignedNamespace);
						row = attr->Row;
						column = attr->Column;
						return false;
					}
					else
					{
						attr->NamespaceUri = n->Uri;
					}
				}
			}
		}

		// Assign URI to element.
		if (!prefix.empty())
		{
			if (prefix == xmlnsString)
			{
				// row and column => '<'
				SizeType tempRow = row;
				SizeType tempColumn = column + 1;
				Reset();
				SetError(ErrorCode::PrefixWithoutAssignedNamespace);
				row = tempRow;
				column = tempColumn;
				return false;
			}
			else if (prefix == lowerXmlString)
			{
				namespaceUri = xmlUriString;
			}
			else
			{
				bool found = false;
				NamespaceIter n = namespaceLast;
				for ( ; n != namespaces.begin(); --n)
				{
					if (prefix == n->Prefix)
					{
						found = true;
						break;
					}
				}
				if (!found && (namespacesSize == 0 || prefix != n->Prefix))
				{
					// row and column => '<'
					SizeType tempRow = row;
					SizeType tempColumn = column + 1;
					Reset();
					SetError(ErrorCode::PrefixWithoutAssignedNamespace);
					row = tempRow;
					column = tempColumn;
					return false;
				}
				else
				{
					namespaceUri = n->Uri;
				}
			}
		}
		else // prefix.empty() == true.
		{
			// Find default namespace.
			bool found = false;
			NamespaceIter n = namespaceLast;
			for ( ; n != namespaces.begin(); --n)
			{
				if (n->Prefix.empty())
				{
					found = true;
					break;
				}
			}
			if (found || (namespacesSize != 0 && n->Prefix.empty()))
				namespaceUri = n->Uri;
		}

		// Ensure no double attribute name like:
		// <a x:local="first" y:local="second">
		// where x and y prefixes are bound to the same namespace URI.
		if (attributesSize > 1)
		{
			AttrIter attrEndMinus1 = attributes.begin() + (attributesSize - 1);
			for (AttrIter attr = attributes.begin(); attr != attrEndMinus1; ++attr)
			{
				if (!attr->Prefix.empty())
				{
					for (AttrIter next = attr + 1; next != attrEnd; ++next)
					{
						if (attr->LocalName == next->LocalName &&
							attr->NamespaceUri == next->NamespaceUri)
						{
							Reset();
							SetError(ErrorCode::DoubleAttributeName);
							row = next->Row;
							column = next->Column;
							return false;
						}
					}
				}
			}
		}

		return true;
	}

	template <typename TCharactersWriter>
	inline bool Inspector<TCharactersWriter>::ParseCharacterReference(char32_t& result, bool insideTag)
	{
		// currentCharacter == Hash.
		
		SizeType tempRow = currentRow;
		SizeType tempColumn = currentColumn - 1;

		if (NextCharBad(insideTag))
		{
			if (!insideTag && eof)
			{
				UnclosedTagType& ref = unclosedTags[unclosedTagsSize - 1];
				Reset();
				SetError(ErrorCode::UnclosedTag);
				row = ref.Row;
				column = ref.Column;
				eof = true;
			}
			return false;
		}

		const int BufferSize = 7;
		unsigned char buffer[BufferSize];
		int digitCount = 0;
		int digit = 0;
		int radix = 10;

		if (currentCharacter == X)
		{
			// Hexadecimal representation of the character's code point.
			radix = 16;
			if (NextCharBad(insideTag))
			{
				if (!insideTag && eof)
				{
					UnclosedTagType& ref = unclosedTags[unclosedTagsSize - 1];
					Reset();
					SetError(ErrorCode::UnclosedTag);
					row = ref.Row;
					column = ref.Column;
					eof = true;
				}
				return false;
			}
		}

		digit = Encoding::CharactersReader::GetHexDigitValue(currentCharacter);

		// Ignore leading zeros.
		bool leadingZeros = false;
		while (digit == 0)
		{
			leadingZeros = true;
			if (NextCharBad(insideTag))
			{
				if (!insideTag && eof)
				{
					UnclosedTagType& ref = unclosedTags[unclosedTagsSize - 1];
					Reset();
					SetError(ErrorCode::UnclosedTag);
					row = ref.Row;
					column = ref.Column;
					eof = true;
				}
				return false;
			}
			digit = Encoding::CharactersReader::GetHexDigitValue(currentCharacter);
		}

		while (digitCount < BufferSize && digit >= 0 && digit < radix)
		{
			buffer[digitCount++] = static_cast<unsigned char>(digit);
			if (NextCharBad(insideTag))
			{
				if (!insideTag && eof)
				{
					UnclosedTagType& ref = unclosedTags[unclosedTagsSize - 1];
					Reset();
					SetError(ErrorCode::UnclosedTag);
					row = ref.Row;
					column = ref.Column;
					eof = true;
				}
				return false;
			}
			digit = Encoding::CharactersReader::GetHexDigitValue(currentCharacter);
		}

		// Now should be a semicolon.
		if (currentCharacter == Semicolon)
		{
			// &#x0; is invalid code point in character reference.
			// &#x; is invalid syntax of reference.
			if (digitCount == 0)
			{
				if (leadingZeros)
				{
					Reset();
					SetError(ErrorCode::InvalidCharacterReference);
					row = tempRow;
					column = tempColumn;
				}
				else
				{
					Reset();
					SetError(ErrorCode::InvalidReferenceSyntax);
					row = tempRow;
					column = tempColumn;
				}
				return false;
			}

			// We are sure that digitCount is 7 max.
			result = 0;
			for (int i = 0; i < digitCount; ++i)
				result = result * radix + static_cast<char32_t>(buffer[i]);

			if (!Encoding::CharactersReader::IsChar(result))
			{
				Reset();
				SetError(ErrorCode::InvalidCharacterReference);
				row = tempRow;
				column = tempColumn;
				return false;
			}
		}
		else if (digit >= 0 && digit < radix)
		{
			// To many digits, but we must check the syntax to set appropriate error.

			do
			{
				if (NextCharBad(insideTag))
				{
					if (!insideTag && eof)
					{
						UnclosedTagType& ref = unclosedTags[unclosedTagsSize - 1];
						Reset();
						SetError(ErrorCode::UnclosedTag);
						row = ref.Row;
						column = ref.Column;
						eof = true;
					}
					return false;
				}
				digit = Encoding::CharactersReader::GetHexDigitValue(currentCharacter);
			}
			while (digit >= 0 && digit < radix);

			if (currentCharacter == Semicolon)
			{
				// just invalid code point.
				Reset();
				SetError(ErrorCode::InvalidCharacterReference);
				row = tempRow;
				column = tempColumn;
			}
			else
			{
				// Invalid syntax of character reference, not just invalid code point.
				Reset();
				SetError(ErrorCode::InvalidReferenceSyntax);
				row = tempRow;
				column = tempColumn;
			}
			return false;
		}
		else
		{
			// Not allowed character.
			Reset();
			SetError(ErrorCode::InvalidReferenceSyntax);
			row = tempRow;
			column = tempColumn;
			return false;
		}

		return true;
	}

	template <typename TCharactersWriter>
	inline int Inspector<TCharactersWriter>::ParseEntityReference(bool insideTag)
	{
		// IsNameStartChar(currentCharacter) == true
		// &&
		// currentCharacter != colon.
		
		SizeType tempRow = currentRow;
		SizeType tempColumn = currentColumn - 1;

		entityName.reserve(NameReserve);
		entityNameCharCount = 0;

		if (currentCharacter == LtEntityName[0])
		{
			// "&l"
			CharactersWriterType::WriteCharacter(entityName, currentCharacter);
			++entityNameCharCount;
			if (NextCharBad(insideTag))
			{
				if (!insideTag && eof)
				{
					UnclosedTagType& ref = unclosedTags[unclosedTagsSize - 1];
					Reset();
					SetError(ErrorCode::UnclosedTag);
					row = ref.Row;
					column = ref.Column;
					eof = true;
				}
				return -1;
			}

			if (currentCharacter == LtEntityName[1])
			{
				// "&lt"
				CharactersWriterType::WriteCharacter(entityName, currentCharacter);
				++entityNameCharCount;
				if (NextCharBad(insideTag))
				{
					if (!insideTag && eof)
					{
						UnclosedTagType& ref = unclosedTags[unclosedTagsSize - 1];
						Reset();
						SetError(ErrorCode::UnclosedTag);
						row = ref.Row;
						column = ref.Column;
						eof = true;
					}
					return -1;
				}

				if (currentCharacter == Semicolon)
				{
					// "&lt;"
					currentCharacter = LessThan;
					entityName.clear();
					return 1;
				}
			}
			
			if (currentCharacter == Semicolon)
			{
				return 0;
			}
			else if (!Encoding::CharactersReader::IsNameChar(currentCharacter) ||
				currentCharacter == Colon)
			{
				Reset();
				SetError(ErrorCode::InvalidReferenceSyntax);
				row = tempRow;
				column = tempColumn;
				return -1;
			}
		}
		else if (currentCharacter == GtEntityName[0])
		{
			// "&g"
			CharactersWriterType::WriteCharacter(entityName, currentCharacter);
			++entityNameCharCount;
			if (NextCharBad(insideTag))
			{
				if (!insideTag && eof)
				{
					UnclosedTagType& ref = unclosedTags[unclosedTagsSize - 1];
					Reset();
					SetError(ErrorCode::UnclosedTag);
					row = ref.Row;
					column = ref.Column;
					eof = true;
				}
				return -1;
			}

			if (currentCharacter == GtEntityName[1])
			{
				// "&gt"
				CharactersWriterType::WriteCharacter(entityName, currentCharacter);
				++entityNameCharCount;
				if (NextCharBad(insideTag))
				{
					if (!insideTag && eof)
					{
						UnclosedTagType& ref = unclosedTags[unclosedTagsSize - 1];
						Reset();
						SetError(ErrorCode::UnclosedTag);
						row = ref.Row;
						column = ref.Column;
						eof = true;
					}
					return -1;
				}

				if (currentCharacter == Semicolon)
				{
					// "&gt;"
					currentCharacter = GreaterThan;
					entityName.clear();
					return 1;
				}
			}
			
			if (currentCharacter == Semicolon)
			{
				return 0;
			}
			else if (!Encoding::CharactersReader::IsNameChar(currentCharacter) ||
				currentCharacter == Colon)
			{
				Reset();
				SetError(ErrorCode::InvalidReferenceSyntax);
				row = tempRow;
				column = tempColumn;
				return -1;
			}
		}
		else if (currentCharacter == AmpEntityName[0])
		{
			// "&a"
			CharactersWriterType::WriteCharacter(entityName, currentCharacter);
			++entityNameCharCount;
			if (NextCharBad(insideTag))
			{
				if (!insideTag && eof)
				{
					UnclosedTagType& ref = unclosedTags[unclosedTagsSize - 1];
					Reset();
					SetError(ErrorCode::UnclosedTag);
					row = ref.Row;
					column = ref.Column;
					eof = true;
				}
				return -1;
			}

			if (currentCharacter == AmpEntityName[1])
			{
				// "&am"
				CharactersWriterType::WriteCharacter(entityName, currentCharacter);
				++entityNameCharCount;
				if (NextCharBad(insideTag))
				{
					if (!insideTag && eof)
					{
						UnclosedTagType& ref = unclosedTags[unclosedTagsSize - 1];
						Reset();
						SetError(ErrorCode::UnclosedTag);
						row = ref.Row;
						column = ref.Column;
						eof = true;
					}
					return -1;
				}

				if (currentCharacter == AmpEntityName[2])
				{
					// "&amp"
					CharactersWriterType::WriteCharacter(entityName, currentCharacter);
					++entityNameCharCount;
					if (NextCharBad(insideTag))
					{
						if (!insideTag && eof)
						{
							UnclosedTagType& ref = unclosedTags[unclosedTagsSize - 1];
							Reset();
							SetError(ErrorCode::UnclosedTag);
							row = ref.Row;
							column = ref.Column;
							eof = true;
						}
						return -1;
					}

					if (currentCharacter == Semicolon)
					{
						// "&amp;"
						currentCharacter = Ampersand;
						entityName.clear();
						return 1;
					}
				}
			}
			else if (currentCharacter == AposEntityName[1])
			{
				// "&ap"
				CharactersWriterType::WriteCharacter(entityName, currentCharacter);
				++entityNameCharCount;
				if (NextCharBad(insideTag))
				{
					if (!insideTag && eof)
					{
						UnclosedTagType& ref = unclosedTags[unclosedTagsSize - 1];
						Reset();
						SetError(ErrorCode::UnclosedTag);
						row = ref.Row;
						column = ref.Column;
						eof = true;
					}
					return -1;
				}

				if (currentCharacter == AposEntityName[2])
				{
					// "&apo"
					CharactersWriterType::WriteCharacter(entityName, currentCharacter);
					++entityNameCharCount;
					if (NextCharBad(insideTag))
					{
						if (!insideTag && eof)
						{
							UnclosedTagType& ref = unclosedTags[unclosedTagsSize - 1];
							Reset();
							SetError(ErrorCode::UnclosedTag);
							row = ref.Row;
							column = ref.Column;
							eof = true;
						}
						return -1;
					}

					if (currentCharacter == AposEntityName[3])
					{
						// "&apos"
						CharactersWriterType::WriteCharacter(entityName, currentCharacter);
						++entityNameCharCount;
						if (NextCharBad(insideTag))
						{
							if (!insideTag && eof)
							{
								UnclosedTagType& ref = unclosedTags[unclosedTagsSize - 1];
								Reset();
								SetError(ErrorCode::UnclosedTag);
								row = ref.Row;
								column = ref.Column;
								eof = true;
							}
							return -1;
						}

						if (currentCharacter == Semicolon)
						{
							// "&apos;"
							currentCharacter = SingleQuote;
							entityName.clear();
							return 1;
						}
					}
				}
			}

			if (currentCharacter == Semicolon)
			{
				return 0;
			}
			else if (!Encoding::CharactersReader::IsNameChar(currentCharacter) ||
				currentCharacter == Colon)
			{
				Reset();
				SetError(ErrorCode::InvalidReferenceSyntax);
				row = tempRow;
				column = tempColumn;
				return -1;
			}
		}
		else if (currentCharacter == QuotEntityName[0])
		{
			// "&q"
			CharactersWriterType::WriteCharacter(entityName, currentCharacter);
			++entityNameCharCount;
			std::size_t i;
			for (i = 1; i < 4; ++i)
			{
				if (NextCharBad(insideTag))
				{
					if (!insideTag && eof)
					{
						UnclosedTagType& ref = unclosedTags[unclosedTagsSize - 1];
						Reset();
						SetError(ErrorCode::UnclosedTag);
						row = ref.Row;
						column = ref.Column;
						eof = true;
					}
					return -1;
				}

				if (currentCharacter != QuotEntityName[i])
					break;

				CharactersWriterType::WriteCharacter(entityName, currentCharacter);
				++entityNameCharCount;
			}

			if (i == 4)
			{
				if (NextCharBad(insideTag))
				{
					if (!insideTag && eof)
					{
						UnclosedTagType& ref = unclosedTags[unclosedTagsSize - 1];
						Reset();
						SetError(ErrorCode::UnclosedTag);
						row = ref.Row;
						column = ref.Column;
						eof = true;
					}
					return -1;
				}
			}

			if (currentCharacter == Semicolon)
			{
				if (i == 4)
				{
					// "&quot;"
					currentCharacter = DoubleQuote;
					entityName.clear();
					return 1;
				}
				else
				{
					return 0;
				}
			}
			else if (!Encoding::CharactersReader::IsNameChar(currentCharacter) ||
				currentCharacter == Colon)
			{
				Reset();
				SetError(ErrorCode::InvalidReferenceSyntax);
				row = tempRow;
				column = tempColumn;
				return -1;
			}
		}

		do
		{
			CharactersWriterType::WriteCharacter(entityName, currentCharacter);
			++entityNameCharCount;
			if (NextCharBad(insideTag))
			{
				if (!insideTag && eof)
				{
					UnclosedTagType& ref = unclosedTags[unclosedTagsSize - 1];
					Reset();
					SetError(ErrorCode::UnclosedTag);
					row = ref.Row;
					column = ref.Column;
					eof = true;
				}
				return -1;
			}

			if (currentCharacter == Colon)
			{
				Reset();
				SetError(ErrorCode::InvalidReferenceSyntax);
				row = tempRow;
				column = tempColumn;
				return -1;
			}
		}
		while (Encoding::CharactersReader::IsNameChar(currentCharacter));

		if (currentCharacter != Semicolon)
		{
			Reset();
			SetError(ErrorCode::InvalidReferenceSyntax);
			row = tempRow;
			column = tempColumn;
			return -1;
		}

		return 0;
	}

	template <typename TCharactersWriter>
	inline bool Inspector<TCharactersWriter>::AttributeUniqueness()
	{
		if (attributesSize > 1)
		{
			const AttributesSizeType lastIndex = attributesSize - 1;
			const AttributeType& last = attributes[lastIndex];
			for (AttributesSizeType i = 0; i < lastIndex; ++i)
			{
				if (last.Name == attributes[i].Name)
				{
					Reset();
					SetError(ErrorCode::DoubleAttributeName);
					row = last.Row;
					column = last.Column;
					return false;
				}
			}
		}

		return true;
	}

	template <typename TCharactersWriter>
	inline bool Inspector<TCharactersWriter>::ResolveEncoding(const AttributeType& encoding)
	{
		// comparingName is already set.

		SizeType tempRow;
		SizeType tempColumn;

		if (IsUtf8Charset())
		{
			if (bom == Details::Bom::Utf8 ||
				bom == Details::Bom::None)
				return true;
		}
		else if (IsUtf16Charset())
		{
			if (bom == Details::Bom::Utf16BE ||
				bom == Details::Bom::Utf16LE)
				return true;
		}
		else if (IsUtf32Charset())
		{
			if (bom == Details::Bom::Utf32BE ||
				bom == Details::Bom::Utf32LE)
				return true;
		}
		else if (IsUtf16BECharset())
		{
			if (bom == Details::Bom::Utf16BE)
				return true;
		}
		else if (IsUtf16LECharset())
		{
			if (bom == Details::Bom::Utf16LE)
				return true;
		}
		else if (IsUtf32BECharset())
		{
			if (bom == Details::Bom::Utf32BE)
				return true;
		}
		else if (IsUtf32LECharset())
		{
			if (bom == Details::Bom::Utf32LE)
				return true;
		}
		else if (IsISO_8859_1_Charset())
		{
			if (bom == Details::Bom::None)
			{
				Encoding::CharactersReader* newReader;
				if (sourceType == SourcePath)
					newReader = new Encoding::ISO_8859_1_StreamReader(&fileStream);
				else
					newReader = new Encoding::ISO_8859_1_StreamReader(inputStreamPtr);
				delete reader;
				reader = newReader;
				return true;
			}
		}
		else if (IsISO_8859_2_Charset())
		{
			if (bom == Details::Bom::None)
			{
				Encoding::CharactersReader* newReader;
				if (sourceType == SourcePath)
					newReader = new Encoding::ISO_8859_2_StreamReader(&fileStream);
				else
					newReader = new Encoding::ISO_8859_2_StreamReader(inputStreamPtr);
				delete reader;
				reader = newReader;
				return true;
			}
		}
		else if (IsWindows874Charset())
		{
			if (bom == Details::Bom::None)
			{
				Encoding::CharactersReader* newReader;
				if (sourceType == SourcePath)
					newReader = new Encoding::Windows874StreamReader(&fileStream);
				else
					newReader = new Encoding::Windows874StreamReader(inputStreamPtr);
				delete reader;
				reader = newReader;
				return true;
			}
		}
		else if (IsWindows1250Charset())
		{
			if (bom == Details::Bom::None)
			{
				Encoding::CharactersReader* newReader;
				if (sourceType == SourcePath)
					newReader = new Encoding::Windows1250StreamReader(&fileStream);
				else
					newReader = new Encoding::Windows1250StreamReader(inputStreamPtr);
				delete reader;
				reader = newReader;
				return true;
			}
		}
		else if (IsWindows1251Charset())
		{
			if (bom == Details::Bom::None)
			{
				Encoding::CharactersReader* newReader;
				if (sourceType == SourcePath)
					newReader = new Encoding::Windows1251StreamReader(&fileStream);
				else
					newReader = new Encoding::Windows1251StreamReader(inputStreamPtr);
				delete reader;
				reader = newReader;
				return true;
			}
		}
		else if (IsWindows1252Charset())
		{
			if (bom == Details::Bom::None)
			{
				Encoding::CharactersReader* newReader;
				if (sourceType == SourcePath)
					newReader = new Encoding::Windows1252StreamReader(&fileStream);
				else
					newReader = new Encoding::Windows1252StreamReader(inputStreamPtr);
				delete reader;
				reader = newReader;
				return true;
			}
		}
		else if (IsWindows1253Charset())
		{
			if (bom == Details::Bom::None)
			{
				Encoding::CharactersReader* newReader;
				if (sourceType == SourcePath)
					newReader = new Encoding::Windows1253StreamReader(&fileStream);
				else
					newReader = new Encoding::Windows1253StreamReader(inputStreamPtr);
				delete reader;
				reader = newReader;
				return true;
			}
		}
		else if (IsWindows1254Charset())
		{
			if (bom == Details::Bom::None)
			{
				Encoding::CharactersReader* newReader;
				if (sourceType == SourcePath)
					newReader = new Encoding::Windows1254StreamReader(&fileStream);
				else
					newReader = new Encoding::Windows1254StreamReader(inputStreamPtr);
				delete reader;
				reader = newReader;
				return true;
			}
		}
		else if (IsWindows1255Charset())
		{
			if (bom == Details::Bom::None)
			{
				Encoding::CharactersReader* newReader;
				if (sourceType == SourcePath)
					newReader = new Encoding::Windows1255StreamReader(&fileStream);
				else
					newReader = new Encoding::Windows1255StreamReader(inputStreamPtr);
				delete reader;
				reader = newReader;
				return true;
			}
		}
		else if (IsWindows1256Charset())
		{
			if (bom == Details::Bom::None)
			{
				Encoding::CharactersReader* newReader;
				if (sourceType == SourcePath)
					newReader = new Encoding::Windows1256StreamReader(&fileStream);
				else
					newReader = new Encoding::Windows1256StreamReader(inputStreamPtr);
				delete reader;
				reader = newReader;
				return true;
			}
		}
		else if (IsWindows1257Charset())
		{
			if (bom == Details::Bom::None)
			{
				Encoding::CharactersReader* newReader;
				if (sourceType == SourcePath)
					newReader = new Encoding::Windows1257StreamReader(&fileStream);
				else
					newReader = new Encoding::Windows1257StreamReader(inputStreamPtr);
				delete reader;
				reader = newReader;
				return true;
			}
		}
		else if (IsWindows1258Charset())
		{
			if (bom == Details::Bom::None)
			{
				Encoding::CharactersReader* newReader;
				if (sourceType == SourcePath)
					newReader = new Encoding::Windows1258StreamReader(&fileStream);
				else
					newReader = new Encoding::Windows1258StreamReader(inputStreamPtr);
				delete reader;
				reader = newReader;
				return true;
			}
		}
		else if (IsISO_8859_3_Charset())
		{
			if (bom == Details::Bom::None)
			{
				Encoding::CharactersReader* newReader;
				if (sourceType == SourcePath)
					newReader = new Encoding::ISO_8859_3_StreamReader(&fileStream);
				else
					newReader = new Encoding::ISO_8859_3_StreamReader(inputStreamPtr);
				delete reader;
				reader = newReader;
				return true;
			}
		}
		else if (IsISO_8859_4_Charset())
		{
			if (bom == Details::Bom::None)
			{
				Encoding::CharactersReader* newReader;
				if (sourceType == SourcePath)
					newReader = new Encoding::ISO_8859_4_StreamReader(&fileStream);
				else
					newReader = new Encoding::ISO_8859_4_StreamReader(inputStreamPtr);
				delete reader;
				reader = newReader;
				return true;
			}
		}
		else if (IsISO_8859_5_Charset())
		{
			if (bom == Details::Bom::None)
			{
				Encoding::CharactersReader* newReader;
				if (sourceType == SourcePath)
					newReader = new Encoding::ISO_8859_5_StreamReader(&fileStream);
				else
					newReader = new Encoding::ISO_8859_5_StreamReader(inputStreamPtr);
				delete reader;
				reader = newReader;
				return true;
			}
		}
		else if (IsISO_8859_6_Charset())
		{
			if (bom == Details::Bom::None)
			{
				Encoding::CharactersReader* newReader;
				if (sourceType == SourcePath)
					newReader = new Encoding::ISO_8859_6_StreamReader(&fileStream);
				else
					newReader = new Encoding::ISO_8859_6_StreamReader(inputStreamPtr);
				delete reader;
				reader = newReader;
				return true;
			}
		}
		else if (IsISO_8859_7_Charset())
		{
			if (bom == Details::Bom::None)
			{
				Encoding::CharactersReader* newReader;
				if (sourceType == SourcePath)
					newReader = new Encoding::ISO_8859_7_StreamReader(&fileStream);
				else
					newReader = new Encoding::ISO_8859_7_StreamReader(inputStreamPtr);
				delete reader;
				reader = newReader;
				return true;
			}
		}
		else if (IsISO_8859_8_Charset())
		{
			if (bom == Details::Bom::None)
			{
				Encoding::CharactersReader* newReader;
				if (sourceType == SourcePath)
					newReader = new Encoding::ISO_8859_8_StreamReader(&fileStream);
				else
					newReader = new Encoding::ISO_8859_8_StreamReader(inputStreamPtr);
				delete reader;
				reader = newReader;
				return true;
			}
		}
		else if (IsISO_8859_9_Charset())
		{
			if (bom == Details::Bom::None)
			{
				Encoding::CharactersReader* newReader;
				if (sourceType == SourcePath)
					newReader = new Encoding::ISO_8859_9_StreamReader(&fileStream);
				else
					newReader = new Encoding::ISO_8859_9_StreamReader(inputStreamPtr);
				delete reader;
				reader = newReader;
				return true;
			}
		}
		else if (IsISO_8859_10_Charset())
		{
			if (bom == Details::Bom::None)
			{
				Encoding::CharactersReader* newReader;
				if (sourceType == SourcePath)
					newReader = new Encoding::ISO_8859_10_StreamReader(&fileStream);
				else
					newReader = new Encoding::ISO_8859_10_StreamReader(inputStreamPtr);
				delete reader;
				reader = newReader;
				return true;
			}
		}
		else if (IsISO_8859_13_Charset())
		{
			if (bom == Details::Bom::None)
			{
				Encoding::CharactersReader* newReader;
				if (sourceType == SourcePath)
					newReader = new Encoding::ISO_8859_13_StreamReader(&fileStream);
				else
					newReader = new Encoding::ISO_8859_13_StreamReader(inputStreamPtr);
				delete reader;
				reader = newReader;
				return true;
			}
		}
		else if (IsISO_8859_14_Charset())
		{
			if (bom == Details::Bom::None)
			{
				Encoding::CharactersReader* newReader;
				if (sourceType == SourcePath)
					newReader = new Encoding::ISO_8859_14_StreamReader(&fileStream);
				else
					newReader = new Encoding::ISO_8859_14_StreamReader(inputStreamPtr);
				delete reader;
				reader = newReader;
				return true;
			}
		}
		else if (IsISO_8859_15_Charset())
		{
			if (bom == Details::Bom::None)
			{
				Encoding::CharactersReader* newReader;
				if (sourceType == SourcePath)
					newReader = new Encoding::ISO_8859_15_StreamReader(&fileStream);
				else
					newReader = new Encoding::ISO_8859_15_StreamReader(inputStreamPtr);
				delete reader;
				reader = newReader;
				return true;
			}
		}
		else if (IsISO_8859_16_Charset())
		{
			if (bom == Details::Bom::None)
			{
				Encoding::CharactersReader* newReader;
				if (sourceType == SourcePath)
					newReader = new Encoding::ISO_8859_16_StreamReader(&fileStream);
				else
					newReader = new Encoding::ISO_8859_16_StreamReader(inputStreamPtr);
				delete reader;
				reader = newReader;
				return true;
			}
		}
		else if (IsTIS620Charset())
		{
			if (bom == Details::Bom::None)
			{
				Encoding::CharactersReader* newReader;
				if (sourceType == SourcePath)
					newReader = new Encoding::TIS620StreamReader(&fileStream);
				else
					newReader = new Encoding::TIS620StreamReader(inputStreamPtr);
				delete reader;
				reader = newReader;
				return true;
			}
		}
		else
		{
			tempRow = encoding.Row;
			tempColumn = encoding.Column;
			Reset();
			SetError(ErrorCode::UnknownEncoding);
			row = tempRow;
			column = tempColumn;
			return false;
		}
		
		tempRow = encoding.Row;
		tempColumn = encoding.Column;
		Reset();
		SetError(ErrorCode::EncodingConfusion);
		row = tempRow;
		column = tempColumn;
		return false;
	}

	template <typename TCharactersWriter>
	inline bool Inspector<TCharactersWriter>::IsUtf8Charset()
	{
		// UTF-8
		static const unsigned char c1[5] =
			{ 0x55, 0x54, 0x46, 0x2D, 0x38 };
		// csUTF8
		static const unsigned char c2[6] =
			{ 0x63, 0x73, 0x55, 0x54, 0x46, 0x38 };
		return (
			CharsetEqual(c1, 5) ||
			CharsetEqual(c2, 6));
		/*
		return (
			CharsetEqual(U"UTF-8") ||
			CharsetEqual(U"csUTF8"));
		*/
	}

	template <typename TCharactersWriter>
	inline bool Inspector<TCharactersWriter>::IsUtf16Charset()
	{
		// UTF-16
		static const unsigned char c1[6] =
			{ 0x55, 0x54, 0x46, 0x2D, 0x31, 0x36 };
		// csUTF16
		static const unsigned char c2[7] =
			{ 0x63, 0x73, 0x55, 0x54, 0x46, 0x31, 0x36 };
		return (
			CharsetEqual(c1, 6) ||
			CharsetEqual(c2, 7));
		/*
		return (
			CharsetEqual(U"UTF-16") ||
			CharsetEqual(U"csUTF16"));
		*/
	}

	template <typename TCharactersWriter>
	inline bool Inspector<TCharactersWriter>::IsUtf16BECharset()
	{
		// UTF-16BE
		static const unsigned char c1[8] =
			{ 0x55, 0x54, 0x46, 0x2D, 0x31, 0x36, 0x42, 0x45 };
		// csUTF16BE
		static const unsigned char c2[9] =
			{ 0x63, 0x73, 0x55, 0x54, 0x46, 0x31, 0x36, 0x42, 0x45 };
		return (
			CharsetEqual(c1, 8) ||
			CharsetEqual(c2, 9));
		/*
		return (
			CharsetEqual(U"UTF-16BE") ||
			CharsetEqual(U"csUTF16BE"));
		*/
	}

	template <typename TCharactersWriter>
	inline bool Inspector<TCharactersWriter>::IsUtf16LECharset()
	{
		// UTF-16LE
		static const unsigned char c1[8] =
			{ 0x55, 0x54, 0x46, 0x2D, 0x31, 0x36, 0x4C, 0x45 };
		// csUTF16LE
		static const unsigned char c2[9] =
			{ 0x63, 0x73, 0x55, 0x54, 0x46, 0x31, 0x36, 0x4C, 0x45 };
		return (
			CharsetEqual(c1, 8) ||
			CharsetEqual(c2, 9));
		/*
		return (
			CharsetEqual(U"UTF-16LE") ||
			CharsetEqual(U"csUTF16LE"));
		*/
	}

	template <typename TCharactersWriter>
	inline bool Inspector<TCharactersWriter>::IsUtf32Charset()
	{
		// UTF-32
		static const unsigned char c1[6] =
			{ 0x55, 0x54, 0x46, 0x2D, 0x33, 0x32 };
		// csUTF32
		static const unsigned char c2[7] =
			{ 0x63, 0x73, 0x55, 0x54, 0x46, 0x33, 0x32 };
		return (
			CharsetEqual(c1, 6) ||
			CharsetEqual(c2, 7));
		/*
		return (
			CharsetEqual(U"UTF-32") ||
			CharsetEqual(U"csUTF32"));
		*/
	}

	template <typename TCharactersWriter>
	inline bool Inspector<TCharactersWriter>::IsUtf32BECharset()
	{
		// UTF-32BE
		static const unsigned char c1[8] =
			{ 0x55, 0x54, 0x46, 0x2D, 0x33, 0x32, 0x42, 0x45 };
		// csUTF32BE
		static const unsigned char c2[9] =
			{ 0x63, 0x73, 0x55, 0x54, 0x46, 0x33, 0x32, 0x42, 0x45 };
		return (
			CharsetEqual(c1, 8) ||
			CharsetEqual(c2, 9));
		/*
		return (
			CharsetEqual(U"UTF-32BE") ||
			CharsetEqual(U"csUTF32BE"));
		*/
	}

	template <typename TCharactersWriter>
	inline bool Inspector<TCharactersWriter>::IsUtf32LECharset()
	{
		// UTF-32LE
		static const unsigned char c1[8] =
			{ 0x55, 0x54, 0x46, 0x2D, 0x33, 0x32, 0x4C, 0x45 };
		// csUTF32LE
		static const unsigned char c2[9] =
			{ 0x63, 0x73, 0x55, 0x54, 0x46, 0x33, 0x32, 0x4C, 0x45 };
		return (
			CharsetEqual(c1, 8) ||
			CharsetEqual(c2, 9));
		/*
		return (
			CharsetEqual(U"UTF-32LE") ||
			CharsetEqual(U"csUTF32LE"));
		*/
	}

	template <typename TCharactersWriter>
	inline bool Inspector<TCharactersWriter>::IsISO_8859_1_Charset()
	{
		// ISO-8859-1
		static const unsigned char c1[10] =
			{ 0x49, 0x53, 0x4F, 0x2D, 0x38, 0x38, 0x35, 0x39, 0x2D, 0x31 };
		// iso-ir-100
		static const unsigned char c2[10] =
			{ 0x69, 0x73, 0x6F, 0x2D, 0x69, 0x72, 0x2D, 0x31, 0x30, 0x30 };
		// ISO_8859-1
		static const unsigned char c3[10] =
			{ 0x49, 0x53, 0x4F, 0x5F, 0x38, 0x38, 0x35, 0x39, 0x2D, 0x31 };
		// latin1
		static const unsigned char c4[6] =
			{ 0x6C, 0x61, 0x74, 0x69, 0x6E, 0x31 };
		// l1
		static const unsigned char c5[2] =
			{ 0x6C, 0x31 };
		// IBM819
		static const unsigned char c6[6] =
			{ 0x49, 0x42, 0x4D, 0x38, 0x31, 0x39 };
		// CP819
		static const unsigned char c7[5] =
			{ 0x43, 0x50, 0x38, 0x31, 0x39 };
		// csISOLatin1
		static const unsigned char c8[11] =
			{ 0x63, 0x73, 0x49, 0x53, 0x4F, 0x4C, 0x61, 0x74, 0x69, 0x6E, 0x31 };
		return (
			CharsetEqual(c1, 10) ||
			CharsetEqual(c2, 10) ||
			CharsetEqual(c3, 10) ||
			CharsetEqual(c4, 6) ||
			CharsetEqual(c5, 2) ||
			CharsetEqual(c6, 6) ||
			CharsetEqual(c7, 5) ||
			CharsetEqual(c8, 11));
		/*
		return (
			CharsetEqual(U"ISO-8859-1") ||
			CharsetEqual(U"iso-ir-100") ||
			CharsetEqual(U"ISO_8859-1") ||
			CharsetEqual(U"latin1") ||
			CharsetEqual(U"l1") ||
			CharsetEqual(U"IBM819") ||
			CharsetEqual(U"CP819") ||
			CharsetEqual(U"csISOLatin1"));
		*/
	}

	template <typename TCharactersWriter>
	inline bool Inspector<TCharactersWriter>::IsISO_8859_2_Charset()
	{
		// ISO-8859-2
		static const unsigned char c1[10] =
			{ 0x49, 0x53, 0x4F, 0x2D, 0x38, 0x38, 0x35, 0x39, 0x2D, 0x32 };
		// iso-ir-101
		static const unsigned char c2[10] =
			{ 0x69, 0x73, 0x6F, 0x2D, 0x69, 0x72, 0x2D, 0x31, 0x30, 0x31 };
		// ISO_8859-2
		static const unsigned char c3[10] =
			{ 0x49, 0x53, 0x4F, 0x5F, 0x38, 0x38, 0x35, 0x39, 0x2D, 0x32 };
		// latin2
		static const unsigned char c4[6] =
			{ 0x6C, 0x61, 0x74, 0x69, 0x6E, 0x32 };
		// l2
		static const unsigned char c5[2] =
			{ 0x6C, 0x32 };
		// csISOLatin2
		static const unsigned char c6[11] =
			{ 0x63, 0x73, 0x49, 0x53, 0x4F, 0x4C, 0x61, 0x74, 0x69, 0x6E, 0x32 };
		return (
			CharsetEqual(c1, 10) ||
			CharsetEqual(c2, 10) ||
			CharsetEqual(c3, 10) ||
			CharsetEqual(c4, 6) ||
			CharsetEqual(c5, 2) ||
			CharsetEqual(c6, 11));
		/*
		return (
			CharsetEqual(U"ISO-8859-2") ||
			CharsetEqual(U"iso-ir-101") ||
			CharsetEqual(U"ISO_8859-2") ||
			CharsetEqual(U"latin2") ||
			CharsetEqual(U"l2") ||
			CharsetEqual(U"csISOLatin2"));
		*/
	}

	template <typename TCharactersWriter>
	inline bool Inspector<TCharactersWriter>::IsISO_8859_3_Charset()
	{
		// ISO-8859-3
		static const unsigned char c1[10] =
			{ 0x49, 0x53, 0x4F, 0x2D, 0x38, 0x38, 0x35, 0x39, 0x2D, 0x33 };
		// iso-ir-109
		static const unsigned char c2[10] =
			{ 0x69, 0x73, 0x6F, 0x2D, 0x69, 0x72, 0x2D, 0x31, 0x30, 0x39 };
		// ISO_8859-3
		static const unsigned char c3[10] =
			{ 0x49, 0x53, 0x4F, 0x5F, 0x38, 0x38, 0x35, 0x39, 0x2D, 0x33 };
		// latin3
		static const unsigned char c4[6] =
			{ 0x6C, 0x61, 0x74, 0x69, 0x6E, 0x33 };
		// l3
		static const unsigned char c5[2] =
			{ 0x6C, 0x33 };
		// csISOLatin3
		static const unsigned char c6[11] =
			{ 0x63, 0x73, 0x49, 0x53, 0x4F, 0x4C, 0x61, 0x74, 0x69, 0x6E, 0x33 };
		return (
			CharsetEqual(c1, 10) ||
			CharsetEqual(c2, 10) ||
			CharsetEqual(c3, 10) ||
			CharsetEqual(c4, 6) ||
			CharsetEqual(c5, 2) ||
			CharsetEqual(c6, 11));
		/*
		return (
			CharsetEqual(U"ISO-8859-3") ||
			CharsetEqual(U"iso-ir-109") ||
			CharsetEqual(U"ISO_8859-3") ||
			CharsetEqual(U"latin3") ||
			CharsetEqual(U"l3") ||
			CharsetEqual(U"csISOLatin3"));
		*/
	}

	template <typename TCharactersWriter>
	inline bool Inspector<TCharactersWriter>::IsISO_8859_4_Charset()
	{
		// ISO-8859-4
		static const unsigned char c1[10] =
			{ 0x49, 0x53, 0x4F, 0x2D, 0x38, 0x38, 0x35, 0x39, 0x2D, 0x34 };
		// iso-ir-110
		static const unsigned char c2[10] =
			{ 0x69, 0x73, 0x6F, 0x2D, 0x69, 0x72, 0x2D, 0x31, 0x31, 0x30 };
		// ISO_8859-4
		static const unsigned char c3[10] =
			{ 0x49, 0x53, 0x4F, 0x5F, 0x38, 0x38, 0x35, 0x39, 0x2D, 0x34 };
		// latin4
		static const unsigned char c4[6] =
			{ 0x6C, 0x61, 0x74, 0x69, 0x6E, 0x34 };
		// l4
		static const unsigned char c5[2] =
			{ 0x6C, 0x34 };
		// csISOLatin4
		static const unsigned char c6[11] =
			{ 0x63, 0x73, 0x49, 0x53, 0x4F, 0x4C, 0x61, 0x74, 0x69, 0x6E, 0x34 };
		return (
			CharsetEqual(c1, 10) ||
			CharsetEqual(c2, 10) ||
			CharsetEqual(c3, 10) ||
			CharsetEqual(c4, 6) ||
			CharsetEqual(c5, 2) ||
			CharsetEqual(c6, 11));
		/*
		return (
			CharsetEqual(U"ISO-8859-4") ||
			CharsetEqual(U"iso-ir-110") ||
			CharsetEqual(U"ISO_8859-4") ||
			CharsetEqual(U"latin4") ||
			CharsetEqual(U"l4") ||
			CharsetEqual(U"csISOLatin4"));
		*/
	}
	
	template <typename TCharactersWriter>
	inline bool Inspector<TCharactersWriter>::IsISO_8859_5_Charset()
	{
		// ISO-8859-5
		static const unsigned char c1[10] =
			{ 0x49, 0x53, 0x4F, 0x2D, 0x38, 0x38, 0x35, 0x39, 0x2D, 0x35 };
		// iso-ir-144
		static const unsigned char c2[10] =
			{ 0x69, 0x73, 0x6F, 0x2D, 0x69, 0x72, 0x2D, 0x31, 0x34, 0x34 };
		// ISO_8859-5
		static const unsigned char c3[10] =
			{ 0x49, 0x53, 0x4F, 0x5F, 0x38, 0x38, 0x35, 0x39, 0x2D, 0x35 };
		// cyrillic
		static const unsigned char c4[8] =
			{ 0x63, 0x79, 0x72, 0x69, 0x6C, 0x6C, 0x69, 0x63 };
		// csISOLatinCyrillic
		static const unsigned char c5[18] =
		{
			0x63, 0x73, 0x49, 0x53, 0x4F, 0x4C, 0x61, 0x74, 0x69,
			0x6E, 0x43, 0x79, 0x72, 0x69, 0x6C, 0x6C, 0x69, 0x63
		};
		return (
			CharsetEqual(c1, 10) ||
			CharsetEqual(c2, 10) ||
			CharsetEqual(c3, 10) ||
			CharsetEqual(c4, 8) ||
			CharsetEqual(c5, 18));
		/*
		return (
			CharsetEqual(U"ISO-8859-5") ||
			CharsetEqual(U"iso-ir-144") ||
			CharsetEqual(U"ISO_8859-5") ||
			CharsetEqual(U"cyrillic") ||
			CharsetEqual(U"csISOLatinCyrillic"));
		*/
	}

	template <typename TCharactersWriter>
	inline bool Inspector<TCharactersWriter>::IsISO_8859_6_Charset()
	{
		// ISO-8859-6
		static const unsigned char c1[10] =
			{ 0x49, 0x53, 0x4F, 0x2D, 0x38, 0x38, 0x35, 0x39, 0x2D, 0x36 };
		// iso-ir-127
		static const unsigned char c2[10] =
			{ 0x69, 0x73, 0x6F, 0x2D, 0x69, 0x72, 0x2D, 0x31, 0x32, 0x37 };
		// ISO_8859-6
		static const unsigned char c3[10] =
			{ 0x49, 0x53, 0x4F, 0x5F, 0x38, 0x38, 0x35, 0x39, 0x2D, 0x36 };
		// ECMA-114
		static const unsigned char c4[8] =
			{ 0x45, 0x43, 0x4D, 0x41, 0x2D, 0x31, 0x31, 0x34 };
		// ASMO-708
		static const unsigned char c5[8] =
			{ 0x41, 0x53, 0x4D, 0x4F, 0x2D, 0x37, 0x30, 0x38 };
		// arabic
		static const unsigned char c6[6] =
			{ 0x61, 0x72, 0x61, 0x62, 0x69, 0x63 };
		// csISOLatinArabic
		static const unsigned char c7[16] =
			{ 0x63, 0x73, 0x49, 0x53, 0x4F, 0x4C, 0x61, 0x74, 0x69, 0x6E, 0x41, 0x72, 0x61, 0x62, 0x69, 0x63 };
		return (
			CharsetEqual(c1, 10) ||
			CharsetEqual(c2, 10) ||
			CharsetEqual(c3, 10) ||
			CharsetEqual(c4, 8) ||
			CharsetEqual(c5, 8) ||
			CharsetEqual(c6, 6) ||
			CharsetEqual(c7, 16));
		/*
		return (
			CharsetEqual(U"ISO-8859-6") ||
			CharsetEqual(U"iso-ir-127") ||
			CharsetEqual(U"ISO_8859-6") ||
			CharsetEqual(U"ECMA-114") ||
			CharsetEqual(U"ASMO-708") ||
			CharsetEqual(U"arabic") ||
			CharsetEqual(U"csISOLatinArabic"));
		*/
	}

	template <typename TCharactersWriter>
	inline bool Inspector<TCharactersWriter>::IsISO_8859_7_Charset()
	{
		// ISO-8859-7
		static const unsigned char c1[10] =
			{ 0x49, 0x53, 0x4F, 0x2D, 0x38, 0x38, 0x35, 0x39, 0x2D, 0x37 };
		// iso-ir-126
		static const unsigned char c2[10] =
			{ 0x69, 0x73, 0x6F, 0x2D, 0x69, 0x72, 0x2D, 0x31, 0x32, 0x36 };
		// ISO_8859-7
		static const unsigned char c3[10] =
			{ 0x49, 0x53, 0x4F, 0x5F, 0x38, 0x38, 0x35, 0x39, 0x2D, 0x37 };
		// ELOT_928
		static const unsigned char c4[8] =
			{ 0x45, 0x4C, 0x4F, 0x54, 0x5F, 0x39, 0x32, 0x38 };
		// ECMA-118
		static const unsigned char c5[8] =
			{ 0x45, 0x43, 0x4D, 0x41, 0x2D, 0x31, 0x31, 0x38 };
		// greek
		static const unsigned char c6[5] =
			{ 0x67, 0x72, 0x65, 0x65, 0x6B };
		// greek8
		static const unsigned char c7[6] =
			{ 0x67, 0x72, 0x65, 0x65, 0x6B, 0x38 };
		// csISOLatinGreek
		static const unsigned char c8[15] =
			{ 0x63, 0x73, 0x49, 0x53, 0x4F, 0x4C, 0x61, 0x74, 0x69, 0x6E, 0x47, 0x72, 0x65, 0x65, 0x6B };
		return (
			CharsetEqual(c1, 10) ||
			CharsetEqual(c2, 10) ||
			CharsetEqual(c3, 10) ||
			CharsetEqual(c4, 8) ||
			CharsetEqual(c5, 8) ||
			CharsetEqual(c6, 5) ||
			CharsetEqual(c7, 6) ||
			CharsetEqual(c8, 15));
		/*
		return (
			CharsetEqual(U"ISO-8859-7") ||
			CharsetEqual(U"iso-ir-126") ||
			CharsetEqual(U"ISO_8859-7") ||
			CharsetEqual(U"ELOT_928") ||
			CharsetEqual(U"ECMA-118") ||
			CharsetEqual(U"greek") ||
			CharsetEqual(U"greek8") ||
			CharsetEqual(U"csISOLatinGreek"));
		*/
	}

	template <typename TCharactersWriter>
	inline bool Inspector<TCharactersWriter>::IsISO_8859_8_Charset()
	{
		// ISO-8859-8
		static const unsigned char c1[10] =
			{ 0x49, 0x53, 0x4F, 0x2D, 0x38, 0x38, 0x35, 0x39, 0x2D, 0x38 };
		// iso-ir-138
		static const unsigned char c2[10] =
			{ 0x69, 0x73, 0x6F, 0x2D, 0x69, 0x72, 0x2D, 0x31, 0x33, 0x38 };
		// ISO_8859-8
		static const unsigned char c3[10] =
			{ 0x49, 0x53, 0x4F, 0x5F, 0x38, 0x38, 0x35, 0x39, 0x2D, 0x38 };
		// hebrew
		static const unsigned char c4[6] =
			{ 0x68, 0x65, 0x62, 0x72, 0x65, 0x77 };
		// csISOLatinHebrew
		static const unsigned char c5[16] =
			{ 0x63, 0x73, 0x49, 0x53, 0x4F, 0x4C, 0x61, 0x74, 0x69, 0x6E, 0x48, 0x65, 0x62, 0x72, 0x65, 0x77 };
		return (
			CharsetEqual(c1, 10) ||
			CharsetEqual(c2, 10) ||
			CharsetEqual(c3, 10) ||
			CharsetEqual(c4, 6) ||
			CharsetEqual(c5, 16));
		/*
		return (
			CharsetEqual(U"ISO-8859-8") ||
			CharsetEqual(U"iso-ir-138") ||
			CharsetEqual(U"ISO_8859-8") ||
			CharsetEqual(U"hebrew") ||
			CharsetEqual(U"csISOLatinHebrew"));
		*/
	}

	template <typename TCharactersWriter>
	inline bool Inspector<TCharactersWriter>::IsISO_8859_9_Charset()
	{
		// ISO-8859-9
		static const unsigned char c1[10] =
			{ 0x49, 0x53, 0x4F, 0x2D, 0x38, 0x38, 0x35, 0x39, 0x2D, 0x39 };
		// iso-ir-148
		static const unsigned char c2[10] =
			{ 0x69, 0x73, 0x6F, 0x2D, 0x69, 0x72, 0x2D, 0x31, 0x34, 0x38 };
		// ISO_8859-9
		static const unsigned char c3[10] =
			{ 0x49, 0x53, 0x4F, 0x5F, 0x38, 0x38, 0x35, 0x39, 0x2D, 0x39 };
		// latin5
		static const unsigned char c4[6] =
			{ 0x6C, 0x61, 0x74, 0x69, 0x6E, 0x35 };
		// l5
		static const unsigned char c5[2] =
			{ 0x6C, 0x35 };
		// csISOLatin5
		static const unsigned char c6[11] =
			{ 0x63, 0x73, 0x49, 0x53, 0x4F, 0x4C, 0x61, 0x74, 0x69, 0x6E, 0x35 };
		return (
			CharsetEqual(c1, 10) ||
			CharsetEqual(c2, 10) ||
			CharsetEqual(c3, 10) ||
			CharsetEqual(c4, 6) ||
			CharsetEqual(c5, 2) ||
			CharsetEqual(c6, 11));
		/*
		return (
			CharsetEqual(U"ISO-8859-9") ||
			CharsetEqual(U"iso-ir-148") ||
			CharsetEqual(U"ISO_8859-9") ||
			CharsetEqual(U"latin5") ||
			CharsetEqual(U"l5") ||
			CharsetEqual(U"csISOLatin5"));
		*/
	}

	template <typename TCharactersWriter>
	inline bool Inspector<TCharactersWriter>::IsISO_8859_10_Charset()
	{
		// ISO-8859-10
		static const unsigned char c1[11] =
			{ 0x49, 0x53, 0x4F, 0x2D, 0x38, 0x38, 0x35, 0x39, 0x2D, 0x31, 0x30 };
		// iso-ir-157
		static const unsigned char c2[10] =
			{ 0x69, 0x73, 0x6F, 0x2D, 0x69, 0x72, 0x2D, 0x31, 0x35, 0x37 };
		// l6
		static const unsigned char c3[2] =
			{ 0x6C, 0x36 };
		// ISO_8859-10
		static const unsigned char c4[11] =
			{ 0x49, 0x53, 0x4F, 0x5F, 0x38, 0x38, 0x35, 0x39, 0x2D, 0x31, 0x30 };
		// csISOLatin6
		static const unsigned char c5[11] =
			{ 0x63, 0x73, 0x49, 0x53, 0x4F, 0x4C, 0x61, 0x74, 0x69, 0x6E, 0x36 };
		// latin6
		static const unsigned char c6[6] =
			{ 0x6C, 0x61, 0x74, 0x69, 0x6E, 0x36 };
		return (
			CharsetEqual(c1, 11) ||
			CharsetEqual(c2, 10) ||
			CharsetEqual(c3, 2) ||
			CharsetEqual(c4, 11) ||
			CharsetEqual(c5, 11) ||
			CharsetEqual(c6, 6));
		/*
		return (
			CharsetEqual(U"ISO-8859-10") ||
			CharsetEqual(U"iso-ir-157") ||
			CharsetEqual(U"l6") ||
			CharsetEqual(U"ISO_8859-10") ||
			CharsetEqual(U"csISOLatin6") ||
			CharsetEqual(U"latin6"));
		*/
	}

	template <typename TCharactersWriter>
	inline bool Inspector<TCharactersWriter>::IsISO_8859_13_Charset()
	{
		// ISO-8859-13
		static const unsigned char c1[11] =
			{ 0x49, 0x53, 0x4F, 0x2D, 0x38, 0x38, 0x35, 0x39, 0x2D, 0x31, 0x33 };
		// csISO885913
		static const unsigned char c2[11] =
			{ 0x63, 0x73, 0x49, 0x53, 0x4F, 0x38, 0x38, 0x35, 0x39, 0x31, 0x33 };
		return (
			CharsetEqual(c1, 11) ||
			CharsetEqual(c2, 11));
		/*
		return (
			CharsetEqual(U"ISO-8859-13") ||
			CharsetEqual(U"csISO885913"));
		*/
	}

	template <typename TCharactersWriter>
	inline bool Inspector<TCharactersWriter>::IsISO_8859_14_Charset()
	{
		// ISO-8859-14
		static const unsigned char c1[11] =
			{ 0x49, 0x53, 0x4F, 0x2D, 0x38, 0x38, 0x35, 0x39, 0x2D, 0x31, 0x34 };
		// iso-ir-199
		static const unsigned char c2[10] =
			{ 0x69, 0x73, 0x6F, 0x2D, 0x69, 0x72, 0x2D, 0x31, 0x39, 0x39 };
		// ISO_8859-14
		static const unsigned char c3[11] =
			{ 0x49, 0x53, 0x4F, 0x5F, 0x38, 0x38, 0x35, 0x39, 0x2D, 0x31, 0x34 };
		// latin8
		static const unsigned char c4[6] =
			{ 0x6C, 0x61, 0x74, 0x69, 0x6E, 0x38 };
		// iso-celtic
		static const unsigned char c5[10] =
			{ 0x69, 0x73, 0x6F, 0x2D, 0x63, 0x65, 0x6C, 0x74, 0x69, 0x63 };
		// l8
		static const unsigned char c6[2] =
			{ 0x6C, 0x38 };
		// csISO885914
		static const unsigned char c7[11] =
			{ 0x63, 0x73, 0x49, 0x53, 0x4F, 0x38, 0x38, 0x35, 0x39, 0x31, 0x34 };
		return (
			CharsetEqual(c1, 11) ||
			CharsetEqual(c2, 10) ||
			CharsetEqual(c3, 11) ||
			CharsetEqual(c4, 6) ||
			CharsetEqual(c5, 10) ||
			CharsetEqual(c6, 2) ||
			CharsetEqual(c7, 11));
		/*
		return (
			CharsetEqual(U"ISO-8859-14") ||
			CharsetEqual(U"iso-ir-199") ||
			CharsetEqual(U"ISO_8859-14") ||
			CharsetEqual(U"latin8") ||
			CharsetEqual(U"iso-celtic") ||
			CharsetEqual(U"l8") ||
			CharsetEqual(U"csISO885914"));
		*/
	}

	template <typename TCharactersWriter>
	inline bool Inspector<TCharactersWriter>::IsISO_8859_15_Charset()
	{
		// ISO-8859-15
		static const unsigned char c1[11] =
			{ 0x49, 0x53, 0x4F, 0x2D, 0x38, 0x38, 0x35, 0x39, 0x2D, 0x31, 0x35 };
		// ISO_8859-15
		static const unsigned char c2[11] =
			{ 0x49, 0x53, 0x4F, 0x5F, 0x38, 0x38, 0x35, 0x39, 0x2D, 0x31, 0x35 };
		// Latin-9
		static const unsigned char c3[7] =
			{ 0x4C, 0x61, 0x74, 0x69, 0x6E, 0x2D, 0x39 };
		// csISO885915
		static const unsigned char c4[11] =
			{ 0x63, 0x73, 0x49, 0x53, 0x4F, 0x38, 0x38, 0x35, 0x39, 0x31, 0x35 };
		return (
			CharsetEqual(c1, 11) ||
			CharsetEqual(c2, 11) ||
			CharsetEqual(c3, 7) ||
			CharsetEqual(c4, 11));
		/*
		return (
			CharsetEqual(U"ISO-8859-15") ||
			CharsetEqual(U"ISO_8859-15") ||
			CharsetEqual(U"Latin-9") ||
			CharsetEqual(U"csISO885915"));
		*/
	}

	template <typename TCharactersWriter>
	inline bool Inspector<TCharactersWriter>::IsISO_8859_16_Charset()
	{
		// ISO-8859-16
		static const unsigned char c1[11] =
			{ 0x49, 0x53, 0x4F, 0x2D, 0x38, 0x38, 0x35, 0x39, 0x2D, 0x31, 0x36 };
		// iso-ir-226
		static const unsigned char c2[10] =
			{ 0x69, 0x73, 0x6F, 0x2D, 0x69, 0x72, 0x2D, 0x32, 0x32, 0x36 };
		// ISO_8859-16
		static const unsigned char c3[11] =
			{ 0x49, 0x53, 0x4F, 0x5F, 0x38, 0x38, 0x35, 0x39, 0x2D, 0x31, 0x36 };
		// latin10
		static const unsigned char c4[7] =
			{ 0x6C, 0x61, 0x74, 0x69, 0x6E, 0x31, 0x30 };
		// l10
		static const unsigned char c5[3] =
			{ 0x6C, 0x31, 0x30 };
		// csISO885916
		static const unsigned char c6[11] =
			{ 0x63, 0x73, 0x49, 0x53, 0x4F, 0x38, 0x38, 0x35, 0x39, 0x31, 0x36 };
		return (
			CharsetEqual(c1, 11) ||
			CharsetEqual(c2, 10) ||
			CharsetEqual(c3, 11) ||
			CharsetEqual(c4, 7) ||
			CharsetEqual(c5, 3) ||
			CharsetEqual(c6, 11));
		/*
		return (
			CharsetEqual(U"ISO-8859-16") ||
			CharsetEqual(U"iso-ir-226") ||
			CharsetEqual(U"ISO_8859-16") ||
			CharsetEqual(U"latin10") ||
			CharsetEqual(U"l10") ||
			CharsetEqual(U"csISO885916"));
		*/
	}

	template <typename TCharactersWriter>
	inline bool Inspector<TCharactersWriter>::IsTIS620Charset()
	{
		// TIS-620
		static const unsigned char c1[7] =
			{ 0x54, 0x49, 0x53, 0x2D, 0x36, 0x32, 0x30 };
		// csTIS620
		static const unsigned char c2[8] =
			{ 0x63, 0x73, 0x54, 0x49, 0x53, 0x36, 0x32, 0x30 };
		return (
			CharsetEqual(c1, 7) ||
			CharsetEqual(c2, 8));
		/*
		return (
			CharsetEqual(U"TIS-620") ||
			CharsetEqual(U"csTIS620"));
		*/
	}

	template <typename TCharactersWriter>
	inline bool Inspector<TCharactersWriter>::IsWindows874Charset()
	{
		// windows-874
		static const unsigned char c1[11] =
			{ 0x77, 0x69, 0x6E, 0x64, 0x6F, 0x77, 0x73, 0x2D, 0x38, 0x37, 0x34 };
		// cswindows874
		static const unsigned char c2[12] =
			{ 0x63, 0x73, 0x77, 0x69, 0x6E, 0x64, 0x6F, 0x77, 0x73, 0x38, 0x37, 0x34 };
		return (
			CharsetEqual(c1, 11) ||
			CharsetEqual(c2, 12));
		/*
		return (
			CharsetEqual(U"windows-874") ||
			CharsetEqual(U"cswindows874"));
		*/
	}

	template <typename TCharactersWriter>
	inline bool Inspector<TCharactersWriter>::IsWindows1250Charset()
	{
		// windows-1250
		static const unsigned char c1[12] =
			{ 0x77, 0x69, 0x6E, 0x64, 0x6F, 0x77, 0x73, 0x2D, 0x31, 0x32, 0x35, 0x30 };
		// cswindows1250
		static const unsigned char c2[13] =
			{ 0x63, 0x73, 0x77, 0x69, 0x6E, 0x64, 0x6F, 0x77, 0x73, 0x31, 0x32, 0x35, 0x30 };
		return (
			CharsetEqual(c1, 12) ||
			CharsetEqual(c2, 13));
		/*
		return (
			CharsetEqual(U"windows-1250") ||
			CharsetEqual(U"cswindows1250"));
		*/
	}

	template <typename TCharactersWriter>
	inline bool Inspector<TCharactersWriter>::IsWindows1251Charset()
	{
		// windows-1251
		static const unsigned char c1[12] =
			{ 0x77, 0x69, 0x6E, 0x64, 0x6F, 0x77, 0x73, 0x2D, 0x31, 0x32, 0x35, 0x31 };
		// cswindows1251
		static const unsigned char c2[13] =
			{ 0x63, 0x73, 0x77, 0x69, 0x6E, 0x64, 0x6F, 0x77, 0x73, 0x31, 0x32, 0x35, 0x31 };
		return (
			CharsetEqual(c1, 12) ||
			CharsetEqual(c2, 13));
		/*
		return (
			CharsetEqual(U"windows-1251") ||
			CharsetEqual(U"cswindows1251"));
		*/
	}

	template <typename TCharactersWriter>
	inline bool Inspector<TCharactersWriter>::IsWindows1252Charset()
	{
		// windows-1252
		static const unsigned char c1[12] =
			{ 0x77, 0x69, 0x6E, 0x64, 0x6F, 0x77, 0x73, 0x2D, 0x31, 0x32, 0x35, 0x32 };
		// cswindows1252
		static const unsigned char c2[13] =
			{ 0x63, 0x73, 0x77, 0x69, 0x6E, 0x64, 0x6F, 0x77, 0x73, 0x31, 0x32, 0x35, 0x32 };
		return (
			CharsetEqual(c1, 12) ||
			CharsetEqual(c2, 13));
		/*
		return (
			CharsetEqual(U"windows-1252") ||
			CharsetEqual(U"cswindows1252"));
		*/
	}

	template <typename TCharactersWriter>
	inline bool Inspector<TCharactersWriter>::IsWindows1253Charset()
	{
		// windows-1253
		static const unsigned char c1[12] =
			{ 0x77, 0x69, 0x6E, 0x64, 0x6F, 0x77, 0x73, 0x2D, 0x31, 0x32, 0x35, 0x33 };
		// cswindows1253
		static const unsigned char c2[13] =
			{ 0x63, 0x73, 0x77, 0x69, 0x6E, 0x64, 0x6F, 0x77, 0x73, 0x31, 0x32, 0x35, 0x33 };
		return (
			CharsetEqual(c1, 12) ||
			CharsetEqual(c2, 13));
		/*
		return (
			CharsetEqual(U"windows-1253") ||
			CharsetEqual(U"cswindows1253"));
		*/
	}

	template <typename TCharactersWriter>
	inline bool Inspector<TCharactersWriter>::IsWindows1254Charset()
	{
		// windows-1254
		static const unsigned char c1[12] =
			{ 0x77, 0x69, 0x6E, 0x64, 0x6F, 0x77, 0x73, 0x2D, 0x31, 0x32, 0x35, 0x34 };
		// cswindows1254
		static const unsigned char c2[13] =
			{ 0x63, 0x73, 0x77, 0x69, 0x6E, 0x64, 0x6F, 0x77, 0x73, 0x31, 0x32, 0x35, 0x34 };
		return (
			CharsetEqual(c1, 12) ||
			CharsetEqual(c2, 13));
		/*
		return (
			CharsetEqual(U"windows-1254") ||
			CharsetEqual(U"cswindows1254"));
		*/
	}

	template <typename TCharactersWriter>
	inline bool Inspector<TCharactersWriter>::IsWindows1255Charset()
	{
		// windows-1255
		static const unsigned char c1[12] =
			{ 0x77, 0x69, 0x6E, 0x64, 0x6F, 0x77, 0x73, 0x2D, 0x31, 0x32, 0x35, 0x35 };
		// cswindows1255
		static const unsigned char c2[13] =
			{ 0x63, 0x73, 0x77, 0x69, 0x6E, 0x64, 0x6F, 0x77, 0x73, 0x31, 0x32, 0x35, 0x35 };
		return (
			CharsetEqual(c1, 12) ||
			CharsetEqual(c2, 13));
		/*
		return (
			CharsetEqual(U"windows-1255") ||
			CharsetEqual(U"cswindows1255"));
		*/
	}

	template <typename TCharactersWriter>
	inline bool Inspector<TCharactersWriter>::IsWindows1256Charset()
	{
		// windows-1256
		static const unsigned char c1[12] =
			{ 0x77, 0x69, 0x6E, 0x64, 0x6F, 0x77, 0x73, 0x2D, 0x31, 0x32, 0x35, 0x36 };
		// cswindows1256
		static const unsigned char c2[13] =
			{ 0x63, 0x73, 0x77, 0x69, 0x6E, 0x64, 0x6F, 0x77, 0x73, 0x31, 0x32, 0x35, 0x36 };
		return (
			CharsetEqual(c1, 12) ||
			CharsetEqual(c2, 13));
		/*
		return (
			CharsetEqual(U"windows-1256") ||
			CharsetEqual(U"cswindows1256"));
		*/
	}

	template <typename TCharactersWriter>
	inline bool Inspector<TCharactersWriter>::IsWindows1257Charset()
	{
		// windows-1257
		static const unsigned char c1[12] =
			{ 0x77, 0x69, 0x6E, 0x64, 0x6F, 0x77, 0x73, 0x2D, 0x31, 0x32, 0x35, 0x37 };
		// cswindows1257
		static const unsigned char c2[13] =
			{ 0x63, 0x73, 0x77, 0x69, 0x6E, 0x64, 0x6F, 0x77, 0x73, 0x31, 0x32, 0x35, 0x37 };
		return (
			CharsetEqual(c1, 12) ||
			CharsetEqual(c2, 13));
		/*
		return (
			CharsetEqual(U"windows-1257") ||
			CharsetEqual(U"cswindows1257"));
		*/
	}

	template <typename TCharactersWriter>
	inline bool Inspector<TCharactersWriter>::IsWindows1258Charset()
	{
		// windows-1258
		static const unsigned char c1[12] =
			{ 0x77, 0x69, 0x6E, 0x64, 0x6F, 0x77, 0x73, 0x2D, 0x31, 0x32, 0x35, 0x38 };
		// cswindows1258
		static const unsigned char c2[13] =
			{ 0x63, 0x73, 0x77, 0x69, 0x6E, 0x64, 0x6F, 0x77, 0x73, 0x31, 0x32, 0x35, 0x38 };
		return (
			CharsetEqual(c1, 12) ||
			CharsetEqual(c2, 13));
		/*
		return (
			CharsetEqual(U"windows-1258") ||
			CharsetEqual(U"cswindows1258"));
		*/
	}

	template <typename TCharactersWriter>
	inline typename Inspector<TCharactersWriter>::AttributeType&
		Inspector<TCharactersWriter>::NewAttribute()
	{
		AttributesSizeType fakeSize = static_cast<AttributesSizeType>(attributesSize);
		if (fakeSize < attributes.size())
		{
			++attributesSize;
			AttributeType& ref = attributes[fakeSize];
			ref.Name.clear();
			ref.Value.clear();
			ref.LocalName.clear();
			ref.Prefix.clear();
			ref.NamespaceUri.clear();
			return ref;
		}

		// fakeSize == attributes.size().
		attributes.push_back(AttributeType());
		++attributesSize;
		AttributeType& ref = attributes.back();
		ref.Name.reserve(NameReserve);
		ref.Value.reserve(ValueReserve);
		ref.LocalName.reserve(LocalNameReserve);
		ref.Prefix.reserve(PrefixReserve);
		ref.NamespaceUri.reserve(NamespaceUriReserve);
		return ref;
	}

	template <typename TCharactersWriter>
	inline typename Inspector<TCharactersWriter>::UnclosedTagType&
		Inspector<TCharactersWriter>::NewUnclosedTag()
	{
		UnclosedTagsSizeType fakeSize = static_cast<UnclosedTagsSizeType>(unclosedTagsSize);
		if (fakeSize < unclosedTags.size())
		{
			++unclosedTagsSize;
			UnclosedTagType& ref = unclosedTags[fakeSize];
			ref.Name.clear();
			ref.LocalName.clear();
			ref.Prefix.clear();
			ref.NamespaceUri.clear();
			return ref;
		}

		// fakeSize == unclosedTags.size().
		unclosedTags.push_back(UnclosedTagType());
		++unclosedTagsSize;
		UnclosedTagType& ref = unclosedTags.back();
		ref.Name.reserve(NameReserve);
		ref.LocalName.reserve(LocalNameReserve);
		ref.Prefix.reserve(PrefixReserve);
		ref.NamespaceUri.reserve(NamespaceUriReserve);
		return ref;
	}

	template <typename TCharactersWriter>
	inline typename Inspector<TCharactersWriter>::NamespaceDeclarationType&
		Inspector<TCharactersWriter>::NewNamespace()
	{
		NamespacesSizeType fakeSize = static_cast<NamespacesSizeType>(namespacesSize);
		if (fakeSize < namespaces.size())
		{
			++namespacesSize;
			NamespaceDeclarationType& ref = namespaces[fakeSize];
			ref.Prefix.clear();
			ref.Uri.clear();
			ref.TagIndex = 0;
			return ref;
		}

		// fakeSize == namespaces.size().
		namespaces.push_back(NamespaceDeclarationType());
		++namespacesSize;
		NamespaceDeclarationType& ref = namespaces.back();
		ref.Prefix.reserve(PrefixReserve);
		ref.Uri.reserve(NamespaceUriReserve);
		return ref;
	}

	template <typename TCharactersWriter>
	inline bool Inspector<TCharactersWriter>::CharsetEqual(const char32_t* charset)
	{
		std::size_t length = 0;
		const char32_t* pt = charset;
		while (*pt != 0)
		{
			++length;
			++pt;
		}

		// comparingName contains encoding name.
		if (length != comparingName.size())
			return false;

		for (std::size_t i = 0; i < length; ++i)
		{
			if (ToLower[static_cast<unsigned char>(comparingName[i])] !=
				ToLower[static_cast<unsigned char>(charset[i])])
				return false;
		}

		return true;
	}

	template <typename TCharactersWriter>
	inline bool Inspector<TCharactersWriter>::CharsetEqual(
		const unsigned char* charset, std::size_t len)
	{
		// comparingName contains encoding name.
		if (len != comparingName.size())
			return false;

		for (std::size_t i = 0; i < len; ++i)
		{
			if (ToLower[static_cast<unsigned char>(comparingName[i])] !=
				ToLower[charset[i]])
				return false;
		}

		return true;
	}

	template <typename TCharactersWriter>
	inline void Inspector<TCharactersWriter>::InitStrings()
	{
		name.reserve(NameReserve);
		value.reserve(ValueReserve);
		localName.reserve(LocalNameReserve);
		prefix.reserve(PrefixReserve);
		namespaceUri.reserve(NamespaceUriReserve);

		lowerXmlString.reserve(3);
		xmlnsString.reserve(5);
		xmlUriString.reserve(36);
		xmlnsUriString.reserve(29);

		for (std::size_t i = 0; i < 3; ++i)
			CharactersWriterType::WriteCharacter(lowerXmlString, LowerXml[i]);

		for (std::size_t i = 0; i < 5; ++i)
			CharactersWriterType::WriteCharacter(xmlnsString, Xmlns[i]);

		for (std::size_t i = 0; i < 36; ++i)
			CharactersWriterType::WriteCharacter(xmlUriString, XmlUri[i]);

		for (std::size_t i = 0; i < 29; ++i)
			CharactersWriterType::WriteCharacter(xmlnsUriString, XmlnsUri[i]);
	}

	template <typename TCharactersWriter>
	inline void Inspector<TCharactersWriter>::SavePosition()
	{
		row = currentRow;
		column = currentColumn;
	}

	template <typename TCharactersWriter>
	inline bool Inspector<TCharactersWriter>::NextCharBad(bool insideTag)
	{
		// x, CR, LF, y => x, LF, y
		// x, CR, y => x, LF, y
		// x, LF, CR, y => x, LF, LF, y
		// Check http://www.w3.org/TR/2008/REC-xml-20081126/#sec-line-ends.

		SizeType tempRow;
		SizeType tempColumn;

		if (currentCharacter == LineFeed)
		{
			++currentRow;
			currentColumn = 1;
		}
		else
		{
			++currentColumn;
		}

		if (bufferedCharacter != 0)
		{
			if (bufferedCharacter > 3) // Allowed character.
			{
				currentCharacter = bufferedCharacter;
				bufferedCharacter = 0;
				return false;
			}

			if (bufferedCharacter == 1) // No more characters to read.
			{
				if (insideTag)
				{
					// Start token position.
					tempRow = row;
					tempColumn = column;
					Reset();
					row = tempRow;
					column = tempColumn;
					SetError(ErrorCode::UnclosedToken);
					eof = true;
				}
				else
				{
					eof = true;
					bufferedCharacter = 0;
				}
			}
			else if (bufferedCharacter == 2) // Character is not allowed in XML document.
			{
				// Invalid character position.
				tempRow = currentRow;
				tempColumn = currentColumn;
				Reset();
				row = tempRow;
				column = tempColumn;
				SetError(ErrorCode::InvalidByteSequence);
			}
			else // bufferedCharacter == 3 // Stream error.
			{
				// Character at stream error position.
				tempRow = currentRow;
				tempColumn = currentColumn;
				Reset();
				row = tempRow;
				column = tempColumn;
				SetError(ErrorCode::StreamError);
			}
			afterBom = true;
			return true;
		}

		int result = reader->ReadCharacter(currentCharacter);

		if (result == 1) // Character was read successfully.
		{
			if (currentCharacter == CarriageReturn) // We don't like CR.
			{
				result = reader->ReadCharacter(currentCharacter);
				if (result == 1) // Second character was read successfully.
				{
					if (currentCharacter != LineFeed)
					{
						// CR, x => LF, x
						bufferedCharacter = currentCharacter;
						currentCharacter = LineFeed;
					}
					// else CR, LF => LF
				}
				else if (result == 0) // No more characters to read.
				{
					// CR, [end of document] => LF, [end of document]
					bufferedCharacter = 1;
					currentCharacter = LineFeed;
				}
				else if (result == -1) // Character is not allowed in XML document.
				{
					// CR, [not allowed character] => LF, [not allowed character]
					bufferedCharacter = 2;
					currentCharacter = LineFeed;
				}
				else // result == -2 // Stream error.
				{
					// CR, [stream error] => LF, [stream error]
					bufferedCharacter = 3;
					currentCharacter = LineFeed;
				}
			}
			return false;
		}

		if (result == 0) // No more characters to read.
		{
			if (insideTag)
			{
				// Start token position.
				tempRow = row;
				tempColumn = column;
				Reset();
				row = tempRow;
				column = tempColumn;
				SetError(ErrorCode::UnclosedToken);
				eof = true;
			}
			else
			{
				eof = true;
			}
		}
		else if (result == -1) // Character is not allowed in XML document.
		{
			// Invalid character position.
			tempRow = currentRow;
			tempColumn = currentColumn;
			Reset();
			row = tempRow;
			column = tempColumn;
			SetError(ErrorCode::InvalidByteSequence);
		}
		else // result == -2 // Stream error.
		{
			// Character at stream error position.
			tempRow = currentRow;
			tempColumn = currentColumn;
			Reset();
			row = tempRow;
			column = tempColumn;
			SetError(ErrorCode::StreamError);
		}
		afterBom = true;
		return true;
	}

	template <typename TCharactersWriter>
	inline bool Inspector<TCharactersWriter>::Inspect()
	{
		SizeType tempRow;
		SizeType tempColumn;
		if (!afterBom && (err == ErrorCode::None || err == ErrorCode::StreamError))
		{
			// First call of Inspect method or after stream error while BOM parsing.
			ParseBom();
			if (err != ErrorCode::None)
				return false;
			row = 1;
			column = 1;
			if (eof)
			{
				SetError(ErrorCode::NoElement);
				return false;
			}
			currentRow = 1;
			currentColumn = 0; // Don't worry,
			// it will be 1 after first call of NextCharBad method.

			// First character.
			if (NextCharBad(false))
			{
				if (eof)
				{
					SetError(ErrorCode::NoElement);
					return false;
				}
			}

			if (sourceType != SourceReader &&
				bom != Details::Bom::None &&
				bom != Details::Bom::Utf8 &&
				bom != Details::Bom::Utf16BE &&
				bom != Details::Bom::Utf16LE)
			{
				// Encoding declaration is required here.

				if (currentCharacter != LessThan)
				{
					Reset();
					SetError(ErrorCode::EncodingDeclarationRequired);
					row = 1;
					column = 1;
					return false;
				}

				if (NextCharBad(true))
					return false;

				if (currentCharacter != Question)
				{
					Reset();
					SetError(ErrorCode::EncodingDeclarationRequired);
					row = 1;
					column = 1;
					return false;
				}

				for (std::size_t i = 0; i < 3; ++i)
				{
					if (NextCharBad(true))
						return false;
					if (currentCharacter != LowerXml[i])
					{
						Reset();
						SetError(ErrorCode::EncodingDeclarationRequired);
						row = 1;
						column = 1;
						return false;
					}
					CharactersWriterType::WriteCharacter(name, currentCharacter);
					CharactersWriterType::WriteCharacter(localName, currentCharacter);
				}

				// '<?xml'
				if (NextCharBad(true))
					return false;

				if (!IsWhiteSpace(currentCharacter))
				{
					Reset();
					SetError(ErrorCode::EncodingDeclarationRequired);
					row = 1;
					column = 1;
					return false;
				}

				return ParseXmlDeclaration();
			}

			if (IsWhiteSpace(currentCharacter))
			{
				do
				{
					CharactersWriterType::WriteCharacter(value, currentCharacter);
					if (NextCharBad(false))
					{
						if (eof) // White spaces followed by end of file.
						{
							// In XML document at least one root element is required.
							tempRow = currentRow;
							tempColumn = currentColumn;
							Reset();
							SetError(ErrorCode::NoElement);
							row = tempRow;
							column = tempColumn;
							return false;
						}
						else // White spaces followed by invalid character or stream error.
						{
							// Error is set already.
							return false;
						}
					}
				}
				while (IsWhiteSpace(currentCharacter));

				if (currentCharacter != LessThan)
				{
					tempRow = currentRow;
					tempColumn = currentColumn;
					Reset();
					SetError(ErrorCode::InvalidSyntax);
					row = tempRow;
					column = tempColumn;
					return false;
				}
				
				node = Inspected::Whitespace;
				return true;
			}

			if (currentCharacter != LessThan)
			{
				tempRow = currentRow;
				tempColumn = currentColumn;
				Reset();
				SetError(ErrorCode::InvalidSyntax);
				row = tempRow;
				column = tempColumn;
			}
		}

		if (err != ErrorCode::None)
			return false;

		if (currentCharacter == GreaterThan) 
		{
			// End of token.
			if (NextCharBad(false) && !eof)
				return false;
		}
		else if (currentCharacter == Semicolon)
		{
			if (!entityName.empty())
			{
				PrepareNode();
				name = entityName;
				localName = entityName;
				entityName.clear();
				node = Inspected::EntityReference;
				row = currentRow;
				column = (currentColumn - entityNameCharCount - 1);
				return true;
			}
			else
			{
				// End of reference.
				if (NextCharBad(false) && !eof)
					return false;
			}
		}

		if (eof)
		{
			if (!foundElement)
			{
				// In XML document at least one root element is required.
				tempRow = currentRow;
				tempColumn = currentColumn;
				Reset();
				SetError(ErrorCode::NoElement);
				row = tempRow;
				column = tempColumn;
			}
			else if (unclosedTagsSize != 0)
			{
				UnclosedTagType& ref = unclosedTags[unclosedTagsSize - 1];
				Reset();
				SetError(ErrorCode::UnclosedTag);
				row = ref.Row;
				column = ref.Column;
				foundElement = true;
			}
			else
			{
				// XML document is fully parsed without any error.
				tempRow = currentRow;
				tempColumn = currentColumn;
				Reset();
				row = tempRow;
				column = tempColumn;
				currentRow = tempRow;
				currentColumn = tempColumn;
				foundElement = true;
				afterBom = true;
			}
			currentCharacter = 0;
			eof = true;
			return false;
		}

		if (currentCharacter == LessThan) // New tag to parse.
		{
			SavePosition();

			// Go to the next character after '<'.
			if (NextCharBad(true))
				return false;

			if (currentCharacter == Slash)
			{
				// EndTag.
				return ParseEndTag();
			}

			if (currentCharacter != Colon &&
				Encoding::CharactersReader::IsNameStartChar(currentCharacter))
			{
				// StartTag or EmptyElementTag.
				return ParseElement();
			}

			if (currentCharacter == Exclamation)
			{
				// Comment or DocumentType.
				return ParseExclamation();
			}

			if (currentCharacter == Question)
			{
				// XmlDeclaration or ProcessingInstruction.
				return ParseQuestion();
			}

			// currentCharacter is not allowed here.
			tempRow = currentRow;
			tempColumn = currentColumn;
			if (Encoding::CharactersReader::IsNameChar(currentCharacter))
			{
				// Not allowed as start character of the name,
				// but allowed as a part of this name.
				Reset();
				SetError(ErrorCode::InvalidTagName);
			}
			else
			{
				// Some weird character.
				Reset();
				SetError(ErrorCode::InvalidSyntax);
			}
			row = tempRow;
			column = tempColumn;
			return false;
		}

		// currentCharacter == first character of text.
		SavePosition();
		return ParseText();
	}

	template <typename TCharactersWriter>
	inline Inspected Inspector<TCharactersWriter>::GetInspected() const
	{
		return node;
	}

	template <typename TCharactersWriter>
	inline const typename Inspector<TCharactersWriter>::StringType&
		Inspector<TCharactersWriter>::GetName() const
	{
		return name;
	}

	template <typename TCharactersWriter>
	inline const typename Inspector<TCharactersWriter>::StringType&
		Inspector<TCharactersWriter>::GetValue() const
	{
		return value;
	}

	template <typename TCharactersWriter>
	inline const typename Inspector<TCharactersWriter>::StringType&
		Inspector<TCharactersWriter>::GetLocalName() const
	{
		return localName;
	}

	template <typename TCharactersWriter>
	inline const typename Inspector<TCharactersWriter>::StringType&
		Inspector<TCharactersWriter>::GetPrefix() const
	{
		return prefix;
	}

	template <typename TCharactersWriter>
	inline const typename Inspector<TCharactersWriter>::StringType&
		Inspector<TCharactersWriter>::GetNamespaceUri() const
	{
		return namespaceUri;
	}

	template <typename TCharactersWriter>
	inline bool Inspector<TCharactersWriter>::HasAttributes() const
	{
		return (attributesSize != 0);
	}

	template <typename TCharactersWriter>
	inline typename Inspector<TCharactersWriter>::SizeType
		Inspector<TCharactersWriter>::GetAttributesCount() const
	{
		return static_cast<SizeType>(attributesSize);
	}

	template <typename TCharactersWriter>
	inline const typename Inspector<TCharactersWriter>::AttributeType&
		Inspector<TCharactersWriter>::GetAttributeAt(SizeType index) const
	{
		if (index >= attributesSize)
			throw std::out_of_range("Attempt to access out of range element.");
		return attributes[static_cast<AttributesSizeType>(index)];
	}

	template <typename TCharactersWriter>
	inline const char* Inspector<TCharactersWriter>::GetErrorMessage() const
	{
		return errMsg;
	}

	template <typename TCharactersWriter>
	inline ErrorCode Inspector<TCharactersWriter>::GetErrorCode() const
	{
		return err;
	}

	template <typename TCharactersWriter>
	inline typename Inspector<TCharactersWriter>::SizeType
		Inspector<TCharactersWriter>::GetRow() const
	{
		return row;
	}

	template <typename TCharactersWriter>
	inline typename Inspector<TCharactersWriter>::SizeType
		Inspector<TCharactersWriter>::GetColumn() const
	{
		return column;
	}

	template <typename TCharactersWriter>
	inline typename Inspector<TCharactersWriter>::SizeType
		Inspector<TCharactersWriter>::GetDepth() const
	{
		if (node == Inspected::StartTag)
			return static_cast<SizeType>(unclosedTagsSize - 1);
		else
			return static_cast<SizeType>(unclosedTagsSize);
	}

	template <typename TCharactersWriter>
	inline void Inspector<TCharactersWriter>::Reset()
	{
		row = 0;
		column = 0;
		currentRow = 0;
		currentColumn = 0;
		node = Inspected::None;
		err = ErrorCode::None;
		errMsg = nullptr;
		afterBom = false;
		bom = Details::Bom::None;
		name.clear();
		value.clear();
		localName.clear();
		prefix.clear();
		namespaceUri.clear();
		entityName.clear();
		entityNameCharCount = 0;
		currentCharacter = 0;
		bufferedCharacter = 0;
		foundElement = false;
		foundDOCTYPE = false;
		eof = false;
		attributesSize = 0;
		unclosedTagsSize = 0;
		namespacesSize = 0;
		if (sourceType == SourcePath)
		{
			fPath.clear();
			if (fileStream.is_open())
				fileStream.close();
			fileStream.clear();
			delete reader;
			reader = nullptr;
		}
		else if (sourceType == SourceReader)
		{
			reader = nullptr;
		}
		else if (sourceType == SourceStream)
		{
			inputStreamPtr = nullptr;
			delete reader;
			reader = nullptr;
		}
		else if (sourceType == SourceIterators)
		{
			delete inputStreamPtr->rdbuf();
			delete inputStreamPtr;
			inputStreamPtr = nullptr;
			delete reader;
			reader = nullptr;
		}
		sourceType = SourceNone;
	}

	template <typename TCharactersWriter>
	inline void Inspector<TCharactersWriter>::Reset(const char* filePath)
	{
		Reset();
		fPath = filePath;
		sourceType = SourcePath;
	}

	template <typename TCharactersWriter>
	inline void Inspector<TCharactersWriter>::Reset(const std::string& filePath)
	{
		Reset();
		fPath = filePath;
		sourceType = SourcePath;
	}

	template <typename TCharactersWriter>
	inline void Inspector<TCharactersWriter>::Reset(std::istream* inputStream)
	{
		Reset();
		inputStreamPtr = inputStream;
		sourceType = SourceStream;
	}

	template <typename TCharactersWriter>
	template <typename TInputIterator>
	inline void Inspector<TCharactersWriter>::Reset(
		TInputIterator first, TInputIterator last)
	{
		Reset();
		std::unique_ptr<Details::BasicIteratorsBuf<TInputIterator, char> > buf(
			new Details::BasicIteratorsBuf<TInputIterator, char>(first, last));
		inputStreamPtr = new std::istream(buf.get());
		buf.release();
		sourceType = SourceIterators;
	}

	template <typename TCharactersWriter>
	inline void Inspector<TCharactersWriter>::Reset(Encoding::CharactersReader* r)
	{
		Reset();
		if (r != nullptr)
		{
			reader = r;
			sourceType = SourceReader;
		}
	}

	template <typename TCharactersWriter>
	inline void Inspector<TCharactersWriter>::Clear()
	{
		Reset();
		attributes.clear();
		unclosedTags.clear();
		namespaces.clear();
	}

	/// @cond DETAILS
	namespace Details
	{
		inline Bom ReadBom(std::istream* inputStream)
		{
			// UTF-8          EF BB BF
			// UTF-16 (BE)    FE FF
			// UTF-16 (LE)    FF FE
			// UTF-32 (BE)    00 00 FE FF
			// UTF-32 (LE)    FF FE 00 00

			if (inputStream != nullptr)
			{
				// Check first byte.
				int oneByte = inputStream->peek();

				if (oneByte == std::char_traits<char>::eof())
				{
					if ((inputStream->rdstate() & std::istream::eofbit) != 0)
						return Bom::None; // End of the stream.
					return Bom::StreamError;
				}

				if (oneByte == 0xEF) // Should be UTF-8
				{
					// Extract first byte.
					inputStream->get();

					// Extract second byte.
					oneByte = inputStream->get();
					if (oneByte == std::char_traits<char>::eof())
					{
						if ((inputStream->rdstate() & std::istream::eofbit) != 0)
							return Bom::Invalid;
						return Bom::StreamError;
					}
					if (oneByte != 0xBB)
						return Bom::Invalid;

					// Extract third byte.
					oneByte = inputStream->get();
					if (oneByte == std::char_traits<char>::eof())
					{
						if ((inputStream->rdstate() & std::istream::eofbit) != 0)
							return Bom::Invalid;
						return Bom::StreamError;
					}
					if (oneByte != 0xBF)
						return Bom::Invalid;

					return Bom::Utf8;
				}
				else if (oneByte == 0xFE) // Should be UTF-16 (BE)
				{
					// Extract first byte.
					inputStream->get();

					// Extract second byte.
					oneByte = inputStream->get();
					if (oneByte == std::char_traits<char>::eof())
					{
						if ((inputStream->rdstate() & std::istream::eofbit) != 0)
							return Bom::Invalid;
						return Bom::StreamError;
					}
					if (oneByte != 0xFF)
						return Bom::Invalid;

					return Bom::Utf16BE;
				}
				else if (oneByte == 0xFF) // Should be UTF-16 (LE) of UTF-32 (LE)
				{
					// Extract first byte.
					inputStream->get();

					// Extract second byte.
					oneByte = inputStream->get();
					if (oneByte == std::char_traits<char>::eof())
					{
						if ((inputStream->rdstate() & std::istream::eofbit) != 0)
							return Bom::Invalid;
						return Bom::StreamError;
					}
					if (oneByte != 0xFE)
						return Bom::Invalid;

					// Check third byte.
					oneByte = inputStream->peek();
					if (oneByte == std::char_traits<char>::eof())
					{
						if ((inputStream->rdstate() & std::istream::eofbit) != 0)
							return Bom::Utf16LE;
						return Bom::StreamError;
					}
					if (oneByte != 0x00)
						return Bom::Utf16LE;

					// Should be UTF-32 (LE)
					// Extract third byte.
					inputStream->get();

					// Extract fourth byte.
					oneByte = inputStream->get();
					if (oneByte == std::char_traits<char>::eof())
					{
						if ((inputStream->rdstate() & std::istream::eofbit) != 0)
							return Bom::Invalid;
						return Bom::StreamError;
					}
					if (oneByte != 0x00)
						return Bom::Invalid;

					return Bom::Utf32LE;
				}
				else if (oneByte == 0x00) // Should be UTF-32 (BE)
				{
					// Extract first byte.
					inputStream->get();

					// Extract second byte.
					oneByte = inputStream->get();
					if (oneByte == std::char_traits<char>::eof())
					{
						if ((inputStream->rdstate() & std::istream::eofbit) != 0)
							return Bom::Invalid;
						return Bom::StreamError;
					}
					if (oneByte != 0x00)
						return Bom::Invalid;

					// Extract third byte.
					oneByte = inputStream->get();
					if (oneByte == std::char_traits<char>::eof())
					{
						if ((inputStream->rdstate() & std::istream::eofbit) != 0)
							return Bom::Invalid;
						return Bom::StreamError;
					}
					if (oneByte != 0xFE)
						return Bom::Invalid;

					// Extract fourth byte.
					oneByte = inputStream->get();
					if (oneByte == std::char_traits<char>::eof())
					{
						if ((inputStream->rdstate() & std::istream::eofbit) != 0)
							return Bom::Invalid;
						return Bom::StreamError;
					}
					if (oneByte != 0xFF)
						return Bom::Invalid;

					return Bom::Utf32BE;
				}
				else
				{
					return Bom::None;
				}
			}
			return Bom::StreamError;
		}
		
		template <typename TInputIterator>
		inline Bom ReadBom(TInputIterator& first, TInputIterator& last)
		{
			// UTF-8          EF BB BF
			// UTF-16 (BE)    FE FF
			// UTF-16 (LE)    FF FE
			// UTF-32 (BE)    00 00 FE FF
			// UTF-32 (LE)    FF FE 00 00

			if (first == last)
				return Bom::None;

			if (*first == 0xEF) // Should be UTF-8
			{
				++first;
				if (first == last || *first != 0xBB)
					return Bom::Invalid;

				++first;
				if (first == last || *first != 0xBF)
					return Bom::Invalid;

				++first;
				return Bom::Utf8;
			}
			else if (*first == 0xFE) // Should be UTF-16 (BE)
			{
				++first;
				if (first == last || *first != 0xFF)
					return Bom::Invalid;

				++first;
				return Bom::Utf16BE;
			}
			else if (*first == 0xFF) // Should be UTF-16 (LE) of UTF-32 (LE)
			{
				++first;
				if (first == last || *first != 0xFE)
					return Bom::Invalid;

				++first;
				if (first == last || *first != 0x00)
					return Bom::Utf16LE;

				// Should be UTF-32 (LE)
				++first;
				if (first == last || *first != 0x00)
					return Bom::Invalid;

				++first;
				return Bom::Utf32LE;
			}
			else if (*first == 0x00) // Should be UTF-32 (BE)
			{
				++first;
				if (first == last || *first != 0x00)
					return Bom::Invalid;

				++first;
				if (first == last || *first != 0xFE)
					return Bom::Invalid;

				++first;
				if (first == last || *first != 0xFF)
					return Bom::Invalid;

				++first;
				return Bom::Utf32BE;
			}
			else
			{
				return Bom::None;
			}
		}

		template <
			typename TInputIterator,
			typename TCharacterType,
			typename TTraits>
		inline typename BasicIteratorsBuf<TInputIterator, TCharacterType, TTraits>::int_type
			BasicIteratorsBuf<TInputIterator, TCharacterType, TTraits>::underflow()
		{
			if (curIter == endIter)
				return traits_type::eof();

			return traits_type::to_int_type(*curIter);
		}

		template <
			typename TInputIterator,
			typename TCharacterType,
			typename TTraits>
		inline typename BasicIteratorsBuf<TInputIterator, TCharacterType, TTraits>::int_type
			BasicIteratorsBuf<TInputIterator, TCharacterType, TTraits>::uflow()
		{
			if (curIter == endIter)
				return traits_type::eof();

			return traits_type::to_int_type(*curIter++);
		}

		template <
			typename TInputIterator,
			typename TCharacterType,
			typename TTraits>
		inline std::streamsize
			BasicIteratorsBuf<TInputIterator, TCharacterType, TTraits>::showmanyc()
		{
			return (curIter != endIter)
				? 1
				: 0;
		}
	}
	/// @endcond
}

#endif

