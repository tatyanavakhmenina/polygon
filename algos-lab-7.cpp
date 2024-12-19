class Solution {
public:
    int candy(vector<int>& ratings) {
        int n = ratings.size();
        vector<int> candies(n, 1); // Минимум 1 конфета каждому ребенку
        
        // Первый проход слева направо
        for (int i = 1; i < n; i++) {
            if (ratings[i] > ratings[i - 1]) {
                candies[i] = candies[i - 1] + 1; // Если рейтинг больше, даем больше конфет
            }
        }
        
        // Второй проход справа налево
        for (int i = n - 2; i >= 0; i--) {
            if (ratings[i] > ratings[i + 1]) {
                candies[i] = max(candies[i], candies[i + 1] + 1); // Если рейтинг больше, обновляем количество конфет
            }
        }
        
        // Подсчет общего количества конфет
        return accumulate(candies.begin(), candies.end(), 0);
    }
};
