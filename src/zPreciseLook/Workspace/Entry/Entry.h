namespace NAMESPACE
{
	struct RotationInfo
	{
		static constexpr float magicCombineX = -128.0f;

		static std::vector<RotationInfo*> infos;

		std::vector<Sub<void>> subs;

		enum class State
		{
			NoTarget,
			Initialized,
			Normal,
			Prolongation
		} state;

		ZOwner<oCNpc> npc;
		zVEC3 targetPos;
		zVEC3 lastTarget;
		float stateTime;
		zCQuat rotation;

		static RotationInfo* Get(oCNpc* npc)
		{
			auto it = std::find_if(infos.begin(), infos.end(), [npc](RotationInfo* info) { return npc == info->npc.get(); });
			return (it == infos.end()) ? nullptr : *it;
		}

		static RotationInfo* GetOrCreate(oCNpc* npc)
		{
			if (RotationInfo* info = Get(npc))
				return info;

			return new RotationInfo{ npc };
		}

		RotationInfo(oCNpc* npc) :
			npc{ npc },
			state{ State::NoTarget }
		{
			npc->AddRef();
			npc->GetAnictrl()->lookTargetx = MakeMagicValue(0.5f);
			npc->GetAnictrl()->lookTargety = MakeMagicValue(0.5f);

			ADDSUB(LoadBegin);
			ADDSUB(Exit);

			infos += this;
		}

		~RotationInfo()
		{
			if (oCAniCtrl_Human* controller = npc->GetAnictrl())
			{
				controller->lookTargetx = GetMagicValue(controller->lookTargetx);
				controller->lookTargety = GetMagicValue(controller->lookTargety);
			}

			infos -= this;
		}

		RotationInfo(RotationInfo&) = delete;
		RotationInfo& operator=(RotationInfo&) = delete;

		zVEC3 GetTarget() const
		{
			ASSERT(state != State::NoTarget);

			if (zCVob* target = COA(npc, GetAnictrl())->targetVob)
				return LookEngine::GetEyes(target);

			return targetPos;
		}

		void SetState(State newState)
		{
			if (state == newState)
				return;

			state = newState;
			stateTime = 0.0f;
		}

		void OnLoadBegin()
		{
			delete this;
		}

		void OnExit()
		{
			delete this;
		}

		bool IsNewTarget(const zVEC3& target) const
		{
			return state == State::NoTarget || npc->anictrl->targetVob || targetPos.Distance(target) < 0.01f;
		}

		bool IsNewTarget(zCVob* target) const
		{
			return state == State::NoTarget || npc->anictrl->targetVob != target;
		}

		static bool IsMagic(const float value)
		{
			return value >= magicCombineX && value <= magicCombineX + 1.0f;
		}

		static float GetMagicValue(const float value)
		{
			return IsMagic(value) ? value - magicCombineX : value;
		}

		static float MakeMagicValue(const float value)
		{
			if (IsMagic(value))
				return value;

			return magicCombineX + value;
		}

		static void MakeMagicAni(zCModelAniActive* ani)
		{
			ani->combAniX = MakeMagicValue(ani->combAniX);
			ani->combAniY = MakeMagicValue(ani->combAniY);
		}

		static void UnmakeMagicAni(zCModelAniActive* ani)
		{
			ani->combAniX = GetMagicValue(ani->combAniX);
			ani->combAniY = GetMagicValue(ani->combAniY);
		}
	};

	std::vector<RotationInfo*> RotationInfo::infos;

	void __fastcall Hook_oCAniCtrl_Human_SetLookAtTarget_Pos(oCAniCtrl_Human*, void*, zVEC3&);
	Hook<void(__thiscall*)(oCAniCtrl_Human*, zVEC3&)> Ivk_oCAniCtrl_Human_SetLookAtTarget_Pos(ZENFOR(0x0062CD10, 0x00652410, 0x006599D0, 0x006B6360), &Hook_oCAniCtrl_Human_SetLookAtTarget_Pos, HookMode::Patch);
	void __fastcall Hook_oCAniCtrl_Human_SetLookAtTarget_Pos(oCAniCtrl_Human* controller, void* vtable, zVEC3& pos)
	{
		RotationInfo* const info = RotationInfo::GetOrCreate(controller->npc);
		const bool isNewTarget = info->IsNewTarget(pos);

		if (controller->targetVob)
		{
			controller->targetVob->Release();
			controller->targetVob = nullptr;
		}

		zCModel* const model = controller->model;

		if (!model)
		{
			delete info;
			return;
		}

		float& ax = controller->lookTargetx;
		float& ay = controller->lookTargety;

		ax = RotationInfo::MakeMagicValue(ax);
		ay = RotationInfo::MakeMagicValue(ay);

		info->targetPos = pos;

		switch (info->state)
		{
		case RotationInfo::State::NoTarget:
			info->SetState(RotationInfo::State::Initialized);
			break;

		case RotationInfo::State::Initialized:
			if (isNewTarget)
				info->stateTime = 0.0f;

			break;

		case RotationInfo::State::Normal:
			if (isNewTarget)
				info->SetState(RotationInfo::State::Initialized);

			break;

		case RotationInfo::State::Prolongation:
			if (isNewTarget)
				info->SetState(RotationInfo::State::Initialized);

			break;

		default:
			ASSERT(false);
			break;
		}
	}

	void __fastcall Hook_oCAniCtrl_Human_SetLookAtTarget(oCAniCtrl_Human*, void*, zCVob*);
	Hook<void(__thiscall*)(oCAniCtrl_Human*, zCVob*)> Ivk_oCAniCtrl_Human_SetLookAtTarget(ZENFOR(0x0062CE10, 0x00652510, 0x00659B00, 0x006B6490), &Hook_oCAniCtrl_Human_SetLookAtTarget, HookMode::Patch);
	void __fastcall Hook_oCAniCtrl_Human_SetLookAtTarget(oCAniCtrl_Human* controller, void* vtable, zCVob* target)
	{
		if (target == controller->targetVob)
			return;

		RotationInfo* const info = RotationInfo::GetOrCreate(controller->npc);
		const bool isNewTarget = info->IsNewTarget(target);

		if (controller->targetVob)
		{
			controller->targetVob->Release();
			controller->targetVob = nullptr;
		}

		zCModel* const model = controller->model;

		if (!model)
		{
			delete info;
			return;
		}

		float& ax = controller->lookTargetx;
		float& ay = controller->lookTargety;

		ax = RotationInfo::MakeMagicValue(ax);
		ay = RotationInfo::MakeMagicValue(ay);

		if (target)
			target->AddRef();

		controller->targetVob = target;

		switch (info->state)
		{
		case RotationInfo::State::NoTarget:
			info->SetState(RotationInfo::State::Initialized);
			break;

		case RotationInfo::State::Initialized:
			if (isNewTarget)
				info->stateTime = 0.0f;

			break;

		case RotationInfo::State::Normal:
			if (isNewTarget)
				info->SetState(RotationInfo::State::Initialized);

			break;

		case RotationInfo::State::Prolongation:
			if (isNewTarget)
				info->SetState(RotationInfo::State::Initialized);

			break;

		default:
			ASSERT(false);
			break;
		}
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
		RotationInfo* info = RotationInfo::Get(controller->npc);
		zCModelAniActive* ani = controller->npc->GetModel()->GetActiveAni(controller->s_look);

		float& ax = controller->lookTargetx;
		float& ay = controller->lookTargety;

		if (!RotationInfo::IsMagic(ax) || !RotationInfo::IsMagic(ay))
		{
			if (info)
			{
				delete info;
				info = nullptr;
			}
			else
			{
				ax = RotationInfo::GetMagicValue(ax);
				ay = RotationInfo::GetMagicValue(ay);
			}

			if (ani)
				RotationInfo::UnmakeMagicAni(ani);
		}

		if (!info && ax == 0.5f && ay == 0.5f)
			if (!ani || ani->combAniX == ax && ani->combAniY == ay)
			{
				if (ani)
					controller->npc->GetModel()->FadeOutAni(ani);
				
				return;
			}

		if (!ani || ani->isFadingOut)
		{
			controller->npc->GetModel()->StartAni(controller->s_look, zCModel::zMDL_STARTANI_DEFAULT);
			ani = controller->npc->GetModel()->GetActiveAni(controller->s_look);
		}

		if (ani)
		{
			if (info)
				RotationInfo::MakeMagicAni(ani);

			ani->combAniX = Move(ani->combAniX, ax, Options::DegreesPerSecond / 180.0f, ztimer->frameTimeFloat / 1000.0f);
			ani->combAniY = Move(ani->combAniY, ay, Options::DegreesPerSecond / 180.0f, ztimer->frameTimeFloat / 1000.0f);
		}
		else
		{
			if (info)
			{
				delete info;
				info = nullptr;
			}

			return;
		}

		if (!info)
			return;

		info->stateTime += ztimer->frameTimeFloat;

		switch (info->state)
		{
		case RotationInfo::State::NoTarget:
		{
			LookEngine lookEngine{ controller->npc, nullptr, controller->s_look };

			if (!lookEngine.IsValid())
			{
				delete info;
				RotationInfo::UnmakeMagicAni(ani);
				return;
			}

			float t;
			info->rotation = lookEngine.Look(t);

			if (t == 1.0f)
			{
				RotationInfo::UnmakeMagicAni(ani);
				controller->npc->GetModel()->FadeOutAni(ani);
				delete info;
				info = nullptr;
			}

			break;
		}

		case RotationInfo::State::Initialized:
		case RotationInfo::State::Normal:
		{
			info->lastTarget = info->GetTarget();
			LookEngine lookEngine{ controller->npc, &info->lastTarget, controller->s_look };

			if (!lookEngine.IsValid())
			{
				delete info;
				RotationInfo::UnmakeMagicAni(ani);
				return;
			}

			float t;
			zCQuat oldRotation = info->rotation;
			info->rotation = lookEngine.Look(t);

			if (t == 1.0f && info->state == RotationInfo::State::Initialized)
				info->SetState(RotationInfo::State::Normal);
			else if (t != 1.0f && info->state == RotationInfo::State::Normal && Options::ProlongationTime > 0.0f)
			{
				info->rotation = oldRotation;
				info->SetState(RotationInfo::State::Prolongation);
			}

			break;
		}

		case RotationInfo::State::Prolongation:
		{
			if (info->stateTime >= Options::ProlongationTime * 1000.0f)
				info->SetState(RotationInfo::State::Initialized);

			LookEngine lookEngine{ controller->npc, &info->lastTarget, controller->s_look };

			if (!lookEngine.IsValid())
			{
				delete info;
				RotationInfo::UnmakeMagicAni(ani);
				return;
			}

			float t;
			info->rotation = lookEngine.Look(t);

			break;
		}

		default:
			ASSERT(false);
			break;
		}
	}

	void __fastcall Hook_zCModelAniActive_DoCombineAni(zCModelAniActive*, void*, zCModel*, int, int);
	Hook<void(__thiscall*)(zCModelAniActive*, zCModel*, int, int)> Ivk_zCModelAniActive_DoCombineAni(ZENFOR(0x00565D30, 0x0057E780, 0x0057A890, 0x0057FDB0), &Hook_zCModelAniActive_DoCombineAni, HookMode::Patch);
	void __fastcall Hook_zCModelAniActive_DoCombineAni(zCModelAniActive* ani, void* vtable, zCModel* model, int frame, int nextFrame)
	{
		RotationInfo* info = RotationInfo::Get(dynamic_cast<oCNpc*>(model->homeVob));

		if (!RotationInfo::IsMagic(ani->combAniX) || !info)
			return Ivk_zCModelAniActive_DoCombineAni(ani, model, frame, nextFrame);

		{
			auto scopeX = AssignTemp(ani->combAniX, RotationInfo::GetMagicValue(ani->combAniX));
			auto scopeY = AssignTemp(ani->combAniY, RotationInfo::GetMagicValue(ani->combAniY));
			Ivk_zCModelAniActive_DoCombineAni(ani, model, frame, nextFrame);
		}

		zCModelNodeInst* const head = GetHead(model->homeVob->CastTo<oCNpc>());
		int nodeIndex = 0;

		for (int i = 0; i < ani->protoAni->numNodes; i++)
			if (model->nodeList[ani->protoAni->nodeIndexList[i]] == head)
			{
				nodeIndex = i;
				break;
			}

		for (int f : { frame, nextFrame })
			ani->protoAni->SetQuat(f, nodeIndex, info->rotation);
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

	//zCModelPrototype* __cdecl Hook_zCModelPrototype_Load(zSTRING const&, zCModelPrototype*);
	//Hook<zCModelPrototype* (__cdecl*)(zSTRING const&, zCModelPrototype*)> Ivk_zCModelPrototype_Load(ZENFOR(0x0056EA80, 0x00587E10, 0x00583CF0, 0x00589250), &Hook_zCModelPrototype_Load, HookMode::Patch);
	//zCModelPrototype* __cdecl Hook_zCModelPrototype_Load(zSTRING const& a0, zCModelPrototype* a1)
	//{
	//	zCModelPrototype* proto = Ivk_zCModelPrototype_Load(a0, a1);

	//	if (proto)
	//	{
	//		for (zCModelAni* ani : proto->protoAnis)
	//			if (ani && ani->aniName.StartWith("R_CHAIR_RANDOM_"))
	//				ani->layer = 10;
	//	}

	//	return proto;
	//}
}
