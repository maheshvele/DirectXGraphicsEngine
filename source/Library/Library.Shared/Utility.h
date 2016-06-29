#pragma once

namespace Library
{
	class Utility
	{
	public:

#if (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
		static std::wstring ExecutableDirectory();
#endif
		static void ToWideString(const std::string& source, std::wstring& dest);

		static std::wstring ToWideString(const std::string& source);

		static std::string LoadBinaryFile(const std::string& filename);

		static float ConvertDipsToPixels(float dips, float dpi);

		inline bool SdkLayersAvailable();
	};
}
