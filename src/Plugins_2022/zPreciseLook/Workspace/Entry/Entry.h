namespace NAMESPACE
{
	std::unordered_map<zCVob*, zVEC3> lookAtPosMap;
	std::unordered_map<zCVob*, zCQuat> headRotationMap;
	bool lookAtVob = false;

	Sub resetGlobals(ZSUB(GameEvent::LoadBegin), []
		{
			lookAtPosMap.clear();
			headRotationMap.clear();
		});

	constexpr float magicCombineX = -128.0f;

	void __fastcall Hook_oCAniCtrl_Human_SetLookAtTarget_Pos(oCAniCtrl_Human*, void*, zVEC3&);
	Hook<void(__thiscall*)(oCAniCtrl_Human*, zVEC3&)> Ivk_oCAniCtrl_Human_SetLookAtTarget_Pos(ZENFOR(0x0062CD10, 0x00652410, 0x006599D0, 0x006B6360), &Hook_oCAniCtrl_Human_SetLookAtTarget_Pos, HookMode::Patch);
	void __fastcall Hook_oCAniCtrl_Human_SetLookAtTarget_Pos(oCAniCtrl_Human* controller, void* vtable, zVEC3& pos)
	{
		if (controller->targetVob)
		{
			controller->targetVob->Release();
			controller->targetVob = nullptr;
		}

		float azi, elev;
		LookEngine::GetAngles(controller->npc, pos, azi, elev);

		float& ax = controller->lookTargetx;
		float& ay = controller->lookTargety;

		zCModel* const model = controller->npc->GetModel();

		if (fabs(azi) > Options::MaxAngleX || fabs(elev) > Options::MaxAngleY)
		{
			ax = magicCombineX - 0.5f;
			ay = 0.5f;
			return;
		}

		ax = magicCombineX - std::clamp(0.5f + azi / 180.0f, 0.0f, 1.0f);
		ay = std::clamp(0.5f - elev / 90.0f, 0.0f, 1.0f);

		zCModelAniActive* const oldAni = model->GetActiveAni(controller->s_look);
		const bool wasLooking = oldAni && !oldAni->isFadingOut && oldAni->combAniX <= magicCombineX && oldAni->combAniX >= magicCombineX - 1.0f;
		
		model->StartAni(controller->s_look, zCModel::zMDL_STARTANI_DEFAULT);

		if (zCModelAniActive* ani = model->GetActiveAni(controller->s_look))
			if (!wasLooking)
				ani->combAniX = magicCombineX - 0.5f;

		if (!lookAtVob)
			lookAtPosMap[controller->npc] = pos;
	}

	void __fastcall Hook_oCAniCtrl_Human_SetLookAtTarget(oCAniCtrl_Human*, void*, zCVob*);
	Hook<void(__thiscall*)(oCAniCtrl_Human*, zCVob*)> Ivk_oCAniCtrl_Human_SetLookAtTarget(ZENFOR(0x0062CE10, 0x00652510, 0x00659B00, 0x006B6490), &Hook_oCAniCtrl_Human_SetLookAtTarget, HookMode::Patch);
	void __fastcall Hook_oCAniCtrl_Human_SetLookAtTarget(oCAniCtrl_Human* controller, void* vtable, zCVob* target)
	{
		if (controller->targetVob)
		{
			controller->targetVob->Release();
			controller->targetVob = nullptr;
		}

		zCModel* const model = controller->npc->GetModel();

		float& ax = controller->lookTargetx;
		float& ay = controller->lookTargety;

		if (!target || target->homeWorld != controller->npc->homeWorld)
		{
			if (ax <= magicCombineX && ax >= magicCombineX - 1.0f)
				ax = magicCombineX - 0.5f;
			else
				ax = 0.5f;

			controller->lookTargety = 0.5f;
			return;
		}

		lookAtVob = true;
		Hook_oCAniCtrl_Human_SetLookAtTarget_Pos(controller, vtable, LookEngine::GetEyes(target));
		lookAtVob = false;

		target->AddRef();
		controller->targetVob = target;
	}

	float Move(float source, float dest, float speed, float time)
	{
		if (source < dest)
			return std::clamp(source + speed * time, source, dest);

		return std::clamp(source - speed * time, dest, source);
	}

	void __fastcall Hook_oCAniCtrl_Human_LookAtTarget(oCAniCtrl_Human*, void*);
	Hook<void(__thiscall*)(oCAniCtrl_Human*)> Ivk_oCAniCtrl_Human_LookAtTarget(ZENFOR(0x0062CCD0, 0x006523D0, 0x00659960, 0x006B62F0), &Hook_oCAniCtrl_Human_LookAtTarget, HookMode::Patch);
	void __fastcall Hook_oCAniCtrl_Human_LookAtTarget(oCAniCtrl_Human* controller, void* vtable)
	{
		float& ax = controller->lookTargetx;
		float& ay = controller->lookTargety;

		zCModelAniActive* const ani = controller->npc->GetModel()->GetActiveAni(controller->s_look);

		if (ax < magicCombineX - 1.0f || ax > magicCombineX)
		{
			if (ani && ani->combAniX <= magicCombineX && ani->combAniX >= magicCombineX - 1.0f)
				ani->combAniX = magicCombineX - ani->combAniX;

			return Ivk_oCAniCtrl_Human_LookAtTarget(controller);
		}

		if (!ani || ani->isFadingOut)
		{
			ax = 0.5f;
			ay = 0.5f;

			if (auto it = lookAtPosMap.find(controller->npc); it != lookAtPosMap.end())
				lookAtPosMap.erase(it);

			headRotationMap.erase(controller->npc);
			controller->SetLookAtTarget(nullptr);
			return;
		}

		if (controller->targetVob)
		{
			controller->targetVob->AddRef();
			controller->SetLookAtTarget(controller->targetVob);
			controller->targetVob->Release();
		}
		else
			controller->SetLookAtTarget(lookAtPosMap[controller->npc]);

		LookEngine engine{ controller->npc, controller->targetVob ? LookEngine::GetEyes(controller->targetVob) : lookAtPosMap[controller->npc], controller->s_look };

		if (controller->targetVob && controller->targetVob->homeWorld != controller->npc->homeWorld)
		{
			ax = magicCombineX - 0.5f;
			ay = 0.5f;

			float t;
			headRotationMap[controller->npc] = engine.Relax(t);

			if (t == 1.0f)
				controller->npc->GetModel()->FadeOutAni(ani);
		}
		else
		{
			LookEngine engine{ controller->npc, controller->targetVob ? LookEngine::GetEyes(controller->targetVob) : lookAtPosMap[controller->npc], controller->s_look };
			
			float t;
			headRotationMap[controller->npc] = engine.Look(t);
		}

		ani->combAniX = Move(ani->combAniX, ax, 1000.0f, ztimer->frameTimeFloat);
		ani->combAniY = Move(ani->combAniY, ay, 1000.0f, ztimer->frameTimeFloat);
	}

	void __fastcall Hook_zCModelAniActive_DoCombineAni(zCModelAniActive*, void*, zCModel*, int, int);
	Hook<void(__thiscall*)(zCModelAniActive*, zCModel*, int, int)> Ivk_zCModelAniActive_DoCombineAni(ZENFOR(0x00565D30, 0x0057E780, 0x0057A890, 0x0057FDB0), &Hook_zCModelAniActive_DoCombineAni, HookMode::Patch);
	void __fastcall Hook_zCModelAniActive_DoCombineAni(zCModelAniActive* ani, void* vtable, zCModel* model, int frame, int nextFrame)
	{
		if (ani->combAniX < magicCombineX - 1.0f || ani->combAniX > magicCombineX)
			return Ivk_zCModelAniActive_DoCombineAni(ani, model, frame, nextFrame);

		{
			auto scope = AssignTemp(ani->combAniX, magicCombineX - ani->combAniX);
			Ivk_zCModelAniActive_DoCombineAni(ani, model, frame, nextFrame);
		}

		auto it = headRotationMap.find(model->homeVob);

		if (it == headRotationMap.end())
			return;

		zCModelNodeInst* const head = GetHead(model->homeVob->CastTo<oCNpc>());
		int nodeIndex = 0;

		for (int i = 0; i < ani->protoAni->numNodes; i++)
			if (model->nodeList[ani->protoAni->nodeIndexList[i]] == head)
			{
				nodeIndex = i;
				break;
			}

		for (int f : { frame, nextFrame })
			ani->protoAni->SetQuat(f, nodeIndex, it->second);
	}

	/*
	void __fastcall Hook_zCModelNodeInst_CalcWeights(zCModelNodeInst*, void*, zCModel*);
	Hook<void(__thiscall*)(zCModelNodeInst*, zCModel*)> Ivk_zCModelNodeInst_CalcWeights(ZENFOR(0x005653F0, 0x0057DD70, 0x00579F50, 0x0057F470), &Hook_zCModelNodeInst_CalcWeights, HookMode::Patch);
	void __fastcall Hook_zCModelNodeInst_CalcWeights(zCModelNodeInst* node, void* vtable, zCModel* model)
	{
		if (node->masterAni != -1)
		{
			zCModelNodeInst::zTNodeAni& aniInfo = node->nodeAniList[node->masterAni];

			if (aniInfo.weight == 1.0f && aniInfo.weightSpeed == 0.0f && aniInfo.blendState == 1)
				aniInfo.weightSpeed = aniInfo.modelAni->blendInOverride;
		}

		Ivk_zCModelNodeInst_CalcWeights(node, model);
	}
	*/

	zCModelPrototype* __cdecl Hook_zCModelPrototype_Load(zSTRING const&, zCModelPrototype*);
	Hook<zCModelPrototype* (__cdecl*)(zSTRING const&, zCModelPrototype*)> Ivk_zCModelPrototype_Load(ZENFOR(0x0056EA80, 0x00587E10, 0x00583CF0, 0x00589250), &Hook_zCModelPrototype_Load, HookMode::Patch);
	zCModelPrototype* __cdecl Hook_zCModelPrototype_Load(zSTRING const& a0, zCModelPrototype* a1)
	{
		zCModelPrototype* proto = Ivk_zCModelPrototype_Load(a0, a1);

		if (proto)
		{
			for (zCModelAni* ani : proto->protoAnis)
				if (ani && ani->aniName.StartWith("R_CHAIR_RANDOM_"))
					ani->layer = 10;
		}

		return proto;
	}
}
