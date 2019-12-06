#include "XmlReader.h"

namespace hdt
{
	static inline float convertFloat(const std::string& str)
	{
		char* end;
		float ret = strtof(str.c_str(), &end);
		if (end != str.c_str() + str.length())
			throw std::string("not a float value");
		return ret;
	}

	static inline int convertInt(const std::string& str)
	{
		auto begin = str.c_str();
		char* end;

		int radix = 10;
		if (!str.compare(0, 2, "0x"))
		{
			radix = 16;
			begin += 2;
		}
		else if (str.length() > 1 && str[0] == '0')
		{
			begin += 1;
			radix = 8;
		}

		int ret = strtol(str.c_str(), &end, radix);
		if (end != str.c_str() + str.length())
			throw std::string("not a int value");
		return ret;
	}

	static inline bool convertBool(const std::string& str)
	{
		if (str == "true" || str == "1")
			return true;
		else if (str == "false" || str == "0")
			return false;
		else throw std::string("not a boolean");
	}

	bool XMLReader::Inspect()
	{
		if (Base::GetInspected() == Inspected::EmptyElementTag && isEmptyStart)
			return isEmptyStart = false, true;
		if (!Base::Inspect()) return false;
		if (Base::GetInspected() == Inspected::EmptyElementTag)
			isEmptyStart = true;
		return true;
	}

	Xml::Inspected XMLReader::GetInspected()
	{
		auto ret = Base::GetInspected();
		if (ret == Inspected::EmptyElementTag)
			if (isEmptyStart) return Inspected::StartTag;
			else return Inspected::EndTag;
		else return ret;
	}

	void XMLReader::skipCurrentElement()
	{
		if (GetInspected() == Inspected::EndTag) return;

		int currentDepth = 1;
		while (currentDepth && Inspect())
		{
			switch (GetInspected())
			{
			case Inspected::StartTag:
				++currentDepth; break;
			case Inspected::EndTag:
				--currentDepth; break;
			}
		}
	}

	void XMLReader::nextStartElement()
	{
		while (Inspect() && GetInspected() != Inspected::StartTag);
	}

	bool XMLReader::hasAttribute(const std::string& name)
	{
		for (int i = 0; i < GetAttributesCount(); ++i)
		{
			auto attr = GetAttributeAt(i);
			if (attr.Name == name)
				return true;
		}
		return false;
	}

	std::string XMLReader::getAttribute(const std::string& name)
	{
		for (int i = 0; i < GetAttributesCount(); ++i)
		{
			auto attr = GetAttributeAt(i);
			if (attr.Name == name)
				return attr.Value;
		}
		throw std::string("missing attribute : " + name);
	}

	std::string XMLReader::getAttribute(const std::string& name, const std::string& def)
	{
		for (int i = 0; i < GetAttributesCount(); ++i)
		{
			auto attr = GetAttributeAt(i);
			if (attr.Name == name)
				return attr.Value;
		}
		return def;
	}

	float XMLReader::getAttributeAsFloat(const std::string& name)
	{
		return convertFloat(getAttribute(name));
	}

	int XMLReader::getAttributeAsInt(const std::string& name)
	{
		return convertInt(getAttribute(name));
	}

	bool XMLReader::getAttributeAsBool(const std::string& name)
	{
		return convertBool(getAttribute(name));
	}

	std::string XMLReader::readText()
	{
		Inspect();
		auto ret = GetValue();
		skipCurrentElement();
		return ret;
	}

	float XMLReader::readFloat()
	{
		Inspect();
		auto ret = convertFloat(GetValue());
		skipCurrentElement();
		return ret;
	}

	int XMLReader::readInt()
	{
		Inspect();
		auto ret = convertInt(GetValue());
		skipCurrentElement();
		return ret;
	}

	bool XMLReader::readBool()
	{
		Inspect();
		auto ret = convertBool(GetValue());
		skipCurrentElement();
		return ret;
	}

	btVector3 XMLReader::readVector3()
	{
		float x = getAttributeAsFloat("x");
		float y = getAttributeAsFloat("y");
		float z = getAttributeAsFloat("z");
		skipCurrentElement();
		return btVector3(x, y, z);
	}

	btQuaternion XMLReader::readQuaternion()
	{
		float x = getAttributeAsFloat("x");
		float y = getAttributeAsFloat("y");
		float z = getAttributeAsFloat("z");
		float w = getAttributeAsFloat("w");
		skipCurrentElement();
		btQuaternion q(x, y, z, w);
		if (btFuzzyZero(q.length2()))
			q = btQuaternion::getIdentity();
		else q.normalize();
		return q;
	}

	btQuaternion XMLReader::readAxisAngle()
	{
		float x = getAttributeAsFloat("x");
		float y = getAttributeAsFloat("y");
		float z = getAttributeAsFloat("z");
		float w = getAttributeAsFloat("angle");
		skipCurrentElement();
		btQuaternion q;
		btVector3 axis(x, y, z);
		if (axis.fuzzyZero())
		{
			axis.setX(1);
			w = 0;
		}
		else axis.normalize();
		q.setRotation(axis, w);
		return q;
	}

	btTransform XMLReader::readTransform()
	{
		btTransform ret(btTransform::getIdentity());
		while (Inspect())
		{
			switch (GetInspected())
			{
			case Inspected::StartTag:
				if (GetName() == "basis")
					ret.setRotation(readQuaternion());
				else if (GetName() == "basis-axis-angle")
					ret.setRotation(readAxisAngle());
				else if (GetName() == "origin")
					ret.setOrigin(readVector3());
				break;
			case Inspected::EndTag:
				return ret;
			}
		}
		return ret;
	}
}
