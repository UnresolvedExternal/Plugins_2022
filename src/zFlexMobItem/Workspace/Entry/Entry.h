namespace NAMESPACE
{
	zSTRING GetScemeName(oCMobInter* mob, oCNpc* npc)
	{
#if ENGINE == Engine_G1
		mob->SearchFreePosition(npc);
#else
		mob->SearchFreePosition(npc, 150.0f);
#endif

		zSTRING dummy;

		void** vtable = *reinterpret_cast<void***>(mob);
		vtable += 42;
		zSTRING*(__thiscall* func)(oCMOB*, zSTRING*) = reinterpret_cast<decltype(func)>(*vtable);
		return zSTRING{ *func(mob, &dummy) };
	}

	int __stdcall GetUseWithItem(oCMobInter* mob, oCNpc* npc, int instance)
	{
		static int scriptFunc = parser->GetIndex("G_GetUseWithItem");

		if (scriptFunc != -1)
			return CallParser<int>(parser, scriptFunc, GetScemeName(mob, npc), npc, instance);

		return instance;
	}

	Sub executePatch(ZSUB(GameEvent::Entry), []
		{
			CPatchInteger getUseWithItem;
			getUseWithItem.Init();
			getUseWithItem.SetObjectName("GetUseWithItem");
			getUseWithItem.SetValue(reinterpret_cast<int>(&GetUseWithItem));
			getUseWithItem.DontRemove();

			CPatch::ExecuteResource(CPlugin::GetCurrentPlugin()->GetModule(), MAKEINTRESOURCE(IDR_PATCH1), "PATCH");
		});
}
