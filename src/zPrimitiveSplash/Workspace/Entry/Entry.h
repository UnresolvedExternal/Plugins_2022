namespace NAMESPACE
{
	void __fastcall Hook_oCAniCtrl_Human_CreateHit(oCAniCtrl_Human*, void*, zCVob*);
	Hook<void(__thiscall*)(oCAniCtrl_Human*, zCVob*)> Ivk_oCAniCtrl_Human_CreateHit(ZENFOR(0x00627C80, 0x0064CCF0, 0x00653EF0, 0x006B0830), &Hook_oCAniCtrl_Human_CreateHit, HookMode::Patch);

	bool IsPartyMember(oCNpc* npc)
	{
		if (npc->IsSelfPlayer())
			return true;

		static const Symbol aiv_partymember{ parser, "AIV_PARTYMEMBER" };

		if (aiv_partymember.GetType() != Symbol::Type::VarInt)
			return false;

		const int index = aiv_partymember.GetValue<int>(0);

		if (index < 0 || index >= static_cast<int>(std::size(npc->aiscriptvars)))
			return false;

		return npc->aiscriptvars[index];
	}

	bool IsValidAOEVictim(oCNpc* attacker, oCNpc* victim)
	{
		float range;

		bool isValid = victim->GetAttribute(NPC_ATR_HITPOINTS) > 0;
		isValid = isValid && attacker != victim;
		isValid = isValid && !victim->state.IsInState(NPC_AISTATE_DEAD) && !victim->state.IsInState(NPC_AISTATE_UNCONSCIOUS);
		isValid = isValid && attacker->IsInFightRange(victim, range);
		isValid = isValid && abs(attacker->GetAngle(victim)) < Options::HalfAngle;
		isValid = isValid && !(IsPartyMember(attacker) && IsPartyMember(victim));

		if (!isValid)
			return false;

		if (!victim->IsHuman())
			return true;

		static const int zs_attack = parser->GetIndex("ZS_ATTACK");
		static const int zs_mm_attack = parser->GetIndex("ZS_MM_ATTACK");

		if (victim->enemy)
			for (int state : { zs_attack, zs_mm_attack })
				if (state != -1)
					if (victim->state.IsInState(state))
						if (victim->enemy == attacker || IsPartyMember(attacker) && IsPartyMember(victim->enemy))
							return true;

		return false;
	}

	void ApplyAOE(oCNpc* attacker, oCNpc* target)
	{
		for (zCVob* vob : ogame->GetWorld()->activeVobList)
			if (vob && vob->GetVobType() == zVOB_TYPE_NSC && vob != attacker && vob != target)
				if (oCNpc* victim = vob->CastTo<oCNpc>())
					if (IsValidAOEVictim(attacker, victim))
						Ivk_oCAniCtrl_Human_CreateHit(attacker->anictrl, victim);
	}

	void __fastcall Hook_oCAniCtrl_Human_CreateHit(oCAniCtrl_Human* controller, void* vtable, zCVob* vob)
	{
		Ivk_oCAniCtrl_Human_CreateHit(controller, vob);

		if (player && controller->npc == player && vob && vob->GetVobType() == zVOB_TYPE_NSC)
			ApplyAOE(player, vob->CastTo<oCNpc>());
	}
}
