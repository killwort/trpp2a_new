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

void copy_till(std::istreambuf_iterator<char>& begin, std::istreambuf_iterator<char>& end, std::ostream& destination, std::string suffix);

void actionReplace(const searchable_t& matched, std::istreambuf_iterator<char>& begin, std::istreambuf_iterator<char>& end, std::ostream& destination, std::stringstream& secondaryOutput) {
    destination << matched.end;
    begin++;
}
void actionDontTouch(const searchable_t& matched, std::istreambuf_iterator<char>& begin, std::istreambuf_iterator<char>& end, std::ostream& destination, std::stringstream& secondaryOutput) {
    destination << *begin++;
}

void actionMoveToEnd(const searchable_t& matched, std::istreambuf_iterator<char>& iter, std::istreambuf_iterator<char>& end, std::ostream& output, std::stringstream& secondaryOutput) {
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

void actionRemoveAddEnd(const searchable_t& matched, std::istreambuf_iterator<char>& begin, std::istreambuf_iterator<char>& end, std::ostream& destination, std::stringstream& secondaryOutput) {
    if (matched.end.size()) { // Если задан конец фрагмента, надо его найти и удалить (не копировать) символы до его конца.
        auto ee = search(begin, end, matched.end.begin(), matched.end.end());
        if (ee == end) {
            std::cout << " FATAL ERROR: Cannot find end of removable section " << matched.end;
            throw 1;
        }
        begin = ee;
        if (matched.action == A_REMOVE_ADD_END)
            destination << matched.end;
    }
}
void actionRemove(const searchable_t& matched, std::istreambuf_iterator<char>& begin, std::istreambuf_iterator<char>& end, std::ostream& destination, std::stringstream& secondaryOutput) {
    begin++;
    actionRemoveAddEnd(matched, begin, end, destination, secondaryOutput);
}
void actionMoveHere(const searchable_t& matched, std::istreambuf_iterator<char>& begin, std::istreambuf_iterator<char>& end, std::ostream& destination, std::stringstream& secondaryOutput) {
    destination << secondaryOutput.str() << matched.begin;
    begin++;
}
typedef void(*fnAction)(const searchable_t& matched, std::istreambuf_iterator<char>& begin, std::istreambuf_iterator<char>& end, std::ostream& destination, std::stringstream& secondaryOutput);

//Порядок должен соответствовать нумерации констант A_* !
fnAction actionMethods[] = {
    &actionDontTouch,
    &actionMoveToEnd,
    &actionRemove,
    &actionRemoveAddEnd,
    &actionReplace,
    &actionMoveHere
};
#endif