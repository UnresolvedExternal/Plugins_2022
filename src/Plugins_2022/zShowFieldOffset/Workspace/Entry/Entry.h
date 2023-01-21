namespace NAMESPACE
{
#define STR(arg) #arg

#define FIELDS(className, fieldName) \
	const std::vector<size_t> offsets{ offsetof(Gothic_I_Classic::className, fieldName),  offsetof(Gothic_I_Addon::className, fieldName), offsetof(Gothic_II_Classic::className, fieldName), offsetof(Gothic_II_Addon::className, fieldName) }; \
	const std::string name = STR(field_##className##_##fieldName);
	
	Sub showOffset(ZSUB(GameEvent::Execute), []
		{
			FIELDS(zCVob, nextOnTimer);

			std::vector<string> strings;
			strings.resize(4);
			int minZeroes = 8;

			for (int i = 0; i < 4; i++)
			{
				strings[i] = AHEX32(offsets[i]);
				
				for (int zeroes = 0; zeroes < minZeroes; zeroes++)
					if (strings[i][zeroes + 2] != '0')
					{
						minZeroes = zeroes;
						break;
					}
			}

			for (int i = 0; i < 4; i++)
				strings[i].Cut(2, minZeroes);

			std::ostringstream out;
			out << "INT " << name << " = ";

			if (*std::min_element(offsets.begin(), offsets.end()) == *std::max_element(offsets.begin(), offsets.end()))
				out << strings.front();
			else
			{
				out << "ZenDef(";

				for (int i = 0; i < 4; i++)
				{
					if (i)
						out << ", ";

					out << strings[i];
				}

				out << ")";
			}
			
			std::string str = out.str();
			cmd << str << endl; 

			OpenClipboard(NULL);
			EmptyClipboard();
			HGLOBAL global = GlobalAlloc(GMEM_MOVEABLE, str.length() + 1);
			void* memory = GlobalLock(global);
			memcpy(memory, str.c_str(), str.length() + 1);
			GlobalUnlock(global);
			SetClipboardData(CF_TEXT, global);
			CloseClipboard();
			exit(0);
		});
}
