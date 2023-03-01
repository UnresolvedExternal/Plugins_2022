namespace NAMESPACE
{
	Sub onLoadEnd(ZSUB(GameEvent::LoadEnd), []
		{
			oCWorld* const world = ogame->GetGameWorld();

			if (!world)
				return;

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
			{
				trigger->levelName = levelName;
				trigger->startVob = startVob;
			}
		});
}
