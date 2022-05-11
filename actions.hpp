#ifndef ACTIONS_HPP
#define ACTIONS_HPP
#include <iostream>
#include <string>
#include <algorithm>
#include <fstream>
#include <iterator>
#include <sstream>

template<class TChar>
void copy_till(std::istreambuf_iterator<TChar>& begin, std::istreambuf_iterator<TChar>& end, std::basic_ostream<TChar>& destination, std::basic_string<TChar> suffix);

template<class TChar>
void rollback(const searchable_t<TChar>& matched, std::basic_ostream<TChar>& destination) {
    destination.seekp(-((int)matched.begin.size()) + 1, std::ios::cur);
}

template<class TChar>
void actionReplace(const searchable_t<TChar>& matched, std::istreambuf_iterator<TChar>& begin, std::istreambuf_iterator<TChar>& end, std::basic_ostream<TChar>& destination, std::basic_stringstream<TChar>& secondaryOutput) {
    rollback(matched, destination);
    destination << matched.end;
    begin++;
}
template<class TChar>
void actionDontTouch(const searchable_t<TChar>& matched, std::istreambuf_iterator<TChar>& begin, std::istreambuf_iterator<TChar>& end, std::basic_ostream<TChar>& destination, std::basic_stringstream<TChar>& secondaryOutput) {
    //����� rollback(match, destination) �� �����!
    destination << *begin++;
}
template<class TChar>
void actionMoveToEnd(const searchable_t<TChar>& matched, std::istreambuf_iterator<TChar>& iter, std::istreambuf_iterator<TChar>& end, std::basic_ostream<TChar>& output, std::basic_stringstream<TChar>& secondaryOutput) {
    rollback(matched, output);
    // ��������� �� ��������� ��������� ������ ��������� - ��� ���� ������� �������
    secondaryOutput << matched.begin;
    iter++;
    // ���� ����� ����� ��������� - ������� ���, ������� ������� �� ��������� ���������
    if (matched.end.size()) {
        copy_till(iter, end, secondaryOutput, matched.end);
        secondaryOutput << *iter++;
    }
    secondaryOutput << std::endl << std::endl;
    output << " ..... ";
    std::istreambuf_iterator<TChar> lookahead (iter);
    lookahead++;
    if (*iter == ' ' && (*lookahead == '\r' || *lookahead == '\n')) {
        iter++;
        iter++;
        if (*iter == '\n') iter++;
    }
    else if (*iter == '\r' && *lookahead == '\n') {
        iter++;
        iter++;
    }
    else if (*iter == '\n')
        iter++;
}
template<class TChar>
void actionRemoveAddEnd(const searchable_t<TChar>& matched, std::istreambuf_iterator<TChar>& begin, std::istreambuf_iterator<TChar>& end, std::basic_ostream<TChar>& destination, std::basic_stringstream<TChar>& secondaryOutput) {
    rollback(matched, destination);
    if (matched.end.size()) { // ���� ����� ����� ���������, ���� ��� ����� � ������� (�� ����������) ������� �� ��� �����.
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
    rollback(matched, destination);
    destination << secondaryOutput.str() << matched.begin;
    begin++;
}


template<class TChar>
//������� ������ ��������������� ��������� �������� A_* !
void(*actionMethods[])(const searchable_t<TChar>& matched, std::istreambuf_iterator<TChar>& begin, std::istreambuf_iterator<TChar>& end, std::basic_ostream<TChar>& destination, std::basic_stringstream<TChar>& secondaryOutput)  = {
    &actionDontTouch,
    &actionMoveToEnd,
    &actionRemove,
    &actionRemoveAddEnd,
    &actionReplace,
    &actionMoveHere
};
#endif