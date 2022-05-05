#ifndef SEARCHABLE_HPP
#define SEARCHABLE_HPP
#include <string>
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
    std::string begin;
    //Конец фрагмента (или пустая строка, если фрагмент состоит только из начала, или строка на которую будет заменяться начало для действия A_REPLACE
    std::string end;
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
#endif