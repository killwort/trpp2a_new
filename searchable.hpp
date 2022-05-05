#ifndef SEARCHABLE_HPP
#define SEARCHABLE_HPP
#include <string>
//��������� �������� � �����������:
//--------------
//�� ������� ��������� - ������ ����������� �� �����. ������� ��� escape-�������������������, ��������, \\ ��� \%
#define A_DONT_TOUCH 0
//����������� �������� � ����� ���������
#define A_MOVE_TO_END 1
//������� ��������
#define A_REMOVE 2
//������� ��������, �� �������� ������� ������������ ����� ���������. ������� ��� �������� ������������ ������������ � ����� ��������� (\n) �� ���������� ��������
#define A_REMOVE_ADD_END 3
//�������� ������ �������� searchable_t::begin �� searchable_t::end
#define A_REPLACE 4
//������, ���� ����� ��������� ������������ ���������
#define A_MOVE_HERE 5

typedef struct {
    //������ ���������
    std::string begin;
    //����� ��������� (��� ������ ������, ���� �������� ������� ������ �� ������, ��� ������ �� ������� ����� ���������� ������ ��� �������� A_REPLACE
    std::string end;
    //��������
    int action;
} searchable_t;

//����������� �������� �� ��������
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