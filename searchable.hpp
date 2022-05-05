#ifndef SEARCHABLE_HPP
#define SEARCHABLE_HPP
#include <string>
#include "char.h"
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

template<class TChar>
struct searchable_t {
    //Начало фрагмента
    std::basic_string<TChar> begin;
    //Конец фрагмента (или пустая строка, если фрагмент состоит только из начала, или строка на которую будет заменяться начало для действия A_REPLACE
    std::basic_string<TChar> end;
    //Действие
    int action;
};

//Определение действий со строками
searchable_t<CHAR> searchable_items[] = {
        {_S("\\\""),                              _S(""),                                                                    A_DONT_TOUCH},
        {_S("\\%"),                               _S(""),                                                                    A_DONT_TOUCH},
        {_S("\\\\"),                              _S(""),                                                                    A_DONT_TOUCH},
        {_S("\\~"),                               _S(""),                                                                    A_DONT_TOUCH},
        {_S("$$"),                                _S("$$"),                                                                  A_MOVE_TO_END},
        {_S("\\["),                               _S("\\]"),                                                                 A_MOVE_TO_END},
        {_S("\\midinsert"),                       _S("\\endinsert"),                                                         A_MOVE_TO_END},
        {_S("\\topinsert"),                       _S("\\endinsert"),                                                         A_MOVE_TO_END},
        {_S("\\botinsert"),                       _S("\\endinsert"),                                                         A_MOVE_TO_END},
        {_S("\\begin{equation}"),                 _S("\\end{equation}"),                                                     A_MOVE_TO_END},
        {_S("\\begin{equation*}"),                _S("\\end{equation*}"),                                                    A_MOVE_TO_END},
        {_S("\\begin{gather}"),                   _S("\\end{gather}"),                                                       A_MOVE_TO_END},
        {_S("\\begin{gather*}"),                  _S("\\end{gather*}"),                                                      A_MOVE_TO_END},
        {_S("\\begin{multline}"),                 _S("\\end{multline}"),                                                     A_MOVE_TO_END},
        {_S("\\begin{multline*}"),                _S("\\end{multline*}"),                                                    A_MOVE_TO_END},
        {_S("\\begin{align}"),                    _S("\\end{align}"),                                                        A_MOVE_TO_END},
        {_S("\\begin{align*}"),                   _S("\\end{align*}"),                                                       A_MOVE_TO_END},
        {_S("\\begin{aligned}"),                  _S("\\end{aligned}"),                                                      A_MOVE_TO_END},
        {_S("\\begin{aligned*}"),                 _S("\\end{aligned*}"),                                                     A_MOVE_TO_END},
        {_S("\\begin{alignat}"),                  _S("\\end{alignat}"),                                                      A_MOVE_TO_END},
        {_S("\\begin{alignat*}"),                 _S("\\end{alignat*}"),                                                     A_MOVE_TO_END},
        {_S("\\begin{alignedat}"),                _S("\\end{alignedat}"),                                                    A_MOVE_TO_END},
        {_S("\\begin{alignedat*}"),               _S("\\end{alignedat*}"),                                                   A_MOVE_TO_END},
        {_S("\\begin{figure}"),                   _S("\\end{figure}"),                                                       A_MOVE_TO_END},
        {_S("\\begin{figure*}"),                  _S("\\end{figure*}"),                                                      A_MOVE_TO_END},
        {_S("\\begin{tabular}"),                  _S("\\end{tabular}"),                                                      A_MOVE_TO_END},
        {_S("\\begin{tabular*}"),                 _S("\\end{tabular*}"),                                                     A_MOVE_TO_END},
        {_S("\\begin{table}"),                    _S("\\end{table}"),                                                        A_MOVE_TO_END},
        {_S("\\begin{table*}"),                   _S("\\end{table*}"),                                                       A_MOVE_TO_END},
        {_S("\\begin{longtable}"),                _S("\\end{longtable}"),                                                    A_MOVE_TO_END},
        {_S("%"),                                 _S("\n"),                                                                  A_REMOVE_ADD_END},
        {_S("\\Lenth"),                           _S("\n"),                                                                  A_REMOVE_ADD_END},
        {_S("\\enlargethispage"),                 _S("\n"),                                                                  A_REMOVE_ADD_END},
        {_S("\\renewcommand{\\baselinestretch}"), _S("\n"),                                                                  A_REMOVE_ADD_END},
        {_S("\\def\\baselinestretch"),            _S("\n"),                                                                  A_REMOVE_ADD_END},
        {_S("\\normalbaselineskip"),              _S("\n"),                                                                  A_REMOVE_ADD_END},
        {_S("\\baselineskip"),                    _S("\n"),                                                                  A_REMOVE_ADD_END},
        {_S("\\vadjust{\\eject}"),                _S("\n"),                                                                  A_REMOVE_ADD_END},
        {_S("\\pagebreak"),                       _S(""),                                                                    A_REMOVE},
        {_S("\\goodbreak"),                       _S(""),                                                                    A_REMOVE},
        {_S("\\newpage"),                         _S(""),                                                                    A_REMOVE},
        {_S("~"),                                 _S(" "),                                                                   A_REPLACE},
        {_S("\""),                                _S(" "),                                                                   A_REPLACE},
        {_S("\\linebreak"),                       _S("\\relax"),                                                             A_REPLACE},
        {_S("\\nobreak"),                         _S("\\relax"),                                                             A_REPLACE},
        {_S("\\break"),                           _S("\\relax"),                                                             A_REPLACE},
        {_S("\\begin{document}"),                 _S("\\hfuzz=4cm\n\\hbadness=10000\n\\tolerance1000\n\n\\begin{document}"), A_REPLACE},
        {_S("\\end{document}"),                   _S(""),                                                                    A_MOVE_HERE}
};
#endif