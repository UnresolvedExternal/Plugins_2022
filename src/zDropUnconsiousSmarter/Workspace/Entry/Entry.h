#if ENGINE >= Engine_G2

namespace NAMESPACE
{
	FASTHOOK_PATCH(oCNpc, OnDamage_Condition);

	bool CheckDuelConditions(oCNpc* victim, oCNpc::oSDamageDescriptor& desc)
	{
		if (desc.pNpcAttacker && desc.pNpcAttacker->IsSelfPlayer())
			if (victim->IsHuman())
				if (desc.enuModeWeapon == oETypeWeapon_Fist || desc.enuModeWeapon == oETypeWeapon_Melee)
					for (auto damageType : { oEDamageType_Point, oEDamageType_Fire, oEDamageType_Magic })
						if (victim->HasFlag(desc.enuModeDamage, damageType))
							return true;

		return false;
	}

	void oCNpc::OnDamage_Condition_Union(oSDamageDescriptor& desc)
	{
		// ������ �� ������ ����, �� �� ��...
		if (!GetAnictrl())
			return;

		// ����������������� ����� ������������� �������,
		// ������� ���� ������ false
		desc.bIsUnconscious = false;

		// �������������� ������������� �����
		desc.bIsDead = GetAttribute(NPC_ATR_HITPOINTS) <= 0;

		// ���� �������� ��� ����� - �� ��������� �� � ���� �� ���������
		if (GetAttribute(NPC_ATR_HITPOINTS) > 1)
			return;

		// ���� ���� ���������� ��� ��� ������ � ����, �� ��������������� ��������� �����������,
		// � ������������� ����������� �� ����� ����������
		if (!desc.pNpcAttacker || GetAnictrl()->IsInWater())
			return;

		const bool hasBlunt = HasFlag(desc.enuModeDamage, oEDamageType_Blunt);
		const bool hasEdge = HasFlag(desc.enuModeDamage, oEDamageType_Edge);

		// ������ ������ �� ��������������� ��������� �������� ������ � �������, �����
		// ���� �������� ���� ��� ���� ������� ���� �� �������� ��� ���������� ���� bDamageDontKill
		if (desc.bIsDead && !desc.bDamageDontKill && !hasBlunt && !(desc.pNpcAttacker->IsHuman() && hasEdge) && !CheckDuelConditions(this, desc))
			return;

		static const int C_DropUnconscious = parser->GetIndex("C_DropUnconscious");

		if (C_DropUnconscious != Invalid)
		{
			ParserScope scope{ parser };
			static Symbol self{ parser, "SELF" };
			static Symbol other{ parser, "OTHER" };

			self.SetValue(0, reinterpret_cast<int>(this));
			other.SetValue(0, reinterpret_cast<int>(desc.pNpcAttacker));

			// �������� func int C_DropUnconscious()
			desc.bIsUnconscious = CallParser<bool>(parser, C_DropUnconscious);
		}
		else
			// ���� ������� ���, �� ������ � ��������������� ��������� �����
			desc.bIsUnconscious = IsHuman();

		// ���� ���������� ��������������� ���������, �� �������� ������
		if (desc.bIsUnconscious)
		{
			desc.bIsDead = false;

			// ��������� ��������� ��
			if (IsSelfPlayer())
				CloseInventory();
		}
	}
}

#endif
