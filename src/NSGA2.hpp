#pragma once
#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <stdio.h>
#include <vector>

#include "Individual.cpp"

class Nsga
{
public:
	Nsga();
	Nsga(int n, int m, int itterations, int sampleSize, int numberOfProcesses);

	void run();

private:
	void initalizePopulation();
	void determineFitnessValue();
	void rankNonDominatedAndCrowding();
	void competitionSelection();
	void crossoverAndMutation();
	void cleanupOldValues();
	void outputOptimalSolution();

	void calculateFrontLevel();
	void calculateCrowdingDistance();
	void calculateFrontCrowdingDistance();
	void selectionTournament(std::vector<int> &tournamnetIndexVector,
							 std::vector<int> &winnerIndexVector,
							 int iter);

	// sorting methods
	void sortNonDominated();
	void sortPopulationByMaxCompletionTime();
	void sortPopulationByTotalEquipmentLoad();

	// utility methods
	void printPopulation();
	void printDurationMap();
	void printDominationValues();

	int n_, m_;
	int itterations_;
	int sampleSize_;
	int numberOfProcesses_;
	const int maxDuration_ = 10;
	const float maxCrowdingDistance_ = 10000;

	std::vector<IndividualPtr> population_;
	// duration for workpieces in order to calculate KPIs
	std::map<int, int> durationMap_;

	std::vector<int> front1_;
	std::vector<int> front2_;
	std::vector<int> front3_;
	std::vector<int> front4_;
};
