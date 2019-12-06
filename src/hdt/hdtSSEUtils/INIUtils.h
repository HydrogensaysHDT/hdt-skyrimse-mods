#pragma once

#include <cstdlib>
#include <string>

namespace hdt
{
	template <class T> T GetPrivateProfile(const char* clsnm, const char* rcrdnm, T def, const char* ini);
	template <> short GetPrivateProfile(const char* clsnm, const char* rcrdnm, short def, const char* ini);
	template <> unsigned GetPrivateProfile(const char* clsnm, const char* rcrdnm, unsigned def, const char* ini);
	template <> int GetPrivateProfile(const char* clsnm, const char* rcrdnm, int def, const char* ini);
	template <> double GetPrivateProfile(const char* clsnm, const char* rcrdnm, double def, const char* ini);
	template <> float GetPrivateProfile(const char* clsnm, const char* rcrdnm, float def, const char* ini);
	template <> bool GetPrivateProfile(const char* clsnm, const char* rcrdnm, bool def, const char* ini);
	template <> std::string GetPrivateProfile(const char* clsnm, const char* rcrdnm, std::string def, const char* ini);
	template <> long long GetPrivateProfile(const char* clsnm, const char* rcrdnm, long long def, const char* ini);

	template<class T> inline void WritePrivateProfile(const char* clsnm, const char* rcrdnm, const T& val, const char* ini)
	{
		WritePrivateProfile(clsnm, rcrdnm, std::to_string(val), ini);
	}

	template<> void WritePrivateProfile(const char* clsnm, const char* rcrdnm, const char* const& val, const char* ini);
	template<> void WritePrivateProfile(const char* clsnm, const char* rcrdnm, const std::string& val, const char* ini);
	template<> void WritePrivateProfile(const char* clsnm, const char* rcrdnm, const bool& val, const char* ini);

	template<class T>
	class INIWrapper
	{
	public:
		INIWrapper(const char* clsnm, const char* rcrdnm, const T& def, const char* flnm)
			: clsnm(clsnm), rcrdnm(rcrdnm), flnm(flnm), def(def)
		{
			reload();
		}

		operator const T&()const { return val; }
		void set(const T& rhs) { val = rhs; WritePrivateProfile(clsnm, rcrdnm, val, flnm); }

		void reload()
		{
			val = GetPrivateProfile(clsnm, rcrdnm, def, flnm);
			WritePrivateProfile(clsnm, rcrdnm, val, flnm);
		}

	private:
		T val;
		T def;
		const char* clsnm;
		const char* rcrdnm;
		const char* flnm;
	};
}
