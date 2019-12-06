#pragma once

namespace hdt
{
	struct FrameEvent
	{
		bool frameEnd;
	};

	struct ShutdownEvent
	{
	};

	void hookEngine();
	void unhookEngine();
}
