namespace NAMESPACE
{
	namespace Options
	{
		ZOPTION(LogChanges, false);
		ZOPTION(BlendInValue, 0.2f);
		ZOPTION(BlendOutValue, 0.2f);
		ZOPTION(BlendInAnis, VectorOption<std::string>{ "auto" });
		ZOPTION(BlendOutAnis, VectorOption<std::string>{ "auto" });
		ZOPTION(InstantCombatSneakToStand, true);
		ZOPTION(CorrectJumpBBoxes, false);
		ZOPTION(SmoothOverlaySwitch, false);
	}

	namespace Options
	{
		Sub addTrivia(ZSUB(GameEvent::Execute), []
			{
				LogChanges.endTrivia += A"... enables animation change logging";
				BlendInValue.endTrivia += A"... the blend-in override value for BlendInAnis";
				BlendOutValue.endTrivia += A"... the blend-out override value for BlendOutAnis";
				BlendInAnis.endTrivia += A"... animation list for blend-in overwriting";
				BlendOutAnis.endTrivia += A"... animation list for blend-out overwriting";
				InstantCombatSneakToStand.endTrivia += A"... makes anis like t_1hsneakr_2_sneak to be instant";
				CorrectJumpBBoxes.endTrivia += A"... smoother landings for HumansRemaster";
				SmoothOverlaySwitch.endTrivia += A"... more accurate animation transitions when an overlay gets applied/removed";
				SmoothOverlaySwitch.endTrivia += A"incompatible with zModelProtoExtender";
			});

		Sub load(ZSUB(GameEvent::DefineExternals), []()
			{
				ActiveOptionBase::LoadAll();

				const VectorOption<std::string> blendIn =
				{
					".*",
					"s_{walk}",
					"s_{walk}w?{foot}",
					"t_{weapon}{walk}_2_{weapon}{walk}l",
					"t.*_stand_2_.*",
					"s_{weapon}{walk}",
					"s_{weapon}{walk}l",
					"t_jump_2_{weapon}{walk}{foot}",
					"t_jump_2_stand",
					"t_run_2_{weapon}",
					"t_{weapon}run_2_{weapon}",
					"t_magrun_2_.*shoot",
					"t_{weapon}{walk}strafe{foot}",
					"t_{weapon}parade.*",
					"t_{weapon}attack{foot}",
					"s_fistattack",
					"s_[12]hattack",
					"t_{weapon}sfinish",
					"t_{walk}_2_{walk}w{foot}",
					"s_swim[fb]",
					"t_swim_2_swim[fb]",
					"t_[12]hspecialattack"
				};

				const VectorOption<std::string> blendOut =
				{
					".*",
					"s_{walk}",
					"s_{walk}w?{foot}",
					"t_{weapon}{walk}_2_{weapon}{walk}l",
					"t_{weapon}_2_{weapon}run",
					"t_{weapon}_2_run",
					"t_.*_2_stand",
					"t_bowaim_2_bowwalk",
					"t_cbowaim_2_cbowwalk",
					"s_{weapon}{walk}",
					"s_{walk}l",
					"t_{weapon}{walk}turn{foot}",
					"s_fistattack",
					"s_[12]hattack",
					"t_{weapon}{walk}strafe{foot}",
					"s_{weapon}{walk}l",
					"t_{weapon}parade.*",
					"t_{weapon}attack{foot}",
					"t_{weapon}sfinish",
					"t_magrun_2_.*shoot",
					"t_magrun_2_.*cast",
					"t_.*cast_2_.*shoot",
					"t_jump_2_{weapon}{walk}{foot}",
					"t_{weapon}run{foot}_2_jump",
					"t_{walk}{foot}_2_{walk}",
					"t_{walk}_2_{walk}w{foot}",
					"s_swim[fb]",
					"t_swim_2_swim[fb]",
					"t_[12]hspecialattack",
					"t_stand_2_iget",
					"t_{walk}_2_{walk}iget",
					"t_c?bow{walk}_2_c?bowaim"
				};

				const bool remastered = vdf_fexists("_WORK\\DATA\\ANIMS\\_COMPILED\\HUMANS.REMASTER.MDH", VDF_DEFAULT);
				const bool protoExtenderLoaded = CPlugin::FindModule("ZMODELPROTOEXTENDER.DLL");

				if (BlendInAnis->size() == 1u && BlendInAnis->front() == "auto")
					BlendInAnis = blendIn;

				if (BlendOutAnis->size() == 1u && BlendOutAnis->front() == "auto")
				{
					BlendOutAnis = blendOut;

					if (remastered)
					{
						InstantCombatSneakToStand = false;
						CorrectJumpBBoxes = true;
					}

					SmoothOverlaySwitch = !protoExtenderLoaded;
				}
			});
	}
}
