#include <iomanip>

namespace NAMESPACE
{
	typedef std::chrono::high_resolution_clock::time_point TimePoint;
	typedef std::chrono::high_resolution_clock::duration Duration;

	class Timer
	{
	private:
		bool run;
		TimePoint last;
		Duration total;

	public:
		Timer(bool run = false) :
			run{ run },
			total{}
		{
			if (run)
				last = std::chrono::high_resolution_clock::now();
		}

		Timer(const Timer&) = default;
		Timer(Timer&&) = default;
		Timer& operator=(const Timer&) = default;
		Timer& operator=(Timer&&) = default;

		Duration GetTotal() const
		{
			Duration duration = total;

			if (run)
				duration += std::chrono::high_resolution_clock::now() - last;

			return duration;
		}

		void SetRun(bool run = true)
		{
			if (this->run == run)
				return;

			if (run)
				last = std::chrono::high_resolution_clock::now();
			else
				total = GetTotal();

			this->run = run;
		}

		void Reset(bool run = false)
		{
			this->run = run;
			total = {};
			
			if (run)
				last = std::chrono::high_resolution_clock::now();
		}
	};

	struct FunctionInfo
	{
		std::string name;
		Timer total;
		Timer exclusive;
		size_t recursion;

		void Reset()
		{
			total.Reset();
			exclusive.Reset();
			recursion = 0u;
		}
	};

	std::vector<std::unique_ptr<FunctionInfo>> functions;
	FunctionInfo* lineToFunction[10000u];
	std::vector<FunctionInfo*> callStack;

	struct PerformanceScope
	{
		PerformanceScope(FunctionInfo& info)
		{
			if (!callStack.empty())
				callStack.back()->exclusive.SetRun(false);
			
			info.total.SetRun(true);
			info.exclusive.SetRun(true);
			info.recursion += 1;
			callStack += &info;
		}

		~PerformanceScope()
		{
			auto& info = *callStack.back();
			callStack.pop_back();

			info.recursion -= 1;

			if (!info.recursion)
			{
				info.total.SetRun(false);
				info.exclusive.SetRun(false);
			}
			else
				if (callStack.empty() || callStack.back() != &info)
					info.exclusive.SetRun(false);
		}
	};

	FunctionInfo& CreateFunction(const char* name, size_t line)
	{
		if (lineToFunction[line])
			return *lineToFunction[line];

		functions += std::make_unique<FunctionInfo>();
		functions.back()->Reset();
		functions.back()->name = name;

		size_t index = functions.back()->name.rfind("Hook_");
		std::string functionName = functions.back()->name;

		LOG(functionName);
		LOG(index);

		if (index != std::string::npos)
			functions.back()->name.erase(0, index + 5);

		lineToFunction[line] = functions.back().get();
		return *functions.back();
	}

	double GetSeconds(Duration duration)
	{
		return std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count() / 1'000'000'000.0;
	}

#define SCOPE PerformanceScope scope{ CreateFunction(__FUNCTION__, __LINE__) }

	Sub addLoopFunction(ZSUB(GameEvent::Execute), []
		{
			functions += std::make_unique<FunctionInfo>();
			functions.back()->name = "LoadTotal";
		});

	bool firstFrame = false;

	Sub setFirstFrame(ZSUB(GameEvent::LoadBegin), []
		{
			// firstFrame = true;

			for (auto& function : functions)
				function->Reset();

			functions.front()->total.SetRun(true);
			functions.front()->exclusive.SetRun(true);
		});

	Sub processLogs(ZSUB(GameEvent::LoadEnd), []
		{
			if (!firstFrame)
			{
				static std::ofstream out("performance.log");

				out << std::endl;

				for (auto& function : functions)
					out << std::setw(std::max(16u, function->name.size() + 4)) << function->name;

				out << std::endl;

				for (auto& function : functions)
					out << std::setw(std::max(16u, function->name.size() + 4)) << std::fixed << std::setprecision(3) << GetSeconds(function->exclusive.GetTotal()) * 1000;

				out << std::endl;

				for (auto& function : functions)
					out << std::setw(std::max(16u, function->name.size() + 4)) << std::fixed << std::setprecision(3) << GetSeconds(function->total.GetTotal()) * 1000;

				out << std::endl;
			}

			firstFrame = false;
		});

	//void __fastcall Hook_oCGame_LoadSavegame(oCGame*, void*, int, int);
	//Hook<void(__thiscall*)(oCGame*, int, int)> Ivk_oCGame_LoadSavegame(ZENFOR(0x0063C2A0, 0x00662D60, 0x00669BA0, 0x006C67D0), &Hook_oCGame_LoadSavegame, HookMode::Patch);
	//void __fastcall Hook_oCGame_LoadSavegame(oCGame* _this, void* vtable, int a0, int a1)
	//{
	//	SCOPE;
	//	Ivk_oCGame_LoadSavegame(_this, a0, a1);
	//}

	void __fastcall Hook_oCGame_LoadWorldStat(oCGame*, void*, zSTRING);
	Hook<void(__thiscall*)(oCGame*, zSTRING)> Ivk_oCGame_LoadWorldStat(ZENFOR(0x0063F590, 0x006664E0, 0x0066D290, 0x006CA010), &Hook_oCGame_LoadWorldStat, HookMode::Patch);
	void __fastcall Hook_oCGame_LoadWorldStat(oCGame* _this, void* vtable, zSTRING a0)
	{
		SCOPE;
		Ivk_oCGame_LoadWorldStat(_this, a0);
	}

	void __fastcall Hook_oCGame_LoadWorldDyn(oCGame*, void*, zSTRING const&);
	Hook<void(__thiscall*)(oCGame*, zSTRING const&)> Ivk_oCGame_LoadWorldDyn(ZENFOR(0x0063F800, 0x00666790, 0x0066D500, 0x006CA280), &Hook_oCGame_LoadWorldDyn, HookMode::Patch);
	void __fastcall Hook_oCGame_LoadWorldDyn(oCGame* _this, void* vtable, zSTRING const& a0)
	{
		SCOPE;
		Ivk_oCGame_LoadWorldDyn(_this, a0);
	}

	int __fastcall Hook_oCWorld_LoadWorld(oCWorld*, void*, zSTRING const&, zCWorld::zTWorldLoadMode);
	Hook<int(__thiscall*)(oCWorld*, zSTRING const&, zCWorld::zTWorldLoadMode)> Ivk_oCWorld_LoadWorld(ZENFOR(0x006D69B0, 0x0070E400, 0x00720100, 0x0077FB40), &Hook_oCWorld_LoadWorld, HookMode::Patch);
	int __fastcall Hook_oCWorld_LoadWorld(oCWorld* _this, void* vtable, zSTRING const& a0, zCWorld::zTWorldLoadMode a1)
	{
		SCOPE;
		int result = Ivk_oCWorld_LoadWorld(_this, a0, a1);
		return result;
	}

	int __fastcall Hook_zCWorld_LoadWorld(zCWorld*, void*, zSTRING const&, zCWorld::zTWorldLoadMode);
	Hook<int(__thiscall*)(zCWorld*, zSTRING const&, zCWorld::zTWorldLoadMode)> Ivk_zCWorld_LoadWorld(ZENFOR(0x005F8A00, 0x00619EC0, 0x0061F940, 0x006270D0), &Hook_zCWorld_LoadWorld, HookMode::Patch);
	int __fastcall Hook_zCWorld_LoadWorld(zCWorld* _this, void* vtable, zSTRING const& a0, zCWorld::zTWorldLoadMode a1)
	{
		SCOPE;
		int result = Ivk_zCWorld_LoadWorld(_this, a0, a1);
		return result;
	}

	oCNpc* __fastcall Hook_oCNpc_oCNpc(oCNpc*, void*);
	Hook<oCNpc* (__thiscall*)(oCNpc*)> Ivk_oCNpc_oCNpc(ZENFOR(0x0068B3D0, 0x006BB9F0, 0x006CF750, 0x0072D950), &Hook_oCNpc_oCNpc, HookMode::Patch);
	oCNpc* __fastcall Hook_oCNpc_oCNpc(oCNpc* _this, void* vtable)
	{
		SCOPE;
		oCNpc* result = Ivk_oCNpc_oCNpc(_this);
		return result;
	}

	void __fastcall Hook_oCNpc_InitByScript(oCNpc*, void*, int, int);
	Hook<void(__thiscall*)(oCNpc*, int, int)> Ivk_oCNpc_InitByScript(ZENFOR(0x0068C840, 0x006BCFB0, 0x006D0C10, 0x0072EE70), &Hook_oCNpc_InitByScript, HookMode::Patch);
	void __fastcall Hook_oCNpc_InitByScript(oCNpc* _this, void* vtable, int a0, int a1)
	{
		SCOPE;
		Ivk_oCNpc_InitByScript(_this, a0, a1);
	}

	oCItem* __fastcall Hook_oCItem_oCItem(oCItem*, void*);
	Hook<oCItem* (__thiscall*)(oCItem*)> Ivk_oCItem_oCItem(ZENFOR(0x00670DE0, 0x0069E6B0, 0x006B35F0, 0x00711290), &Hook_oCItem_oCItem, HookMode::Patch);
	oCItem* __fastcall Hook_oCItem_oCItem(oCItem* _this, void* vtable)
	{
		SCOPE;
		oCItem* result = Ivk_oCItem_oCItem(_this);
		return result;
	}

	void __fastcall Hook_oCItem_InitByScript(oCItem*, void*, int, int);
	Hook<void(__thiscall*)(oCItem*, int, int)> Ivk_oCItem_InitByScript(ZENFOR(0x00671660, 0x0069EF80, 0x006B3EA0, 0x00711BD0), &Hook_oCItem_InitByScript, HookMode::Patch);
	void __fastcall Hook_oCItem_InitByScript(oCItem* _this, void* vtable, int a0, int a1)
	{
		SCOPE;
		Ivk_oCItem_InitByScript(_this, a0, a1);
	}

	void __fastcall Hook_oCNpc_Unarchive(oCNpc*, void*, zCArchiver&);
	Hook<void(__thiscall*)(oCNpc*, zCArchiver&)> Ivk_oCNpc_Unarchive(ZENFOR(0x006A31E0, 0x006D5AD0, 0x006E8790, 0x00747230), &Hook_oCNpc_Unarchive, HookMode::Patch);
	void __fastcall Hook_oCNpc_Unarchive(oCNpc* _this, void* vtable, zCArchiver& a0)
	{
		SCOPE;
		Ivk_oCNpc_Unarchive(_this, a0);
	}

	void __fastcall Hook_oCItem_Unarchive(oCItem*, void*, zCArchiver&);
	Hook<void(__thiscall*)(oCItem*, zCArchiver&)> Ivk_oCItem_Unarchive(ZENFOR(0x00673370, 0x006A0E30, 0x006B5E20, 0x00713EB0), &Hook_oCItem_Unarchive, HookMode::Patch);
	void __fastcall Hook_oCItem_Unarchive(oCItem* _this, void* vtable, zCArchiver& a0)
	{
		SCOPE;
		Ivk_oCItem_Unarchive(_this, a0);
	}

	void __fastcall Hook_zCVob_Unarchive(zCVob*, void*, zCArchiver&);
	Hook<void(__thiscall*)(zCVob*, zCArchiver&)> Ivk_zCVob_Unarchive(ZENFOR(0x005D4820, 0x005F3BE0, 0x005F8E80, 0x005FFC70), &Hook_zCVob_Unarchive, HookMode::Patch);
	void __fastcall Hook_zCVob_Unarchive(zCVob* _this, void* vtable, zCArchiver& a0)
	{
		SCOPE;
		Ivk_zCVob_Unarchive(_this, a0);
	}
}
