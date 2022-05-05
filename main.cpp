#include "char.h"
#include "AhoCorasick.hpp"
#include "searchable.hpp"
#include "actions.hpp"
#include "glob.hpp"

template<class TChar>
//Ищет подстроку в потоке попутно копируя символы в выходной поток. Использует тот же алгоритм Ахо-Корасик что и основной поиск (в processFile)
void copy_till(std::istreambuf_iterator<TChar>& begin, std::istreambuf_iterator<TChar>& end, std::basic_ostream<TChar>& destination, std::basic_string<TChar> suffix) {
    aho_corasick::basic_trie<TChar> suffixSearcher;
    //Подготовим "бор" (ДКА) для алгоритма Ахо-Корасик
    auto n = sizeof(searchable_items) / sizeof(*searchable_items);
    std::vector<searchable_t<TChar>*> currentItems;
    for (auto i = 0; i < n; i++)
        //Всё кроме перемещаемых секций
        if (searchable_items[i].action != A_MOVE_TO_END && searchable_items[i].action != A_MOVE_HERE) {
            currentItems.push_back(&searchable_items[i]);
            suffixSearcher.insert(searchable_items[i].begin);
        }
    //Плюс, конец текущего фрагмента
    suffixSearcher.insert(suffix);
    std::basic_stringstream<TChar> ignore;

    //Основной цикл
    while (true) {
        //Ищем что-нибудь из списка, попутно выводя симолы в выходной файл
        auto m = suffixSearcher.nextMatch(begin, end, destination);
        if (m.size() > 0) {
            //Это то, что мы нашли. Нужно выбрать совпадение максимальной длины.
            auto maxMatch = max_element(m.begin(), m.end(), [](auto& a, auto& b) { return a.size() < b.size(); });
            //Если мы нашли конец фрагмента, просто возвращаем управление
            if (maxMatch->get_index() == currentItems.size())
                return;            
            auto& matched = *currentItems[maxMatch->get_index()];
            actionMethods<TChar>[matched.action](matched, begin, end, destination, ignore);
        }
        if (begin == end) break;
    }
    //Если мы выпали из цикла (==дошли до конца файла так и не найдя конец секции - это ошибка)
    COUT << _S(" FATAL ERROR: Cannot find end of movable section ") << suffix;
    throw 1;
}
//Обрабатывает один файл
template<typename TChar>
void processFile(aho_corasick::basic_trie<TChar> &trie, const std::filesystem::path &filename) {
    std::cout << filename.string();
    //Подготавливаем файлы
    auto input = std::basic_ifstream<TChar>(filename, std::ios::binary | std::ios::in);
    input >> std::noskipws;
    auto outputFilename = std::filesystem::path(filename).replace_filename(
            filename.filename().replace_extension("").string() + "$" + filename.extension().string());
    auto output = std::basic_ofstream<TChar>(outputFilename, std::ios::binary | std::ios::out);
    std::istreambuf_iterator<TChar> iter(input);
    std::istreambuf_iterator<TChar> end;
    std::basic_stringstream<TChar> end_insert;

    try {
        //Основной цикл
        while (true) {
            //Ищем что-нибудь из списка, попутно выводя симолы в выходной файл
            auto m = trie.nextMatch(iter, end, output);
            if (m.size() > 0) {
                //Это то, что мы нашли. Нужно выбрать совпадение максимальной длины.
                auto maxMatch = max_element(m.begin(), m.end(), [](auto& a, auto& b) { return a.size() < b.size(); });
                auto &matched = searchable_items[maxMatch->get_index()];
                actionMethods<TChar>[matched.action](matched, iter, end, output, end_insert);
            }
            if (iter == end) break;
        }
    } catch (...) {
        output.flush();
        output.close();
        std::cout << " NO OUTPUT DUE TO ERROR(s)" << std::endl;
        std::filesystem::remove(outputFilename);
        return;
    }
    output.flush();
    auto len = output.tellp();
    output.close();
    //Нужно укоротить файл до текущей позиции, т.к. seekp оставляет мусор в конце файла.
    std::filesystem::resize_file(outputFilename, len);
    std::cout << " -> " << outputFilename.string() << std::endl;
}

int main(int argc, char **argv) {
    std::list<std::filesystem::path> filenames;
    //Сначала сформируем список файлов
    for (auto i = 1; i < argc; i++) {
        enumerateFiles(std::filesystem::path(argv[i]), filenames);
    }
    //Проблема - ничего не найдено
    if (!filenames.size()) {
        std::cerr << "No files found." << std::endl << "Sample usage:" << std::endl
             << "trpp2a_new c:\\folder1\\*.tex d:\\folder2\\*.tex" << std::endl;
    }

    //Подготовим "бор" (ДКА) для алгоритма Ахо-Корасик
    auto n = sizeof(searchable_items) / sizeof(*searchable_items);
    aho_corasick::basic_trie<CHAR> trie;
    for (auto i = 0; i < n; i++)
        trie.insert(searchable_items[i].begin);


    //Теперь обработаем файлы
    for (auto it = filenames.begin(); it != filenames.end(); it++)
        processFile<CHAR>(trie, std::filesystem::path(*it));

    return 0;
}