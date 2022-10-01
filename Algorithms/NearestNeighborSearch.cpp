// NearestNeighborSearch.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
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

using Graph = vector<vector<double>>;
using Visit = vector <bool>;

int N = 1; httplib::Request req; double progress; bool busyornot = false; int counter = 2;

//Graph NNSGraphGenerator() {
//	Graph graph(N);
//	for (int i = 0; i < graph.size(); i++) {
//		graph[i].resize(N);
//	}
//	uniform_real_distribution<double> distr(1, 10);
//	for (int i = 0; i < graph.size(); i++) {
//		for (int j = 0; j <= i; j++) {
//			if (i == j) {
//				graph[i][j] = 0;
//				graph[j][i] = 0;
//				continue;
//			}
//			double dist = distr(eng);
//			graph[i][j] = dist;
//			graph[j][i] = graph[i][j];
//		}
//	}
//	return graph;
//}
Visit NNSVisitGenerator() {
	Visit visit(N);
	for (int i = 0; i < N; i++)
		visit[i] = false;
	return visit;
}
void NNSGraphPrinter(const Graph& graph) {
	for (int i = 0; i < graph.size(); i++) {
		for (int j = 0; j < graph.size(); j++)
			cout << graph[i][j] << " ";
		cout << endl;
	}
	cout << endl;
}
void NNSVisitPrinter(Visit visit) {
	for (int i = 0; i < visit.size(); i++)
		cout << visit[i] << " ";
	cout << endl;
}

int NNSNearestWayFinder(const Graph& graph, int index, double& wayLength, Visit& visit) {
	double minimalWay = INFINITY;
	int minimalWayIndex = 0;
	for (int i = 0; i < N; i++)
		if (graph[index][i] < minimalWay && visit[i] == false) {
			minimalWay = graph[index][i];
			minimalWayIndex = i;
		}
	wayLength += minimalWay;
	return (minimalWayIndex);
}
vector<int> Visited;
double NNSTravelingSalesmanNeighbour(const Graph& graph, Visit& visit) {
	int visitedCounter = 1;
	int currentIndex = 0, nearestIndex = 0;  double wayLength = 0;
	visit[0] = true; Visited.push_back(0);
	while (visitedCounter != N) {
		progress = counter / (N * 1.0);
		nearestIndex = NNSNearestWayFinder(graph, nearestIndex, wayLength, visit);
		visitedCounter++;
		visit[nearestIndex] = true;
		Visited.push_back(nearestIndex);
		counter++;

	}
	wayLength += graph[0][nearestIndex];
	return wayLength;
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
void NGA() {


	Graph graph; GraphReader(graph, N, req);
	Visit visit = NNSVisitGenerator();
	mainresult = NNSTravelingSalesmanNeighbour(graph, visit);
	cout << mainresult << endl;
	//NNSVisitPrinter(visit);
	for (int i = 0; i < Visited.size(); i++)
		cout << Visited[i] << " ";


}
std::condition_variable NGATcond;
void NGAThread()
{
	std::mutex mtx;
	std::unique_lock<std::mutex> lock{ mtx };
	while (true)
	{
		NGATcond.wait(lock);
		busyornot = true;
		NGA();
		busyornot = false;
	}
}
int main() {

	httplib::Server svr;

	thread NGAT(NGAThread);
	//svr.Get("/hi", [](const httplib::Request&, httplib::Response& res) {
	//	res.set_content("Hello World!", "text/plain");
	//	});
	svr.Post("/graph", [&](const auto& request, auto& res) {
		if (busyornot == false) {
			req = request;
			NGATcond.notify_one();
		}
		else cout << "WE'RE BUSY NOW\n";
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
			for (int i = 0; i < Visited.size(); i++) {
				stringresult += to_string(Visited[i]) + " ";
			}
			stringresult += " 0";
			stringresult += "\n" + to_string(mainresult);


			res.set_content(stringresult, "text/plain");
		}

		});
	svr.listen("0.0.0.0", 5004);




}