#include "acequia_manager.h"
#include <iostream>
#include <vector>
#include <algorithm>

const double MOVE_PER_HOUR = 3600.0 / 10000.0;

double clampRate(double rate)
{
    if(rate < 0.0)
    {
        return 0.0;
    }

    if(rate > 1.0)
    {
        return 1.0;
    }

    return rate;
}

double getSurplus(Region* region, double targetBuffer)
{
    double target = region->waterNeed + targetBuffer;

    if(region->waterLevel > target)
    {
        return region->waterLevel - target;
    }

    return 0.0;
}

double getDeficit(Region* region, double targetBuffer)
{
    double target = region->waterNeed + targetBuffer;

    if(region->waterLevel < target)
    {
        return target - region->waterLevel;
    }

    return 0.0;
}

double calculateRate(double surplus, double deficit)
{
    double amountToMove = std::min(surplus, deficit);
    double rate = amountToMove / MOVE_PER_HOUR;

    return clampRate(rate);
}

void closeAllCanals(const std::vector<Canal*>& canals)
{
    for(int i = 0; i < canals.size(); i++)
    {
        canals[i]->setFlowRate(0.0);
        canals[i]->toggleOpen(false);
    }
}

void openCanal(Canal* canal, double rate)
{
    rate = clampRate(rate);

    if(rate > 0.0)
    {
        canal->setFlowRate(rate);
        canal->toggleOpen(true);
    }
}

void solveProblems(AcequiaManager& manager)
{
    auto canals = manager.getCanals();
    auto regions = manager.getRegions();

    Region* north = regions[0];
    Region* south = regions[1];
    Region* east = regions[2];

    Canal* canalA = canals[0]; // North -> South
    Canal* canalB = canals[1]; // South -> East
    Canal* canalC = canals[2]; // North -> East
    Canal* canalD = canals[3]; // East -> North

    double targetBuffer = 1.0;

    while(!manager.isSolved && manager.hour != manager.SimulationMax)
    {
        closeAllCanals(canals);

        double northSurplus = getSurplus(north, targetBuffer);
        double southSurplus = getSurplus(south, targetBuffer);
        double eastSurplus = getSurplus(east, targetBuffer);

        double northDeficit = getDeficit(north, targetBuffer);
        double southDeficit = getDeficit(south, targetBuffer);
        double eastDeficit = getDeficit(east, targetBuffer);

        if(southDeficit > 0.0)
        {
            if(northSurplus > 0.0)
            {
                double rate = calculateRate(northSurplus, southDeficit);
                openCanal(canalA, rate);
            }
            else if(eastSurplus > 0.0)
            {
                double rate = calculateRate(eastSurplus, southDeficit);

                openCanal(canalD, rate);
                openCanal(canalA, rate);
            }
        }
        else if(eastDeficit > 0.0)
        {
            if(northSurplus > 0.0)
            {
                double rate = calculateRate(northSurplus, eastDeficit);
                openCanal(canalC, rate);
            }
            else if(southSurplus > 0.0)
            {
                double rate = calculateRate(southSurplus, eastDeficit);
                openCanal(canalB, rate);
            }
        }
        else if(northDeficit > 0.0)
        {
            if(eastSurplus > 0.0)
            {
                double rate = calculateRate(eastSurplus, northDeficit);
                openCanal(canalD, rate);
            }
            else if(southSurplus > 0.0)
            {
                double rate = calculateRate(southSurplus, northDeficit);

                openCanal(canalB, rate);
                openCanal(canalD, rate);
            }
        }

        manager.nexthour();
    }

    closeAllCanals(canals);
}