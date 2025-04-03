#include "Individual.hpp"


std::stringstream Individual::getGenesAsSStream()
{
	std::stringstream result;

	for (auto process : processes_)
	{
		result << std::setw(2) << std::to_string(process) << " ";
	}

	for (auto machine : machines_)
	{
		result << std::setw(2) << std::to_string(machine) << " ";
	}

	result << std::setw(3) << std::to_string(maxCompletionTime_) << " ";
	result << std::setw(3) << std::to_string(totalEquipmentLoad_) << " ";
	return result;
}

void Individual::mutate()
{
	int randomProcessMutation = rand() % processes_.size();
	int randomMachineMutation = rand() % processes_.size();

	int mutation = rand() % 2;

	processes_[randomProcessMutation] += (mutation==1?-1:+1);
	machines_[randomMachineMutation] += (mutation==1?-1:+1);
}
