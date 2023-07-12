META
{
	Parser = Game;
	Namespace = zImprovedLegacyFrying;
	MergeMode = False;
};

test (ItFoMutton && ItFoMuttonRaw)
{

test (!True) const int True = 1;
test (!False) const int False = 0;

var int IsFrying;

func void OnMobState_S1()
{
	if Npc_IsPlayer(self) && Npc_HasItems(self, ItFoMuttonRaw)
	{
		Ext_EnableSelfDialogs(self, False);
		Ext_AssignFryDialogsTo(Hlp_GetInstanceID(self));
		IsFrying = True;
		test (AIV_INVINCIBLE) self.aivar[AIV_INVINCIBLE] = True;
		AI_ProcessInfos(self);
	};
};

func void StopFrying()
{
	Ext_EnableSelfDialogs(self, True);
	Ext_AssignFryDialogsTo(-1);
	IsFrying = False;
	test (AIV_INVINCIBLE) self.aivar[AIV_INVINCIBLE] = False;
	AI_StopProcessInfos(self);
};

func void Fry(var int amount)
{
	Npc_RemoveInvItems(self, ItFoMuttonRaw, amount);
	CreateInvItems(self, ItFoMutton, amount);
	
	if !Npc_HasItems(self, ItFoMuttonRaw)
	{
		StopFrying();
	};
};

func int CheckDiaConditions(var int amount)
{
	return IsFrying && Npc_IsPlayer(self) && Npc_HasItems(self, ItFoMuttonRaw) >= amount;
};

func string GetRussianPieceWord(var int amount)
{
	if amount >= 11 && amount <= 19 || amount % 10 == 0 || amount % 10 >= 5
	{
		return "кусков";
	};
	
	if amount % 10 == 1
	{
		return "кусок";
	};
	
	return "куска";
};

func string GetDiaDescription(var int amount)
{
	if !amount
	{
		test (Dialog_Ende) return Dialog_Ende;
		
		return Str_GetLocalizedString(
			"КОНЕЦ",
			"END",
			"ENDE",
			"KONIEC"
		);
	};
	
	var string description;
	
	if Str_GetCurrentCP() == 1251
	{
		description = GetRussianPieceWord(amount);
		description = Str_Format("Пожарить еще %i %s", amount, description); 
		return Str_UTF8_to_ANSI(description, 1251);
	};

	if amount == 1
	{
		description = Str_GetLocalizedString(
			"Пожарить еще %i кусок",
			"Fry %i more piece",
			"%i weiteres Stuck braten",
			"Usmaz jeszcze %i kawałek"
		);
	}
	else
	{
		description = Str_GetLocalizedString(
			"Пожарить еще %i кусков",
			"Fry %i more pieces",
			"%i weitere Stuck braten",
			"Usmaz jeszcze %i kawalkow"
		);
	};
	
	return Str_Format(description, amount);
};

prototype Dia_Fry_Default(C_INFO)
{
	npc = -1;
	important = False;
	permanent = True;
};

instance Dia_Fry_1(Dia_Fry_Default)
{
	nr = 1;
	condition = Dia_Fry_1_Condition;
	information = Dia_Fry_1_Information;
	description = GetDiaDescription(1);
};

func int Dia_Fry_1_Condition()
{
	return CheckDiaConditions(1);
};

func void Dia_Fry_1_Information()
{
	Fry(1);
};

instance Dia_Fry_5(Dia_Fry_Default)
{
	nr = 5;
	condition = Dia_Fry_5_Condition;
	information = Dia_Fry_5_Information;
	description = GetDiaDescription(5);
};

func int Dia_Fry_5_Condition()
{
	return CheckDiaConditions(5);
};

func void Dia_Fry_5_Information()
{
	Fry(5);
};

instance Dia_Fry_10(Dia_Fry_Default)
{
	nr = 10;
	condition = Dia_Fry_10_Condition;
	information = Dia_Fry_10_Information;
	description = GetDiaDescription(10);
};

func int Dia_Fry_10_Condition()
{
	return CheckDiaConditions(10);
};

func void Dia_Fry_10_Information()
{
	Fry(10);
};

instance Dia_Fry_All(Dia_Fry_Default)
{
	nr = 998;
	condition = Dia_Fry_All_Condition;
	information = Dia_Fry_All_Information;
};

func int Dia_Fry_All_Condition()
{
	var int amount;
	amount = Npc_HasItems(self, ItFoMuttonRaw);
	
	if amount <= 1 || amount == 5 || amount == 10
	{
		return False;
	};
	
	if !CheckDiaConditions(amount)
	{
		return False;
	};

	Dia_Fry_All.description = GetDiaDescription(amount);
	return True;
};

func void Dia_Fry_All_Information()
{
	var int amount;
	amount = Npc_HasItems(self, ItFoMuttonRaw);	
	Fry(amount);
};

instance Dia_Fry_End(Dia_Fry_Default)
{
	nr = 999;
	condition = Dia_Fry_End_Condition;
	information = Dia_Fry_End_Information;
	description = GetDiaDescription(0);
};

func int Dia_Fry_End_Condition()
{
	return CheckDiaConditions(0);
};

func void Dia_Fry_End_Information()
{
	StopFrying();
};

};