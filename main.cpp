#include <iostream>
#include <string>
#include <list>
#include <algorithm>
#include <fstream>
#include <regex>
#include <iterator>
#include <filesystem>
#include <sstream>
#include "AhoCorasick.hpp"
#include "searchable.hpp"
#include "actions.hpp"
#include "glob.hpp"
using namespace std;


//Ищет подстроку в потоке попутно копируя символы в выходной поток. Использует тот же алгоритм Ахо-Корасик что и основной поиск (в processFile)
void copy_till(istreambuf_iterator<char>& begin, istreambuf_iterator<char>& end, ostream& destination, string suffix) {
    aho_corasick::trie suffixSearcher;
    //Подготовим "бор" (ДКА) для алгоритма Ахо-Корасик
    auto n = sizeof(searchable_items) / sizeof(*searchable_items);
    vector<searchable_t*> currentItems;
    for (auto i = 0; i < n; i++)
        //Всё кроме перемещаемых секций
        if (searchable_items[i].action != A_MOVE_TO_END && searchable_items[i].action != A_MOVE_HERE) {
            currentItems.push_back(&searchable_items[i]);
            suffixSearcher.insert(searchable_items[i].begin);
        }
    //Плюс, конец текущего фрагмента
    suffixSearcher.insert(suffix);
    stringstream ignore;

    //Основной цикл
    while (true) {
        //Ищем что-нибудь из списка, попутно выводя симолы в выходной файл
        auto m = suffixSearcher.nextMatch(begin, end, destination);
        if (m.size() > 0) {
            //Это то, что мы нашли. Нужно выбрать совпадение максимальной длины.
            auto maxMatch = max_element(m.begin(), m.end(), [](auto& a, auto& b) {
                return a.size() < b.size();
                });
            auto ix = maxMatch->get_index();
            if (maxMatch->get_index() == currentItems.size()) {
                //Здесь мы нашли конец, просто возвращаем управление
                return;
            }
            auto& matched = *currentItems[maxMatch->get_index()];
            //Откатываемся обратно на длину найденного, т.к. в общем случае ничего делать не надо
            if (matched.action != A_DONT_TOUCH)
                destination.seekp(-((int)matched.begin.size()) + 1, ios::cur);

            actionMethods[matched.action](matched, begin, end, destination, ignore);            
        }
        if (begin == end) break;
    }
    //Если мы выпали из цикла (==дошли до конца файла так и не найдя конец секции - это ошибка)
    cout << " FATAL ERROR: Cannot find end of movable section " << suffix;
    throw 1;
}

//Обрабатывает один файл
void processFile(aho_corasick::trie &trie, const filesystem::path &filename) {
    cout << filename.string();
    //Подготавливаем файлы
    auto input = ifstream(filename, ios::binary | ios::in);
    input >> noskipws;
    auto outputFilename = filesystem::path(filename).replace_filename(
            filename.filename().replace_extension("").string() + "$" + filename.extension().string());
    auto output = ofstream(outputFilename, ios::binary | ios::out);
    istreambuf_iterator<char> iter(input);
    istreambuf_iterator<char> end;
    stringstream end_insert;

    try {
        //Основной цикл
        while (true) {
            //Ищем что-нибудь из списка, попутно выводя симолы в выходной файл
            auto m = trie.nextMatch(iter, end, output);
            if (m.size() > 0) {
                //Это то, что мы нашли. Нужно выбрать совпадение максимальной длины.
                auto maxMatch = max_element(m.begin(), m.end(), [](auto& a, auto& b) {
                    return a.size() < b.size();
                    });
                auto &matched = searchable_items[maxMatch->get_index()];
                //Откатываемся обратно на длину найденного, т.к. в общем случае ничего делать не надо
                if (matched.action != A_DONT_TOUCH)
                    output.seekp(-((int) matched.begin.size()) + 1, ios::cur);

                actionMethods[matched.action](matched, iter, end, output, end_insert);
            }
            if (iter == end) break;
        }
    } catch (...) {
        output.flush();
        output.close();
        cout << " NO OUTPUT DUE TO ERROR(s)" << endl;
        std::filesystem::remove(outputFilename);
        return;
    }
    output.flush();
    auto len = output.tellp();
    output.close();
    //Нужно укоротить файл до текущей позиции, т.к. seekp оставляет мусор в конце файла.
    std::filesystem::resize_file(outputFilename, len);
    cout << " -> " << outputFilename.string() << endl;
}

int main(int argc, char **argv) {
    list<filesystem::path> filenames;
    //Сначала сформируем список файлов
    for (auto i = 1; i < argc; i++) {
        enumerateFiles(filesystem::path(argv[i]), filenames);
    }
    //Проблема - ничего не найдено
    if (!filenames.size()) {
        cerr << "No files found." << endl << "Sample usage:" << endl
             << "trpp2a_new c:\\folder1\\*.tex d:\\folder2\\*.tex" << endl;
    }

    //Подготовим "бор" (ДКА) для алгоритма Ахо-Корасик
    auto n = sizeof(searchable_items) / sizeof(*searchable_items);
    aho_corasick::trie trie;
    for (auto i = 0; i < n; i++)
        trie.insert(searchable_items[i].begin);


    //Теперь обработаем файлы
    for (auto it = filenames.begin(); it != filenames.end(); it++)
        processFile(trie, filesystem::path(*it));

    return 0;
}