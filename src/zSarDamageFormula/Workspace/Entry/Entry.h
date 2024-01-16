namespace NAMESPACE
{
    // ��������������� �������
    // ����� ���� totalDamage ����� ������ �� damageType
    void SplitDamage(oEDamageType damageType, unsigned long aryDamage[oEDamageIndex_MAX], float& totalDamage)
    {
        if (damageType == oEDamageType_Unknown)
            return;

        float divisor = 0.0f;

        // ������� ���������� ����� �����
        for (int i = 0; i < oEDamageIndex_MAX; i++)
            if (damageType & (1 << i))
                divisor += 1.0f;

        if (!divisor)
            return;

        // ���������� ����� �� ������ ���
        const int damage = static_cast<int>(totalDamage / divisor + 0.5f);
        totalDamage = 0;

        // ���������� ���� � aryDamage, ���� ��� ����
        for (int i = 0; i < oEDamageIndex_MAX; i++)
            if ((damageType & (1 << i)) && !aryDamage[i])
            {
                aryDamage[i] = damage;
                totalDamage += damage;
            }
    }

    oCNpc* gVictim;
    oCNpc::oSDamageDescriptor* gDescriptor;
    bool gIsCrit;
    float gTalent;

    std::unordered_map<string, std::function<void(std::vector<float>&)>> operations =
    {
        { "+", [](std::vector<float>& v) { v[v.size() - 2] += v.back(); v.pop_back(); }},
        { "-", [](std::vector<float>& v) { v[v.size() - 2] -= v.back(); v.pop_back(); }},
        { "*", [](std::vector<float>& v) { v[v.size() - 2] *= v.back(); v.pop_back(); }},
        { "/", [](std::vector<float>& v) { v[v.size() - 2] /= v.back(); v.pop_back(); }},
        { "isbow", [](std::vector<float>& v) { if (gDescriptor->pNpcAttacker->GetWeaponMode() != NPC_WEAPON_BOW) v.back() = 0.0f; }},
        { "iscbow", [](std::vector<float>& v) { if (gDescriptor->pNpcAttacker->GetWeaponMode() != NPC_WEAPON_CBOW) v.back() = 0.0f; }},
        { "iscrit", [](std::vector<float>& v) { if (!gIsCrit) v.back() = 0.0f; }},
        { "ismelee", [](std::vector<float>& v) { if (gDescriptor->pNpcAttacker->GetWeaponMode() < NPC_WEAPON_DAG || gDescriptor->pNpcAttacker->GetWeaponMode() > NPC_WEAPON_2HS) v.back() = 0.0f; }},
        { "str", [](std::vector<float>& v) { v += static_cast<float>(gDescriptor->pNpcAttacker->GetAttribute(NPC_ATR_STRENGTH)); }},
        { "str2", [](std::vector<float>& v) { v += static_cast<float>(gVictim->GetAttribute(NPC_ATR_STRENGTH)); }},
        { "dex", [](std::vector<float>& v) { v += static_cast<float>(gDescriptor->pNpcAttacker->GetAttribute(NPC_ATR_DEXTERITY)); }},
        { "dex2", [](std::vector<float>& v) { v += static_cast<float>(gVictim->GetAttribute(NPC_ATR_DEXTERITY)); }},
        { "talent", [](std::vector<float>& v) { v += gTalent; }}

    };

    FASTHOOK_PATCH(oCNpc, OnDamage_Hit);

    void oCNpc::OnDamage_Hit_Union(oSDamageDescriptor& desc)
    {
        if (!desc.pNpcAttacker || !desc.pItemWeapon || desc.enuModeWeapon != oETypeWeapon_Melee && desc.enuModeWeapon != oETypeWeapon_Range)
            return THISCALL(Hook_oCNpc_OnDamage_Hit)(desc);

        gVictim = this;
        gDescriptor = &desc;

        // ����������, ����������� ���� ��� ���
        // ������ ����, ���� ����� ���������� ��� ��������� - �� NPC ��� ��������� - ������ ��� ����� �� � ������� ��� � �������
        bool hasHit = desc.pVisualFX || !desc.pNpcAttacker || desc.pNpcAttacker->IsMonster() || desc.enuModeWeapon != oETypeWeapon_Melee;

        // ���� ������ ��������, ���������� ���� ��������� � ���� ���������
        if (!hasHit)
        {
            int talentNr = Invalid;

            switch (desc.pNpcAttacker->GetWeaponMode())
            {
            case NPC_WEAPON_1HS:
            case NPC_WEAPON_DAG:
                talentNr = NPC_HITCHANCE_1H;
                break;

            case NPC_WEAPON_2HS:
                talentNr = NPC_HITCHANCE_2H;
                break;

            case NPC_WEAPON_BOW:
                talentNr = NPC_HITCHANCE_BOW;
                break;

            case NPC_WEAPON_CBOW:
                talentNr = NPC_HITCHANCE_CROSSBOW;
                break;
            }

            if (talentNr == Invalid)
            {
                gIsCrit = hasHit = true;
                gTalent = 0.0f;
            }
            else
            {
                gTalent = static_cast<float>(desc.pNpcAttacker->ZENDEF2(GetTalentValue, GetHitChance)(talentNr));
                const float hitChance = Options::CritChance->Calculate(operations) / 100.0f;
                const float random = static_cast<float>(rand()) / static_cast<float>(RAND_MAX + 1);
                gIsCrit = hasHit = random < hitChance;
            }
        }

        // ��������� ��������� �����
        for (int i = 0; i < oEDamageIndex_MAX; i++)
            desc.aryDamage[i] = static_cast<int>(desc.aryDamage[i] * desc.fDamageMultiplier);

        desc.fDamageTotal *= desc.fDamageMultiplier;

        // ��� ����������� ������������ ����� �������������� � ��������
        const bool isSemiHuman = desc.pNpcAttacker &&
            (desc.pNpcAttacker->IsHuman() || desc.pNpcAttacker->IsOrc() || desc.pNpcAttacker->IsGoblin() || desc.pNpcAttacker->IsSkeleton());

        bool divideDamage = true;

        for (int i = 0; i < oEDamageIndex_MAX && divideDamage; i++)
            if (HasFlag(desc.enuModeDamage, 1 << i) && desc.aryDamage[i])
                divideDamage = false;

        // ���� � aryDamage ����, �� ��������� ��� ����������� fDamageTotal �� �������� ����� �����
        if (divideDamage)
        {
            // ���� ��������� NPC �� �����������, ����� �� ���������� � ����� ���� �� �����, �� ���������� ���� ��� ����
            // ����������: pFXHit �� ����� ����� ��� ���������� �����, ���� C_CanNpcCollideWithSpell �� �������
            // �� ����� COLL_DOEVERYTHING, �� COLL_APPLYVICTIMSTATE
            if (desc.pNpcAttacker && !isSemiHuman && !desc.pFXHit && !desc.fDamageTotal)
                desc.fDamageTotal = static_cast<float>(desc.pNpcAttacker->GetAttribute(NPC_ATR_STRENGTH));

            // ������������ ����� ���� ����� ��������� ������ �����
            SplitDamage(static_cast<oEDamageType>(desc.enuModeDamage), desc.aryDamage, desc.fDamageTotal);
        }

        // ��� ������������ ����������� ����� ���� ����������� ������
        if (isSemiHuman)
            for (int i = 0; i < oEDamageIndex_MAX; i++)
                desc.aryDamage[i] += desc.pNpcAttacker->GetDamageByIndex(static_cast<oEIndexDamage>(i));

        // ���� ������������ ����� ������� �������������
        if (!desc.pFXHit && isSemiHuman)
        {
            // ����� �������� ����� ����������� ������ �����
            int divisor =
                (HasFlag(desc.enuModeDamage, oEDamageType_Blunt) ? 1 : 0) +
                (HasFlag(desc.enuModeDamage, oEDamageType_Edge) ? 1 : 0) +
                (HasFlag(desc.enuModeDamage, oEDamageType_Point) ? 1 : 0);

            // ������ (������ ��� ������ � ������ ������) ��� ����������� ����� ����� ����� ������� ����
            if (desc.pNpcAttacker->IsMonster() && !divisor)
            {
                desc.enuModeDamage |= oEDamageType_Edge;
                divisor = 1;
            }

            // ����� �������� ����� ����������� ������ �����
            if (divisor)
                for (int damageIndex : { oEDamageIndex_Blunt, oEDamageIndex_Edge, oEDamageIndex_Point })
                {
                    const int damageType = 1 << damageIndex;

                    if (!HasFlag(desc.enuModeDamage, damageType))
                        continue;

                    operations["weapon"] = [&desc, damageIndex](auto& v) { v += static_cast<float>(desc.aryDamage[damageIndex]); };
                    
                    auto& formula = desc.pNpcAttacker->GetWeaponMode() >= NPC_WEAPON_DAG && desc.pNpcAttacker->GetWeaponMode() <= NPC_WEAPON_2HS ?
                        Options::MeleeRawDamage :
                        Options::RangeRawDamage;

                    desc.aryDamage[damageIndex] = std::max(0, static_cast<int>(formula->Calculate(operations) + 0.5f));
                }
        }

        // ���� ������
        bool immortal = true;
        int damageTotal = 0;

        for (int i = 0; i < oEDamageIndex_MAX; i++)
        {
            if (!HasFlag(desc.enuModeDamage, 1 << i))
            {
                desc.aryDamageEffective[i] = 0;
                continue;
            }

            const int protection = GetProtectionByIndex(static_cast<oEDamageIndex>(i));
            operations["raw"] = [&desc, i](auto& v) { v += static_cast<float>(desc.aryDamage[i]); };
            operations["protection"] = [protection](auto& v) { v += static_cast<float>(protection); };
            immortal = immortal && protection < 0; // �������� � �����, ���� �������� � ������� �� �������� ����� �����

            // �������� ������
            desc.aryDamageEffective[i] = protection < 0 ? 0 : std::max(0, static_cast<int>(Options::Protection->Calculate(operations) + 0.5f));

            // ��������� ��������� ����
            damageTotal += desc.aryDamageEffective[i];
        }

        immortal = immortal || HasFlag(NPC_FLAG_IMMORTAL);
        desc.fDamageTotal = immortal ? 0.0f : static_cast<float>(damageTotal);
        desc.fDamageEffective = desc.fDamageTotal;
        desc.fDamageReal = desc.fDamageTotal;

        int damage = static_cast<int>(desc.fDamageTotal);

        // ��������� ����������� ����, ���� ����� �� ����������
        if (!desc.pFXHit)
        {
            static Symbol npcMinimalDamage{ parser, "NPC_MINIMAL_DAMAGE" };
            static const int minimalDamage = npcMinimalDamage ? npcMinimalDamage.GetValue<int>(0) : 0;

            if (damage < minimalDamage)
            {
                damage = minimalDamage;
                desc.fDamageReal = static_cast<float>(minimalDamage);
            }
        }

        // ������ ��������� ��� ��������� NPC
        if (HasFlag(desc.enuModeDamage, oEDamageType_Barrier) && COA(GetAnictrl(), GetWaterLevel()) >= 2)
        {
            damage = GetAttribute(NPC_ATR_HITPOINTS);
            desc.fDamageReal = static_cast<float>(damage);
        }

        // ��������� ��������, ���� ���� �������
        if (!immortal)
            ChangeAttribute(NPC_ATR_HITPOINTS, -damage);

        // ���������� ������� �����������
        hpHeal = GetAttribute(NPC_ATR_REGENERATEHP) * 1000.0f;
        manaHeal = GetAttribute(NPC_ATR_REGENERATEMANA) * 1000.0f;
    }
}
