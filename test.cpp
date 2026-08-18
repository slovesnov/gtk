#include <algorithm> // Для std::next_permutation
#include <iostream>
#include <numeric> // Для std::iota
#include <vector>

// g++ test.cpp && ./a.exe

using VInt = std::vector<int>;

std::vector<std::vector<int>> get_arrangements_without_repeats(int n, int k) {
  std::vector<std::vector<int>> result;
  if (k > n || k <= 0 || n <= 0)
    return result;

  // 1. Исходные элементы (0, 1, 2, ..., n-1)
  std::vector<int> elements(n);
  for (int i = 0; i < n; ++i)
    elements[i] = i;

  // 2. Маска для сочетаний в ПРЯМОМ порядке: сначала k единиц, затем (n - k)
  // нулей
  std::vector<bool> mask(k, true);
  mask.insert(mask.end(), n - k, false);

  // Временный вектор для сборки текущего сочетания
  std::vector<int> current_combination;
  current_combination.reserve(k);

  // 3. Шаг А: Перебираем все уникальные СОЧЕТАНИЯ элементов
  do {
    current_combination.clear();
    for (int i = 0; i < n; ++i) {
      if (mask[i]) {
        current_combination.push_back(elements[i]);
      }
    }

    // Шаг Б: Для каждого сочетания генерируем ВСЕ ЕГО ПЕРЕСТАНОВКИ.
    // Сначала сортируем, чтобы гарантировать правильный лексикографический
    // порядок
    std::sort(current_combination.begin(), current_combination.end());

    do {
      result.push_back(current_combination);
    } while (std::next_permutation(current_combination.begin(),
                                   current_combination.end()));

  } while (std::prev_permutation(mask.begin(), mask.end()));

  // 4. Опционально: сортируем общий результат, чтобы все шло строго по порядку
  std::sort(result.begin(), result.end());

  return result;
}

std::vector<std::vector<int>> get_arrangements_with_repeats(int n, int k) {
  std::vector<std::vector<int>> result;
  if (k <= 0 || n <= 0)
    return result;

  // Вектор текущего состояния счетчика (хранит индексы от 0 до n-1)
  std::vector<int> current(k, 0);

  while (true) {
    // Добавляем текущую комбинацию в итоговый результат
    result.push_back(current);

    // Логика "счетчика": увеличиваем правый разряд
    int pos = k - 1;
    while (pos >= 0) {
      current[pos]++;
      if (current[pos] < n) {
        // Если разряд не переполнился, выходим из цикла переноса
        break;
      }
      // Если переполнился (стал равен n), сбрасываем в 0 и идем левее
      current[pos] = 0;
      pos--;
    }

    // Если pos стал меньше 0, значит переполнился самый левый разряд —перебор
    // окончен
    if (pos < 0) {
      break;
    }
  }

  return result;
}

std::vector<VInt> combinations(int n, int k) {

  std::vector<VInt> r;
  VInt elements(n), a;
  for (int i = 0; i < n; ++i)
    elements[i] = i;

  // 2. Маска в ПРЯМОМ порядке: сначала k единиц, затем (n - k) нулей
  std::vector<bool> mask(k, true);
  mask.insert(mask.end(), n - k, false);

  do {
    a.clear();
    for (int i = 0; i < n; ++i) {
      if (mask[i]) {
        a.push_back(elements[i]);
      }
    }
    r.push_back(a);

  } while (std::prev_permutation(mask.begin(), mask.end()));
  return r;
}

int main() {
  int n = 4;
  int k = 2;
  int i;
  std::vector<VInt> b;

  for (i = 0; i < 3; i++) {
    if (i == 0)
      b = get_arrangements_without_repeats(n, k);//n!/(n-k)!
    else if (i == 1)
      b = get_arrangements_with_repeats(n, k);//k^n
    else
      b = combinations(n, k);//n!/( (n-k)!k! )
      printf("\n---------%d---------\n",b.size());

    for (auto &e : b) {
      for (auto &v : e) {
        printf("%d ", v);
      }
      printf("\n");
    }
  }
}
