import numpy as np

# Для решения задачи определим функцию
def longest_sequence_length(arr):
    # Вычисляем разности между соседними элементами массива, чтобы узнать отличаются ли они
    diffs = np.diff(arr)
    # Ищем индексы, где разность не равна нулю
    changes = np.where(diffs != 0)[0]
    # Добавляем начальный и конечный индексы
    boundaries = np.concatenate(([0], changes + 1, [len(arr)]))
    # Вычисление длины всех последовательностей
    lengths = np.diff(boundaries)
    # Возвращение максимальной паследовательности
    return lengths.max()
# Пример
array = np.array([1, 1, 2, 2, 2, 3, 3, 1, 1, 1, 1])
print(longest_sequence_length(array))
