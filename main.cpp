#include <iostream>
#include <list>
#include <map>
#include <set>
#include "File.h"
#include <algorithm>
#include <thread>
#include <future>

template<class Iterator, class F, class Q>
auto parallel_map_reduce(Iterator p, Iterator q, F f1, Q f2,  size_t threads) -> decltype(f2(f1(*p), f1(*p)))
{
    Iterator m;
    auto result = f1(*p);
    if (threads <= 1) m = q;
    else {
        size_t d = distance(p, q) / threads;
        m = p;
        advance(m, d);
        decltype(f2(f1(*p), f1(*p))) h;
        thread t([&](){ h = parallel_map_reduce<Iterator, F, Q>(m, q, f1, f2, threads - 1); });
        t.join();
        result = f2(result , h);
    }
    while(++p != m)
        result = f2(result, f1(*p));
    return result;
};

template<class Iterator, class F>
void parallel(Iterator p, Iterator q, F f1, size_t threads)
{
    Iterator m;
    if (threads <= 1|| distance(p, q) <= threads) m = q;
    else {
        size_t d = distance(p, q) / threads;
        m = p;
        advance(m, d);
        thread t([&](){ parallel<Iterator, F>(m, q, f1, threads - 1); });
        t.join();
    }
    f1(*p);
    while(++p != m)
        f1(*p);
};

template<class Iterator, class F, class G>
void parallel(Iterator p, Iterator q, F f1, G f2, size_t threads)
{
    Iterator m;
    if (threads <= 1 || distance(p, q) <= threads) m = q;
    else {
        size_t d = distance(p, q) / threads;
        m = p;
        advance(m, d);
        thread t([&](){ parallel<Iterator, F, G>(m, q, f1, f2, threads - 1); });
        t.join();
    }
    f1(*p, f2);
    while(++p != m)
        f1(*p, f2);
};

struct comparator {
    bool operator()(pair<string, int> const& a, pair<string, int> const& b) const {
        if (a.second == b.second) return true;
        return a.second > b.second;
    };

};

mutex mtx;
map<string, int> words;

void process_stream(ifstream& f) {
     pair<map<string, int >::iterator, bool> existing;
     string s;
     while (f >> s) {
         lock_guard<mutex> lk(mtx);
         existing = words.insert(pair<string, int>(s, 1)); // Добавить новое слово
         if (!existing.second) existing.first->second++; // Если слово уже существует, увеличить его счетчик
     }
};

// Использование принципа IOC:
// Шаблонные функции не знают что им передадут в аргументах, они независимы и просто выполняют то, что скажут им аргументы
// Здесь главное в этих функцииях - асинхронность
// последний параметр везде - колличество потоков
// parallel_1 принимает два итератора на элементы любой последовательности, одну или две функции и ничего не возвращает
// В случае передачи одной функции (она должна принимать один аргумент типа элемента последовательности): просто ее выполнение на каждом элементе последовательности
// В случае двух функции предполагается что первая принимает вторую как обратный вызов вторым аргументом
// parallel_map_reduce принимает два итератора на элементы любой последовательности, две функции
// первая функция одного аргумента применяется к каждому элементу последовательности
// вторая идет по парам элементов, сворачивая их в одно значение


// Здесь реализованы некоторые случаи применения функции
int main() {
    list<string> l = {"../a.txt", "../b.txt", "../c.txt"};
    parallel(l.begin(), l.end(), File::read, process_stream, 3); // читает файл и наполняет map словами
    set<pair<string, int>, comparator> se;
    for (auto & word : words) // переносим слова в set для сортировки
        se.insert(pair<string, int>(word.first, word.second));
    cout << "Слова, встречающиеся в файлах и их колличество: " << endl;
    for (auto & i : se)
        cout << i.first << ' ' << i.second << endl;
    string text = parallel_map_reduce(se.begin(), se.end(), [](auto& i){ return i.first; }, [](string a, string b){ return a + ' ' + b; }, 2);
    cout << "Уникальные слова: " << text << endl;
    string filtered;
    parallel(se.begin(), se.end(), [sub = "hel", &filtered](auto& i) {
        if (i.first.find(sub) !=std::string::npos) {
            lock_guard<mutex> lk(mtx);
            filtered += i.first + ' ';
        } }, 2);
    cout << "Слова, содержащие 'hel': " << filtered << endl;
    std::list<int> l2 = {1,2,3,4,5,6,7,8,9,10};
    auto sum_of_squares = parallel_map_reduce(l2.begin(), l2.end(), [](int i){ return i * i; }, [](int a, int b){ return a + b; }, 3);
    auto product = parallel_map_reduce(l2.begin(), l2.end(), [](int i){ return i; }, [](int a, int b){ return a * b; }, 3);
    auto any_is_even = parallel_map_reduce(l2.begin(), l2.end(), [](int i){ return i % 2 == 0; }, [](bool a, bool b){ return a || b; }, 4);
    auto is_sorted = parallel_map_reduce(l2.begin(), l2.end(), [](int i){ return i; }, [](int a, int b){ return a < b; }, 4);
    cout << "Для списка: ";
    for (auto & i : l2)
        cout << i << ' ';
    cout << endl <<
         "Сумма квадратов: " << sum_of_squares << endl <<
         "Произведение элементов: " << product << endl <<
         "Существование четных: " << (any_is_even == 1 ? "есть" : "нет") << endl <<
         "Сортировка в порядке возрастания: " << (is_sorted == 1 ? "есть" : "нет") << endl;
    return 0;
}