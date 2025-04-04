#include "NSGA2.hpp"

Nsga::Nsga()
{
	n_ = m_ = itterations_ = sampleSize_ = numberOfProcesses_ = 0;
}

Nsga::Nsga(int n, int m, int itterations, int sampleSize, int numberOfProcesses)
{
	n_ = n;
	m_ = m;
	itterations_ = itterations;
	sampleSize_ = sampleSize;
	numberOfProcesses_ = numberOfProcesses;
}

void Nsga::run()
{
	int itteration = 1;

	std::cout << "\nSTEP 1: Population initialization\n";
	initalizePopulation();

	while (itteration <= itterations_)
	{

		std::cout << "\nItteration number " << itteration << std::endl;

		std::cout << "\nSTEP 2: Determination of the objective function fitness value\n";
		determineFitnessValue();

		std::cout << "\nSTEP 3: Fast non-dominated and crowding ranking\n";
		rankNonDominatedAndCrowding();

		std::cout << "\nSTEP 4: Competition selection\n";
		competitionSelection();

		std::cout << "\nSTEP 5: Crossover and mutation operation\n";
		crossoverAndMutation();

		std::cout << "\nCleanup old values: \n";
		cleanupOldValues();
		// Genetic frequency plus one
		itteration++;
	}

	std::cout << "\nSTEP 6: Determination of the optimal solution\n";
	outputOptimalSolution();
}

void Nsga::initalizePopulation()
{
	for (int i = 0; i < sampleSize_; i++)
	{
		IndividualPtr individual = std::make_shared<Individual>();

		// generating random workpieces/processes
		for (int j = 0; j < numberOfProcesses_; j++)
		{
			int randomProcessNumber = rand() % n_ + 1;
			individual->processes_.push_back(randomProcessNumber);

			auto result = durationMap_.find(randomProcessNumber);
			if (result == durationMap_.end())
			{
				durationMap_.insert({randomProcessNumber, (rand() % maxDuration_ + 1)});
			}
		}

		// generating random machines for previous workpieces
		for (int k = 0; k < numberOfProcesses_; k++)
		{
			individual->machines_.push_back(rand() % m_ + 1);
		}

		// initialize with 0 maximum completion time
		individual->maxCompletionTime_ = 0;
		// initialize with 0 total delay time
		individual->totalEquipmentLoad_ = 0;

		population_.push_back(individual);
	}

	printPopulation();
	printDurationMap();
	getchar(); // g++ pause
}

void Nsga::determineFitnessValue()
{
	int individualNumber = 0;

	for (auto &individual : population_)
	{
		// calculate and set maximum completion time
		int maxCompletionTime = 0;
		// calculate and set total equipment load
		int totalEquipmentLoad = 0;

		std::map<int, int> machinesRuntime;

		for (int gene = 0; gene < numberOfProcesses_; gene++)
		{
			int currentWorkpieceTime = (durationMap_.find(individual->processes_[gene]))->second;

			// seaching if the corresponding machine is already running
			auto machineTime = machinesRuntime.find(individual->machines_[gene]);
			if (machineTime != machinesRuntime.end())
			{
				machineTime->second += currentWorkpieceTime;
			}
			else
			{
				machinesRuntime.insert({individual->machines_[gene], currentWorkpieceTime});
			}
		}
		for (auto it : machinesRuntime)
		{
			if (it.second > maxCompletionTime)
			{
				maxCompletionTime = it.second;
			}
			totalEquipmentLoad += it.second;
		}
		population_[individualNumber]->maxCompletionTime_ = maxCompletionTime;
		population_[individualNumber++]->totalEquipmentLoad_ = totalEquipmentLoad;
	}

	printPopulation();
	getchar(); // g++ pause
}

void Nsga::rankNonDominatedAndCrowding()
{

	// Non-dominated sorting
	for (std::size_t i = 0; i < (population_.size() - 1); i++)
	{
		for (uint j = i + 1; j < population_.size(); j++)
		{
			// check if point 1 < point 2
			if ((population_[i]->maxCompletionTime_ <= population_[j]->maxCompletionTime_ &&
				 population_[i]->totalEquipmentLoad_ <= population_[j]->totalEquipmentLoad_) &&
				(population_[i]->maxCompletionTime_ < population_[j]->maxCompletionTime_ ||
				 population_[i]->totalEquipmentLoad_ < population_[j]->totalEquipmentLoad_))
			{
				population_[i]->dominatedPoints_.push_back(j);
				population_[j]->dominationCount_ += 1;
				// (dominationValues[i].second).push_back(j);
				// dominationValues[j].first+= 1;
			}

			// check if point 2 < point 1
			if ((population_[j]->maxCompletionTime_ <= population_[i]->maxCompletionTime_ &&
				 population_[j]->totalEquipmentLoad_ <= population_[i]->totalEquipmentLoad_) &&
				(population_[j]->maxCompletionTime_ < population_[i]->maxCompletionTime_ ||
				 population_[j]->totalEquipmentLoad_ < population_[i]->totalEquipmentLoad_))
			{
				population_[j]->dominatedPoints_.push_back(i);
				population_[i]->dominationCount_ += 1;
			}
		}
	}

	//  1st front sorting
	sortNonDominated();

	//  2nd front sorting
	sortNonDominated();

	//  3rd front sorting, the remaining ones are front 4
	sortNonDominated();

	calculateFrontLevel();

	calculateCrowdingDistance();

	//	sort population based on front and then sort based on crowding distance
	std::sort(population_.begin(), population_.end(),
			  [](const IndividualPtr &a, const IndividualPtr &b) -> bool
			  {
				  return a->frontLevel_ < b->frontLevel_;
			  });

	std::sort(population_.begin(), population_.end(),
			  [](const IndividualPtr &a, const IndividualPtr &b) -> bool
			  {
				  if (a->frontLevel_ == b->frontLevel_)
					  return a->crowdingDistance_ > b->crowdingDistance_;
				  else
					  return false;
			  });

	printPopulation();

	// since the population is sorted by both front and crowding distance, we can delete all the weak individuals
	auto newPopulationSize = population_.size() / 2;

	// if newPopulation is not %2, the number of parents is odd, next generation will be smaller to correct issue
	newPopulationSize = (newPopulationSize % 2 == 0 ? newPopulationSize : newPopulationSize - 1);

	for (uint i = 0; i < newPopulationSize; i++)
	{
		population_.pop_back();
	}

	printPopulation();
	getchar(); // g++ pause
}

void Nsga::competitionSelection()
{
	std::srand(unsigned(std::time(0)));
	std::vector<int> tournamentIndexVector;
	std::vector<int> winnerIndexVector;

	for (uint i = 0; i < population_.size(); i++)
	{
		tournamentIndexVector.push_back(i);
	}
	std::random_shuffle(tournamentIndexVector.begin(), tournamentIndexVector.end());

	while (tournamentIndexVector.size() > 2)
	{

		for (uint j = 0; j < tournamentIndexVector.size() && j + 1 < tournamentIndexVector.size(); j++)
		{
			selectionTournament(tournamentIndexVector, winnerIndexVector, j);
		}
	}
	winnerIndexVector.push_back(tournamentIndexVector.back());
	tournamentIndexVector.pop_back();
	winnerIndexVector.push_back(tournamentIndexVector.back());
	tournamentIndexVector.pop_back();

	for (uint k = 0; k < winnerIndexVector.size() && k + 1 < winnerIndexVector.size(); k += 2)
	{
		population_[winnerIndexVector[k]]->partner_ = winnerIndexVector[k + 1];
		population_[winnerIndexVector[k + 1]]->partner_ = winnerIndexVector[k];
	}

	printPopulation();
	getchar(); // g++ pause
}

void Nsga::crossoverAndMutation()
{
	std::vector<int> processMask;
	std::vector<int> machineMask;
	std::srand(unsigned(std::time(0)));

	for (int i = 0; i < numberOfProcesses_ / 2; i++)
	{
		processMask.push_back(1);
		machineMask.push_back(1);
	}

	for (int j = numberOfProcesses_ / 2; j < numberOfProcesses_; j++)
	{
		processMask.push_back(0);
		machineMask.push_back(0);
	}

	int initialPopulationSize = population_.size();
	for (int k = 0; k < initialPopulationSize; k++)
	{
		int partner = population_[k]->partner_;
		if (partner != -1)
		{
			IndividualPtr child1 = std::make_shared<Individual>();
			IndividualPtr child2 = std::make_shared<Individual>();

			std::random_shuffle(processMask.begin(), processMask.end());
			std::random_shuffle(machineMask.begin(), machineMask.end());

			for (uint iter1 = 0; iter1 < processMask.size(); iter1++)
			{
				int gene1 = population_[k]->processes_[iter1];
				int gene2 = population_[partner]->processes_[iter1];

				child1->processes_.push_back(processMask[iter1] == 1 ? gene1 : gene2);
				child2->processes_.push_back(processMask[iter1] == 1 ? gene2 : gene1);
			}

			for (uint iter2 = 0; iter2 < machineMask.size(); iter2++)
			{
				int gene1 = population_[k]->machines_[iter2];
				int gene2 = population_[partner]->machines_[iter2];

				child1->machines_.push_back(machineMask[iter2] == 1 ? gene1 : gene2);
				child2->machines_.push_back(machineMask[iter2] == 1 ? gene2 : gene1);
			}

			child1->mutate();
			child2->mutate();

			population_.push_back(child1);
			population_.push_back(child2);

			population_[partner]->partner_ = -1;
		}
	}

	printPopulation();
	getchar(); // g++ pause
}

void Nsga::cleanupOldValues()
{
	for (auto iter : population_)
	{
		for (auto &i : iter->processes_)
		{
			if (i == 0 || i > n_)
			{
				i = rand() % n_ + 1;
			}
		}
		for (auto &j : iter->machines_)
		{
			if (j == 0 || j > m_)
			{
				j = rand() % m_ + 1;
			}
		}
		iter->maxCompletionTime_ = 0;
		iter->totalEquipmentLoad_ = 0;
		iter->dominationCount_ = 0;
		while (!iter->dominatedPoints_.empty())
		{
			iter->dominatedPoints_.pop_back();
		}
		iter->frontLevel_ = 0;
		iter->crowdingDistance_ = 0.0;
		iter->partner_ = 0;
	}

	printPopulation();
	getchar(); // g++ pause
}

void Nsga::outputOptimalSolution()
{
	determineFitnessValue();
	rankNonDominatedAndCrowding();

	std::cout << "\nOptimal solutions sorted by maximum completion time: \n";
	sortPopulationByMaxCompletionTime();
	printPopulation();

	std::cout << "\nOptimal solutions sorted by total equipment load: \n";
	sortPopulationByTotalEquipmentLoad();
	printPopulation();

	std::cout << "\nOptimal solutions sorted by total front level and crowding distance: \n";
	//	sort population based on front and then sort based on crowding distance
	std::sort(population_.begin(), population_.end(),
			  [](const IndividualPtr &a, const IndividualPtr &b) -> bool
			  {
				  return a->frontLevel_ < b->frontLevel_;
			  });

	std::sort(population_.begin(), population_.end(),
			  [](const IndividualPtr &a, const IndividualPtr &b) -> bool
			  {
				  if (a->frontLevel_ == b->frontLevel_)
					  return a->crowdingDistance_ > b->crowdingDistance_;
				  else
					  return false;
			  });
	printPopulation();
}

void Nsga::calculateFrontLevel()
{
	std::vector<int> frontCountVector(5, 0);

	for (auto &iter : population_)
	{
		iter->frontLevel_ = iter->dominationCount_ + 4;
		if (iter->frontLevel_ > 4)
		{
			iter->frontLevel_ = 4;
		}
		frontCountVector[iter->frontLevel_]++;
	}
	printPopulation();

	std::cout << "Numbers of individual in: f1=" << frontCountVector[1] << " f2=" << frontCountVector[2]
			  << " f3=" << frontCountVector[3] << " f4=" << frontCountVector[4] << std::endl;
}

void Nsga::calculateCrowdingDistance()
{
	//	add maxCompletionTime to crowding distance
	sortPopulationByMaxCompletionTime();
	for (auto iter : population_)
	{
		if (iter->frontLevel_ == 1)
		{
			front1_.push_back(iter->maxCompletionTime_);
		}
		if (iter->frontLevel_ == 2)
		{
			front2_.push_back(iter->maxCompletionTime_);
		}
		if (iter->frontLevel_ == 3)
		{
			front3_.push_back(iter->maxCompletionTime_);
		}
		if (iter->frontLevel_ == 4)
		{
			front4_.push_back(iter->maxCompletionTime_);
		}
	}
	calculateFrontCrowdingDistance();

	//	add totalEquipmentLoad to crowding distance
	sortPopulationByTotalEquipmentLoad();
	for (auto iter : population_)
	{
		if (iter->frontLevel_ == 1)
		{
			front1_.push_back(iter->totalEquipmentLoad_);
		}
		if (iter->frontLevel_ == 2)
		{
			front2_.push_back(iter->totalEquipmentLoad_);
		}
		if (iter->frontLevel_ == 3)
		{
			front3_.push_back(iter->totalEquipmentLoad_);
		}
		if (iter->frontLevel_ == 4)
		{
			front4_.push_back(iter->totalEquipmentLoad_);
		}
	}
	calculateFrontCrowdingDistance();
}

void Nsga::calculateFrontCrowdingDistance()
{
	uint front1Iter, front2Iter, front3Iter, front4Iter;
	front1Iter = front2Iter = front3Iter = front4Iter = 0;

	for (auto &individual : population_)
	{
		if (individual->frontLevel_ == 1)
		{
			if (front1Iter == 0 || front1Iter == (front1_.size() - 1))
			{
				individual->crowdingDistance_ += maxCrowdingDistance_;
			}
			else
			{
				individual->crowdingDistance_ +=
					float(front1_[front1Iter + 1] - front1_[front1Iter - 1]) / (*(--front1_.end()) - *(front1_.begin()));
			}
			front1Iter++;
		}

		if (individual->frontLevel_ == 2)
		{
			if (front2Iter == 0 || front2Iter == (front2_.size() - 1))
			{
				individual->crowdingDistance_ += maxCrowdingDistance_;
			}
			else
			{
				individual->crowdingDistance_ +=
					float(front2_[front2Iter + 1] - front2_[front2Iter - 1]) / (*(--front2_.end()) - *(front2_.begin()));
			}
			front2Iter++;
		}

		if (individual->frontLevel_ == 3)
		{
			if (front3Iter == 0 || front3Iter == (front3_.size() - 1))
			{
				individual->crowdingDistance_ += maxCrowdingDistance_;
			}
			else
			{
				individual->crowdingDistance_ +=
					float(front3_[front3Iter + 1] - front3_[front3Iter - 1]) / (*(--front3_.end()) - *(front3_.begin()));
			}
			front3Iter++;
		}

		if (individual->frontLevel_ == 4)
		{
			if (front4Iter == 0 || front4Iter == (front4_.size() - 1))
			{
				individual->crowdingDistance_ += maxCrowdingDistance_;
			}
			else
			{
				individual->crowdingDistance_ +=
					float(front4_[front4Iter + 1] - front4_[front4Iter - 1]) / (*(--front4_.end()) - *(front4_.begin()));
			}
			front4Iter++;
		}
	}
	front1_.clear();
	front2_.clear();
	front3_.clear();
	front4_.clear();
}

void Nsga::selectionTournament(std::vector<int> &tournamentIndexVector,
							   std::vector<int> &winnerIndexVector,
							   int iter)
{
	int pos1 = iter;
	int pos2 = iter + 1;
	int individual1 = tournamentIndexVector[pos1];
	int individual2 = tournamentIndexVector[pos2];
	if (population_[individual1]->frontLevel_ < population_[individual2]->frontLevel_)
	{
		winnerIndexVector.push_back(individual1);
		tournamentIndexVector.erase(tournamentIndexVector.begin() + pos1);
		return;
	}

	if (population_[individual2]->frontLevel_ < population_[individual1]->frontLevel_)
	{
		winnerIndexVector.push_back(individual2);
		tournamentIndexVector.erase(tournamentIndexVector.begin() + pos2);
		return;
	}

	if (population_[individual1]->crowdingDistance_ > population_[individual2]->crowdingDistance_)
	{
		winnerIndexVector.push_back(individual1);
		tournamentIndexVector.erase(tournamentIndexVector.begin() + pos1);
		return;
	}

	if (population_[individual2]->crowdingDistance_ > population_[individual1]->crowdingDistance_)
	{
		winnerIndexVector.push_back(individual2);
		tournamentIndexVector.erase(tournamentIndexVector.begin() + pos2);
		return;
	}

	int randomSelection = rand() % 2;
	std::cout << randomSelection << std::endl;
	if (randomSelection == 0)
	{
		winnerIndexVector.push_back(individual1);
		tournamentIndexVector.erase(tournamentIndexVector.begin() + pos1);
		return;
	}
	else
	{
		winnerIndexVector.push_back(individual2);
		tournamentIndexVector.erase(tournamentIndexVector.begin() + pos2);
		return;
	}
}

// sorting methods

void Nsga::sortNonDominated()
{
	for (uint i = 0; i < population_.size(); i++)
	{
		if (population_[i]->dominationCount_ < 0)
		{
			// negative values for individuals that already dominate
			population_[i]->dominationCount_ -= 1;
		}
	}

	for (uint j = 0; j < population_.size(); j++)
	{
		if (population_[j]->dominationCount_ == 0)
		{
			// separating the 1st sorting from the 2nd
			population_[j]->dominationCount_ = -1;
		}
	}

	for (uint k = 0; k < population_.size(); k++)
	{
		if (population_[k]->dominationCount_ == -1)
		{
			for (auto it : population_[k]->dominatedPoints_)
			{
				// for every dominatedPoint, go to corresponding individual and decrement the dominationCount
				population_[it]->dominationCount_ -= 1;
			}
		}
	}
	printDominationValues();
}

void Nsga::sortPopulationByMaxCompletionTime()
{
	std::sort(population_.begin(), population_.end(),
			  [](const IndividualPtr &a, const IndividualPtr &b) -> bool
			  {
				  return a->maxCompletionTime_ < b->maxCompletionTime_;
			  });
}

void Nsga::sortPopulationByTotalEquipmentLoad()
{
	std::sort(population_.begin(), population_.end(),
			  [](const IndividualPtr &a, const IndividualPtr &b) -> bool
			  {
				  return a->totalEquipmentLoad_ < b->totalEquipmentLoad_;
			  });
}

// utility methods

void Nsga::printPopulation()
{
	int itteration = 0;

	std::cout << "\nCurrent population:\n\n";
	for (auto &individual : population_)
	{
		std::cout << "Individual " << std::setw(2) << itteration++ << ": " << individual->getGenesAsSStream().str()
				  << "  Front Level: " << std::setw(2) << individual->frontLevel_
				  << "  Crowding Distance: " << std::setw(10) << individual->crowdingDistance_
				  << "  Partner: " << std::setw(2) << individual->partner_ << std::endl;
	}
	std::cout << "\n";
}

void Nsga::printDurationMap()
{
	std::cout << "DurationMap:\n\n";
	for (auto iter = durationMap_.begin(); iter != durationMap_.end(); iter++)
	{
		std::cout << "Workpieces" << std::setw(2) << iter->first << " Durration: " << iter->second << std::endl;
	}
	std::cout << "\n";
}

void Nsga::printDominationValues()
{
	int i = 0;
	std::cout << "\ndominationValues:\n";
	for (auto &iter : population_)
	{
		std::cout << "Individual " << std::setw(2) << i++
				  << ": domination_count: " << std::setw(2) << iter->dominationCount_ << " dominated_list: ";
		for (auto iter2 : iter->dominatedPoints_)
		{
			std::cout << iter2 << " ";
		}
		std::cout << std::endl;
	}
}

int main()
{
	int n, m;
	int itterations;
	int sampleSize;
	int numberOfProcesses;

	std::cout << "Input N value: ";
	std::cin >> n;

	std::cout << "Input M value: ";
	std::cin >> m;

	std::cout << "Input number of itterations: ";
	std::cin >> itterations;

	std::cout << "Input sampleSize: ";
	std::cin >> sampleSize;

	std::cout << "Input number of processes: ";
	std::cin >> numberOfProcesses;

	std::unique_ptr<Nsga> workshop = std::make_unique<Nsga>(n, m, itterations, sampleSize, numberOfProcesses);
	workshop->run();
	return 0;
}
