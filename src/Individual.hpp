#pragma once
#include <iomanip>
#include <sstream>
#include <memory>
#include <vector>

struct Individual{
public:

	std::stringstream getGenesAsSStream();
	void mutate();

	std::vector<int> processes_;
	std::vector<int> machines_;
	int maxCompletionTime_ = 0;
	int totalEquipmentLoad_ = 0;

	int  dominationCount_ = 0;
	std::vector<int> dominatedPoints_;

	int frontLevel_ = 0;
	float crowdingDistance_ = 0;

	int partner_ = 0;
};

using IndividualPtr = std::shared_ptr<Individual>;
