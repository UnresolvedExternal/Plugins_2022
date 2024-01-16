namespace NAMESPACE
{
	struct Formula
	{
		std::vector<string> tokens;

		Formula() {}

		Formula(const char* input)
		{
			std::istringstream in{ input };
			in >> *this;
		}

		float Calculate(const std::unordered_map<string, std::function<void(std::vector<float>&)>>& operations) const
		{
			std::vector<float> stack{};

			for (const string& token : tokens)
				if (auto it = operations.find(token); it != operations.end())
					it->second(stack);
				else
					stack += token.ToReal32();

			ASSERT(stack.size() == 1);
			return stack[0];
		}

		friend std::istream& operator>>(std::istream& in, Formula& formula)
		{
			formula.tokens.clear();

			while (true)
			{
				std::string token;
				in >> token;

				if (!in)
					return in;

				if (!token.empty())
					formula.tokens += string{ token.c_str() };
			}
		}

		friend std::ostream& operator<<(std::ostream& out, const Formula& formula)
		{
			for (size_t i = 0; i < formula.tokens.size(); i++)
			{
				if (i)
					out << " ";

				out << formula.tokens[i];
			}

			return out;
		}

		bool operator==(const Formula& other) const
		{
			if (tokens.size() != other.tokens.size())
				return false;

			for (size_t i = 0; i < tokens.size(); i++)
				if (tokens[i] != other.tokens[i])
					return false;

			return true;
		}
	};

	namespace Options
	{
		// talent + (dex - dex2) / 2
		ZOPTION(CritChance, Formula{ "talent dex dex2 - 2 / +" });

		// weapon * (0.5 + str / 200 + iscrit(talent / 100))
		ZOPTION(MeleeRawDamage, Formula{ "weapon 0.5 str 200 / + talent 100 / iscrit + *" });

		// weapon * (1 + isbow(str / 200))
		ZOPTION(RangeRawDamage, Formula{ "weapon 1 str 200 / isbow + *" });
		
		// raw - protection / (1 + iscbow(1))
		ZOPTION(Protection, Formula{ "raw protection 1 1 iscbow + / - " });
	}

	namespace Options
	{
		Sub load(ZSUB(GameEvent::DefineExternals), []()
			{
				ActiveOptionBase::LoadAll();
			});
	}
}
