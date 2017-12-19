// Licensed under the MIT License.
// Based on WindowsAudioSession sample from https://github.com/Microsoft/Windows-universal-samples

#include "pch.h"
#include "WazappyDllInterface.h"
#include "WASAPIRenderer.h"

using namespace Wazappy;

class WASAPIClient :
	public RuntimeClass< RuntimeClassFlags< ClassicCom >, FtmBase, IWASAPIClient >
{
	ComPtr<IWASAPIRenderer> CreateRenderer()
	{
		return Make<WASAPIRenderer>();
	}
};

ComPtr<IWASAPIClient> WASAPIClientFactory::CreateClient()
{
	return Make<WASAPIClient>();
}

