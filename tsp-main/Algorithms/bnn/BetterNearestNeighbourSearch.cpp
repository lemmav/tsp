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
int N = 1; httplib::Request req; double progress; bool busyornot = false;

string mainstr = "C:\\Users\\asus\\Desktop\\petya\\Coursework\\Coursework\\graphs\\";
default_random_engine eng(time(0));


using Graph = vector<vector<double>>;
using Visit = vector <bool>;
using VisitedPoints = vector<int>;

Graph BNNSGraphGenerator() {
	Graph graph(N);
	for (int i = 0; i < graph.size(); i++) {
		graph[i].resize(N);
	}
	uniform_real_distribution<double> distr(1, 10);
	for (int i = 0; i < graph.size(); i++) {
		for (int j = 0; j <= i; j++) {
			if (i == j) {
				graph[i][j] = 0;
				graph[j][i] = 0;
				continue;
			}
			double dist = distr(eng);
			graph[i][j] = dist;
			graph[j][i] = graph[i][j];
		}
	}
	return graph;
}
Visit BNNSVisitGenerator() {
	Visit visit(N);
	for (int i = 0; i < N; i++)
		visit[i] = false;
	return visit;
}
void BNNSGraphPrinter(Graph graph) {
	for (int i = 0; i < graph.size(); i++) {
		for (int j = 0; j < graph.size(); j++)
			cout << graph[i][j] << " ";
		cout << endl;
	}
	cout << endl;
}
void BNNSVisitPrinter(Visit visit) {
	for (int i = 0; i < visit.size(); i++)
		cout << visit[i] << " ";
	cout << endl;
}

int BNNSNearestWayFinder(Graph graph, int index, double& wayLength, Visit& visit) {
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

double BNNSTravelingSalesmanNeighbour(Graph& graph, Visit& visit, int i, VisitedPoints& Visited) {

	int visitedCounter = 1;
	int nearestIndex = i;  double wayLength = 0;
	visit[i] = true; Visited.push_back(i);
	while (visitedCounter != N) {
		nearestIndex = BNNSNearestWayFinder(graph, nearestIndex, wayLength, visit);
		visitedCounter++;
		visit[nearestIndex] = true; Visited.push_back(nearestIndex);
	}
	wayLength += graph[i][nearestIndex];
	return wayLength;
}

pair < VisitedPoints, double> BNNSFinder(Graph& graph, Visit& visit) {
	pair <VisitedPoints, double> result;
	VisitedPoints minVisited;
	double minWaylen = INFINITY;
	for (int i = 0; i < N; i++) {
		progress = (i + 1) / (N * 1.0);
		visit = BNNSVisitGenerator();
		VisitedPoints Visited;
		double wayLen = BNNSTravelingSalesmanNeighbour(graph, visit, i, Visited);
		if (wayLen < minWaylen) {
			minWaylen = wayLen;
			minVisited = Visited;
		}
	}
	return make_pair(minVisited, minWaylen);
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
pair <VisitedPoints, double> mainresult;
void NGA() {

	Graph graph; GraphReader(graph, N, req);
	Visit visit = BNNSVisitGenerator();

	mainresult = BNNSFinder(graph, visit);
	cout << mainresult.second << endl;
	for (int i = 0; i < mainresult.first.size(); i++)
		cout << mainresult.first[i] << " ";

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
			for (int i = 0; i < mainresult.first.size(); i++) {
				stringresult += to_string(mainresult.first[i]) + " ";
			}
			stringresult += " 0";
			stringresult += "\n" + to_string(mainresult.second);


			res.set_content(stringresult, "text/plain");
		}

		});
	svr.listen("0.0.0.0", 5005);


}