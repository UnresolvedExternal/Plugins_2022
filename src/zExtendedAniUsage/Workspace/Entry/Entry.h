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

	void __fastcall Hook_oCAniCtrl_Human_SetWalkMode(oCAniCtrl_Human*, void*, int);
	Hook<void(__thiscall*)(oCAniCtrl_Human*, int)> Ivk_oCAniCtrl_Human_SetWalkMode(ZENFOR(0x006211E0, 0x00645750, 0x0064CFA0, 0x006A9820), &Hook_oCAniCtrl_Human_SetWalkMode, HookMode::Patch);
	void __fastcall Hook_oCAniCtrl_Human_SetWalkMode(oCAniCtrl_Human* anictrl, void* vtable, int walkMode)
	{
		Ivk_oCAniCtrl_Human_SetWalkMode(anictrl, walkMode);

		zCModel* const model = anictrl->GetModel();

		if (!model)
			return;

		const zSTRING walk = GetWalkModeString(walkMode);
		const zSTRING iget = walk + "IGET";

		anictrl->t_stand_2_iget = ChooseAni(model, { Z"T_" + walk + "_2_" + iget, Z"T_STAND_2_IGET" });
		anictrl->s_iget = ChooseAni(model, { Z"S_" + iget, "S_IGET" });
		anictrl->t_iget_2_stand = ChooseAni(model, { Z"T_" + iget + "_2_" + walk, Z"T_IGET_2_STAND" });
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
