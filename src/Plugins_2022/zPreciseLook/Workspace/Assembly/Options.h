namespace NAMESPACE
{
	namespace Options
	{
		ZOPTION(MaxAngleX, 80.0f);
		ZOPTION(MaxAngleY, 45.0f);
		ZOPTION(DegreesPerSecond, 180.0f);
	}

	namespace Options
	{
		Sub load(ZSUB(GameEvent::DefineExternals), []()
			{
				ActiveOptionBase::LoadAll();
			});
	}
}
