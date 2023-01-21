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
				info.total.SetRun(false);
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

		LOG(functions.back()->name.c_str());
		LOG(index);

		if (index != std::string::npos)
			functions.back()->name.erase(0, index + 5);

		lineToFunction[line] = functions.back().get();
	}

	double GetSeconds(Duration duration)
	{
		return std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count() / 1'000'000'000.0;
	}

#define SCOPE PerformanceScope scope{ CreateFunction(__FUNCTION__, __LINE__) }

	Sub addLoopFunction(ZSUB(GameEvent::Execute), []
		{
			functions += std::make_unique<FunctionInfo>();
			functions.back()->name = "Frame";
		});

	bool firstFrame;

	Sub setFirstFrame(ZSUB(GameEvent::LoadEnd), []
		{
			firstFrame = true;
		});

	Sub processLogs(ZSUB(GameEvent::PreLoop), []
		{
			if (!firstFrame)
			{
				static std::ofstream out("performance.log");

				out << std::endl;

				for (auto& function : functions)
					out << std::setw(std::max(16u, function->name.size() + 4)) << function->name;

				out << std::endl;

				for (auto& function : functions)
					out << std::setw(std::max(16u, function->name.size() + 4)) << std::fixed << std::setprecision(3) << GetSeconds(function->total.GetTotal()) * 1000;

				out << std::endl;
			}

			firstFrame = false;

			for (auto& function : functions)
				function->Reset();

			functions.front()->total.SetRun(true);
			functions.front()->exclusive.SetRun(true);
		});

	void __fastcall Hook_zCBspTree_Render(zCBspTree*, void*);
	Hook<void(__thiscall*)(zCBspTree*)> Ivk_zCBspTree_Render(ZENFOR(0x0051D840, 0x005335A0, 0x0052D130, 0x00530080), &Hook_zCBspTree_Render, HookMode::Patch);
	void __fastcall Hook_zCBspTree_Render(zCBspTree* _this, void* vtable)
	{
		SCOPE;
		Ivk_zCBspTree_Render(_this);
	}

	int __fastcall Hook_zCPolygon_RenderPoly(zCPolygon*, void*, int);
	Hook<int(__thiscall*)(zCPolygon*, int)> Ivk_zCPolygon_RenderPoly(ZENFOR(0x00518F30, 0x0052EB30, 0x00526FE0, 0x00529DD0), &Hook_zCPolygon_RenderPoly, HookMode::Patch);
	int __fastcall Hook_zCPolygon_RenderPoly(zCPolygon* _this, void* vtable, int a0)
	{
		SCOPE;
		int result = Ivk_zCPolygon_RenderPoly(_this, a0);
		return result;
	}

	// WARNING: supported versions are G2, G2A
	void __fastcall Hook_zCPolygon_ApplyMorphing(zCPolygon*, void*);
	Hook<void(__thiscall*)(zCPolygon*)> Ivk_zCPolygon_ApplyMorphing(ZENFOR(0x00000000, 0x00000000, 0x00527620, 0x0052A420), &Hook_zCPolygon_ApplyMorphing, HookMode::Patch);
	void __fastcall Hook_zCPolygon_ApplyMorphing(zCPolygon* _this, void* vtable)
	{
		SCOPE;
		Ivk_zCPolygon_ApplyMorphing(_this);
	}

	// WARNING: supported versions are G2, G2A
	void __fastcall Hook_zCPolygon_LightClippedPoly(zCPolygon*, int);
	Hook<void(__fastcall*)(zCPolygon*, int)> Ivk_zCPolygon_LightClippedPoly(ZENFOR(0x00000000, 0x00000000, 0x00527F40, 0x0052AD40), &Hook_zCPolygon_LightClippedPoly, HookMode::Patch);
	void __fastcall Hook_zCPolygon_LightClippedPoly(zCPolygon* _this, int a0)
	{
		SCOPE;
		Ivk_zCPolygon_LightClippedPoly(_this, a0);
	}

	zCVertexTransform* __fastcall Hook_zCVertex_CreateVertexTransform(zCVertex*, void*);
	Hook<zCVertexTransform* (__thiscall*)(zCVertex*)> Ivk_zCVertex_CreateVertexTransform(ZENFOR(0x004FC1C0, 0x0050F290, 0x00508E50, 0x0050BC00), &Hook_zCVertex_CreateVertexTransform, HookMode::Patch);
	zCVertexTransform* __fastcall Hook_zCVertex_CreateVertexTransform(zCVertex* _this, void* vtable)
	{
		SCOPE;
		zCVertexTransform* result = Ivk_zCVertex_CreateVertexTransform(_this);
		return result;
	}

	// WARNING: supported versions are G2, G2A
	void __fastcall Hook_zCCamera_Project(zCCamera*, void*, zVEC3 const* const, float, float&, float&);
	Hook<void(__thiscall*)(zCCamera*, zVEC3 const* const, float, float&, float&)> Ivk_zCCamera_Project(ZENFOR(0x00000000, 0x00000000, 0x005FE050, 0x00604ED0), &Hook_zCCamera_Project, HookMode::Patch);
	void __fastcall Hook_zCCamera_Project(zCCamera* _this, void* vtable, zVEC3 const* const a0, float a1, float& a2, float& a3)
	{
		Ivk_zCCamera_Project(_this, a0, a1, a2, a3);
	}
}
