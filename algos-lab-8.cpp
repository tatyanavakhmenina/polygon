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
#include <chrono>

using namespace std;

// Узел графа
struct Node {
    double lon, lat;                          // Координаты узла (O(1) по памяти на узел) (2 переменные double по 8 байт каждая, итого 16 байт)
    vector<pair<Node*, double>> neighbors;    // Список соседей с весами рёбер (O(E) памяти, где E — количество рёбер для узла)  std::vector занимает 24 байта пустым, + 16 байт на каждый элемент)
};

// Класс для представления графа
class Graph {
public:
    vector<Node*> nodes; // Список всех узлов (O(V) по памяти, где V — количество узлов) (std::vector занимает 24 байта пустым, + 8 байт на каждый элемент Node*)
    unordered_map<string, Node*> nodeMap; // Хэш-таблица для быстрого доступа к узлам по ключу (O(V) по памяти) (std::unordered_map занимает ~56 байт пустым, + 32 байта на ключ std::string + 8 байт на Node*)

    // Загрузка графа из файла
    void loadFromFile(const string& filename) {
        ifstream file(filename); // Открытие файла (O(1) по памяти)
        if (!file.is_open()) {
            cerr << "Ошибка открытия файла: " << filename << endl;
            return;
        }

        string line; //(std::string ~32 байта пустым)
        while (getline(file, line)) { // Проходим по каждой строке файла (O(V + E) по времени)
            stringstream ss(line); // Поток для обработки строки (std::stringstream ~40 байт пустым)
            string parentStr, edgeStr; //  (2 std::string по ~32 байта каждая пустыми)

            getline(ss, parentStr, ':');
            double parentLon, parentLat; // Переменные для координат (2 переменные double по 8 байт каждая, итого 16 байт)
            sscanf(parentStr.c_str(), "%lf,%lf", &parentLon, &parentLat);
            Node* parent = getNode(parentLon, parentLat, parentStr);  // Указатель на узел (8 байт)

            while (getline(ss, edgeStr, ';')) {
                double childLon, childLat, weight; // Переменные для координат и веса (3 double по 8 байт каждая, итого 24 байта)
                sscanf(edgeStr.c_str(), "%lf,%lf,%lf", &childLon, &childLat, &weight);
                Node* child = getNode(childLon, childLat, toKey(childLon, childLat));// Указатель на узел (8 байт)

                parent->neighbors.emplace_back(child, weight); // Добавляем ребро (O(1)) (std::pair<Node*, double> занимает 16 байт: 8 байт на Node* и 8 байт на double)
            }
        }

        file.close(); // Закрытие файла (O(1))
    }

    // Поиск ближайшего узла к заданной точке
    Node* findClosestNode(double lat, double lon) {
        Node* closest = nullptr; // Указатель на узел (8 байт)
        double minDist = numeric_limits<double>::infinity(); // Минимальное расстояние (double 8 байт)

        for (auto* node : nodes) { // Перебор всех узлов графа (O(V) по времени) Перебор всех узлов графа (каждый указатель Node* 8 байт)
            double dist = sqrt(pow(node->lat - lat, 2) + pow(node->lon - lon, 2)); // Вычисление расстояния (O(1))
            if (dist < minDist) { // Сравнение текущего расстояния с минимальным (O(1))
                minDist = dist; // Обновление минимального расстояния (double 8 байт)
                closest = node; // Обновление ближайшего узла (8 байт)
            }
        }

        return closest; // Возвращаем ближайший узел (O(1))
    }

private:
    // Получить узел из хэш-таблицы или создать новый
    Node* getNode(double lon, double lat, const string& key) {
        if (nodeMap.find(key) == nodeMap.end()) { // Проверка наличия узла в хэш-таблице (O(1))
            Node* newNode = new Node{lon, lat}; // Создание нового узла (O(1) по памяти) Создание нового узла (sizeof(Node) = 24 байта + память для вектора)
            nodes.push_back(newNode);          // Добавление в список узлов (O(1))  Добавление в список узлов (8 байт на указатель Node*)
            nodeMap[key] = newNode;            // Добавление в хэш-таблицу (O(1)) Добавление в хэш-таблицу (32 байта на ключ + 8 байт на Node*)
        }
        }
        return nodeMap[key]; // Возвращаем узел (O(1))
    }

    // Генерация ключа для узла на основе координат
    string toKey(double lon, double lat) {
        return to_string(lon) + "," + to_string(lat); // Преобразование координат в строку (O(1))
    }
};

// Реализация DFS (Поиск в глубину)
bool dfs(Node* current, Node* goal, unordered_set<Node*>& visited, double& distance) {
    if (current == goal) return true; // Если достигли цели, возвращаем true (O(1))

    visited.insert(current); // Помечаем текущий узел как посещённый (O(1))

    for (auto& neighbor : current->neighbors) { // Перебор соседей узла (O(E) в худшем случае)
        if (visited.find(neighbor.first) == visited.end()) { // Проверяем, посещён ли сосед (O(1))
            distance += neighbor.second; // Добавляем вес ребра к общему расстоянию (O(1))
            if (dfs(neighbor.first, goal, visited, distance)) { // Рекурсивный вызов для соседнего узла
                return true; // Если путь найден, возвращаем true
            }
distance -= neighbor.second; // Откатываем расстояние, если путь не найден (O(1))
        }
    }

    return false; // Если путь не найден, возвращаем false
}

// Реализация BFS (Поиск в ширину)
double bfs(Node* start, Node* goal) {
    queue<pair<Node*, double>> q; // Очередь для BFS (O(V) памяти в худшем случае)  (std::queue 24 байта + std::pair<Node*, double> 16 байт)
    unordered_set<Node*> visited; // Множество посещённых узлов (O(V) памяти) (std::unordered_set ~56 байт пустым, + 8 байт на Node*)

    q.push({start, 0.0}); // Добавляем начальный узел в очередь (O(1))
    visited.insert(start); // Помечаем начальный узел как посещённый (O(1))

    while (!q.empty()) { // Пока очередь не пуста (O(V + E) по времени)
        auto [current, dist] = q.front(); // Получаем узел из начала очереди (O(1))  (std::pair<Node*, double> 16 байт)
        q.pop(); // Удаляем узел из очереди (O(1)) (std::pair<double, Node*> 16 байт)

        if (current == goal) return dist; // Если достигли цели, возвращаем расстояние (O(1))

        for (auto& neighbor : current->neighbors) { // Перебираем соседей узла (O(E))
            if (!visited.count(neighbor.first)) { // Проверяем, посещён ли сосед (O(1))
                visited.insert(neighbor.first); // Помечаем соседа как посещённого (O(1))
                q.push({neighbor.first, dist + neighbor.second}); // Добавляем соседа в очередь (O(1))
            }
        }
    }

    return -1.0; // Если путь не найден, возвращаем -1 (O(1))
}

// Реализация алгоритма Дейкстры
double dijkstra(Node* start, Node* goal, const vector<Node*>& nodes) {
    unordered_map<Node*, double> distances; // Словарь расстояний (O(V) памяти)
    for (auto* node : nodes) distances[node] = numeric_limits<double>::infinity(); // Инициализация расстояний (O(V))
    distances[start] = 0.0; // Расстояние до начального узла равно 0 (O(1))

    priority_queue<pair<double, Node*>, vector<pair<double, Node*>>, greater<>> pq; // Очередь с приоритетом (O(V)) (~24 байта пустой объект)
    pq.push({0.0, start}); // Добавляем начальный узел в очередь (O(log(V)))

    while (!pq.empty()) { // Пока очередь не пуста (O((V + E) * log(V)))
        auto [currentDist, currentNode] = pq.top(); // Получаем узел с минимальным расстоянием (O(1)) (std::pair<double, Node*> 16 байт)
        pq.pop(); // Удаляем узел из очереди (O(log(V)))

        if (currentNode == goal) return currentDist; // Если достигли цели, возвращаем расстояние (O(1)) (double 8 байт)

        for (auto& [neighbor, weight] : currentNode->neighbors) { // Перебираем соседей узла (O(E)) (std::pair<Node*, double> 16 байт)
            double newDist = currentDist + weight; // Вычисляем новое расстояние (O(1))
            if (newDist < distances[neighbor]) { // Если расстояние меньше текущего, обновляем (O(1))
                distances[neighbor] = newDist; // Обновляем расстояние (O(1))
                pq.push({newDist, neighbor}); // Добавляем узел в очередь (O(log(V)))
            }
        }
    }

    return -1.0; // Если путь не найден, возвращаем -1 (O(1))
}

// Реализация алгоритма A*
double aStar(Node* start, Node* goal, const vector<Node*>& nodes) {
    auto heuristic = [](Node* a, Node* b) { // Эвристика для A* (O(1))
        return sqrt(pow(a->lat - b->lat, 2) + pow(a->lon - b->lon, 2)); // Евклидово расстояние (O(1)) (double 8 байт)
    };

    unordered_map<Node*, double> gScore, fScore; // O(V) памяти для оценки стоимости пути
    for (auto* node : nodes) {
        gScore[node] = numeric_limits<double>::infinity(); // Инициализация gScore (O(V)) (double 8 байт)
        fScore[node] = numeric_limits<double>::infinity(); // Инициализация fScore (O(V)) (double 8 байт)
    }
    gScore[start] = 0.0; // Стоимость пути до начального узла равна 0 (O(1))
    fScore[start] = heuristic(start, goal); // Эвристика начального узла (O(1))

    priority_queue<pair<double, Node*>, vector<pair<double, Node*>>, greater<>> pq; // Очередь с приоритетом (O(V) памяти)
    pq.push({fScore[start], start}); // Добавляем начальный узел в очередь (O(log(V)))

    while (!pq.empty()) { // Пока очередь не пуста (O((V + E) * log(V)))
        auto [_, current] = pq.top(); // Получаем узел с минимальным приоритетом (O(1))
        pq.pop(); // Удаляем узел из очереди (O(log(V)))

        if (current == goal) return gScore[current]; // Если достигли цели, возвращаем расстояние (O(1))
for (auto& [neighbor, weight] : current->neighbors) { // Перебираем соседей (O(E))
            double tentativeG = gScore[current] + weight; // Вычисляем временную оценку стоимости (O(1))
            if (tentativeG < gScore[neighbor]) { // Если найден более короткий путь (O(1))
                gScore[neighbor] = tentativeG; // Обновляем gScore (O(1))
                fScore[neighbor] = tentativeG + heuristic(neighbor, goal); // Обновляем fScore (O(1))
                pq.push({fScore[neighbor], neighbor}); // Добавляем узел в очередь (O(log(V)))
            }
        }
    }

    return -1.0; // Если путь не найден, возвращаем -1 (O(1))
}

int main() {
    Graph graph;
    graph.loadFromFile("spb_graph.txt");

    Node* start = graph.findClosestNode(59.884972, 30.368072); // Указатель на начальный узел (8 байт)
    Node* goal = graph.findClosestNode(59.956248, 30.309215); // Указатель на начальный узел (8 байт)

    unordered_set<Node*> visited;
    double dfsDistance = 0.0;

    cout << "DFS: ";
    if (dfs(start, goal, visited, dfsDistance)) {
        cout << "Расстояние = " << dfsDistance << endl;
    } else {
        cout << "Путь не найден" << endl;
    }

    cout << "BFS: " << bfs(start, goal) << endl;
    cout << "Dijkstra: " << dijkstra(start, goal, graph.nodes) << endl;
    cout << "A*: " << aStar(start, goal, graph.nodes) << endl;

    return 0;
}
