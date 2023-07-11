namespace NAMESPACE
{
	bool __stdcall TestAniEvent(oCNpc* npc, zCModelAniEvent* event)
	{
		if (event->tagString != "DEF_CALL_SCRIPT")
			return false;

		oCMsgManipulate* message = new oCMsgManipulate{ oCMsgManipulate::EV_CALLSCRIPT, event->string[0], -1 };
		message->SetHighPriority(true);
		npc->GetEM()->OnMessage(message, npc);

		return true;
	}

	Sub executeScript(ZSUB(GameEvent::Entry), []
		{
			CPatchInteger testAniEvent;
			testAniEvent.Init();
			testAniEvent.SetObjectName("test_ani_event");
			testAniEvent.SetValue(reinterpret_cast<int>(&TestAniEvent));
			testAniEvent.DontRemove();
			CPatch::ExecuteResource(CPlugin::GetCurrentPlugin()->GetModule(), MAKEINTRESOURCE(IDR_PATCH1), "PATCH");
		});
}
