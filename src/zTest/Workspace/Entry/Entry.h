namespace NAMESPACE
{
    class oCVisFX_Lightning : public oCVisualFX
    {
    public:
        zCLASS_UNION_DECLARATION(oCVisFX_Lightning);
        oCVisFX_Lightning();
        virtual ~oCVisFX_Lightning();
    };

    zCLASS_UNION_DEFINITION(oCVisFX_Lightning, oCVisualFX, 0, 0);


    oCVisFX_Lightning::oCVisFX_Lightning() : oCVisualFX()
    {
    }

    oCVisFX_Lightning::~oCVisFX_Lightning()
    {
    }

    oCVisualFX* __fastcall Hook_oCSpell_CreateEffect(oCSpell*, void*);
    Hook<oCVisualFX* (__thiscall*)(oCSpell*)> Ivk_oCSpell_CreateEffect(ZENFOR(0x0047BED0, 0x004865E0, 0x00482CA0, 0x004842E0), &Hook_oCSpell_CreateEffect, HookMode::Patch);
    oCVisualFX* __fastcall Hook_oCSpell_CreateEffect(oCSpell* _this, void* vtable)
    {
        if (_this->isMultiEffect)
            _this->effect = new oCVisFX_MultiTarget();
        else
            _this->effect = new oCVisFX_Lightning();

        _this->effect->SetSpellTargetTypes(_this->targetCollectType);
        _this->effect->SetDamage(_this->damagePerLevel);
        _this->effect->SetDamageType(_this->damageType);
        _this->effect->SetSpellType(_this->spellID);
        _this->effect->SetSpellCat(_this->GetCategory());
        _this->effect->SetByScript("SPELLFX_" + _this->GetSpellInstanceName(_this->spellID));
        _this->effect->GetDamage();

        ogame->GetTextView()->Printwin(_this->effect->fxName);

        return _this->effect;
    }
}
