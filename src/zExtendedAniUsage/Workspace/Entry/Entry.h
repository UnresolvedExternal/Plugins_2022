namespace NAMESPACE
{
	zSTRING GetWalkModeString(int walkMode)
	{
		switch (walkMode)
		{
		case ANI_WALKMODE_RUN: return "RUN";
		case ANI_WALKMODE_WALK: return "WALK";
		case ANI_WALKMODE_SNEAK: return "SNEAK";
		case ANI_WALKMODE_WATER: return "W";
		case ANI_WALKMODE_SWIM: return "SWIM";
		case ANI_WALKMODE_DIVE: return "DIVE";
		}

		return "STAND";
	}

	zSTRING GetWeaponString(int mode)
	{
		static auto getWeaponString = reinterpret_cast<zSTRING*(__cdecl*)(zSTRING*, int)>(ZENDEF(0x00626100, 0x0064AF70, 0x006523E0, 0x006AEC60));

		zSTRING result;
		return *getWeaponString(&result, mode);
	}

	int ChooseAni(zCModel* model, std::initializer_list<zSTRING> names)
	{
		for (const zSTRING& name : names)
		{
			const int id = model->GetAniIDFromAniName(name);

			if (id != Invalid)
				return id;
		}

		return Invalid;
	}

	void ApplyRules(oCAniCtrl_Human* controller, int weaponMode, int walkMode)
	{
		const zSTRING walk = GetWalkModeString(walkMode);
		const zSTRING weapon = GetWeaponString(weaponMode);

		for (const auto& pairs : Options::aniPatterns)
		{
			int& ani = *reinterpret_cast<int*>(reinterpret_cast<char*>(controller) + pairs.first);
			ani = Invalid;

			for (const auto& pattern : pairs.second)
			{
				const zSTRING aniName = pattern.GetName(weapon, walk);
				ani = controller->model->GetAniIDFromAniName(aniName);

				if (ani != Invalid)
					break;
			}
		}
	}

	void __fastcall Hook_oCAniCtrl_Human_SetWalkMode(oCAniCtrl_Human*, void*, int);
	Hook<void(__thiscall*)(oCAniCtrl_Human*, int)> Ivk_oCAniCtrl_Human_SetWalkMode(ZENFOR(0x006211E0, 0x00645750, 0x0064CFA0, 0x006A9820), &Hook_oCAniCtrl_Human_SetWalkMode, HookMode::Patch);
	void __fastcall Hook_oCAniCtrl_Human_SetWalkMode(oCAniCtrl_Human* anictrl, void* vtable, int walkMode)
	{
		Ivk_oCAniCtrl_Human_SetWalkMode(anictrl, walkMode);

		zCModel* const model = anictrl->GetModel();

		if (!anictrl->model || !anictrl->npc)
			return;

		ApplyRules(anictrl, anictrl->npc->GetWeaponMode(), walkMode);
	}

	void __fastcall Hook_oCAniCtrl_Human_SetFightAnis(oCAniCtrl_Human*, void*, int);
	Hook<void(__thiscall*)(oCAniCtrl_Human*, int)> Ivk_oCAniCtrl_Human_SetFightAnis(ZENFOR(0x00622400, 0x00646AF0, 0x0064E1C0, 0x006AAA40), &Hook_oCAniCtrl_Human_SetFightAnis, HookMode::Patch);
	void __fastcall Hook_oCAniCtrl_Human_SetFightAnis(oCAniCtrl_Human* anictrl, void* vtable, int mode)
	{
		Ivk_oCAniCtrl_Human_SetFightAnis(anictrl, mode);
		
		if (!anictrl->model || !anictrl->npc)
			return;

		ApplyRules(anictrl, mode, anictrl->walkmode);
	}

	Sub addExtraChestScemes(ZSUB(GameEvent::LoadEnd), []
		{
			for (zCVob* vob : ogame->GetGameWorld()->voblist)
				if (oCMobContainer* chest = dynamic_cast<oCMobContainer*>(vob))
					if (zCModel* model = chest->GetModel())
						if (!model->modelProtoList.IsEmpty())
						{
							const zSTRING& name = model->modelProtoList[0]->modelProtoName;

							if (name == "CHESTBIG_OCCHESTMEDIUM" || name == "CHESTBIG_OCCHESTMEDIUMLOCKED")
								chest->sceme = "CHESTMEDIUM";
						}
		});
}
