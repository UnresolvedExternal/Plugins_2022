#include <regex>
#include <iomanip>
#include <intrin.h>

namespace NAMESPACE
{
	zSTRING GetWeaponString(int mode)
	{
		static auto getWeaponString = reinterpret_cast<zSTRING*(__cdecl*)(zSTRING*, int)>(ZENDEF(0x00626100, 0x0064AF70, 0x006523E0, 0x006AEC60));

		zSTRING result;
		return *getWeaponString(&result, mode);
	}

	size_t ReplaceAll(std::string& sourceText, const std::string& searchText, const std::string& replaceText)
	{
		size_t count = 0u;

		for (size_t pos = sourceText.find(searchText); pos != std::string::npos; pos = sourceText.find(searchText, pos + replaceText.size()))
		{
			sourceText.replace(pos, searchText.size(), replaceText);
			count++;
		}

		return count;
	}

	std::string ExpandPattern(const std::string& pattern)
	{
		std::string result{ pattern };
		std::transform(result.begin(), result.end(), result.begin(), &toupper);

		std::string weapons;
		weapons.reserve(64u);

		weapons += '(';

		for (int mode = NPC_WEAPON_NONE; mode < NPC_WEAPON_MAX; mode++)
		{
			if (mode != NPC_WEAPON_NONE)
				weapons += "|";

			if (mode != NPC_WEAPON_DAG)
				weapons += GetWeaponString(mode).ToChar();
		}

		weapons += ')';

		ReplaceAll(result, "{WEAPON}", weapons);
		ReplaceAll(result, "{WALK}", "(RUN|WALK|SNEAK|W)");
		ReplaceAll(result, "{FOOT}", "(L|R)");

		return result;
	}

	struct AniBlendOverrideData
	{
		std::regex regex;
		std::optional<float> blendIn;
		std::optional<float> blendOut;
	};

	std::vector<AniBlendOverrideData> GetBlendOverrides()
	{
		std::vector<AniBlendOverrideData> overrides;

		for (const std::string& pattern : *Options::BlendInAnis)
		{
			const std::string aniName = ExpandPattern(pattern);
			AniBlendOverrideData& override = overrides.emplace_back();
			override.regex = std::regex{ std::string{"^"} + aniName + "$" };
			override.blendIn = *Options::BlendInValue;
		}

		for (const std::string& pattern : *Options::BlendOutAnis)
		{
			const std::string aniName = ExpandPattern(pattern);
			AniBlendOverrideData& override = overrides.emplace_back();
			override.regex = std::regex{ std::string{"^"} + aniName + "$" };
			override.blendOut = -*Options::BlendOutValue;
		}

		return overrides;
	}

	std::unordered_set<zSTRING> GetCombatSneakRightToStandAnis()
	{
		std::unordered_set<zSTRING> anis;

		for (int mode = NPC_WEAPON_FIST; mode < NPC_WEAPON_MAX; mode++)
			if (mode != NPC_WEAPON_DAG)
				anis.insert(Z"T_" + GetWeaponString(mode) + "SNEAKR_2_" + GetWeaponString(mode) + "SNEAK");

		return anis;
	}

	class ModelLogger
	{
	private:
		bool logged;
		const zCModelPrototype& proto;

		friend class AniChangeLogger;

		void Log()
		{
			if (logged || !Options::LogChanges)
				return;

			logged = true;
			cmd << endl << Col16{ CMD_PURPLE } << proto.modelProtoName << Col16{ CMD_WHITE } << endl;
		}

	public:
		ModelLogger(zCModelPrototype& proto) :
			proto{ proto }, 
			logged{ false }
		{

		}
	};

	class AniChangeLogger
	{
	private:
		const zCModelAni& ani;
		float blendIn;
		float blendOut;
		const zTMdl_AniType aniType;
		const float bboxMinY;
		ModelLogger& modelLogger;

		static string GetName(zTMdl_AniType type)
		{
			switch (type)
			{
			case zMDL_ANI_TYPE_NORMAL: return "Normal";
			case zMDL_ANI_TYPE_BLEND: return "Blend";
			case zMDL_ANI_TYPE_SYNC: return "Sync";
			case zMDL_ANI_TYPE_ALIAS: return "Alias";
			case zMDL_ANI_TYPE_BATCH: return "Batch";
			case zMDL_ANI_TYPE_COMB: return "Comb";
			case zMDL_ANI_TYPE_DISABLED: return "Disabled";
			}

			return "";
		}

	public:
		AniChangeLogger(const zCModelAni& ani, ModelLogger& modelLogger) :
			ani{ ani },
			aniType{ ani.aniType },
			bboxMinY{ ani.aniBBox3DObjSpace.mins[VY] },
			modelLogger{ modelLogger }
		{
			ani.GetBlendingSec(blendIn, blendOut);
		}

		~AniChangeLogger()
		{
			if (!Options::LogChanges)
				return;

			float newBlendIn;
			float newBlendOut;
			ani.GetBlendingSec(newBlendIn, newBlendOut);
			
			const zTMdl_AniType newAniType = ani.aniType;

			if (newBlendIn == blendIn && newBlendOut == blendOut && newAniType == aniType && bboxMinY == ani.aniBBox3DObjSpace.mins[VY])
				return;

			modelLogger.Log();

			cmd << Col16{ CMD_CYAN } << ani.aniName << Col16{ CMD_WHITE } << ":";

			if (newAniType != aniType)
				cmd << Col16{ CMD_GREEN } << " aniType (" << GetName(aniType) << " -> " << GetName(newAniType) << ")" << Col16{ CMD_WHITE };

			if (newBlendIn != blendIn)
				cmd << Col16{ CMD_BLUE } << " blendIn (" << blendIn << " -> " << newBlendIn << ")" << Col16{ CMD_WHITE };

			if (newBlendOut != blendOut)
				cmd << Col16{ CMD_YELLOW } << " blendOut (" << blendOut << " -> " << newBlendOut << ")" << Col16{ CMD_WHITE };

			if (ani.aniBBox3DObjSpace.mins[VY] != bboxMinY)
				cmd << Col16{ CMD_RED } << " bbox minY (" << bboxMinY << " -> " << ani.aniBBox3DObjSpace.mins[VY] << ")" << Col16{ CMD_WHITE };

			cmd << endl;
		}
	};

	void TryCorrectBBox(zCModelPrototype* proto, zCModelAni* ani)
	{
		if (!Options::CorrectJumpBBoxes)
			return;

		if (ani->aniType != zMDL_ANI_TYPE_NORMAL && ani->aniType != zMDL_ANI_TYPE_ALIAS && ani->aniType != zMDL_ANI_TYPE_COMB)
			return;

		if (!ani->aniName.EndWith("_JUMP") && !ani->aniName.HasWord("_JUMP_"))
			return;

		zCModelAni* sourceAni = nullptr;

		for (int mode = NPC_WEAPON_NONE; mode < NPC_WEAPON_MAX; mode++)
			if (mode != NPC_WEAPON_DAG)
			{
				const zSTRING weaponString = Z"_" + GetWeaponString(mode);

				if (ani->aniName.HasWord(weaponString))
				{
					const zSTRING aniName = Z"S" + weaponString + "RUN";
					zCModelAni* s_run = nullptr;

					for (zCModelAni* ani : proto->protoAnis)
						if (ani && ani->aniName.Compare(aniName))
						{
							s_run = ani;
							break;
						}

					if (s_run)
						if (s_run->aniType == zMDL_ANI_TYPE_NORMAL || s_run->aniType == zMDL_ANI_TYPE_ALIAS)
							sourceAni = s_run;
				}

				break;
			}

		if (sourceAni && ani->aniBBox3DObjSpace.mins[VY] != sourceAni->aniBBox3DObjSpace.mins[VY])
			ani->aniBBox3DObjSpace.mins[VY] = sourceAni->aniBBox3DObjSpace.mins[VY];
	}

	zCModelPrototype* __cdecl Hook_zCModelPrototype_Load(zSTRING const&, zCModelPrototype*);
	Hook<zCModelPrototype* (__cdecl*)(zSTRING const&, zCModelPrototype*)> Ivk_zCModelPrototype_Load(ZENFOR(0x0056EA80, 0x00587E10, 0x00583CF0, 0x00589250), &Hook_zCModelPrototype_Load, HookMode::Hook);
	zCModelPrototype* __cdecl Hook_zCModelPrototype_Load(zSTRING const& name, zCModelPrototype* baseProto)
	{
		zCModelPrototype* proto = Ivk_zCModelPrototype_Load(name, baseProto);

		if (!proto)
			return proto;

		if (proto->refCtr != 1)
			return proto;

		ModelLogger modelLogger{ *proto };

		static std::vector<AniBlendOverrideData> overrides = GetBlendOverrides();
		static std::unordered_set<zSTRING> combatSneakRightToStandAnis = GetCombatSneakRightToStandAnis();

		for (zCModelAni* ani : proto->protoAnis)
		{
			if (ani && (ani->aniType == zMDL_ANI_TYPE_NORMAL || ani->aniType == zMDL_ANI_TYPE_ALIAS || ani->aniType == zMDL_ANI_TYPE_COMB || ani->aniType == zMDL_ANI_TYPE_BLEND))
			{
				AniChangeLogger logger{ *ani, modelLogger };

				for (const AniBlendOverrideData& override : overrides)
					if (std::regex_match(ani->aniName.ToChar(), override.regex))
					{
						if (override.blendIn && ani->blendInSpeed > 1.0f / override.blendIn.value())
							ani->blendInSpeed = 1.0f / override.blendIn.value();

						if (override.blendOut && ani->blendOutSpeed < 1.0f / override.blendOut.value())
							ani->blendOutSpeed = 1.0f / override.blendOut.value();
					}

				if (Options::InstantCombatSneakToStand && combatSneakRightToStandAnis.find(ani->aniName) != combatSneakRightToStandAnis.end())
				{
					ani->blendInSpeed = 1.0f / 0.2f;
					ani->blendOutSpeed = 1.0f / -0.2f;
					ani->aniType = zMDL_ANI_TYPE_BLEND;
				}

				TryCorrectBBox(proto, ani);
			}
		}

		return proto;
	}

	class DelayedOverlayRemoval
	{
	private:
		zCModel& model;
		zCModelPrototype& proto;
		Sub<void> callback;
		static int counter;

		void ProcessCalllback()
		{
			if (Publisher::GetInstance().GetCurrentEvent() == GameEvent::LoadBegin || Publisher::GetInstance().GetCurrentEvent() == GameEvent::Exit)
			{
				delete this;
				return;
			}

			for (int i = 0; i < model.numActiveAnis; i++)
				if (zCModelAniActive* const ani = model.aniChannels[i])
					if (ani->protoAni && ani->protoAni->aniID < proto.protoAnis.GetNum())
						if (ani->protoAni == proto.protoAnis[ani->protoAni->aniID])
							return;

			delete this;
		}

		void ProcessAniTransitions()
		{
			for (int i = 0; i < model.numActiveAnis; i++)
				if (zCModelAniActive* const ani = model.aniChannels[i])
					if (ani->protoAni && ani->protoAni->aniID < proto.protoAnis.GetNum())
						if (ani->protoAni == proto.protoAnis[ani->protoAni->aniID])
							model.FadeOutAni(ani);

			for (int i = 0; i < model.numActiveAnis; i++)
				if (zCModelAniActive* const ani = model.aniChannels[i])
					for (zCModelAni** nextAni : { &ani->nextAni, &ani->nextAniOverride })
						if (*nextAni && (*nextAni)->aniID < proto.protoAnis.GetNum() && *nextAni == proto.protoAnis[(*nextAni)->aniID])
							*nextAni = model.GetAniFromAniID((*nextAni)->aniID);
		}

		~DelayedOverlayRemoval()
		{
			proto.Release();
			model.Release();
		}

	public:
		DelayedOverlayRemoval(zCModel& model, zCModelPrototype& proto) :
			model{ model },
			proto{ proto }
		{
			model.AddRef();
			model.modelProtoList.RemoveOrder(&proto);
			callback = { GameEvent::Loop | GameEvent::LoadBegin | GameEvent::Exit, std::bind(&DelayedOverlayRemoval::ProcessCalllback, this) };
			ProcessAniTransitions();
		}
	};

	int DelayedOverlayRemoval::counter = 0;

	int __fastcall Hook_zCModel_ApplyModelProtoOverlay(zCModel*, void*, zCModelPrototype*);
	Hook<int(__thiscall*)(zCModel*, zCModelPrototype*), ActiveOption<bool>> Ivk_zCModel_ApplyModelProtoOverlay(ZENFOR(0x0055F000, 0x00577390, 0x00573590, 0x00578840), &Hook_zCModel_ApplyModelProtoOverlay, HookMode::Patch, Options::SmoothOverlaySwitch);
	int __fastcall Hook_zCModel_ApplyModelProtoOverlay(zCModel* model, void* vtable, zCModelPrototype* proto)
	{
		if (!proto || model->modelProtoList.IsInList(proto) || model->modelProtoList.IsEmpty() || proto->baseModelProto != model->modelProtoList[0])
			return false;

		model->modelProtoList.InsertEnd(proto);
		proto->refCtr += 1;

		for (int i = 0; i < model->numActiveAnis; i++)
		{
			zCModelAniActive* const ani = model->aniChannels[i];

			if (!ani || !ani->protoAni)
				continue;

			const int id = ani->protoAni->aniID;

			if (id < proto->protoAnis.GetNum() && proto->protoAnis[id] && ani->protoAni != proto->protoAnis[id])
				model->FadeOutAni(ani);

			for (zCModelAni** nextAni : { &ani->nextAni, &ani->nextAniOverride })
				if (*nextAni && (*nextAni)->aniID < proto->protoAnis.GetNum() && proto->protoAnis[(*nextAni)->aniID])
					*nextAni = model->GetAniFromAniID((*nextAni)->aniID);
		}

		return true;
	}

	void __fastcall Hook_zCModel_RemoveModelProtoOverlay(zCModel*, void*, zCModelPrototype*);
	Hook<void(__thiscall*)(zCModel*, zCModelPrototype*), ActiveOption<bool>> Ivk_zCModel_RemoveModelProtoOverlay(ZENFOR(0x0055F5C0, 0x005779A0, 0x00573B90, 0x00578E40), &Hook_zCModel_RemoveModelProtoOverlay, HookMode::Patch, Options::SmoothOverlaySwitch);
	void __fastcall Hook_zCModel_RemoveModelProtoOverlay(zCModel* model, void* vtable, zCModelPrototype* proto)
	{
		if (proto && model->modelProtoList.IsInList(proto))
			new DelayedOverlayRemoval{ *model, *proto };
	}

	void __fastcall Hook_zCModel_RemoveModelProtoOverlay_2(zCModel*, void*, zSTRING const&);
	Hook<void(__thiscall*)(zCModel*, zSTRING const&), ActiveOption<bool>> Ivk_zCModel_RemoveModelProtoOverlay_2(ZENFOR(0x0055F3C0, 0x00577780, 0x00573990, 0x00578C40), &Hook_zCModel_RemoveModelProtoOverlay_2, HookMode::Patch, Options::SmoothOverlaySwitch);
	void __fastcall Hook_zCModel_RemoveModelProtoOverlay_2(zCModel* model, void* vtable, zSTRING const& name)
	{
		zPATH path{ name };
		const zSTRING protoName = path.GetFilename();

		for (zCModelPrototype* proto : model->modelProtoList)
			if (proto->modelProtoName == protoName)
			{
				new DelayedOverlayRemoval{ *model, *proto };
				return;
			}
	}

	void __fastcall Hook_zCModel_AdvanceAnis(zCModel*, void*);
	Hook<void(__thiscall*)(zCModel*), ActiveOption<bool>> Ivk_zCModel_AdvanceAnis(ZENFOR(0x00562CD0, 0x0057B430, 0x00577570, 0x0057CA90), &Hook_zCModel_AdvanceAnis, HookMode::Patch, Options::SmoothRootPos);
	void __fastcall Hook_zCModel_AdvanceAnis(zCModel* model, void* vtable)
	{
		Ivk_zCModel_AdvanceAnis(model);

		for (zCModelNodeInst* node : model->nodeList)
			if (!node->parentNode)
			{
				float modAnis = 0.0f;
				float totalWeight = 0.0f;

				for (int i = 0; i < node->numNodeAnis; i++)
				{
					const auto& flags = node->nodeAniList[i].modelAni->protoAni->aniFlags;

					if (!flags.flagVobPos && !flags.flagVobRot)
						continue;

					modAnis += 1.0f;
					totalWeight += node->nodeAniList[i].weight;
				}

				if (totalWeight < 0.01f)
					return;

				model->rootPosLocal = {};

				for (int i = 0; i < node->numNodeAnis; i++)
				{
					const auto& flags = node->nodeAniList[i].modelAni->protoAni->aniFlags;

					if (!flags.flagVobPos && !flags.flagVobRot)
						continue;

					model->rootPosLocal += node->nodeAniList[i].modelAni->thisPos * (node->nodeAniList[i].weight / totalWeight);
				}

				if (model->modelScaleOn)
					model->rootPosLocal = Alg_Prod(model->rootPosLocal, model->modelScale);

				return;
			}
	}
}
