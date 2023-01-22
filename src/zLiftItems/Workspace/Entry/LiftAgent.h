namespace NAMESPACE
{
	class LiftAgent
	{
	private:
		enum TDebugMode { ON, OFF };

		std::vector<Sub<void>> subs;
		Lift lift;
		TDebugMode debugMode;
		Timer timer;

		void OnLoadEnd();
		void OnLoop();

	public:
		LiftAgent();
		void OnRemove(zCWorld*, zCVob* vob);
	};
}
