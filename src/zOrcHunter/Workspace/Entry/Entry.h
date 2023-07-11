namespace NAMESPACE
{
	oCMsgManipulate* TriggerAniEventImpl(oCNpc*& npc)
	{
		int subtype;
		zSTRING slotName;
		int instance;

		ZARGS(npc, subtype, slotName, instance);

		if (npc && npc->homeWorld && subtype >= oCMsgManipulate::EV_INSERTINTERACTITEM && subtype <= oCMsgManipulate::EV_EXCHANGEINTERACTITEM)
		{
			zSTRING itemName = (instance > 0) ? COA(parser, GetSymbol(instance), name) : "";
			return new oCMsgManipulate(static_cast<oCMsgManipulate::TManipulateSubType>(subtype), itemName, slotName);
		}

		return nullptr;
	}

	int __cdecl AI_TriggerAniEvent()
	{
		ParserScope scope{ parser };
		oCNpc* npc;

		if (oCMsgManipulate* message = TriggerAniEventImpl(npc))
			npc->GetEM()->OnMessage(message, npc);

		return false;
	}

	int __cdecl Npc_TriggerAniEvent()
	{
		ParserScope scope{ parser };
		oCNpc* npc;

		if (ZOwner<oCMsgManipulate> message{ TriggerAniEventImpl(npc) })
			npc->OnMessage(message.get(), npc);

		return false;
	}

	int __cdecl Npc_SetInteractItem()
	{
		ParserScope scope{ parser };
		oCNpc* npc;
		int instance;
		int amount;
		ZARGS(npc, instance, amount);

		if (!npc)
			return false;

		if (!npc->interactItem && instance == -1)
			return false;

		if (npc->interactItem && npc->interactItem->instanz == instance)
			return false;

		if (instance < 0)
		{
			npc->SetInteractItem(nullptr);
			return false;
		}

		oCItem* item = npc->RemoveFromInv(instance, amount);

		if (!item)
		{
			npc->CreateItems(instance, amount);
			item = npc->RemoveFromInv(instance, amount);
		}

		if (!item)
			return false;

		npc->SetInteractItem(item);
		return false;
	}

	int __cdecl Npc_GetInteractItem()
	{
		oCNpc* npc;
		ZARGS(npc);
		ZRETURN(COA(npc, interactItem));
		return false;
	}

	int __cdecl Item_GetAmount()
	{
		oCItem* item;
		ZARGS(item);
		ZRETURN(COA(item, amount));
		return false;
	}

	ZEXTERNAL(void, AI_TriggerAniEvent, oCNpc*, int, zSTRING, int);
	ZEXTERNAL(void, Npc_TriggerAniEvent, oCNpc*, int, zSTRING, int);
	ZEXTERNAL(void, Npc_SetInteractItem, oCNpc*, int, int);
	ZEXTERNAL(oCItem*, Npc_GetInteractItem, oCNpc*);
	ZEXTERNAL(int, Item_GetAmount, oCItem*);
}
