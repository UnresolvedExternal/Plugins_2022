namespace NAMESPACE
{
	void __fastcall Hook_oCNpc_OnDamage_Effects_Start(oCNpc*, void*, oCNpc::oSDamageDescriptor&);
	Hook<void(__thiscall*)(oCNpc*, oCNpc::oSDamageDescriptor&)> Ivk_oCNpc_OnDamage_Effects_Start(ZENFOR(0x00739A20, 0x007791F0, 0x00785B80, 0x0066EE40), &Hook_oCNpc_OnDamage_Effects_Start, HookMode::Patch);
	void __fastcall Hook_oCNpc_OnDamage_Effects_Start(oCNpc* npc, void* vtable, oCNpc::oSDamageDescriptor& desc)
	{
		Ivk_oCNpc_OnDamage_Effects_Start(npc, desc);

		if (desc.pVisualFX || !desc.strVisualFX.IsEmpty() || !desc.fDamageReal)
			return;

		const float damage = desc.aryDamageEffective[oEDamageIndex_Edge] * 0.2f;

		if (damage < 1 || npc->GetProtectionByIndex(oEDamageIndex_Fall) < 0)
			return;

		oCNpc::oSDamageDescriptor dotDesc;
		ZeroMemory(&dotDesc, sizeof(dotDesc));
		dotDesc = desc;
		std::fill_n(dotDesc.aryDamage, oEDamageIndex_MAX, 0ul);
		dotDesc.enuModeDamage = oEDamageType_Fall;
		dotDesc.fTimeDuration = 3000.0f;
		dotDesc.fTimeInterval = 1000.0f;
		dotDesc.fDamagePerInterval = static_cast<float>(npc->GetProtectionByIndex(oEDamageIndex_Fall)) + damage;

		oCMsgDamage* const message = new oCMsgDamage{ oCMsgDamage::EV_DAMAGE_PER_FRAME, dotDesc };
		message->SetHighPriority(true);
		npc->GetEM()->OnMessage(message, npc);
	}
}