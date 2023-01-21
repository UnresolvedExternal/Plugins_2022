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

	ZEXTERNAL(void, AI_TriggerAniEvent, oCNpc*, int, zSTRING, int);
	ZEXTERNAL(void, Npc_TriggerAniEvent, oCNpc*, int, zSTRING, int);
}
