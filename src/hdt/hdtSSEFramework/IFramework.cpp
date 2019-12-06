#include "stdafx.h"
#include "IFramework.h"
#include "FrameworkImpl.h"
#include <skse64/skse64_common/skse_version.h>

namespace hdt
{
	IFramework* IFramework::instance()
	{
		return FrameworkImpl::instance();
	}
}

hdt::IFramework * hdtGetFramework()
{
	return hdt::IFramework::instance();
}
