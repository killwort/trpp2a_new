#ifndef GLOB_HPP
#define GLOB_HPP
#include <iostream>
#include <string>
#include <list>
#include <algorithm>
#include <fstream>
#include <regex>
#include <iterator>
#include <filesystem>
#include <sstream>

#define GLOB_MANY_CHAR "*"
#define GLOB_ONE_CHAR "?"


template<class BidirIt, class Traits, class CharT, class UnaryFunction>
std::basic_string<CharT> regex_replace(BidirIt first, BidirIt last,
    const std::basic_regex<CharT, Traits>& re, UnaryFunction f) {
    std::basic_string<CharT> s;

    typename std::match_results<BidirIt>::difference_type
        positionOfLastMatch = 0;
    auto endOfLastMatch = first;

    auto callback = [&](const std::match_results<BidirIt>& match) {
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
std::string regex_replace(const std::string& s,
    const std::basic_regex<CharT, Traits>& re, UnaryFunction f) {
    return regex_replace(s.cbegin(), s.cend(), re, f);
}

std::regex globRegex("[" GLOB_MANY_CHAR GLOB_ONE_CHAR ".()\\[\\]{}?*\\\\]");


//Рекурсивный обход дерева файловой системы с учётом масок файлов/папок
void enumerateGlob(const std::filesystem::path& current, std::filesystem::path::iterator& path,
    const std::filesystem::path::const_iterator& end, std::list<std::filesystem::path>& filenames) {
    //Формируем регулярное выражение вместо символов подстановки
    std::string newPath("^");
    auto spath = path->string();
    newPath += regex_replace(spath, globRegex, [](auto const& match) {
        if (match.str() == GLOB_MANY_CHAR)
            return string(".*");
        else if (match.str() == GLOB_ONE_CHAR)
            return string(".");
        return string("\\") + match.str();

        });
    newPath += "$";
    std::regex thisRegex(newPath);

    //Это следующий элемент пути
    auto nextPath(path);
    nextPath++;

    //Теперь перечисляем все файлы/папки в текущей (current)
    for (auto& dirEntry : std::filesystem::directory_iterator(current)) {
        //Имя совпадает с заданным?
        if (std::regex_match(dirEntry.path().string(), thisRegex)) {
            //Это файл и мы сейчас просматриваем последний элемент пути? Кроме того, нет ли $ в конце имени файла
            if (dirEntry.is_regular_file() && nextPath == end &&
                *(dirEntry.path().filename().replace_extension("").string().end() - 1) != '$') {
                filenames.push_back(dirEntry.path());
            }
            else if (dirEntry.is_directory()) { //Это папка, продолжаем рекурсивный обход
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
void enumerateFiles(const std::filesystem::path& xpath, std::list<std::filesystem::path>& filenames) {
    //Быстрый выход - передан путь к существующему файлу
    if (std::filesystem::exists(xpath) && std::filesystem::is_regular_file(xpath)) {
        filenames.push_back(xpath);
        return;
    }

    auto path = absolute(xpath);

    std::filesystem::path current;
    for (auto i = path.begin(); i != path.end(); i++) {
        auto spath = i->string();
        if (find_if(spath.begin(), spath.end(),
            [](auto a) { return a == GLOB_MANY_CHAR[0] || a == GLOB_ONE_CHAR[0]; }) == spath.end()) {
            current /= *i;
        }
        else {
            enumerateGlob(current, i, path.end(), filenames);
        }
    }
}

#endif
