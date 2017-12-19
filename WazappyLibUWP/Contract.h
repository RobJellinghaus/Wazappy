// Licensed under the MIT License.

#pragma once

#include <iostream>
#include <string>

namespace Wazappy
{
	class Contract
	{
	public:
		static void Requires(bool value)
		{
			Requires(value, L"Contractual requirement");
		}

		static void Requires(bool value, std::wstring label)
		{
			if (!value)
			{
				std::wcerr << L"Contract.Requires failure: " << label << L"\r\n";
				std::wcerr.flush();

				std::abort();
			}
		}
	};
}