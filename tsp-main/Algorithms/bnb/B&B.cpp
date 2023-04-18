// BranchAndBound.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include <vector>
#include <queue>
#include <ctime>
#include <random>
#include <chrono>
#include <string>
#include <sstream>
#include "httplib.h"
using namespace std;
using namespace chrono;
string mainstr = "C:\\Users\\asus\\Desktop\\petya\\Coursework\\Coursework\\graphs\\";
default_random_engine eng(time(0));

using Graph = vector<vector<double>>;
using Way = vector<int>;
using Direction = pair<int, int>;
int N = 1; httplib::Request req; double progress = 0; bool busyornot = false; // количество городов

struct HistoryNode {
	int from;
	int to;
	bool accepted;
	HistoryNode(Direction dir, bool accepted) : from(dir.first), to(dir.second), accepted(accepted) {}
};
using History = vector<HistoryNode>;

struct TreeNode {
	History history;
	Way way;
	int waysCounter;
	double cost;
	bool operator<(const TreeNode& tn) const
	{
		return cost > tn.cost;
	}
	TreeNode() : history(), way(), waysCounter(0), cost(0) {}
	TreeNode(int N, double cost) : history(), way(N, -1), waysCounter(0), cost(cost) {}
};

priority_queue<TreeNode> Nodes;

/*Graph GraphGenerator() {
	Graph graph(N);
	for (int i = 0; i < N; i++) {
		graph[i].resize(N);
	}
	uniform_real_distribution<double> distr(1, 10);
	for (int i = 0; i < N; i++) {
		for (int j = 0; j <= i; j++) {
			if (i == j) {
				graph[i][j] = INFINITY;
				graph[j][i] = INFINITY;
				continue;
			}
			graph[i][j] = distr(eng);
			graph[j][i] = distr(eng);
		}
	}
	return graph;
}*/
//Graph GraphGenerator() {
//	Graph graph;
//	graph.push_back({ INFINITY,20,18,12,8 });
//	graph.push_back({ 5,INFINITY,14,7,11 });
//	graph.push_back({ 12,18,INFINITY,6,11 });
//	graph.push_back({ 11,17,11,INFINITY,12 });
//	graph.push_back({ 5,5,5,5,INFINITY });
//	return graph;
//	}

void ReductionDi(Graph& graph, double& di) {
	double min = INFINITY;
	for (int i = 0; i < N; i++) {
		min = INFINITY;
		for (int j = 0; j < N; j++) {
			if (graph[i][j] < min)
				min = graph[i][j];
		}
		if (min != 0 && min != INFINITY) {
			di += min;
			for (int j = 0; j < N; j++) {
				graph[i][j] -= min;
			}
		}
	}
}

void ReductionDj(Graph& graph, double& dj) {
	double min = INFINITY;
	for (int j = 0; j < N; j++) {
		min = INFINITY;
		for (int i = 0; i < N; i++) {
			if (graph[i][j] < min)
				min = graph[i][j];
		}
		if (min != 0 && min != INFINITY) {
			dj += min;
			for (int i = 0; i < N; i++) {
				graph[i][j] -= min;
			}
		}
	}
}

double Assessment(const Graph& graph, const int i, const int j) {
	double min = INFINITY, sum = 0;
	for (int pj = 0; pj < N; pj++) {
		if (graph[i][pj] < min && pj != j)
			min = graph[i][pj];
	}
	sum += min;
	min = INFINITY;
	for (int pi = 0; pi < N; pi++) {
		if (graph[pi][j] < min && pi != i)
			min = graph[pi][j];
	}
	sum += min;
	return sum;
}
Direction MaxCostForRejection(const Graph& graph, double& maxAssessment) { //выбрали в матрице нулевую клетку с наибольшей стоимостью
	maxAssessment = 0; int maxi, maxj; double assessment; Direction direction;
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			if (graph[i][j] == 0) {
				assessment = Assessment(graph, i, j);
				if (assessment == INFINITY) {
					direction.first = i; direction.second = j;
					maxAssessment = INFINITY;
					return direction;
				}
				if (assessment >= maxAssessment) {
					maxAssessment = assessment;
					maxi = i;
					maxj = j;
				}
			}
		}
	}
	direction.first = maxi; direction.second = maxj;
	return direction;
}

void CrossOut(Graph& graph, const Direction& direction) {
	graph[direction.first][direction.second] = INFINITY;
	graph[direction.second][direction.first] = INFINITY;
	for (int k = 0; k < N; k++) {
		graph[k][direction.second] = INFINITY;
		graph[direction.first][k] = INFINITY;
	}

}


Graph ConstructGraph(const Graph& graph, const History& history) {
	Graph result = graph;
	for (const auto& historyNode : history) {
		if (historyNode.accepted) {
			CrossOut(result, make_pair(historyNode.from, historyNode.to));
		}
		else {
			result[historyNode.from][historyNode.to] = INFINITY;
		}

		double di = 0, dj = 0;
		ReductionDi(result, di);
		ReductionDj(result, dj);
	}

	return result;
}

void Handle(const TreeNode& node, const Graph& originalGraph) {
	TreeNode yesNode = node, noNode = node;

	double maxAssessment; double di = 0, dj = 0;
	Graph graph = ConstructGraph(originalGraph, node.history);

	Direction zeroMaxAssessment = MaxCostForRejection(graph, maxAssessment);
	CrossOut(graph, zeroMaxAssessment);
	ReductionDi(graph, di);
	ReductionDj(graph, dj);
	yesNode.cost = di + dj + yesNode.cost;
	yesNode.way[zeroMaxAssessment.first] = zeroMaxAssessment.second;
	yesNode.waysCounter++;
	yesNode.history.emplace_back(zeroMaxAssessment, true);
	Nodes.push(yesNode);

	//maxAssessment = Assessment(graph, zeroMaxAssessment.first, zeroMaxAssessment.second);
	noNode.cost = maxAssessment + noNode.cost;
	noNode.history.emplace_back(zeroMaxAssessment, false);
	Nodes.push(noNode);


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
			gin >> dub;
			if (i == j) dub = INFINITY;
			graph[i][j] = dub;
		}
	}
}

double mainresult;
TreeNode temporaryNode;
void NGA() {
	progress = 0.0;

	int count = 0;
	double di = 0, dj = 0;
	Graph graph;
	GraphReader(graph, N, req);
	ReductionDi(graph, di);
	ReductionDj(graph, dj);
	double lowerBound = di + dj;
	Nodes.emplace(N, lowerBound);


	while (!Nodes.empty()) {

		cout << "progress: " << progress << endl;
		temporaryNode = Nodes.top();
		double newprogress = temporaryNode.waysCounter / (N * 1.0);
		if (newprogress > progress) progress = newprogress;
		if (temporaryNode.waysCounter == N)
			break;

		Nodes.pop();
		Handle(temporaryNode, graph);

	}

	cout << temporaryNode.cost;
	cout << endl;
	for (int i = 0; i < N; i++) {
		cout << temporaryNode.way[i] << "   ";
	}
	Nodes = priority_queue<TreeNode>(); // Почистить Nodes, потому что она глобальная переменная и ее никто не чистил

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

string WayToString(Way w) {
	string result = "0 ";
	int current = w[0];
	while (current != 0) {
		result += to_string(current) + " ";
		current = w[current];
	}
	result += "0";
	return result;
}

int main()
{

	httplib::Server svr;

	thread NGAT(NGAThread);

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
			string stringresult = "1\n" + WayToString(temporaryNode.way);
			stringresult += "\n" + to_string(temporaryNode.cost);


			res.set_content(stringresult, "text/plain");
		}

		});
	svr.listen("0.0.0.0", 5003);









}





