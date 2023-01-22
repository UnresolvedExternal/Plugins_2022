namespace NAMESPACE
{
	bool IsValidTransform(oCSpell* spell)
	{
		return spell->spellID >= ZENDEF2(26, 47) && spell->spellID <= ZENDEF2(39, 58) && spell->saveNpc && spell->spellCasterNpc;
	}

	void HandleTransform(oCNpc* source, oCNpc* target)
	{
		for (zCEventMessage* message : source->GetEM()->messageList)
			if (!message->IsDeleted())
				if (oCMsgDamage* damage = message->CastTo<oCMsgDamage>())
					if (damage->GetSubType() == oCMsgDamage::EV_DAMAGE_PER_FRAME)
						if (damage->descDamage.pVisualFX)
						{
							source->OnDamage_Effects_End(damage->descDamage);
							damage->Delete();
						}

		for (zCEventManager* em : zCEventManager::activeEM)
			for (zCEventMessage* message : em->messageList)
				if (oCMsgDamage* damage = message->CastTo<oCMsgDamage>())
					if (damage->GetSubType() == oCMsgDamage::EV_DAMAGE_PER_FRAME || damage->GetSubType() == oCMsgDamage::EV_DAMAGE_ONCE)
						if (damage->descDamage.pVobAttacker == source)
						{
							damage->descDamage.pVobAttacker = target;
							damage->descDamage.pNpcAttacker = target;
						}

		for (zCVob* vob : ogame->GetWorld()->activeVobList)
			if (oCNpc* npc = vob->CastTo<oCNpc>())
			{
				if (npc->GetFocusVob() == source)
					npc->SetFocusVob(target);

				if (npc->enemy == source)
					npc->SetEnemy(target);
			}
	}

	void __fastcall Hook_oCSpell_EndTimedEffect(oCSpell*, void*);
	Hook<void(__thiscall*)(oCSpell*)> Ivk_oCSpell_EndTimedEffect(ZENFOR(0x0047F330, 0x00489E60, 0x004857F0, 0x00486E10), &Hook_oCSpell_EndTimedEffect, HookMode::Patch);
	void __fastcall Hook_oCSpell_EndTimedEffect(oCSpell* spell, void* vtable)
	{
		Ivk_oCSpell_EndTimedEffect(spell);

		if (IsValidTransform(spell) && spell->CanBeDeleted())
			HandleTransform(spell->spellCasterNpc, spell->saveNpc);
	}

	int __fastcall Hook_oCSpell_CastSpecificSpell(oCSpell*, void*);
	Hook<int(__thiscall*)(oCSpell*)> Ivk_oCSpell_CastSpecificSpell(ZENFOR(0x0047EC70, 0x004896B0, 0x00485340, 0x00486960), &Hook_oCSpell_CastSpecificSpell, HookMode::Patch);
	int __fastcall Hook_oCSpell_CastSpecificSpell(oCSpell* spell, void* vtable)
	{
		int result = Ivk_oCSpell_CastSpecificSpell(spell);

		if (IsValidTransform(spell) && !spell->saveNpc->GetHomeWorld())
			HandleTransform(spell->saveNpc, spell->spellCasterNpc);

		return result;
	}
}