#include <windows.h>
#include <iostream>
#include <string>
#include <list>
#include <algorithm>
#include <fstream>
#include <iterator>
#include <filesystem>
#include "AhoCorasick.hpp"

using namespace std;

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
        {"\\%",                               "",                                                                    A_DONT_TOUCH},
        {"\\\\",                              "",                                                                    A_DONT_TOUCH},
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
        cout << "Cannot find end of movable section " << suffix << endl;
        return;
    }
}

//Обрабатывает один файл
void processFile(aho_corasick::trie &trie, const filesystem::path &filename) {
    cout << filename;
    //Подготавливаем файлы
    auto input = ifstream(filename, ios::binary | ios::in);
    input >> noskipws;
    auto outputFilename = filesystem::path(filename).replace_filename(
            filename.filename().replace_extension("").string() + "$" + filename.extension().string());
    auto output = ofstream(outputFilename, ios::binary | ios::out);
    istream_iterator<char> iter(input);
    istream_iterator<char> end;
    stringstream end_insert;

    //Основной цикл
    while (true) {
        //Ищем что-нибудь из списка, попутно выводя симолы в выходной файл
        auto m = trie.nextMatch(iter, end, output);
        if (m.size() > 0) {
            //Это то, что мы нашли. Нужно выбрать совпадение максимальной длины.
            auto &matched = searchable_items[max_element(m.begin(), m.end(), [](auto &a, auto &b) {
                return a.size() - b.size();
            })->get_index()];
            //Откатываемся обратно на длину найденного, т.к. в общем случае ничего делать не надо
            if (matched.action != A_DONT_TOUCH)
                output.seekp(-matched.begin.size() + 1, ios::cur);

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
                case A_REMOVE_ADD_END:
                    if (matched.end.size()) { // Если задан конец фрагмента, надо его найти и удалить (не копировать) символы до его конца.
                        if (search(iter, end, matched.end.begin(), matched.end.end()) == end) {
                            cout << "Cannot find end of removable section " << matched.end << endl;
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
    output.flush();
    auto len = output.tellp();
    output.close();
    //Нужно укоротить файл до текущей позиции, т.к. seekp оставляет мусор в конце файла.
    std::filesystem::resize_file(outputFilename, len);
    cout << " -> " << outputFilename.string() << endl;
}

int main(int argc, char **argv) {
    list<string> filenames;
    //Сначала сформируем список файлов
    for (auto i = 1; i < argc; i++) {
        string dirname(argv[i]);
        auto index = find(dirname.rbegin(), dirname.rend(), '\\');
        if (index != dirname.rend())
            dirname.erase(index.base(), dirname.end());
        else
            dirname = "";
        WIN32_FIND_DATA fileInfo;
        auto findHandle = FindFirstFile(argv[i], &fileInfo);
        while (findHandle != INVALID_HANDLE_VALUE) {
            string filename(fileInfo.cFileName);
            if (filename.length() <= 5 || *(filename.end() - 5) != '$')
                filenames.push_back(dirname + filename);
            if (!FindNextFile(findHandle, &fileInfo))
                break;
        }
        if (findHandle != INVALID_HANDLE_VALUE)
            FindClose(findHandle);
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