namespace NAMESPACE
{
	void LiftAgent::OnLoadEnd()
	{
		lift.Clear();
		ogame->GetWorld()->TraverseVobTree(lift, nullptr, &ogame->GetWorld()->globalVobTree);
	}

	void LiftAgent::OnLoop()
	{
		if (!ogame->game_testmode)
		{
			debugMode = TDebugMode::OFF;
			return;
		}

		timer[0u].Suspend(ogame->singleStep);

		bool debugPressed = zinput->KeyPressed(KEY_U) && timer[0u].Await(200, true);
		bool teleportPressed = zinput->KeyPressed(KEY_I) && timer[0u].Await(200, true);
		bool togglePressed = zinput->GetMouseButtonPressedRight() && timer[0u].Await(200, true);

		if (debugPressed)
		{
			debugMode = debugMode == TDebugMode::ON ? TDebugMode::OFF : TDebugMode::ON;
		}

		if (debugMode != TDebugMode::ON)
		{
			return;
		}

		if (togglePressed)
		{
			lift.ToggleLifts();
		}

		if (teleportPressed)
		{
			zCVob* vob = lift.GetNext().first;
			player->ResetPos(vob->GetPositionWorld() + zVEC3(0, 200, 0));
		}

		lift.Debug();
	}

	void LiftAgent::OnRemove(zCWorld* world, zCVob* vob)
	{
		lift.Remove(vob);
	}

	LiftAgent::LiftAgent() :
		debugMode(TDebugMode::OFF)
	{
		ADDSUB(LoadEnd);
		ADDSUB(Loop);
	}

	LiftAgent* liftAgent;

	Sub createAgent(ZSUB(GameEvent::Entry), []
		{
			liftAgent = new LiftAgent{};
		});

	void __fastcall Hook_zCWorld_VobRemovedFromWorld(zCWorld*, void*, zCVob*);
	Hook<void(__thiscall*)(zCWorld*, zCVob*)> Ivk_zCWorld_VobRemovedFromWorld(ZENFOR(0x005F64C0, 0x006174A0, 0x0061D1E0, 0x00624970), &Hook_zCWorld_VobRemovedFromWorld, HookMode::Patch);
	void __fastcall Hook_zCWorld_VobRemovedFromWorld(zCWorld* world, void* vtable, zCVob* vob)
	{
		liftAgent->OnRemove(world, vob);
		Ivk_zCWorld_VobRemovedFromWorld(world, vob);
	}
}
