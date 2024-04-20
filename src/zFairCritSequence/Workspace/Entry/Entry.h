#include <random>
#include <array>

namespace NAMESPACE
{
	class CritGenerator
	{
	private:
		float karma;

		static float GetChance(float talent, float karma)
		{
			if (talent <= 0.0f)
				return 0.0f;

			if (talent >= 1.0f)
				return 1.0f;

			karma /= std::max(1.0f - 4.0f * fabs(0.5f - talent), 0.01f);
			return (karma < 0.0f) ? talent + talent * karma : talent + (1.0f - talent) * karma;
		}

		bool Roll(float chance)
		{
			static std::random_device device;
			static std::default_random_engine engine{ device() };
			static std::uniform_real_distribution distribution{ 0.0f, 1.0f };
			return distribution(engine) < chance;
		}

	public:
		CritGenerator(float karma = 0.0f) :
			karma{ karma }
		{

		}

		CritGenerator(const CritGenerator&) = default;
		CritGenerator& operator=(const CritGenerator&) = default;

		float& Karma() 
		{
			return karma;
		}

		const float& Karma() const
		{
			return karma;
		}

		bool operator()(float talent)
		{
			const bool crit = Roll(GetChance(talent, karma));
			karma += std::clamp(talent, 0.0f, 1.0f);
			karma = crit ? karma - 1.0f : karma;
			return crit;
		}
	};

	CritGenerator playerGenerator;
	std::unordered_map<oCNpc*, CritGenerator> generators;

	Sub resetGenerators(ZSUB(GameEvent::LoadBegin), []
		{
			if (!SaveLoadGameInfo.changeLevel)
				playerGenerator.Karma() = 0.0f;

			generators.clear();
		});

	int __stdcall RerollHit(oCNpc::oSDamageDescriptor& desc, int random /*1..100*/, int hitchance)
	{
		if (!desc.pNpcAttacker)
			return random;

		CritGenerator& generator = (desc.pNpcAttacker == player) ? playerGenerator : generators[desc.pNpcAttacker];
		return generator(static_cast<float>(hitchance) / 100.0f) ? hitchance : hitchance + 1;
	}

	Sub executePatch(ZSUB(GameEvent::Entry), []
		{
			CPatchInteger rerollHit;
			rerollHit.Init();
			rerollHit.SetObjectName("RerollHit");
			rerollHit.SetValue(reinterpret_cast<int>(&RerollHit));
			rerollHit.DontRemove();

			CPatch::ExecuteResource(CPlugin::GetCurrentPlugin()->GetModule(), MAKEINTRESOURCE(IDR_PATCH1), "PATCH");
		});

	//Sub test(ZSUB(GameEvent::Execute), []
	//	{
	//		constexpr int maxSequence = 15;
	//		constexpr int testSize = 10'000'000;

	//		cmd << "talent";

	//		for (int sequence = 2; sequence <= maxSequence; sequence++)
	//			cmd << "\t" << sequence;

	//		cmd << endl;

	//		for (int talent = 10; talent <= 90; talent += 10)
	//		{
	//			cmd << talent << "%";
	//			const float floatTalent = static_cast<float>(talent) / 100.0f;

	//			CritGenerator generator;
	//			std::array<int, maxSequence - 1> strikes{};
	//			int critIndex = -1;

	//			for (int i = 0; i < testSize; i++)
	//			{
	//				if (generator(floatTalent))
	//				{
	//					critIndex = i;
	//					continue;
	//				}

	//				for (int sequence = 2; sequence <= maxSequence && sequence <= (i - critIndex); sequence++)
	//					strikes[sequence - 2] += 1;
	//			}

	//			for (int sequence = 2; sequence <= maxSequence; sequence++)
	//			{
	//				float chance = static_cast<float>(strikes[sequence - 2]) / (testSize - sequence + 1);
	//				chance *= 100.0f;
	//				cmd << "\t" << chance << "%";
	//			}

	//			cmd << endl;
	//		}
	//	});
}
