namespace NAMESPACE
{
	const string namespacePrefix = "ZIMPROVEDLEGACYFRYING:";
	std::vector<ZOwner<oCMobInter>> mobs;

	void SearchMobs()
	{
#define TEST(cond) { if (!(cond)) continue; }

		for (zCVob* vob : ogame->GetGameWorld()->voblist)
			if (oCMobInter* mob = dynamic_cast<oCMobInter*>(vob))
			{
				TEST(mob->conditionFunc.IsEmpty());
				TEST(mob->onStateFuncName.IsEmpty());
				TEST(mob->useWithItem.CompareI("ItFoMuttonRaw"));
				TEST(COA(mob->GetModel(), modelProtoList.GetNum()));

				const zSTRING& protoName = mob->GetModel()->modelProtoList[0]->modelProtoName;
				TEST(protoName.StartWith("STOVE_") || protoName.StartWith("PAN_"));

				mobs.emplace_back(mob);
				mob->AddRef();
			}

#undef TEST
	}

	void ChangeMobs()
	{
		static zSTRING onStateFunc;
		
		// workaround: namespace is not added to the function name
		if (onStateFunc.IsEmpty())
			onStateFunc = parser->GetSymbol(namespacePrefix + "ONMOBSTATE_S1") ? (namespacePrefix + "ONMOBSTATE").GetVector() : "ONMOBSTATE";

		for (ZOwner<oCMobInter>& mob : mobs)
			mob->onStateFuncName = onStateFunc;
	}

	void ResetMobs()
	{
		for (ZOwner<oCMobInter>& mob : mobs)
			mob->onStateFuncName = "";
	}

	Sub onLoadEnd(ZSUB(GameEvent::LoadEnd), []
		{
			SearchMobs();
			ChangeMobs();
		});

	Sub onSaveBegin(ZSUB(GameEvent::SaveBegin), []
		{
			ResetMobs();

			if (SaveLoadGameInfo.changeLevel)
				mobs.clear();
		});

	Sub onSaveEnd(ZSUB(GameEvent::SaveEnd), []
		{
			if (!SaveLoadGameInfo.changeLevel)
				ChangeMobs();
		});

	Sub onExit(ZSUB(GameEvent::Exit), []
		{
			mobs.clear();
		});

	std::pair<int*, int*> SearchByPrefix(zCParser* parser, const zSTRING& prefix)
	{
		return std::equal_range(begin(parser->symtab.tablesort), end(parser->symtab.tablesort), -1, [parser, prefix](int x, int y)
			{
				if (x == y)
					return false;

				if (x != -1 && y != -1)
				{
					// should never be executed
					const zSTRING& left = parser->symtab.table[x]->name;
					const zSTRING& right = parser->symtab.table[y]->name;
					return !left.StartWith(prefix) && !right.StartWith(prefix) && strcmp(left, right) < 0;
				}

				if (x == -1)
				{
					const zSTRING& right = parser->symtab.table[y]->name;
					return !right.StartWith(prefix) && strcmp(prefix, right) < 0;
				}

				const zSTRING& left = parser->symtab.table[x]->name;
				return !left.StartWith(prefix) && strcmp(left, prefix) < 0;
			});
	}

	oCInfo::Tpd* GetInfo(const Symbol& symbol)
	{
		static const int c_info = parser->GetIndex("C_INFO");

		if (symbol.GetType() == Symbol::Type::Instance)
			if (parser->GetBaseClass(symbol.GetIndex()) == c_info)
				return reinterpret_cast<oCInfo::Tpd*>(symbol.GetSymbol()->offset);

		return nullptr;
	}

	int __cdecl Ext_AssignFryDialogsTo()
	{
		int npc;
		ZARGS(npc);

		static std::optional<std::vector<Symbol>> instances;

		if (!instances)
		{
			instances.emplace();

			const auto range = SearchByPrefix(parser, namespacePrefix);

			for (auto it = range.first; it != range.second; it++)
			{
				Symbol symbol{ parser, *it };

				if (GetInfo(symbol))
					instances->push_back(symbol);
			}
		}

		for (const Symbol& symbol : instances.value())
			if (oCInfo::Tpd* info = GetInfo(symbol))
				info->npc = npc;

		return false;
	}

	int __cdecl Ext_EnableSelfDialogs()
	{
		oCNpc* npc;
		bool enable;
		ZARGS(npc, enable);

		static std::vector<int> dialogs;
		static int instance;

		if (enable)
		{
			for (int dialog : dialogs)
				if (oCInfo::Tpd* info = GetInfo({ parser, dialog }))
					info->npc = instance;

			return false;
		}

		dialogs.clear();

		if (!ogame || !ogame->infoman)
			return false;

		instance = npc->instanz;

		for (oCInfo* info : ogame->infoman->infoList)
			if (info && info->pd.npc == instance)
			{
				dialogs += info->instance;
				info->pd.npc = -1;
			}

		return false;
	}

	ZEXTERNAL_NS(void, Ext_AssignFryDialogsTo, int);
	ZEXTERNAL_NS(void, Ext_EnableSelfDialogs, oCNpc*, bool);
}
