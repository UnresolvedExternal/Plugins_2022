namespace NAMESPACE
{
	namespace Options
	{
		ZOPTION(MaxLookAngleH, 90.0f);
		ZOPTION(MaxLookAngleV, 181.0f);
	}

	namespace Options
	{
		Sub addTrivia(ZSUB(GameEvent::Execute), []
			{

			});

		Sub load(ZSUB(GameEvent::DefineExternals), []()
			{
				ActiveOptionBase::LoadAll();
			});
	}
}
