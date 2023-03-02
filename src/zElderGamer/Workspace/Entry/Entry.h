namespace NAMESPACE
{
	oCMsgConversation::TConversationSubType oCMsgConversation_EV_PlayAniWithEvents = static_cast<oCMsgConversation::TConversationSubType>(-12345);

	void __fastcall Hook_oCNpc_OnMessage(oCNpc*, void*, zCEventMessage*, zCVob*);
	Hook<void(__thiscall*)(oCNpc*, zCEventMessage*, zCVob*)> Ivk_oCNpc_OnMessage(ZENFOR(0x006A69E0, 0x006D95F0, 0x006EC360, 0x0074B020), &Hook_oCNpc_OnMessage, HookMode::Patch);
	void __fastcall Hook_oCNpc_OnMessage(oCNpc* npc, void* vtable, zCEventMessage* event, zCVob* vob)
	{
		if (oCMsgConversation* conversation = dynamic_cast<oCMsgConversation*>(event))
			if (conversation->subType == oCMsgConversation_EV_PlayAniWithEvents)
			{
				conversation->subType = oCMsgConversation::EV_PLAYANI;
				Ivk_oCNpc_OnMessage(npc, conversation, vob);
				conversation->subType = oCMsgConversation_EV_PlayAniWithEvents;
				npc->DoDoAniEvents();
				return;
			}

		Ivk_oCNpc_OnMessage(npc, event, vob);
	}

	zSTRING* __stdcall Hook_oCMsgConversation_MD_GetSubTypeString(zSTRING*, int);
	Hook<zSTRING* (__stdcall*)(zSTRING*, int)> Ivk_oCMsgConversation_MD_GetSubTypeString(ZENFOR(0x006C39A0, 0x006F8DE0, 0x0070B620, 0x0076AB60), &Hook_oCMsgConversation_MD_GetSubTypeString, HookMode::Patch);
	zSTRING* __stdcall Hook_oCMsgConversation_MD_GetSubTypeString(zSTRING* result, int subType)
	{
		if (subType == oCMsgConversation_EV_PlayAniWithEvents)
		{
			result->zSTRING_OnInit("EV_PLAYANIWITHEVENTS");
			return result;
		}
		
		return Ivk_oCMsgConversation_MD_GetSubTypeString(result, subType);
	}

	int __fastcall Hook_oCMsgConversation_IsOverlay(oCMsgConversation*, void*);
	Hook<int(__thiscall*)(oCMsgConversation*)> Ivk_oCMsgConversation_IsOverlay(ZENFOR(0x006C3960, 0x006F8D90, 0x0070B5C0, 0x0076AB00), &Hook_oCMsgConversation_IsOverlay, HookMode::Patch);
	int __fastcall Hook_oCMsgConversation_IsOverlay(oCMsgConversation* message, void* vtable)
	{
		if (message->subType == oCMsgConversation_EV_PlayAniWithEvents)
			return false;

		return Ivk_oCMsgConversation_IsOverlay(message);
	}

	int __cdecl AI_PlayAniWithEvents()
	{
		oCNpc* npc;
		zSTRING ani;
		int nextBodyState;
		ZARGS(npc, ani, nextBodyState);

		if (npc && !ani.IsEmpty())
		{
			ani.Upper();
			oCMsgConversation* message = new oCMsgConversation{ oCMsgConversation::EV_PLAYANI_NOOVERLAY, ani };
			message->subType = oCMsgConversation_EV_PlayAniWithEvents;
			message->number = nextBodyState;
			npc->GetEM()->OnMessage(message, npc);
		}

		return false;
	}

	ZEXTERNAL(void, AI_PlayAniWithEvents, oCNpc*, zSTRING, int);

	int __fastcall Hook_oCMsgConversation_MD_GetNumOfSubTypes(oCMsgConversation*, void*);
	Hook<int(__thiscall*)(oCMsgConversation*)> Ivk_oCMsgConversation_MD_GetNumOfSubTypes(ZENFOR(0x006C3760, 0x006F8B70, 0x0070B3C0, 0x0076A900), &Hook_oCMsgConversation_MD_GetNumOfSubTypes, HookMode::Patch);
	int __fastcall Hook_oCMsgConversation_MD_GetNumOfSubTypes(oCMsgConversation* message, void* vtable)
	{
		return Ivk_oCMsgConversation_MD_GetNumOfSubTypes(message) + 1;
	}

	Sub changeVirtuals(ZSUB(GameEvent::Entry), []
		{
			oCMsgConversation* message = new oCMsgConversation{};
			oCMsgConversation_EV_PlayAniWithEvents = static_cast<oCMsgConversation::TConversationSubType>(Ivk_oCMsgConversation_MD_GetNumOfSubTypes(message));
			message->Release();
		});

	void __fastcall Hook_oCNpc_CheckSetTorchAni(oCNpc*, void*);
	Hook<void(__thiscall*)(oCNpc*)> Ivk_oCNpc_CheckSetTorchAni(ZENFOR(0x006982D0, 0x006C9FF0, 0x006DCF10, 0x0073B6D0), &Hook_oCNpc_CheckSetTorchAni, HookMode::Patch);
	void __fastcall Hook_oCNpc_CheckSetTorchAni(oCNpc* npc, void* vtable)
	{
		oCNpc::TActiveInfo* info = npc->GetActiveInfoWritable();

		if (zCModel* model = dynamic_cast<zCModel*>(npc->visual))
			if (oCAniCtrl_Human* anictrl = npc->anictrl)
				if (info && info->changeTorchAni)
					if (int weaponMode = npc->GetWeaponMode())
						if (zCModelAniActive* ani = model->GetActiveAni(anictrl->s_runl[weaponMode]))
							if (!ani->isFadingOut)
								return npc->SetTorchAni(info->changeTorchAniTo, true);

		Ivk_oCNpc_CheckSetTorchAni(npc);
	}

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
		zSTRING*(__thiscall *func)(oCMOB*, zSTRING*) = reinterpret_cast<decltype(func)>(*vtable);
		return zSTRING{ *func(mob, &dummy) };
	}

	int __fastcall Hook_oCMobInter_CanInteractWith(oCMobInter*, void*, oCNpc*);
	Hook<int(__thiscall*)(oCMobInter*, oCNpc*)> Ivk_oCMobInter_CanInteractWith(ZENFOR(0x0067F5F0, 0x006AE730, 0x006C2EB0, 0x00720F40), &Hook_oCMobInter_CanInteractWith, HookMode::Patch);
	int __fastcall Hook_oCMobInter_CanInteractWith(oCMobInter* mob, void* vtable, oCNpc* npc)
	{
		zCModel* model = npc->GetModel();

		if (!model)
			return false;
				
		const zSTRING aniName = Z"T_" + GetScemeName(mob, npc) + "_STAND_2_S" + Z mob->state;

		const int aniID = model->GetAniIDFromAniName(aniName);
		zCModelAni* const ani = model->GetAniFromAniID(aniID);

		if (!ani || ani->aniType == zMDL_ANI_TYPE_DISABLED)
			return false;

		return Ivk_oCMobInter_CanInteractWith(mob, npc);
	}

	int __fastcall Hook_oCNpc_StartDialogAni(oCNpc*, void*);
	Hook<int(__thiscall*)(oCNpc*)> Ivk_oCNpc_StartDialogAni(ZENFOR(0x006B2130, 0x006E6470, 0x006F8FB0, 0x00757DE0), &Hook_oCNpc_StartDialogAni, HookMode::Patch);
	int __fastcall Hook_oCNpc_StartDialogAni(oCNpc* npc, void* vtable)
	{
		static Symbol s_sit{ parser, "DISABLE_S_SIT_GESTURES" };
		static Symbol bs_sit{ parser, "DISABLE_BS_SIT_GESTURES" };

		if (s_sit.GetType() == Symbol::Type::VarInt && s_sit.GetValue<int>(0))
			if (zCModel* model = npc->GetModel())
				if (zCModelAniActive* sitAni = model->GetActiveAni(model->GetAniIDFromAniName("S_SIT")))
					if (sitAni && !sitAni->isFadingOut)
						return Invalid;

		if (bs_sit.GetType() == Symbol::Type::VarInt && bs_sit.GetValue<int>(0))
			if (npc->HasBodyStateModifier(BS_SIT))
				return Invalid;

		return Ivk_oCNpc_StartDialogAni(npc);
	}

	void __fastcall Hook_oCAniCtrl_Human_PC_JumpForward(oCAniCtrl_Human*, void*);
	Hook<void(__thiscall*)(oCAniCtrl_Human*)> Ivk_oCAniCtrl_Human_PC_JumpForward(ZENFOR(0x00628C90, 0x0064DEB0, 0x00655470, 0x006B1E00), &Hook_oCAniCtrl_Human_PC_JumpForward, HookMode::Patch);
	void __fastcall Hook_oCAniCtrl_Human_PC_JumpForward(oCAniCtrl_Human* _this, void* vtable)
	{
		Ivk_oCAniCtrl_Human_PC_JumpForward(_this);
		_this->do_jump = false;
	}

	void __fastcall Hook_zCVob_SetVisual(zCVob*, void*, zCVisual*);
	Hook<void(__thiscall*)(zCVob*, zCVisual*)> Ivk_zCVob_SetVisual(ZENFOR(0x005D6E10, 0x005F63E0, 0x005FB6C0, 0x006024F0), &Hook_zCVob_SetVisual, HookMode::Patch);
	void __fastcall Hook_zCVob_SetVisual(zCVob* vob, void* vtable, zCVisual* visual)
	{
		if (vob->CastTo<oCItem>())
			if (visual)
			{
				visual->lodFarDistance = oCSpawnManager::GetInsertRange();
				visual->lodNearFadeOutDistance = oCSpawnManager::GetInsertRange() - 500.0f;
			}

		Ivk_zCVob_SetVisual(vob, visual);
	}

	void __fastcall Hook_zCVob_SetVisual_2(zCVob*, void*, zSTRING const&);
	Hook<void(__thiscall*)(zCVob*, zSTRING const&)> Ivk_zCVob_SetVisual_2(ZENFOR(0x005D6FA0, 0x005F6570, 0x005FB850, 0x00602680), &Hook_zCVob_SetVisual_2, HookMode::Patch);
	void __fastcall Hook_zCVob_SetVisual_2(zCVob* vob, void* vtable, zSTRING const& visualName)
	{
		Ivk_zCVob_SetVisual_2(vob, visualName);

		if (vob->CastTo<oCItem>())
			if (zCVisual* visual = vob->visual)
			{
				visual->lodFarDistance = oCSpawnManager::GetInsertRange();
				visual->lodNearFadeOutDistance = oCSpawnManager::GetInsertRange() - 500.0f;
			}
	}

#if ENGINE == Engine_G1

	// WARNING: supported versions are G1
	TMobOptPos* __fastcall Hook_oCMobInter_SearchFreePosition(oCMobInter*, void*, oCNpc*);
	Hook<TMobOptPos* (__thiscall*)(oCMobInter*, oCNpc*)> Ivk_oCMobInter_SearchFreePosition(ZENFOR(0x0067CD60, 0x00000000, 0x00000000, 0x00000000), &Hook_oCMobInter_SearchFreePosition, HookMode::Patch);
	TMobOptPos* __fastcall Hook_oCMobInter_SearchFreePosition(oCMobInter* mob, void* vtable, oCNpc* npc)
	{
		if (mob->npcsCurrent > mob->npcsMax)
			return nullptr;

		bool toFar = false;
		bool wrongSide = false;
		TMobOptPos* found = nullptr;
		float minDistance = std::numeric_limits<float>::max();
		float minWrongDistance = minDistance;

		for (TMobOptPos* optPos : mob->optimalPosList)
		{
			if (optPos->npc == npc)
				return optPos;

			if (optPos->npc)
				continue;

			const float distance = (optPos->trafo.GetTranslation() - npc->GetPositionWorld()).Length_Sqr();

			if (distance >= minDistance)
				continue;

			if (!found && !optPos->distance && !npc->FreeLineOfSight(optPos->trafo.GetTranslation(), nullptr))
			{
				if (distance < minWrongDistance)
				{
					minWrongDistance = distance;
					toFar = false;
					wrongSide = true;
				}

				continue;
			}

			if (!found && npc->IsSelfPlayer() && !optPos->distance && distance > 150.0f * 150.0f)
			{
				if (distance < minWrongDistance)
				{
					minWrongDistance = distance;
					wrongSide = false;
					toFar = true;
				}

				continue;
			}

			minDistance = distance;
			found = optPos;
		}

		if (!found && (wrongSide || toFar) && npc->IsSelfPlayer())
		{
			const char* const functionName = wrongSide ? "PLAYER_MOB_WRONG_SIDE" : "PLAYER_MOB_TOO_FAR_AWAY";
			npc->GetEM()->OnMessage(new oCMsgManipulate{ oCMsgManipulate::EV_CALLSCRIPT, functionName, -1 }, npc);
		}

		return found;
	}

#endif

	void __fastcall Hook_zCModel_CalcModelBBox3DWorld(zCModel*, void*);
	Hook<void(__thiscall*)(zCModel*)> Ivk_zCModel_CalcModelBBox3DWorld(ZENFOR(0x0055F8F0, 0x00577D10, 0x00573EC0, 0x00579170), &Hook_zCModel_CalcModelBBox3DWorld, HookMode::Patch);
	void __fastcall Hook_zCModel_CalcModelBBox3DWorld(zCModel* model, void* vtable)
	{
		if (model->nodeList.GetNum() == 0)
			return Ivk_zCModel_CalcModelBBox3DWorld(model);

		model->bbox3D.InitZero();

		for (zCModelNodeInst* node : model->nodeList)
		{
			zVEC3 pos = model->GetTrafoNodeToModel(node).GetTranslation();

			for (int i = 0; i < 3; i++)
			{
				model->bbox3D.mins[i] = std::min(model->bbox3D.mins[i], pos[i]);
				model->bbox3D.maxs[i] = std::max(model->bbox3D.maxs[i], pos[i]);
			}
		}
	}
}
