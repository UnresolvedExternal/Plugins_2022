namespace NAMESPACE
{
	namespace Options
	{
		ZOPTION(TimeMs, 2000);
		ZOPTION(DeltaY, 100.0f);
	}

	namespace Options
	{
		Sub addTrivia(ZSUB(GameEvent::Execute), []
			{
				TimeMs.endTrivia += A"... sliding time considered as stucking";
				DeltaY.endTrivia += A"... stucking assumes Y position doesn't change too much";
			});

		Sub load(ZSUB(GameEvent::DefineExternals), []()
			{
				ActiveOptionBase::LoadAll();
			});
	}
}
