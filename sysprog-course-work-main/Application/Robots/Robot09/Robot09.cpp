// Robot01.cpp: определ€ет экспортированные функции дл€ приложени€ DLL.
//

#include "stdafx.h"
#include <ctime>

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

struct VictimInfo
{
	VictimInfo(int _dX, int _dY, UINT _ID) :
		dX(_dX), dY(_dY), ID(_ID) {};
	int dX;
	int dY;
	UINT ID;
};

UINT Pythagoras(int x, int y)
{
	return UINT(sqrt(x * x + y * y));
}


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

VictimInfo getVictimInfo() {
	UINT min_dX = stepInfo->gameConfig.W + 1;
	UINT min_dY = stepInfo->gameConfig.H + 1;
	int real_dX = 0, real_dY = 0;
	UINT victimID = 1000;
	for (auto it = stepInfo->robotsInfo.begin(); it != stepInfo->robotsInfo.end(); ++it)
	{
		Point vec = getDistance(Point(myInfo->x, myInfo->y), Point(it->x, it->y));
		if (Pythagoras(vec.x, vec.y) < Pythagoras(min_dX, min_dY) && it->Alive && myInfo->Author.compare(it->Author) != 0)
		{
			if (!(it->Author == "Lyaskin" || it->Author == "Yeah" || it->Author == "Polyakov"))
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
//ѕоиск станции подзар€дки энергии
Point getCStation(int x, int y) {
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


extern "C" __declspec(dllexport) void DoStep(StepInfo * _stepInfo)
{
	srand(time(NULL));

	//vector <riv> rivals;

	stepInfo = _stepInfo;

	for (auto it = stepInfo->robotsInfo.begin(); it != stepInfo->robotsInfo.end(); ++it) {
		if (_stepInfo->ID == it->ID)
		{
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

	int maxDistToMove = stepInfo->gameConfig.V_max * myInfo->V * myInfo->E / (stepInfo->gameConfig.L_max * stepInfo->gameConfig.E_max);
	int maxDistToAttack = stepInfo->gameConfig.R_max * myInfo->V * myInfo->E / (stepInfo->gameConfig.L_max * stepInfo->gameConfig.E_max);
	//int curDistance = Pythagoras(vInfo.dX, vInfo.dY);
	/*int maxDistToMove = 1.0 * Vmax * V / Lmax * E / Emax;
	int S_amax = 1.0 * Rmax * V / Lmax * E / Emax;
	int Amax = 1.0 * A * E / Emax;
	int Pmax = 1.0 * P * E / Emax;*/
	Point energy;
	//, mech;
	energy = getCStation(x, y);

	int se = Pythagoras(energy.x, energy.y);
	//int sm = Pythagoras(mech.x, mech.y);

	if ((se == 0) && ((E < 0.7 * Emax) || (stepInfo->stepNumber > 900)) && (L > 0.7 * Lmax))
	{
		stepInfo->pRobotActions->addActionRedistribution(0, L, 0);
		return;
	}


	if ((E < 0.7 * Emax) || (stepInfo->stepNumber > 980))
	{

		if (maxDistToMove > se)
		{
			stepInfo->pRobotActions->addActionMove(energy.x, energy.y);
			stepInfo->pRobotActions->addActionRedistribution(0, L, 0);
		}
		else
		{
			energy.x = 1.0 * energy.x * maxDistToMove / se;
			energy.y = 1.0 * energy.y * maxDistToMove / se;
			stepInfo->pRobotActions->addActionRedistribution(0, 0.2 * L, 0.8 * L);
			stepInfo->pRobotActions->addActionMove(energy.x, energy.y);
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

			float A_r = stepInfo->gameConfig.RND_min * myInfo->A;
			float P_r = (1 - stepInfo->gameConfig.RND_min) * victim->P;
			int delta = (A_r * myInfo->E - P_r * victim->E) / stepInfo->gameConfig.E_max;
			if (delta > 0)
			{
				maxDistToAttack = stepInfo->gameConfig.R_max * myInfo->V * myInfo->E / (stepInfo->gameConfig.L_max * stepInfo->gameConfig.E_max);
				int curDistance = Pythagoras(vInfo.dX, vInfo.dY);//дистанци€ между нами и целью
				if (curDistance <= maxDistToAttack && myInfo->E - stepInfo->gameConfig.dE_A > 0)
				{
					stepInfo->pRobotActions->addActionAttack(vInfo.ID);
				}
				else
				{
					//рассто€ние, которое необходимо преодолеть, чтобы дистанци€ 
					//между нами и целью была допустимой дл€ атаки
					int shift = curDistance - maxDistToAttack;
					//если дл€ перемещени€ с последующей атакой нам хватит дистанции
					//и на все это хватит энергии, то...
					if (shift < maxDistToMove && myInfo->E - stepInfo->gameConfig.dE_V - stepInfo->gameConfig.dE_A > 0)
					{
						double ratio_x = abs(vInfo.dX) / (abs(vInfo.dX) + abs(vInfo.dY));
						int shift_x = ratio_x * shift;

						if (vInfo.dY >= 0)
						{
							stepInfo->pRobotActions->addActionMove(shift_x, shift - abs(shift_x));
							myInfo->x += shift_x;
							myInfo->y += shift - abs(vInfo.dX);
						}
						else
						{
							stepInfo->pRobotActions->addActionMove(shift_x, -(shift - abs(shift_x)));
							myInfo->x += shift_x;
							myInfo->y += -(shift - abs(vInfo.dX));
						}

						maxDistToAttack = stepInfo->gameConfig.R_max * myInfo->V * (myInfo->E - stepInfo->gameConfig.dE_V) / (stepInfo->gameConfig.L_max * stepInfo->gameConfig.E_max);
						Point newDist = getDistance(Point(myInfo->x, myInfo->y), Point(victim->x, victim->y));
						if (Pythagoras(newDist.x, newDist.y) <= maxDistToAttack)
							stepInfo->pRobotActions->addActionAttack(vInfo.ID);
					}
					else
					{
						//нам не хватило дистанции => просто пойдем по направлению к
						//противнику и проверим есть ли у нас на это энерги€
						if (myInfo->E - stepInfo->gameConfig.dE_V > 0)
						{

							double ratio_x = abs(vInfo.dX) / (abs(vInfo.dX) + abs(vInfo.dY));
							int shift_x = ratio_x * maxDistToMove;
							if (vInfo.dY >= 0)
							{
								stepInfo->pRobotActions->addActionMove(shift_x, maxDistToMove - abs(shift_x));
							}
							else
							{
								stepInfo->pRobotActions->addActionMove(shift_x, -(maxDistToMove - abs(shift_x)));
							}

						}
					}
				}
			}
		}

	}


	stepInfo->pRobotActions->addActionRedistribution(0.35 * L, 0.35 * L, 0.3 * L);

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

	delete myInfo;
}




