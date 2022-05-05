#ifndef ACTIONS_HPP
#define ACTIONS_HPP
#include <iostream>
#include <string>
#include <list>
#include <algorithm>
#include <fstream>
#include <regex>
#include <iterator>
#include <filesystem>
#include <sstream>

template<class TChar>
void copy_till(std::istreambuf_iterator<TChar>& begin, std::istreambuf_iterator<TChar>& end, std::basic_ostream<TChar>& destination, std::basic_string<TChar> suffix);

template<class TChar>
void actionReplace(const searchable_t<TChar>& matched, std::istreambuf_iterator<TChar>& begin, std::istreambuf_iterator<TChar>& end, std::basic_ostream<TChar>& destination, std::basic_stringstream<TChar>& secondaryOutput) {
    destination << matched.end;
    begin++;
}
template<class TChar>
void actionDontTouch(const searchable_t<TChar>& matched, std::istreambuf_iterator<TChar>& begin, std::istreambuf_iterator<TChar>& end, std::basic_ostream<TChar>& destination, std::basic_stringstream<TChar>& secondaryOutput) {
    destination << *begin++;
}
template<class TChar>
void actionMoveToEnd(const searchable_t<TChar>& matched, std::istreambuf_iterator<TChar>& iter, std::istreambuf_iterator<TChar>& end, std::basic_ostream<TChar>& output, std::basic_stringstream<TChar>& secondaryOutput) {
    // Добавляем во временное хранилище начало фрагмента - оно было съедено поиском
    secondaryOutput << matched.begin;
    iter++;
    // Если задан конец фрагмента - находим его, попутно копируя во временное хранилище
    if (matched.end.size()) {
        copy_till(iter, end, secondaryOutput, matched.end);
        secondaryOutput << *iter++;
    }
    secondaryOutput << std::endl << std::endl;
    output << " ..... ";
}
template<class TChar>
void actionRemoveAddEnd(const searchable_t<TChar>& matched, std::istreambuf_iterator<TChar>& begin, std::istreambuf_iterator<TChar>& end, std::basic_ostream<TChar>& destination, std::basic_stringstream<TChar>& secondaryOutput) {
    if (matched.end.size()) { // Если задан конец фрагмента, надо его найти и удалить (не копировать) символы до его конца.
        auto ee = search(begin, end, matched.end.begin(), matched.end.end());
        if (ee == end) {            
            COUT << _S(" FATAL ERROR: Cannot find end of removable section ") << matched.end;
            throw 1;
        }
        begin = ee;
        if (matched.action == A_REMOVE_ADD_END)
            destination << matched.end;
    }
}
template<class TChar>
void actionRemove(const searchable_t<TChar>& matched, std::istreambuf_iterator<TChar>& begin, std::istreambuf_iterator<TChar>& end, std::basic_ostream<TChar>& destination, std::basic_stringstream<TChar>& secondaryOutput) {
    begin++;
    actionRemoveAddEnd<TChar>(matched, begin, end, destination, secondaryOutput);
}
template<class TChar>
void actionMoveHere(const searchable_t<TChar>& matched, std::istreambuf_iterator<TChar>& begin, std::istreambuf_iterator<TChar>& end, std::basic_ostream<TChar>& destination, std::basic_stringstream<TChar>& secondaryOutput) {
    destination << secondaryOutput.str() << matched.begin;
    begin++;
}


template<class TChar>
//Порядок должен соответствовать нумерации констант A_* !
void(*actionMethods[])(const searchable_t<TChar>& matched, std::istreambuf_iterator<TChar>& begin, std::istreambuf_iterator<TChar>& end, std::basic_ostream<TChar>& destination, std::basic_stringstream<TChar>& secondaryOutput)  = {
    &actionDontTouch,
    &actionMoveToEnd,
    &actionRemove,
    &actionRemoveAddEnd,
    &actionReplace,
    &actionMoveHere
};
#endif