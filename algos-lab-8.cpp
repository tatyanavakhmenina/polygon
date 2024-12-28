#include <iostream>  
#include <vector>    
#include <string>    
#include <sstream>  
#include <fstream>   
#include <unordered_map> 
#include <unordered_set>  
#include <queue>     
#include <cmath>     
#include <limits>   
#include <cassert>   

using namespace std;

// Узел графа
struct Node {
    double lon, lat;                          // 16 байт (8 байт для double * 2)
    vector<pair<Node*, double>> neighbors;    // O(E) памяти, каждый элемент занимает 16 байт (8 байт для указателя + 8 байт для веса)
};

// Класс для представления графа
class Graph {
public:
    vector<Node*> nodes;                      // O(V) памяти, где каждый элемент занимает 8 байт
    unordered_map<string, Node*> nodeMap;     // O(V) памяти, средний размер строки — 32 байта, указатель 8 байт

    // Загрузка графа из файла
    void loadFromFile(const string& filename) {
        ifstream file(filename);              // 8 байт для файлового потока
        if (!file.is_open()) {
            cerr << "Ошибка открытия файла: " << filename << endl;
            return;
        }

        string line;                          // 32 байта для строки
        while (getline(file, line)) {         // O(V + E)
            stringstream ss(line);           // 8 байт для потока строки
            string parentStr, edgeStr;       // 64 байта (по 32 байта на каждую строку)

            getline(ss, parentStr, ':');     // O(1)
            double parentLon, parentLat;     // 16 байт (2 * double)
            sscanf(parentStr.c_str(), "%lf,%lf", &parentLon, &parentLat); // O(1)
            Node* parent = getNode(parentLon, parentLat, toKey(parentLon, parentLat)); // O(1)

            while (getline(ss, edgeStr, ';')) { // O(E)
                double childLon, childLat, weight; // 24 байта (3 * double)
                sscanf(edgeStr.c_str(), "%lf,%lf,%lf", &childLon, &childLat, &weight); // O(1)
                Node* child = getNode(childLon, childLat, toKey(childLon, childLat)); // O(1)

                parent->neighbors.emplace_back(child, weight); // 16 байт на каждую пару
            }
        }

        file.close(); // O(1)
    }

    // Поиск ближайшего узла к заданной точке
    Node* findClosestNode(double lat, double lon) {
        if (nodes.empty()) {
            cerr << "Граф пуст!" << endl;
            return nullptr;
        }

        Node* closest = nullptr;             // 8 байт для указателя
        double minDist = numeric_limits<double>::infinity(); // 8 байт для double

        for (auto* node : nodes) {           // O(V)
            double dist = sqrt(pow(node->lat - lat, 2) + pow(node->lon - lon, 2)); // 8 байт для double
            if (dist < minDist) {            // O(1)
                minDist = dist;              // O(1)
                closest = node;              // O(1)
            }
        }

        return closest; // O(1)
    }

private:
    // Получить узел из хэш-таблицы или создать новый
    Node* getNode(double lon, double lat, const string& key) {
        if (nodeMap.find(key) == nodeMap.end()) { // O(1)
            Node* newNode = new Node{lon, lat};  // 16 байт для нового узла
            nodes.push_back(newNode);           // 8 байт на указатель
            nodeMap[key] = newNode;             // 8 байт для записи в хэш-таблицу
        }
        return nodeMap[key]; // O(1)
    }

    // Генерация ключа для узла на основе координат
    string toKey(double lon, double lat) {
        return to_string(lon) + "," + to_string(lat); // 64 байта (два числа по 32 байта)
    }
};

// Реализация DFS (Поиск в глубину)
bool dfs(Node* current, Node* goal, unordered_set<Node*>& visited, double& distance) {
    if (current == goal) return true;       // O(1)

    visited.insert(current);               // 8 байт на указатель

    for (auto& neighbor : current->neighbors) { // O(E)
        if (visited.find(neighbor.first) == visited.end()) { // O(1)
            distance += neighbor.second;   // 8 байт для double
            if (dfs(neighbor.first, goal, visited, distance)) { // Рекурсивный вызов
                return true;               // O(1)
            }
            distance -= neighbor.second;   // Откат изменения (O(1))
        }
    }

    return false;                          // Если путь не найден, возвращаем false (O(1))
}

// Реализация BFS (Поиск в ширину)
double bfs(Node* start, Node* goal) {
    if (!start || !goal) {                 // Проверка входных данных (O(1))
        cerr << "Начальный или конечный узел отсутствует!" << endl;
        return -1.0;
    }

    queue<pair<Node*, double>> q;          // 16 байт на пару, максимум O(V) пар
    unordered_set<Node*> visited;          // O(V) для хранения узлов

    q.push({start, 0.0});                  // O(1)
    visited.insert(start);                 // O(1)

    while (!q.empty()) {                   // O(V + E)
        auto front = q.front();            // 16 байт на пару
        Node* current = front.first;       // 8 байт на указатель
        double dist = front.second;        // 8 байт для double
        q.pop();                           // O(1)

        if (current == goal) return dist;  // Если достигли цели, возвращаем расстояние (O(1))

        for (auto& neighbor : current->neighbors) { // O(E)
            if (!visited.count(neighbor.first)) {   // O(1)
                visited.insert(neighbor.first);     // O(1)
                q.push({neighbor.first, dist + neighbor.second}); // 16 байт на новую пару
            }
        }
    }

    return -1.0;                           // Если путь не найден (O(1))
}

// Реализация алгоритма Дейкстры
double dijkstra(Node* start, Node* goal, const vector<Node*>& nodes) {
    if (!start || !goal) {                 // Проверка входных данных (O(1))
        cerr << "Начальный или конечный узел отсутствует!" << endl;
        return -1.0;
    }

    unordered_map<Node*, double> distances; // O(V), каждый элемент 16 байт (узел + расстояние)
    for (auto* node : nodes) distances[node] = numeric_limits<double>::infinity(); // O(V)
    distances[start] = 0.0;                 // O(1)

    priority_queue<pair<double, Node*>, vector<pair<double, Node*>>, greater<>> pq; // O(V), каждый элемент 16 байт
    pq.push({0.0, start});                  // O(log(V))

    while (!pq.empty()) {                   // O((V + E) * log(V))
        auto top = pq.top();                // 16 байт на пару
        double currentDist = top.first;     // 8 байт
                Node* currentNode = top.second;      // 8 байт для указателя
        pq.pop();                            // O(log(V))

        if (currentNode == goal) return currentDist; // O(1)

        for (auto& neighbor : currentNode->neighbors) { // O(E)
            double newDist = currentDist + neighbor.second; // 8 байт для нового расстояния
            if (newDist < distances[neighbor.first]) {      // O(1)
                distances[neighbor.first] = newDist;        // Обновление значения в unordered_map
                pq.push({newDist, neighbor.first});         // 16 байт на новую пару
            }
        }
    }

    return -1.0;                           // Если путь не найден, возвращаем -1
}

// Реализация алгоритма A*
double aStar(Node* start, Node* goal, const vector<Node*>& nodes) {
    if (!start || !goal) {                 // Проверка входных данных (O(1))
        cerr << "Начальный или конечный узел отсутствует!" << endl;
        return -1.0;
    }

    auto heuristic = [](Node* a, Node* b) { // Лямбда-функция для эвристики
        return sqrt(pow(a->lat - b->lat, 2) + pow(a->lon - b->lon, 2)); // 8 байт для double
    };

    unordered_map<Node*, double> gScore, fScore; // O(V), каждый элемент 16 байт
    for (auto* node : nodes) {
        gScore[node] = numeric_limits<double>::infinity(); // O(V)
        fScore[node] = numeric_limits<double>::infinity(); // O(V)
    }
    gScore[start] = 0.0;                     // O(1)
    fScore[start] = heuristic(start, goal);  // O(1)

    priority_queue<pair<double, Node*>, vector<pair<double, Node*>>, greater<>> pq; // O(V), каждый элемент 16 байт
    pq.push({fScore[start], start});         // O(log(V))

    while (!pq.empty()) {                    // O((V + E) * log(V))
        auto top = pq.top();                 // 16 байт на пару
        double currentF = top.first;         // 8 байт
        Node* current = top.second;          // 8 байт
        pq.pop();                            // O(log(V))

        if (current == goal) return gScore[current]; // O(1)

        for (auto& neighbor : current->neighbors) {  // O(E)
            double tentativeG = gScore[current] + neighbor.second; // 8 байт
            if (tentativeG < gScore[neighbor.first]) {             // O(1)
                gScore[neighbor.first] = tentativeG;               // Обновление значения
                fScore[neighbor.first] = tentativeG + heuristic(neighbor.first, goal); // O(1)
                pq.push({fScore[neighbor.first], neighbor.first}); // 16 байт на новую пару
            }
        }
    }

    return -1.0;                             // Если путь не найден, возвращаем -1
}

// Тесты
// Тесты
void testLoadFromFile(Graph& graph) {
    graph.loadFromFile("spb_graph.txt");
    assert(!graph.nodes.empty());
    assert(!graph.nodeMap.empty());
    assert(graph.nodes.size() == graph.nodeMap.size());
    cout << "testLoadFromFile passed." << endl;
}

void testFindClosestNode(Graph& graph) {
    Node* closest = graph.findClosestNode(59.884972, 30.368072);
    assert(closest != nullptr);
    assert(abs(closest->lat - 59.884972) < 0.1);
    assert(abs(closest->lon - 30.368072) < 0.1);
    cout << "testFindClosestNode passed." << endl;
}

void testDFS(Graph& graph) {
    Node* start = graph.findClosestNode(59.884972, 30.368072);
    Node* goal = graph.findClosestNode(59.956248, 30.309215);

    assert(start != nullptr);
    assert(goal != nullptr);

    unordered_set<Node*> visited;
    double distance = 0.0;

    bool found = dfs(start, goal, visited, distance);
    assert(found);
    assert(distance > 0.0);
    cout << "testDFS passed." << endl;
}

void testBFS(Graph& graph) {
    Node* start = graph.findClosestNode(59.884972, 30.368072);
    Node* goal = graph.findClosestNode(59.956248, 30.309215);

    assert(start != nullptr);
    assert(goal != nullptr);

    double distance = bfs(start, goal);
    assert(distance > 0.0);
    cout << "testBFS passed." << endl;
}

void testDijkstra(Graph& graph) {
    Node* start = graph.findClosestNode(59.884972, 30.368072);
    Node* goal = graph.findClosestNode(59.956248, 30.309215);

    assert(start != nullptr);
    assert(goal != nullptr);

    double distance = dijkstra(start, goal, graph.nodes);
    assert(distance > 0.0);
    cout << "testDijkstra passed." << endl;
}

void testAStar(Graph& graph) {
    Node* start = graph.findClosestNode(59.884972, 30.368072);
    Node* goal = graph.findClosestNode(59.956248, 30.309215);

    assert(start != nullptr);
    assert(goal != nullptr);

    double distance = aStar(start, goal, graph.nodes);
    assert(distance > 0.0);
    cout << "testAStar passed." << endl;
}

int main() {
    Graph graph;
    testLoadFromFile(graph);
    testFindClosestNode(graph);
    testDFS(graph);
    testBFS(graph);
    testDijkstra(graph);
    testAStar(graph);
    cout << "All tests passed." << endl;
    return 0;
}
