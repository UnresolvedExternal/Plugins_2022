namespace NAMESPACE
{
	namespace Options
	{
		ZOPTION(HalfAngle, 60.0f);
	}

	namespace Options
	{
		Sub addTrivia(ZSUB(GameEvent::Entry), []
			{
				HalfAngle.endTrivia += A"... enemies hit in a cone: +-HalfAngle degrees";
			});

		Sub load(ZSUB(GameEvent::DefineExternals), []()
			{
				ActiveOptionBase::LoadAll();
			});
	}
}
