// Robot04.cpp: определяет экспортированные функции для приложения DLL.
//

#include "stdafx.h"
#include <time.h>

StepInfo* stepInfo;
RobotInfo* myInfo;

struct Point
{
	Point() {};
	Point(int _x, int _y) :
		x(_x), y(_y) {};
	int x;
	int y;
};


UINT Pythagoras(int x, int y)
{
	return UINT(sqrt(x * x + y * y));
}

struct VictimInfo
{
	VictimInfo(int _dX, int _dY, UINT _ID) :
		dX(_dX), dY(_dY), ID(_ID) {};
	int dX;
	int dY;
	UINT ID;
};


Point getDistance(Point posA, Point posB)
{
	Point vec;
	int dist1_x = abs(posA.x - posB.x);
	int dist1_y = abs(posA.y - posB.y);

	int left_x, right_x;
	if (posA.x > posB.x) {
		right_x = posA.x;
		left_x = posB.x;
	}
	else {
		right_x = posB.x;
		left_x = posA.x;
	}
	int dist2_x = left_x + (stepInfo->gameConfig.W - right_x);
	if (dist1_x <= dist2_x) {
		vec.x = posB.x - posA.x;
	}
	else {
		right_x == posB.x ? vec.x = -dist2_x : vec.x = dist2_x;
	}

	int bottom_y, top_y;
	if (posA.y > posB.y) {
		bottom_y = posA.y;
		top_y = posB.y;
	}
	else {
		bottom_y = posB.y;
		top_y = posA.y;
	}
	int dist2_y = top_y + (stepInfo->gameConfig.H - bottom_y);
	if (dist1_y <= dist2_y) {
		vec.y = posB.y - posA.y;
	}
	else {
		bottom_y == posB.y ? vec.y = -dist2_y : vec.y = dist2_y;
	}

	return vec;
}


Point getChargeStation(int x, int y) {
	UINT min_dX = stepInfo->gameConfig.W + 1;
	UINT min_dY = stepInfo->gameConfig.H + 1;
	int real_dX = 1000, real_dY = 1000;
	Point its;
	its.x = x;
	its.y = y;
	for (auto station = stepInfo->chargingStations.begin(); station != stepInfo->chargingStations.end(); ++station) {
		Point vec = getDistance(its, Point(station->first, station->second));
		if (Pythagoras(vec.x, vec.y) < Pythagoras(min_dX, min_dY)) {
			real_dX = vec.x;
			real_dY = vec.y;
			min_dX = abs(vec.x);
			min_dY = abs(vec.y);
		}
	}
	return Point(real_dX, real_dY);
}


Point getRepairStation(int x, int y) {
	UINT min_dX = stepInfo->gameConfig.W + 1;
	UINT min_dY = stepInfo->gameConfig.H + 1;
	int real_dX = 1000, real_dY = 1000;
	Point its;
	its.x = x;
	its.y = y;
	for (auto station = stepInfo->maintenance.begin(); station != stepInfo->maintenance.end(); ++station) {
		Point vec = getDistance(its, Point(station->first, station->second));
		if (Pythagoras(vec.x, vec.y) < Pythagoras(min_dX, min_dY)) {
			real_dX = vec.x;
			real_dY = vec.y;
			min_dX = abs(vec.x);
			min_dY = abs(vec.y);
		}
	}
	return Point(real_dX, real_dY);
}

VictimInfo getVictimInfo() {
	UINT min_dX = stepInfo->gameConfig.W + 1;
	UINT min_dY = stepInfo->gameConfig.H + 1;
	int real_dX = 0, real_dY = 0;
	UINT victimID = 1000;
	for (auto it = stepInfo->robotsInfo.begin(); it != stepInfo->robotsInfo.end(); ++it) {
		Point vec = getDistance(Point(myInfo->x, myInfo->y), Point(it->x, it->y));
		if (Pythagoras(vec.x, vec.y) < Pythagoras(min_dX, min_dY) &&
			it->Alive && myInfo->Author.compare(it->Author) != 0)
		{
			if (!(it->Author == "Yeah" || it->Author == "Lyaskin"))
			{
				real_dX = vec.x;
				real_dY = vec.y;
				min_dX = abs(vec.x);
				min_dY = abs(vec.y);
				victimID = it->ID;
			}
		}
	}

	return VictimInfo(real_dX, real_dY, victimID);
}
extern "C" __declspec(dllexport) void DoStep(StepInfo* _stepInfo)
{
	srand(time(NULL));

	stepInfo = _stepInfo;

	int ID = stepInfo->ID;


	for (auto it = stepInfo->robotsInfo.begin(); it != stepInfo->robotsInfo.end(); ++it) {
		if (_stepInfo->ID == it->ID) {

			myInfo = new RobotInfo(*it);
			break;
		}
	}
	int E = myInfo->E;
	int L = myInfo->L;
	int A = myInfo->A;
	int P = myInfo->P;
	int V = myInfo->V;
	int x = myInfo->x;
	int y = myInfo->y;

	int Emax = stepInfo->gameConfig.E_max;
	int Lmax = stepInfo->gameConfig.L_max;
	int Vmax = stepInfo->gameConfig.V_max;
	int Rmax = stepInfo->gameConfig.R_max;

	int maxDistance = Vmax * V / Lmax * E / Emax;
	int maxAttackDistance = Rmax * V / Lmax * E / Emax;
	int maxAttack = A * E / Emax;
	int maxProtection = P * E / Emax;

	Point energy, repair;
	energy = getChargeStation(x, y);
	repair = getRepairStation(x, y);

	int energyDistance = Pythagoras(energy.x, energy.y);
	int mechanicDistance = Pythagoras(repair.x, repair.y);

	if (E < 0.7 * Emax)
	{

		if (maxDistance > energyDistance)
		{
			stepInfo->pRobotActions->addActionMove(energy.x, energy.y);
		}
		else
		{
			energy.x = energy.x * maxDistance / energyDistance;
			energy.y = energy.y * maxDistance / energyDistance;

			stepInfo->pRobotActions->addActionMove(energy.x, energy.y);
		}
		return;
	}
	else if (L < 0.7 * Lmax)
	{
		if (maxDistance > mechanicDistance)
		{
			stepInfo->pRobotActions->addActionMove(repair.x, repair.y);
		}
		else
		{
			repair.x = 1.0 * repair.x * maxDistance / mechanicDistance;
			repair.y = 1.0 * repair.y * maxDistance / mechanicDistance;
			stepInfo->pRobotActions->addActionMove(repair.x, repair.y);
		}
		return;


	}

	else
	{
		
		VictimInfo vInfo = getVictimInfo();
		if (vInfo.ID != 1000) {
			RobotInfo* victim;
			for (auto it = stepInfo->robotsInfo.begin(); it != stepInfo->robotsInfo.end(); ++it) {
				if (vInfo.ID == it->ID) {
					victim = it._Ptr;
				}
			}
			float A_r = stepInfo->gameConfig.RND_min * A;
			float P_r = (1 - stepInfo->gameConfig.RND_min) * victim->P;
			int delta = (A_r * E - P_r * victim->E) / stepInfo->gameConfig.E_max;
			if (delta > 0) {
				int maxDistToAttack = stepInfo->gameConfig.R_max * V * E /
					(stepInfo->gameConfig.L_max * stepInfo->gameConfig.E_max);
				int curDistance = Pythagoras(vInfo.dX, vInfo.dY);

				if (curDistance <= maxAttackDistance && E - stepInfo->gameConfig.dE_A > 0) {
					stepInfo->pRobotActions->addActionAttack(vInfo.ID);
				}
				else {

					int shift = curDistance - maxDistToAttack;

					if (shift < maxDistance &&
						E - stepInfo->gameConfig.dE_V - stepInfo->gameConfig.dE_A > 0) {

						double ratio_x = abs(vInfo.dX) / (abs(vInfo.dX) + abs(vInfo.dY));
						int shift_x = ratio_x * shift;

						if (vInfo.dY >= 0) {
							stepInfo->pRobotActions->addActionMove(shift_x, shift - abs(shift_x));
							x += shift_x;
							y += shift - abs(vInfo.dX);
						}
						else {
							stepInfo->pRobotActions->addActionMove(shift_x, -(shift - abs(shift_x)));
							x += shift_x;
							y += -(shift - abs(vInfo.dX));
						}

						maxDistToAttack = stepInfo->gameConfig.R_max * V * (E - stepInfo->gameConfig.dE_V) /
							(stepInfo->gameConfig.L_max * stepInfo->gameConfig.E_max);
						Point newDist = getDistance(Point(x, y), Point(victim->x, victim->y));
						if (Pythagoras(newDist.x, newDist.y) <= maxDistToAttack)
							stepInfo->pRobotActions->addActionAttack(vInfo.ID);
					}
					else {

						if (E - stepInfo->gameConfig.dE_V > 0) {

							double ratio_x = abs(vInfo.dX) / (abs(vInfo.dX) + abs(vInfo.dY));
							int shift_x = ratio_x * maxDistance;

							if (vInfo.dY >= 0) {
								stepInfo->pRobotActions->addActionMove(shift_x, maxDistance - abs(shift_x));
							}
							else {
								stepInfo->pRobotActions->addActionMove(shift_x, -(maxDistance - abs(shift_x)));
							}

						}
					}
				}
			}

		}

		stepInfo->pRobotActions->addActionRedistribution(0.5 * L, 0.4 * L, 0.3 * L);

		int direction = (rand() + x) % 8;
		switch (direction)
		{
		case 0:
			stepInfo->pRobotActions->addActionMove(1, 0);
			break;
		case 1:
			stepInfo->pRobotActions->addActionMove(-1, 0);
			break;
		case 2:
			stepInfo->pRobotActions->addActionMove(0, 1);
			break;
		case 3:
			stepInfo->pRobotActions->addActionMove(0, -1);
			break;
		case 4:
			stepInfo->pRobotActions->addActionMove(1, 1);
			break;
		case 5:
			stepInfo->pRobotActions->addActionMove(-1, 1);
			break;
		case 6:
			stepInfo->pRobotActions->addActionMove(-1, -1);
			break;
		case 7:
			stepInfo->pRobotActions->addActionMove(1, -1);
			break;
		default:
			break;
		}

		return;
	}
}

