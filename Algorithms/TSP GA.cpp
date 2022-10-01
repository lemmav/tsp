// TSP GA.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//
#include <iostream>
#include <vector>
#include <sstream>
#include <ctime>
#include <random>
#include <chrono>
#include <string>
#include "httplib.h"

using namespace std;
using namespace chrono;
default_random_engine eng(time(0));
int N = 1; httplib::Request req; double progress; bool busyornot = false;
using Graph = vector<vector<double>>;
using Way = vector<int>; // они же геномы
struct Creature //особь
{
	Way way;
	double fitness;
	Creature() : way(N - 1, 0), fitness(0) {}
};
using Generation = vector <Creature>; //поколение
Creature bestofthebest; double waylongbestofthebest;

//допустим что все пути по умолчанию начинаются в 0 и заканчиваются в 0
Graph GraphGenerator() {
	Graph graph(N);
	for (int i = 0; i < N; i++) {
		graph[i].resize(N);
	}
	uniform_real_distribution<double> distr(1, 10);
	for (int i = 0; i < N; i++) {
		for (int j = 0; j <= i; j++) {
			if (i == j) {
				graph[i][j] = INFINITY;
				continue;
			}
			graph[i][j] = distr(eng);
			graph[j][i] = distr(eng);
		}
	}
	return graph;
}


void WayShuffling(Creature& creature) {
	for (int i = 0; i < creature.way.size(); i++) {
		creature.way[i] = i + 1;
	}
	shuffle(creature.way.begin(), creature.way.end(), eng);

}
Generation GenerationGenerator(int creaturesnumber) {
	Generation generation;
	for (int i = 0; i < creaturesnumber; i++) {
		Creature creature;
		WayShuffling(creature);
		generation.push_back(creature);
	}
	return generation;
}
Generation GenerationGeneratorFromNeighbour(int creaturesnumber) {
	Generation generation;
	for (int i = 0; i < creaturesnumber; i++) {
		Creature creature;
		WayShuffling(creature);
		generation.push_back(creature);
	}
	return generation;
}
void Fitness(Creature& creature, const Graph& graph) {
	double waylong = 0;
	for (int i = 0; i < creature.way.size() - 1; i++) {
		waylong += graph[creature.way[i]][creature.way[i + 1]];
	}
	waylong += graph[0][creature.way[0]] + graph[creature.way.back()][0];
	creature.fitness = 1 / waylong;

}

bool CreaturesComparing(const Creature& creature1, const Creature& creature2) { return (creature1.fitness > creature2.fitness); }

void FitnessSort(Generation& generation) {
	sort(generation.begin(), generation.end(), CreaturesComparing);
}

Creature TournamentSelection(const Generation& generation, int creaturesnumber) {
	int num, maxindex; double max = 0;
	uniform_int_distribution<int> distr(0, creaturesnumber - 1);
	for (int i = 0; i < 0.1 * creaturesnumber; i++) {
		num = distr(eng);
		if (generation[num].fitness > max) {
			max = generation[num].fitness;
			maxindex = num;
		}
	}
	return generation[maxindex];
}

Creature KidMaker(const Creature& parent1, const Creature& parent2, int startInd, int endInd) {
	Creature kid;
	vector<bool>visited(N - 1, 0);
	for (int i = startInd; i <= endInd; i++)
	{
		kid.way[i] = parent1.way[i];
		visited[parent1.way[i] - 1] = true;
	}
	int j = 0;
	for (int i = 0; i < N - 1; ) {
		if (!visited[parent2.way[i] - 1] && kid.way[j] == 0) {
			kid.way[j] = parent2.way[i];
			i++;
			j++;
		}
		else if (visited[parent2.way[i] - 1])
			i++;
		else if (kid.way[j] != 0)
			j++;
	}

	return kid;
}

void KidsMaker(const Creature& parent1, const Creature& parent2, Generation& generation) {
	uniform_int_distribution<int> distr(0, N - 2); //n-1

	int startInd = distr(eng);  int endInd = distr(eng);
	if (startInd > endInd) swap(startInd, endInd);

	Creature kid1 = KidMaker(parent1, parent2, startInd, endInd);
	Creature kid2 = KidMaker(parent2, parent1, startInd, endInd);
	generation.push_back(kid1);
	generation.push_back(kid2);
}

void Mutation(Creature& creature, double mutation) {
	random_device rd;
	mt19937 gen(rd());
	bernoulli_distribution d(mutation);
	uniform_int_distribution<int> distr(0, N - 2); //N-2

	if (d(gen)) {
		int startInd = distr(eng);  int endInd = distr(eng);
		swap(creature.way[startInd], creature.way[endInd]);
	}
}
Creature ShowMeYourBest(const Generation& generation) {
	int maxindex; double max = 0;
	for (int i = 0; i < generation.size(); i++) {
		if (generation[i].fitness > max) {
			max = generation[i].fitness;
			maxindex = i;
		}
	}
	return generation[maxindex];
}

double Waylong(const Creature& creature, const Graph& graph) {
	double waylong = 0;
	for (int i = 0; i < creature.way.size() - 1; i++) {
		waylong += graph[creature.way[i]][creature.way[i + 1]];
	}
	waylong += graph[0][creature.way[0]] + graph[creature.way.back()][0];
	return waylong;
}
void GraphReader(Graph& graph, int& N, const httplib::Request& req) {
	stringstream gin(req.body);
	gin >> N;
	graph.resize(N);
	for (int i = 0; i < N; i++) {
		graph[i].resize(N);
	}
	double dub;
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			gin >> graph[i][j];
		}
	}
}
double mainresult;

void NGA(double mutation, int creaturesnumber) {
	//double mutation = 0.1;
		//const int creaturesnumber = 1000;

	int count = 0;
	Graph graph; GraphReader(graph, N, req);
	Generation generation = GenerationGenerator(creaturesnumber);
	bestofthebest = generation[0];
	waylongbestofthebest = Waylong(bestofthebest, graph);
	while (count <= 100) {
		progress = count / 100.0;
		for (int i = 0; i < generation.size(); i++)
			Fitness(generation[i], graph);

		Creature ruthebest = ShowMeYourBest(generation);
		double waylongruthebest = Waylong(ruthebest, graph);

		if (waylongruthebest < waylongbestofthebest) {
			bestofthebest = ruthebest;
			waylongbestofthebest = waylongruthebest;
		}

		cout << Waylong(ruthebest, graph) << endl;

		Generation newGeneration;
		while (newGeneration.size() < creaturesnumber) {
			Creature parent1 = TournamentSelection(generation, creaturesnumber);
			Creature parent2 = TournamentSelection(generation, creaturesnumber);
			KidsMaker(parent1, parent2, newGeneration);
		}

		for (int i = 0; i < newGeneration.size(); i++)
			Mutation(newGeneration[i], mutation);

		generation = newGeneration;
		count++;

	}

}
std::condition_variable NGATcond;
void NGAThread(double mutation, int creaturesnumber)
{
	std::mutex mtx;
	std::unique_lock<std::mutex> lock{ mtx };
	while (true)
	{
		NGATcond.wait(lock);
		busyornot = true;
		NGA(mutation, creaturesnumber);
		busyornot = false;
	}
}

int main()
{
	httplib::Server svr;
	double mutation = 0.1;
	const int creaturesnumber = 1000;
	thread NGAT(NGAThread, mutation, creaturesnumber);
	//svr.Get("/hi", [](const httplib::Request&, httplib::Response& res) {
	//	res.set_content("Hello World!", "text/plain");
	//	});
	svr.Post("/graph", [&](const auto& request, auto& res) {
		if (busyornot == false) {
			req = request;
			NGATcond.notify_one();
		}
		else cout << "WE'RE BUSY NOW, YOU STUPID DUMBASS\n";
		});
	svr.Get("/progress", [](const httplib::Request&, httplib::Response& res) {
		string stringprogress = to_string(progress);
		res.set_content(stringprogress, "text/plain");
		});
	svr.Get("/result", [](const httplib::Request&, httplib::Response& res) {
		if (progress < 1) {
			res.set_content("0", "text/plain");
		}
		else {
			string stringresult = "1\n";
			stringresult += "0 ";
			for (int i = 0; i < bestofthebest.way.size(); i++) {
				stringresult += to_string(bestofthebest.way[i]) + " ";
			}
			stringresult += " 0";
			stringresult += "\n" + to_string(waylongbestofthebest);


			res.set_content(stringresult, "text/plain");
		}

		});
	svr.listen("0.0.0.0", 5002);

	//ofstream filetime("TSPGABNNS_time-N-ANOTHERONE.txt");
	//ofstream filelongway("TSPGABNNS_longway-N-ANOTHERONE.txt");
	cout << "what a dumbass";




}

// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.