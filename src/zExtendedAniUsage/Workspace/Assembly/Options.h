namespace NAMESPACE
{
	namespace Options
	{
		std::unordered_map<zSTRING, int>& GetAniOffsets()
		{
			static std::unordered_map<zSTRING, int> offsets
			{
				{ "s_dead1", 0x2C8 },
				{ "s_dead2", 0x2CC },
				{ "s_hang", 0x2D0 },
				{ "t_hang_2_stand", 0x2D4 },
				{ "t_stand_2_jumpuplow", 0xA08 },
				{ "s_jumpuplow", 0xA0C },
				{ "t_jumpuplow_2_stand", 0xA10 },
				{ "t_stand_2_jumpupmid", 0xA14 },
				{ "s_jumpupmid", 0xA18 },
				{ "t_jumpupmid_2_stand", 0xA1C },
				{ "t_stumble", 0xFC0 },
				{ "t_stumbleb", 0xFC4 },
				{ "t_fallen_2_stand", 0xFC8 },
				{ "t_fallenb_2_stand", 0xFCC },
				{ "t_walk_2_walkwl", 0xFD0 },
				{ "t_walkwl_2_walk", 0xFD4 },
				{ "s_walkwl", 0xFD8 },
				{ "t_walkwl_2_walkwr", 0xFDC },
				{ "t_walkwr_2_walkwl", 0xFE0 },
				{ "s_walkwr", 0xFE4 },
				{ "t_walkwr_2_walk", 0xFE8 },
				{ "t_walk_2_walkwbl", 0xFEC },
				{ "t_walkwbl_2_walk", 0xFF0 },
				{ "s_walkwbl", 0xFF4 },
				{ "t_walkwbl_2_walkwbr", 0xFF8 },
				{ "t_walkwbr_2_walkwbl", 0xFFC },
				{ "s_walkwbr", 0x1000 },
				{ "t_walkwbr_2_walk", 0x1004 },
				{ "_s_walk", 0x1008 },
				{ "_t_walk_2_walkl", 0x100C },
				{ "_t_walkl_2_walk", 0x1010 },
				{ "_s_walkl", 0x1014 },
				{ "_t_walkl_2_walkr", 0x1018 },
				{ "_t_walkr_2_walkl", 0x101C },
				{ "_s_walkr", 0x1020 },
				{ "_t_walkr_2_walk", 0x1024 },
				{ "_t_turnl", 0x1028 },
				{ "_t_turnr", 0x102C },
				{ "_t_strafel", 0x1030 },
				{ "_t_strafer", 0x1034 },
				{ "_t_walk_2_walkbl", 0x1038 },
				{ "_t_walkbl_2_walk", 0x103C },
				{ "_s_walkbl", 0x1040 },
				{ "_t_walkbl_2_walkbr", 0x1044 },
				{ "_t_walkbr_2_walkbl", 0x1048 },
				{ "_s_walkbr", 0x104C },
				{ "_t_walkbr_2_walk", 0x1050 },
				{ "s_jumpstand", 0x1054 },
				{ "t_stand_2_jumpstand", 0x1058 },
				{ "t_jumpstand_2_stand", 0x105C },
				{ "_t_jumpb", 0x1060 },
				{ "_t_stand_2_jump", 0x1064 },
				{ "_s_jump", 0x1068 },
				{ "t_jump_2_stand", 0x106C },
				{ "_t_stand_2_jumpup", 0x1070 },
				{ "_s_jumpup", 0x1074 },
				{ "_t_jumpup_2_falldn", 0x1078 },
				{ "_t_jump_2_falldn", 0x107C },
				{ "t_walkwl_2_swimf", 0x1080 },
				{ "s_swimf", 0x1084 },
				{ "t_swimf_2_walkwl", 0x1088 },
				{ "t_walkwbl_2_swimb", 0x108C },
				{ "s_swimb", 0x1090 },
				{ "t_swimb_2_walkwbl", 0x1094 },
				{ "t_swimf_2_swim", 0x1098 },
				{ "s_swim", 0x109C },
				{ "t_swim_2_swimf", 0x10A0 },
				{ "t_swim_2_swimb", 0x10A4 },
				{ "t_swimb_2_swim", 0x10A8 },
				{ "t_warn", 0x10AC },
				{ "t_swim_2_dive", 0x10B0 },
				{ "s_dive", 0x10B4 },
				{ "t_divef_2_swim", 0x10B8 },
				{ "t_dive_2_divef", 0x10BC },
				{ "s_divef", 0x10C0 },
				{ "t_divef_2_dive", 0x10C4 },
				{ "t_dive_2_drowned", 0x10C8 },
				{ "s_drowned", 0x10CC },
				{ "t_swimturnl", 0x10D0 },
				{ "t_swimturnr", 0x10D4 },
				{ "t_diveturnl", 0x10D8 },
				{ "t_diveturnr", 0x10DC },
				{ "_t_walkl_2_aim", 0x10E0 },
				{ "_t_walkr_2_aim", 0x10E4 },
				{ "_t_walk_2_aim", 0x10E8 },
				{ "_s_aim", 0x10EC },
				{ "_t_aim_2_walk", 0x10F0 },
				{ "_t_hitl", 0x10F4 },
				{ "_t_hitr", 0x10F8 },
				{ "_t_hitback", 0x10FC },
				{ "_t_hitf", 0x1100 },
				{ "_t_hitfstep", 0x1104 },
				{ "_s_hitf", 0x1108 },
				{ "_t_aim_2_defend", 0x110C },
				{ "_s_defend", 0x1110 },
				{ "_t_defend_2_aim", 0x1114 },
				{ "_t_paradeL", 0x1118 },
				{ "_t_paradeM", 0x111C },
				{ "_t_paradeS", 0x1120 },
				{ "_t_hitfrun", 0x1124 },
				{ "t_stand_2_iaim", 0x1128 },
				{ "s_iaim", 0x112C },
				{ "t_iaim_2_stand", 0x1130 },
				{ "t_iaim_2_idrop", 0x1134 },
				{ "s_idrop", 0x1138 },
				{ "t_idrop_2_stand", 0x113C },
				{ "t_iaim_2_ithrow", 0x1140 },
				{ "s_ithrow", 0x1144 },
				{ "t_ithrow_2_stand", 0x1148 },
				{ "t_stand_2_iget", 0x114C },
				{ "s_iget", 0x1150 },
				{ "t_iget_2_stand", 0x1154 },
				{ "s_oget", 0x1158 },
				{ "_t_stand_2_torch", 0x115C },
				{ "_s_torch", 0x1160 },
				{ "_t_torch_2_stand", 0x1164 },
				{ "hitani", 0x1168 },
				{ "help", 0x116C },
				{ "help1", 0x1170 },
				{ "help2", 0x1174 },
				{ "s_fall", 0x1178 },
				{ "s_fallb", 0x117C },
				{ "s_fallen", 0x1180 },
				{ "s_fallenb", 0x1184 },
				{ "s_falldn", 0x1188 },
				{ "_t_runl_2_jump", 0x118C },
				{ "_t_runr_2_jump", 0x1190 },
				{ "_t_jump_2_runl", 0x1194 },
				{ "s_look", 0x1198 },
				{ "s_point", 0x119C },
				{ "dummy1", 0x11A0 },
				{ "dummy2", 0x11A4 },
				{ "dummy3", 0x11A8 },
				{ "dummy4", 0x11AC },
				{ "togglewalk", 0x11D8 },
				{ "t_stand_2_cast", 0x11DC },
				{ "s_cast", 0x11E0 },
				{ "t_cast_2_shoot", 0x11E4 },
				{ "t_cast_2_stand", 0x11E8 },
				{ "s_shoot", 0x11EC },
				{ "t_shoot_2_stand", 0x11F0 }
			};

			return offsets;
		}

		struct AniNamePattern
		{
			enum class TokenType
			{
				String,
				Weapon,
				Walk
			};

			std::vector<TokenType> tokenTypes;
			std::vector<zSTRING> strings;

			AniNamePattern(const zSTRING& pattern)
			{
				ASSERT(!pattern.IsEmpty());

				size_t tokenStart = 0;
				size_t index = 1;

				while (static_cast<int>(index) <= pattern.Length())
				{
					const char& ch = pattern[index];

					if (ch == '\0')
					{
						ASSERT(pattern[tokenStart] != '{');
						tokenTypes += TokenType::String;
						strings += pattern.Copy(tokenStart, index - tokenStart);
						break;
					}

					if (ch == '{')
					{
						ASSERT(pattern[tokenStart] != '{');
						tokenTypes += TokenType::String;
						strings += pattern.Copy(tokenStart, index - tokenStart);
						tokenStart = index;
						index += 1;
						continue;
					}

					if (ch == '}')
					{
						ASSERT(pattern[tokenStart] == '{');
						const zSTRING word = pattern.Copy(tokenStart + 1, index - tokenStart - 1);

						if (word == "WEAPON")
							tokenTypes += TokenType::Weapon;
						else if (word == "WALK")
							tokenTypes += TokenType::Walk;
						else
							ASSERT(false);

						tokenStart = index + 1;
						index = tokenStart + 1;
						continue;
					}

					index += 1;
				}
			}

			zSTRING GetName(const zSTRING& weapon, const zSTRING& walk) const
			{
				zSTRING name;
				size_t stringIndex = 0;

				for (size_t i = 0; i < tokenTypes.size(); i++)
					switch (tokenTypes[i])
					{
					case TokenType::String:
						name += strings[stringIndex++];
						break;

					case TokenType::Weapon:
						name += weapon;
						break;

					case TokenType::Walk:
						name += walk;
						break;

					default:
						ASSERT(false);
					}

				return name;
			}
		};

		std::vector<std::pair<int, std::vector<AniNamePattern>>> aniPatterns;

		ZOPTION(t_stand_2_iget, A"t_{walk}_2_{walk}iget t_stand_2_iget");
		ZOPTION(s_iget, A"s_{walk}iget s_iget");
		ZOPTION(t_iget_2_stand, A"t_{walk}iget_2_{walk} t_iget_2_stand");
		ZOPTION(_s_jump, A"s_{weapon}jump s_jump");
	}

	namespace Options
	{
		Sub load(ZSUB(GameEvent::DefineExternals), []()
			{
				ActiveOptionBase::LoadAll();

				static const zSTRING sectionName = zSTRING{ PROJECT_NAME }.Upper();
				static zCOptionSection* const section = zoptions->GetSectionByName(sectionName, true);

				for (zCOptionEntry* entry : section->entryList)
					if (entry && !entry->varName.IsEmpty())
					{
						auto it = GetAniOffsets().find(entry->varName);

						if (it == GetAniOffsets().end())
						{
							Message::Info(string{ "Unknown field: '" } + string{ entry->varName } + string{ "'" }, PROJECT_NAME);
							continue;
						}

						aniPatterns.emplace_back(it->second, std::vector<AniNamePattern>{});

						zSTRING value = entry->varValue;
						value.Upper();

						for (const string& pattern : string{ value }.Split(" "))
							aniPatterns.back().second.emplace_back(pattern);
					}
			});
	}
}
