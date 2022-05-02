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

using namespace std;

#define GLOB_MANY_CHAR "#"
#define GLOB_ONE_CHAR "?"

//Возможные действия с фрагментами:
//--------------
//Не трогать найденное - просто скопировать на вывод. Полезно для escape-последовательностей, например, \\ или \%
#define A_DONT_TOUCH 0
//Переместить фрагмент в конец документа
#define A_MOVE_TO_END 1
//Удалить фрагмент
#define A_REMOVE 2
//Удалить фрагмент, но оставить символы обозначающие конец фрагмента. Полезно для удаления комментариев ограниченных в конце переносом (\n) но сохранения переноса
#define A_REMOVE_ADD_END 3
//Заменить строку заданную searchable_t::begin на searchable_t::end
#define A_REPLACE 4
//Маркер, куда нужно поместить перемещаемые фрагменты
#define A_MOVE_HERE 5

typedef struct {
    //Начало фрагмента
    string begin;
    //Конец фрагмента (или пустая строка, если фрагмент состоит только из начала, или строка на которую будет заменяться начало для действия A_REPLACE
    string end;
    //Действие
    int action;
} searchable_t;

//Определение действий со строками
searchable_t searchable_items[] = {
        {"\\\"",                              "",                                                                    A_DONT_TOUCH},
        {"\\%",                               "",                                                                    A_DONT_TOUCH},
        {"\\\\",                              "",                                                                    A_DONT_TOUCH},
        {"\\~",                               "",                                                                    A_DONT_TOUCH},
        {"$$",                                "$$",                                                                  A_MOVE_TO_END},
        {"\\[",                               "\\]",                                                                 A_MOVE_TO_END},
        {"\\midinsert",                       "\\endinsert",                                                         A_MOVE_TO_END},
        {"\\topinsert",                       "\\endinsert",                                                         A_MOVE_TO_END},
        {"\\botinsert",                       "\\endinsert",                                                         A_MOVE_TO_END},
        {"\\begin{equation}",                 "\\end{equation}",                                                     A_MOVE_TO_END},
        {"\\begin{equation*}",                "\\end{equation*}",                                                    A_MOVE_TO_END},
        {"\\begin{gather}",                   "\\end{gather}",                                                       A_MOVE_TO_END},
        {"\\begin{gather*}",                  "\\end{gather*}",                                                      A_MOVE_TO_END},
        {"\\begin{multline}",                 "\\end{multline}",                                                     A_MOVE_TO_END},
        {"\\begin{multline*}",                "\\end{multline*}",                                                    A_MOVE_TO_END},
        {"\\begin{align}",                    "\\end{align}",                                                        A_MOVE_TO_END},
        {"\\begin{align*}",                   "\\end{align*}",                                                       A_MOVE_TO_END},
        {"\\begin{aligned}",                  "\\end{aligned}",                                                      A_MOVE_TO_END},
        {"\\begin{aligned*}",                 "\\end{aligned*}",                                                     A_MOVE_TO_END},
        {"\\begin{alignat}",                  "\\end{alignat}",                                                      A_MOVE_TO_END},
        {"\\begin{alignat*}",                 "\\end{alignat*}",                                                     A_MOVE_TO_END},
        {"\\begin{alignedat}",                "\\end{alignedat}",                                                    A_MOVE_TO_END},
        {"\\begin{alignedat*}",               "\\end{alignedat*}",                                                   A_MOVE_TO_END},
        {"\\begin{figure}",                   "\\end{figure}",                                                       A_MOVE_TO_END},
        {"\\begin{figure*}",                  "\\end{figure*}",                                                      A_MOVE_TO_END},
        {"\\begin{tabular}",                  "\\end{tabular}",                                                      A_MOVE_TO_END},
        {"\\begin{tabular*}",                 "\\end{tabular*}",                                                     A_MOVE_TO_END},
        {"\\begin{table}",                    "\\end{table}",                                                        A_MOVE_TO_END},
        {"\\begin{table*}",                   "\\end{table*}",                                                       A_MOVE_TO_END},
        {"\\begin{longtable}",                "\\end{longtable}",                                                    A_MOVE_TO_END},
        {"%",                                 "\n",                                                                  A_REMOVE_ADD_END},
        {"\\Lenth",                           "\n",                                                                  A_REMOVE_ADD_END},
        {"\\enlargethispage",                 "\n",                                                                  A_REMOVE_ADD_END},
        {"\\renewcommand{\\baselinestretch}", "\n",                                                                  A_REMOVE_ADD_END},
        {"\\def\\baselinestretch",            "\n",                                                                  A_REMOVE_ADD_END},
        {"\\normalbaselineskip",              "\n",                                                                  A_REMOVE_ADD_END},
        {"\\baselineskip",                    "\n",                                                                  A_REMOVE_ADD_END},
        {"\\vadjust{\\eject}",                "\n",                                                                  A_REMOVE_ADD_END},
        {"\\pagebreak",                       "",                                                                    A_REMOVE},
        {"\\goodbreak",                       "",                                                                    A_REMOVE},
        {"\\newpage",                         "",                                                                    A_REMOVE},
        {"~",                                 " ",                                                                   A_REPLACE},
        {"\"",                                " ",                                                                   A_REPLACE},
        {"\\linebreak",                       "\\relax",                                                             A_REPLACE},
        {"\\nobreak",                         "\\relax",                                                             A_REPLACE},
        {"\\break",                           "\\relax",                                                             A_REPLACE},
        {"\\begin{document}",                 "\\hfuzz=4cm\n\\hbadness=10000\n\\tolerance1000\n\n\\begin{document}", A_REPLACE},
        {"\\end{document}",                   "",                                                                    A_MOVE_HERE}
};

//Ищет подстроку в потоке попутно копируя символы в выходной поток. Использует тот же алгоритм Ахо-Корасик что и основной поиск (в processFile)
void copy_till(istream_iterator<char> &begin, istream_iterator<char> &end, ostream &destination, string suffix) {
    aho_corasick::trie suffixSearcher;
    suffixSearcher.insert(suffix);
    auto match = suffixSearcher.nextMatch(begin, end, destination);
    if (match.size() == 0) {
        cout << " FATAL ERROR: Cannot find end of movable section " << suffix;
        throw 1;
    }
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
    istream_iterator<char> iter(input);
    istream_iterator<char> end;
    stringstream end_insert;

    try {
        //Основной цикл
        while (true) {
            //Ищем что-нибудь из списка, попутно выводя симолы в выходной файл
            auto m = trie.nextMatch(iter, end, output);
            if (m.size() > 0) {
                //Это то, что мы нашли. Нужно выбрать совпадение максимальной длины.
                auto &matched = searchable_items[max_element(m.begin(), m.end(), [](auto &a, auto &b) {
                    return a.size() < b.size();
                })->get_index()];
                //Откатываемся обратно на длину найденного, т.к. в общем случае ничего делать не надо
                if (matched.action != A_DONT_TOUCH)
                    output.seekp(-(int) matched.begin.size() + 1, ios::cur);

                switch (matched.action) {
                    case A_REPLACE: // Заменяем найденое на searchable_t::end
                        output << matched.end;
                        iter++;
                        break;
                    case A_MOVE_TO_END: // Перемещаем фрагмент в конец
                        // Добавляем во временное хранилище начало фрагмента - оно было съедено поиском
                        end_insert << matched.begin;
                        iter++;
                        // Если задан конец фрагмента - находим его, попутно копируя во временное хранилище
                        if (matched.end.size()) {
                            copy_till(iter, end, end_insert, matched.end);
                            end_insert << *iter++;
                        }
                        end_insert << endl << endl;
                        output << " ..... ";
                        break;
                    case A_REMOVE: // Удаляем фрагмент
                        iter++;
                    case A_REMOVE_ADD_END:
                        if (matched.end.size()) { // Если задан конец фрагмента, надо его найти и удалить (не копировать) символы до его конца.
                            if (search(iter, end, matched.end.begin(), matched.end.end()) == end) {
                                cout << " FATAL ERROR: Cannot find end of removable section " << matched.end;
                                throw 1;
                            }
                            iter++;
                            if (matched.action == A_REMOVE_ADD_END)
                                output << matched.end;
                        }
                        break;
                    case A_MOVE_HERE: // Вставляем сюда накопленные ранее фрагменты
                        output << end_insert.str() << matched.begin;
                        iter++;
                        break;
                    case A_DONT_TOUCH: // Не трогаем такой фрагмент
                        output << *iter++;
                        break;
                    default:
                        break;
                }
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

template<class BidirIt, class Traits, class CharT, class UnaryFunction>
std::basic_string<CharT> regex_replace(BidirIt first, BidirIt last,
                                       const std::basic_regex<CharT, Traits> &re, UnaryFunction f) {
    std::basic_string<CharT> s;

    typename std::match_results<BidirIt>::difference_type
            positionOfLastMatch = 0;
    auto endOfLastMatch = first;

    auto callback = [&](const std::match_results<BidirIt> &match) {
        auto positionOfThisMatch = match.position(0);
        auto diff = positionOfThisMatch - positionOfLastMatch;

        auto startOfThisMatch = endOfLastMatch;
        std::advance(startOfThisMatch, diff);

        s.append(endOfLastMatch, startOfThisMatch);
        s.append(f(match));

        auto lengthOfMatch = match.length(0);

        positionOfLastMatch = positionOfThisMatch + lengthOfMatch;

        endOfLastMatch = startOfThisMatch;
        std::advance(endOfLastMatch, lengthOfMatch);
    };

    std::regex_iterator<BidirIt> begin(first, last, re), end;
    std::for_each(begin, end, callback);

    s.append(endOfLastMatch, last);

    return s;
}

template<class Traits, class CharT, class UnaryFunction>
std::string regex_replace(const std::string &s,
                          const std::basic_regex<CharT, Traits> &re, UnaryFunction f) {
    return regex_replace(s.cbegin(), s.cend(), re, f);
}

regex globRegex("[" GLOB_MANY_CHAR GLOB_ONE_CHAR ".()\\[\\]{}?*\\\\]");


//Рекурсивный обход дерева файловой системы с учётом масок файлов/папок
void enumerateGlob(const filesystem::path &current, filesystem::path::iterator &path,
                   const filesystem::path::const_iterator &end, list<filesystem::path> &filenames) {
    //Формируем регулярное выражение вместо символов подстановки
    string newPath("^");
    auto spath = path->string();
    newPath += regex_replace(spath, globRegex, [](auto const &match) {
        if (match.str() == GLOB_MANY_CHAR)
            return string(".*");
        else if (match.str() == GLOB_ONE_CHAR)
            return string(".");
        return string("\\") + match.str();

    });
    newPath += "$";
    regex thisRegex(newPath);

    //Это следующий элемент пути
    auto nextPath(path);
    nextPath++;

    //Теперь перечисляем все файлы/папки в текущей (current)
    for (auto &dirEntry: filesystem::directory_iterator(current)) {
        //Имя совпадает с заданным?
        if (regex_match(dirEntry.path().string(), thisRegex)) {
            //Это файл и мы сейчас просматриваем последний элемент пути? Кроме того, нет ли $ в конце имени файла
            if (dirEntry.is_regular_file() && nextPath == end &&
                *(dirEntry.path().filename().replace_extension("").string().end() - 1) != '$') {
                filenames.push_back(dirEntry.path());
            } else if (dirEntry.is_directory()) { //Это папка, продолжаем рекурсивный обход
                //Сначала заходим не переходя к следующему элементу шаблона (поддержка путей вида c:\folder\*\*.tex - вместо первой звёздочки может быть любое количество папок)
                enumerateGlob(dirEntry.path(), path, end, filenames);
                //Если есть ещё элементы шаблона дальше - заходим используя следующий
                if (nextPath != end)
                    enumerateGlob(dirEntry.path(), nextPath, end, filenames);
            }
        }
    }
}

//Проходит по элементам пути-маски до тех, пор пока путь не кончится (т.е. это не маска), или не встретится элемент с подстановочным символом
void enumerateFiles(const filesystem::path &xpath, list<filesystem::path> &filenames) {
    //Быстрый выход - передан путь к существующему файлу
    if (exists(xpath) && is_regular_file(xpath)) {
        filenames.push_back(xpath);
        return;
    }

    auto path = absolute(xpath);

    filesystem::path current;
    for (auto i = path.begin(); i != path.end(); i++) {
        auto spath = i->string();
        if (find_if(spath.begin(), spath.end(),
                    [](auto a) { return a == GLOB_MANY_CHAR[0] || a == GLOB_ONE_CHAR[0]; }) == spath.end()) {
            current /= *i;
        } else {
            enumerateGlob(current, i, path.end(), filenames);
        }
    }
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