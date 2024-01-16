namespace NAMESPACE
{
	const char* const ZS_HEAD = "BIP01 HEAD";

	template <class T>
	bool HasNaN(const T& value)
	{
		static_assert(sizeof(T) % sizeof(float) == 0);

		const float* it = reinterpret_cast<const float*>(&value);
		const float* const end = it + sizeof(value) / sizeof(float);

		while (it != end && !isnan(*it) && !isinf(*it))
			++it;

		return it != end;
	}

	zCModelNodeInst* GetHead(zCVob* vob)
	{
		return COA(vob, GetVisual(), CastTo<zCModel>(), SearchNode(ZS_HEAD));
	}

	zCModelNodeInst* GetNeck(zCVob* vob)
	{
		return COA(GetHead(vob), parentNode);
	}

	zCQuat operator*(float t, const zCQuat& quat)
	{
		zCQuat result;

		for (int i = 0; i < 4; i++)
			result[i] = t * quat[i];

		return result;
	}

#define TEST(cond) { if (!(cond)) { model = nullptr; return; } }

	class LookEngine
	{
	private:
		static constexpr float epsilon = 0.0001f;

		zCModel* model;
		zCModelNodeInst* head;
		zCModelNodeInst* neck;
		zMAT4 worldToNeckTrafo;
		zCQuat headQuat;
		zCQuat targetQuat;
		zCQuat relaxedQuat;
		zVEC3 headAt;
		zVEC3 targetAt;
		zVEC3 relaxedAt;
		zVEC3 relaxedRight;
		zVEC3 minAngles;
		zVEC3 maxAngles;
		int aniId;

		void DebugHead(const zVEC3& direction, const zCOLOR& color) const
		{
			const zVEC3 from = head->trafo.GetTranslation();
			const zVEC3 to = from + direction * 200.0f;
			const zMAT4 trafo = model->homeVob->trafoObjToWorld * model->GetTrafoNodeToModel(neck);
			zlineCache->Line3D(trafo * from, trafo * to, color, true);
		}

		static zCQuat Slerp(const zCQuat& from, const zCQuat& to, float t)
		{
			const float angle = acosf(std::clamp(from.Dot(to), -1.0f, 1.0f));
			const float sinus = sinf(angle);

			if (sinus < epsilon)
				return (t < 0.5f) ? from : to;

			return sinf((1.0f - t) * angle) / sinus * from + sinf(t * angle) / sinus * to;
		}

		void InitModel(oCNpc* npc)
		{
			model = npc->GetModel();
			TEST(model);

			head = model->SearchNode(ZS_HEAD);
			TEST(head);

			neck = head->parentNode;
			TEST(neck);

			worldToNeckTrafo = npc->GetTrafoModelNodeToWorld(neck).Inverse();
		}

		void InitHeadTrafos()
		{
			headQuat.Matrix4ToQuat(head->trafo);
			headQuat = 1.0f / headQuat.Length() * headQuat;
			headAt = head->trafo.GetAtVector();
		}

		void InitRelaxedTrafos()
		{
			int layer = 0;

			zCQuat* quat = nullptr;

			for (zCModelNodeInst::zTNodeAni* nodeAni = head->nodeAniList; nodeAni != head->nodeAniList + head->numNodeAnis; nodeAni++)
				if (zCModelAniActive* ani = nodeAni->modelAni)
					if (!ani->isFadingOut && ani->protoAni->aniID != aniId && ani->protoAni->layer >= layer)
					{
						layer = ani->protoAni->layer;
						quat = &nodeAni->quat;
					}

			if (!quat)
			{
				relaxedAt = { 0.0f, 0.0f, 1.0f };
				relaxedRight = { 0.0f, -1.0f, 0.0f };
				return;
			}

			zMAT4 trafo;
			quat->QuatToMatrix4(trafo);
			relaxedQuat = *quat;
			relaxedAt = trafo.GetAtVector();
			relaxedRight = -trafo.GetUpVector();
		}

		void InitNeckFlexibility()
		{
			zCModelAni* const ani = model->GetAniFromAniID(aniId);
			TEST(ani && ani->aniType == zMDL_ANI_TYPE_COMB && ani->combAniList.GetNum() > 0);

			int headIndex = 0;

			while (model->nodeList[ani->nodeIndexList[headIndex]] != head)
				headIndex += 1;

			for (zCModelAni* combAni : ani->combAniList)
			{
				zVEC3 euler;
				combAni->GetQuat(0, headIndex).QuatToEuler(euler);

				for (int i = 0; i < 3; i++)
				{
					minAngles[i] = std::min(minAngles[i], euler[i]);
					maxAngles[i] = std::max(maxAngles[i], euler[i]);
				}
			}
		}

		void InitTargetTrafos(const zVEC3& targetPos, const zVEC3& up)
		{
			zVEC3 z = worldToNeckTrafo * targetPos - head->trafo.GetTranslation();
			zVEC3 x = zMAT4{ worldToNeckTrafo }.SetTranslation({}) * up;
			zVEC3 y = z.Cross(x);
			x = y.Cross(z);

			if (x.Length_Sqr() < epsilon || y.Length_Sqr() < epsilon || z.Length_Sqr() < epsilon)
				return InitTargetTrafos();

			x.Normalize();
			y.Normalize();
			z.Normalize();

			zMAT4 trafo;
			trafo.MakeIdentity();
			trafo.SetRightVector(x);
			trafo.SetUpVector(y);
			trafo.SetAtVector(z);

			targetQuat.Matrix4ToQuat(trafo);
			targetQuat = 1.0f / targetQuat.Length() * targetQuat;

			targetAt = z;
		}

		void InitTargetTrafos(float ax, float ay, const zVEC3& up)
		{
			ax = std::clamp(ax * 2.0f - 1.0f, -1.0f, 1.0f) * PI / 2.0f;
			ay = std::clamp(ay * 2.0f - 1.0f, -1.0f, 1.0f) * PI / 2.0f;
			const zVEC3 x = relaxedRight * sinf(ax);
			const zVEC3 y = (relaxedAt * relaxedRight) * -sinf(ay);
			const zVEC3 at = x + y;
			InitTargetTrafos(head->trafo.GetTranslation() + at, up);
		}

		void InitTargetTrafos()
		{
			targetQuat = relaxedQuat;
			targetAt = relaxedAt;
		}

		void TryFlippedWay()
		{
			if (relaxedRight.Dot(headAt) * relaxedRight.Dot(targetAt) > 0.0f)
			{
				if (headQuat.Dot(targetQuat) < 0.0f)
					targetQuat = -1.0f * targetQuat;

				return;
			}

			const zCQuat middle = Slerp(headQuat, targetQuat, 0.5f);

			zMAT4 trafo;
			middle.QuatToMatrix4(trafo);
			const zVEC3 middleAt = trafo.GetAtVector();

			if (middleAt.Dot(relaxedAt) < 0.0f)
				targetQuat = -1.0f * targetQuat;
		}

		float GetAngleDegrees() const
		{
			const float cosinus = std::clamp(headAt.Dot(targetAt), -1.0f, 1.0f);
			const float angle = acosf(cosinus) / PI * 180.0f;
			return (headQuat.Dot(targetQuat) < 0.0f) ? 360.0f - angle : angle;
		}

		void Init(oCNpc* npc)
		{
			InitModel(npc);
			TEST(model);

			InitHeadTrafos();
			InitRelaxedTrafos();
			InitNeckFlexibility();
		}

	public:
		LookEngine(oCNpc* npc, const zVEC3* targetPos, int aniId) :
			aniId{ aniId }
		{
			Init(npc);
			TEST(model);

			if (targetPos)
				InitTargetTrafos(*targetPos, npc->GetUpVectorWorld());
			else
				InitTargetTrafos();

			TryFlippedWay();
		}

		bool IsValid() const
		{
			return model;
		}

		zCQuat Look(float& t) const
		{
			const float angle = GetAngleDegrees();
			t = 1.0f;

			if (fabs(angle) > epsilon)
			{
				t = ztimer->frameTimeFloat / 1000.0f * Options::DegreesPerSecond / angle;
				t = std::clamp(t, 0.0f, 1.0f);
			}

			zCQuat current = Slerp(headQuat, targetQuat, t);
			zVEC3 euler;
			current.QuatToEuler(euler);

			DebugHead(head->trafo.GetAtVector(), GFX_RED);

			for (int i = 0; i < 3; i++)
				euler[i] = std::clamp(euler[i], minAngles[i], maxAngles[i]);

			current.EulerToQuat(euler);
			return current;
		}
		
		static void GetAngles(zVEC3 up, zVEC3 at, zVEC3 to, float& azi, float& elev)
		{
			azi = 0.0f;
			elev = 0.0f;

			if (up.Length_Sqr() < epsilon || at.Length_Sqr() < epsilon || to.Length_Sqr() < epsilon)
				return;

			up.Normalize();
			at.Normalize();
			to.Normalize();

			at = at - up * up.Dot(at);

			if (at.Length_Sqr() < epsilon)
				return;

			at.Normalize();

			const zVEC3 right = up.Cross(at);
			zVEC3 to_h = to - up * up.Dot(to);

			if (to_h.Length_Sqr() < epsilon)
			{
				elev = 90.0f;
				return;
			}

			to_h.Normalize();

			azi = acosf(std::clamp(to_h.Dot(at), -1.0f, 1.0f)) / PI * 180.0f;

			if (to_h.Dot(right) < 0.0f)
				azi = -azi;

			elev = acosf(std::clamp(to_h.Dot(to), -1.0f, 1.0f)) / PI * 180.0f;

			if (to.Dot(up) < 0.0f)
				elev = -elev;
		}

		static void GetAngles(oCNpc* npc, const zVEC3& pos, float& azi, float& elev)
		{
			const zVEC3 up = npc->trafoObjToWorld.GetUpVector();
			zVEC3 at = npc->trafoObjToWorld.GetAtVector();

			zMAT4 trafo = npc->trafoObjToWorld;
			trafo.SetTranslation(zVEC3{});

			if (zCModel* model = npc->GetModel())
				for (zCModelNodeInst* node : model->nodeList)
					if (!node->parentNode)
					{
						zVEC3 nodeAt = trafo * node->trafo.GetRightVector();

						if (fabs(nodeAt.Dot(up) - 1.0f) > epsilon)
							at = nodeAt;

						break;
					}

			GetAngles(up, at, pos - GetEyes(npc), azi, elev);
		}

		static void GetAngles(oCNpc* npc, zCVob* target, float& azi, float& elev)
		{
			GetAngles(npc, GetEyes(target), azi, elev);
		}

		static zVEC3 GetEyes(zCVob* vob)
		{
			if (oCNpc* npc = vob->CastTo<oCNpc>())
				if (zCModelNodeInst* head = GetHead(npc))
					return npc->GetTrafoModelNodeToWorld(head).GetTranslation();

			zVEC3 eyes = vob->bbox3D.GetCenter();
			eyes[VY] = vob->bbox3D.maxs[VY];
			return eyes;
		}
	};

#undef TEST
}
