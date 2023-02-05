namespace NAMESPACE
{
	bool IsValidPlayerFocus(zCVob* vob)
	{
		if (!vob)
			return true;

		if (oCItem* item = vob->CastTo<oCItem>())
			return !item->HasFlag(ITM_FLAG_NFOCUS);

		if (oCNpc* npc = vob->CastTo<oCNpc>())
		{
			return
				!npc->IsSelfPlayer() && !npc->HasFlag(NPC_FLAG_NFOCUS)
#if ENGINE >= Engine_G2
				&& !npc->noFocus
#endif
				;
		}

		return true;
	}

	int __fastcall Hook_oCNpc_FocusCheck(oCNpc*, void*, zCVob const*, int, int, float, float&);
	Hook<int(__thiscall*)(oCNpc*, zCVob const*, int, int, float, float&)> Ivk_oCNpc_FocusCheck(ZENFOR(0x006905D0, 0x006C19C0, 0x006D4BA0, 0x007331C0), &Hook_oCNpc_FocusCheck, HookMode::Patch);
	int __fastcall Hook_oCNpc_FocusCheck(oCNpc* npc, void* vtable, zCVob const* vob, int a1, int a2, float a3, float& a4)
	{
		if (npc->IsSelfPlayer() && !IsValidPlayerFocus(const_cast<zCVob*>(vob)))
			return false;

		return Ivk_oCNpc_FocusCheck(npc, vob, a1, a2, a3, a4);
	}

	int __fastcall Hook_oCNpc_FocusCheckBBox(oCNpc*, void*, zCVob const*, int, int, float, float&);
	Hook<int(__thiscall*)(oCNpc*, zCVob const*, int, int, float, float&)> Ivk_oCNpc_FocusCheckBBox(ZENFOR(0x00690350, 0x006C16E0, 0x006D4920, 0x00732F40), &Hook_oCNpc_FocusCheckBBox, HookMode::Patch);
	int __fastcall Hook_oCNpc_FocusCheckBBox(oCNpc* npc, void* vtable, zCVob const* vob, int a1, int a2, float a3, float& a4)
	{
		if (npc->IsSelfPlayer() && !IsValidPlayerFocus(const_cast<zCVob*>(vob)))
			return false;

		return Ivk_oCNpc_FocusCheckBBox(npc, vob, a1, a2, a3, a4);
	}

	bool filterVobs;

	void __fastcall Hook_oCNpc_GetNearestValidVob(oCNpc*, void*, float);
	Hook<void(__thiscall*)(oCNpc*, float)> Ivk_oCNpc_GetNearestValidVob(ZENFOR(0x00691410, 0x006C2A00, 0x006D5ED0, 0x007344F0), &Hook_oCNpc_GetNearestValidVob, HookMode::Patch);
	void __fastcall Hook_oCNpc_GetNearestValidVob(oCNpc* npc, void* vtable, float a0)
	{
		auto scope = AssignTemp<bool>(filterVobs, npc->IsSelfPlayer());
		Ivk_oCNpc_GetNearestValidVob(npc, a0);
	}

	void __fastcall Hook_oCNpc_GetNearestVob(oCNpc*, void*, float);
	Hook<void(__thiscall*)(oCNpc*, float)> Ivk_oCNpc_GetNearestVob(ZENFOR(0x00691720, 0x006C2D20, 0x006D6240, 0x00734860), &Hook_oCNpc_GetNearestVob, HookMode::Patch);
	void __fastcall Hook_oCNpc_GetNearestVob(oCNpc* npc, void* vtable, float a0)
	{
		auto scope = AssignTemp<bool>(filterVobs, npc->IsSelfPlayer());
		Ivk_oCNpc_GetNearestVob(npc, a0);
	}

	void __fastcall Hook_oCNpc_ToggleFocusVob(oCNpc*, void*, int);
	Hook<void(__thiscall*)(oCNpc*, int)> Ivk_oCNpc_ToggleFocusVob(ZENFOR(0x00690910, 0x006C1D70, 0x006D4F90, 0x007335B0), &Hook_oCNpc_ToggleFocusVob, HookMode::Patch);
	void __fastcall Hook_oCNpc_ToggleFocusVob(oCNpc* npc, void* vtable, int a0)
	{
		auto scope = AssignTemp<bool>(filterVobs, npc->IsSelfPlayer());
		Ivk_oCNpc_ToggleFocusVob(npc, a0);
	}

#if ENGINE <= Engine_G1A

	// WARNING: supported versions are G1, G1A
	void __fastcall Hook_oCNpc_CollectFocusVob(oCNpc*, void*);
	Hook<void(__thiscall*)(oCNpc*)> Ivk_oCNpc_CollectFocusVob(ZENFOR(0x00690D70, 0x006C2210, 0x00000000, 0x00000000), &Hook_oCNpc_CollectFocusVob, HookMode::Patch);
	void __fastcall Hook_oCNpc_CollectFocusVob(oCNpc* npc, void* vtable)
	{
		auto scope = AssignTemp<bool>(filterVobs, npc->IsSelfPlayer());
		Ivk_oCNpc_CollectFocusVob(npc);
	}

#else

	// WARNING: supported versions are G2, G2A
	void __fastcall Hook_oCNpc_CollectFocusVob(oCNpc*, void*, int);
	Hook<void(__thiscall*)(oCNpc*, int)> Ivk_oCNpc_CollectFocusVob(ZENFOR(0x00000000, 0x00000000, 0x006D53F0, 0x00733A10), &Hook_oCNpc_CollectFocusVob, HookMode::Patch);
	void __fastcall Hook_oCNpc_CollectFocusVob(oCNpc* npc, void* vtable, int a0)
	{
		auto scope = AssignTemp<bool>(filterVobs, npc->IsSelfPlayer());
		Ivk_oCNpc_CollectFocusVob(npc, a0);
	}

#endif

	void __fastcall Hook_zCBspBase_CollectVobsInBBox3D(zCBspBase*, zCArray<zCVob*>&, zTBBox3D const&);
	Hook<void(__fastcall*)(zCBspBase*, zCArray<zCVob*>&, zTBBox3D const&)> Ivk_zCBspBase_CollectVobsInBBox3D(ZENFOR(0x0051E7C0, 0x005345A0, 0x0052E1A0, 0x00531110), &Hook_zCBspBase_CollectVobsInBBox3D, HookMode::Patch);
	void __fastcall Hook_zCBspBase_CollectVobsInBBox3D(zCBspBase* bsp, zCArray<zCVob*>& vobs, zTBBox3D const& bbox)
	{
		const int size = vobs.GetNum();
		Ivk_zCBspBase_CollectVobsInBBox3D(bsp, vobs, bbox);

		if (!filterVobs)
			return;

		for (int i = size; i < vobs.GetNum(); i++)
			if (!IsValidPlayerFocus(vobs[i]))
				vobs.RemoveIndex(i--);
	}

	Sub resetFocus(ZSUB(GameEvent::Loop), []
		{
			if (!player)
				return;

			if (!IsValidPlayerFocus(player->GetFocusVob()))
				player->ClearFocusVob();

			if (!IsValidPlayerFocus(player->enemy))
				player->SetEnemy(nullptr);
		});
}
