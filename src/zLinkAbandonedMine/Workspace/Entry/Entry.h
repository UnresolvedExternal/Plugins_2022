namespace NAMESPACE
{
	class TemporaryTriggerChange
	{
	private:
		std::vector<Sub<void>> subs;
		ZOwner<oCTriggerChangeLevel> trigger;
		zSTRING levelName;
		zSTRING startVob;

		void OnLoadBegin()
		{
			delete this;
		}

		void OnSaveBegin()
		{
			std::swap(trigger->levelName, levelName);
			std::swap(trigger->startVob, startVob);
		}

		void OnSaveEnd()
		{
			std::swap(trigger->levelName, levelName);
			std::swap(trigger->startVob, startVob);
		}

		void OnExit()
		{
			delete this;
		}

		~TemporaryTriggerChange()
		{
			trigger->levelName = levelName;
			trigger->startVob = startVob;
		}

	public:
		TemporaryTriggerChange(oCTriggerChangeLevel* trigger, const zSTRING& levelName, const zSTRING& startVob) :
			trigger{ trigger },
			levelName{ trigger->levelName },
			startVob{ trigger->startVob }
		{
			trigger->AddRef();
			trigger->levelName = levelName;
			trigger->startVob = startVob;

			ADDSUB(LoadBegin);
			ADDSUB(SaveBegin);
			ADDSUB(SaveEnd);
			ADDSUB(Exit);
		}
	};

	Sub changeTrigger(ZSUB(GameEvent::LoadEnd), []
		{
			oCWorld* const world = ogame->GetGameWorld();

			zSTRING triggerName;
			zSTRING levelName;
			zSTRING startVob;

			if (world->worldName.CompareI("NEWWORLD"))
			{
				triggerName = "NW_2_OW_ABANDONEDMINE";
				levelName = "ABANDONEDMINE.ZEN";
				startVob = "START_NW";
			}
			else if (world->worldName.CompareI("OLDWORLD"))
			{
				triggerName = "OW_2_NW_ABANDONEDMINE";
				levelName = "ABANDONEDMINE.ZEN";
				startVob = "START_OW";
			}
			else
				return;

			if (oCTriggerChangeLevel* trigger = dynamic_cast<oCTriggerChangeLevel*>(world->SearchVobByName(triggerName)))
				new TemporaryTriggerChange{ trigger, levelName, startVob };
		});
}
