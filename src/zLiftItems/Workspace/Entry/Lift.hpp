namespace NAMESPACE
{
	zVEC3 Lift::GetFocus(zCVob* vob)
	{
		zVEC3 focus = vob->bbox3D.GetCenter();
		focus[VY] = (vob->bbox3D.maxs[VY] + focus[VY]) / 2.0f;
		return focus;
		
		// return vob->bbox3D.GetCenter() + (vob->bbox3D.GetCenter() - vob->bbox3D.GetCenterFloor()) * 0.5f;
	}

	void Lift::SetPosition(zCVob* vob, const zVEC3& position)
	{
		auto oldStatic = vob->collDetectionStatic;
		auto oldDynamic = vob->collDetectionDynamic;

		vob->SetCollDet(false);
		vob->SetPositionWorld(position);

		vob->SetCollDetStat(oldStatic);
		vob->SetCollDetDyn(oldDynamic);
	}

	bool Lift::TryMoveUp(zCVob* vob)
	{
		if (vob->groundPoly ||
			ogame->GetWorld()->TraceRayFirstHit(GetFocus(vob), zVEC3(0, -500, 0), vob,
				zTRACERAY_POLY_IGNORE_TRANSP | zTRACERAY_VOB_IGNORE_CHARACTER))
		{
			return false;
		}

		if (!ogame->GetWorld()->TraceRayNearestHit(vob->GetPositionWorld(), zVEC3(0, 200, 0), vob,
			zTRACERAY_POLY_2SIDED | zTRACERAY_POLY_IGNORE_TRANSP | zTRACERAY_VOB_IGNORE_CHARACTER))
		{
			return false;
		}

		SetPosition(vob, ogame->GetWorld()->traceRayReport.foundIntersection);
		return true;
	}

	bool Lift::TryMoveDown(zCVob* vob)
	{
		zVEC3 up = vob->bbox3D.GetCenter();
		up[VY] = vob->bbox3D.maxs[VY];

		if (!ogame->GetWorld()->TraceRayNearestHit(up, zVEC3{ 0.0f, -200.0f, 0.0f }, vob,
			zTRACERAY_POLY_IGNORE_TRANSP | zTRACERAY_VOB_IGNORE_CHARACTER))
		{
			return false;
		}

		const zVEC3& floorPos = vob->bbox3D.GetCenterFloor();
		const zVEC3& newFloorPos = ogame->GetWorld()->traceRayReport.foundIntersection;

		if (newFloorPos[VY] > floorPos[VY] - 10.0f)
			return false;

		zVEC3 newPos = newFloorPos;
		newPos[VY] += (vob->bbox3D.maxs[VY] - vob->bbox3D.mins[VY]) / 2.0f;

		SetPosition(vob, newPos);
		return true;
	}

	bool Lift::TryLiftVob(zCVob* vob)
	{
		return TryMoveUp(vob)/* || TryMoveDown(vob)*/;
	}

	void Lift::HandleVob(zCVob* vob, void* wtf)
	{
		if (vob->GetVobType() != zTVobType::zVOB_TYPE_ITEM)
		{
			return;
		}

		itemsTraversed += 1;

		zVEC3 oldPos = vob->GetPositionWorld();

		if (TryLiftVob(vob))
		{
			oldPositions.push_back(std::make_pair(vob, oldPos));
		}
	}

	void Lift::ToggleLifts()
	{
		for (auto it = oldPositions.begin(); it != oldPositions.end(); it++)
		{
			const zVEC3 pos = it->first->GetPositionWorld();
			SetPosition(it->first, it->second);
			it->second = pos;
		}
	}

	void Lift::Debug()
	{
		screen->PrintCX(1000, Z"Items checked: " + Z itemsTraversed);
		screen->PrintCX(1200, Z"Items affected: " + Z static_cast<int>(oldPositions.size()));

		for (auto it = oldPositions.begin(); it != oldPositions.end(); it++)
		{
			zlineCache->Line3D(GetFocus(player), GetFocus(it->first), zCOLOR(255, 0, 0), true);
		}

		if (!player->GetFocusVob())
		{
			return;
		}
	}

	void Lift::Clear()
	{
		oldPositions.clear();
		nextIndex = 0;
	}

	void Lift::Remove(zCVob* vob)
	{
		auto it = std::find_if(oldPositions.begin(), oldPositions.end(), [vob](std::pair<zCVob*, zVEC3> pair) { return pair.first == vob; });

		if (it == oldPositions.end())
		{
			return;
		}

		int index = it - oldPositions.begin();

		if (nextIndex > index)
		{
			nextIndex -= 1;
		}

		oldPositions.erase(it);
	}

	std::pair<zCVob*, zVEC3> Lift::GetNext()
	{
		if (oldPositions.empty())
		{
			return std::make_pair(player, player->GetPositionWorld());
		}

		auto result = oldPositions[nextIndex];
		nextIndex = (nextIndex + 1) % oldPositions.size();
		return result;
	}
}
