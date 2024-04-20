#include <deque>

namespace NAMESPACE
{
	template <typename T>
	struct TimeLimitedVector
	{
		std::deque<T> elements;
		std::deque<int> frameTimes;
		int timeLeft;

		void Add(const T& element)
		{
			const int frameTime = static_cast<int>(ztimer->frameTime);
			elements.push_back(element);
			frameTimes.push_back(frameTime);
			timeLeft -= frameTime;

			while (!elements.empty())
			{
				if (timeLeft + frameTimes.front() >= 0)
					break;

				timeLeft += frameTimes.front();
				elements.pop_front();
				frameTimes.pop_front();
			}
		}
	};

	struct SlidingState
	{
		float y;
		bool isSliding;
	};

	class NpcSlidingTracker
	{
	public:
		NpcSlidingTracker(oCNpc* npc) :
			npc{ npc }
		{
			npc->AddRef();
			Reset();

			ADDSUB(LoadBegin);
			ADDSUB(Exit);
			ADDSUB(Loop);
		}

		NpcSlidingTracker(const NpcSlidingTracker&) = delete;
		NpcSlidingTracker& operator=(const NpcSlidingTracker&) = delete;

	private:
		void Reset()
		{
			states = {};
			states.timeLeft = Options::TimeMs;
		}

		void OnLoadBegin()
		{
			delete this;
		}

		void OnExit()
		{
			delete this;
		}

		static bool IsSliding(zCModel* model)
		{
			for (const zSTRING& aniName : { "S_SLIDE", "S_SLIDEB" })
				for (int i = 0; i < model->numActiveAnis; i++)
					if (zCModelAniActive* ani = model->aniChannels[i])
						if (!ani->isFadingOut && ani->protoAni && ani->protoAni->aniName == aniName)
							return true;

			return false;
		}

		void OnLoop()
		{
			if (!ogame->world || !npc->human_ai || !npc->visual || npc->GetHomeWorld() != ogame->world)
				return Reset();

			zCModel* model = npc->visual->CastTo<zCModel>();
			
			if (!model)
				return Reset();

			SlidingState state{};
			state.y = npc->trafoObjToWorld.GetTranslation()[VY];
			state.isSliding = IsSliding(model);

			if (!state.isSliding)
				return Reset();

			states.Add(state);

			if (states.timeLeft >= 0)
				return;

			float minY = states.elements.front().y;
			float maxY = minY;

			for (const auto& state : states.elements)
			{
				minY = std::min(minY, state.y);
				maxY = std::max(maxY, state.y);
			}

			if (maxY - minY > Options::DeltaY)
				return;

			Unstuck();
			Reset();
		}

		static void GetPositions(zCPolygon* poly, zVEC3 (&positions)[3])
		{
			for (int i = 0; i < 3; i++)
			{
				positions[i] = poly->vertex[i]->position;
				positions[i][VY] = 0.0f;
			}
		}

		static float GetSquare(zCPolygon* poly)
		{
			zVEC3 positions[3];
			GetPositions(poly, positions);
			return (positions[1] - positions[0]).Cross(positions[2] - positions[0]).Length() / 2.0f;
		}

		static float GetMaxHeight(zCPolygon* poly)
		{
			zVEC3 positions[3];
			GetPositions(poly, positions);
			const float square = GetSquare(poly);
			float maxHeight = 0.0f;

			for (int i = 0; i < 3; i++)
			{
				const float height = 2.0f * square / (positions[(i + 1) % 3] - positions[i]).Length();
				maxHeight = std::max(maxHeight, height);
			}

			return maxHeight;
		}

		void Unstuck()
		{
			if (!ogame->world->bspTree.bspRoot)
				return;

			float minPolyHeight = std::min(npc->bbox3D.maxs[VX] - npc->bbox3D.mins[VX], npc->bbox3D.maxs[VZ] - npc->bbox3D.mins[VZ]);
			minPolyHeight /= 2.0f;

			const zVEC3 npcPos = npc->trafoObjToWorld.GetTranslation();
			const float npcFloorY = npc->human_ai->GetModelFloorWorld();
			const zVEC3 npcFloorPos{ npcPos[VX], npcFloorY, npcPos[VZ] };

			zTBBox3D searchBox;
			searchBox.mins = npcPos - zVEC3{ 500.0f, npcPos[VY] - npcFloorY, 500.0f };
			searchBox.maxs = npcPos + zVEC3{ 500.0f, 2.0f * (npcPos[VY] - npcFloorY), 500.0f };

			zCPolygon** polygons;
			int numPolygons;
			ogame->world->bspTree.bspRoot->CollectPolysInBBox3D(searchBox, polygons, numPolygons);

			zCPolygon* bestPoly{};

			for (int i = 0; i < numPolygons; i++)
				if (zCPolygon* poly = polygons[i])
				{
					if (!poly->material || poly->material->matGroup == zMAT_GROUP_UNDEF || poly->material->matGroup == zMAT_GROUP_WATER)
						continue;

					if (!npc->human_ai->CanWalkOnPoly(poly->polyPlane.normal))
						continue;

					if (!bestPoly)
					{
						bestPoly = poly;
						continue;
					}

					if (poly->GetCenter()[VY] < npcFloorY)
						continue;

					if (GetMaxHeight(poly) < minPolyHeight)
						continue;

					if (poly->GetCenter().Distance(npcFloorPos) > bestPoly->GetCenter().Distance(npcFloorPos))
						continue;

					bestPoly = poly;
				}

			if (!bestPoly)
				return;

			zVEC3 newPos = bestPoly->GetCenter();
			newPos[VY] += npcPos[VY] - npcFloorY;
			npc->trafoObjToWorld.SetTranslation(newPos);
			npc->human_ai->SearchStandAni(true);
		}

		ZOwner<oCNpc> npc;
		TimeLimitedVector<SlidingState> states;
		std::vector<Sub<void>> subs;
	};

	Sub createTracker(ZSUB(GameEvent::LoadEnd), []
		{
			if (player)
				new NpcSlidingTracker{ player };
		});
}
