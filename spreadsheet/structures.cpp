#include "common.h"

#include <cctype>
#include <sstream>
#include <algorithm>

const int LETTERS = 26;
const int MAX_POSITION_LENGTH = 17;
const int MAX_POS_LETTER_COUNT = 3;

const Position Position::NONE = {-1, -1};

bool Position::operator==(const Position rhs) const {
    return (row == rhs.row) && (col == rhs.col);
}

bool Position::operator<(const Position rhs) const {
    return std::make_pair(row, col) < std::make_pair(rhs.row, rhs.col);
}

bool Position::IsValid() const {
    return (row >= 0) && (col >= 0) &&
           (row < MAX_ROWS) && (col < MAX_COLS);
}

std::string Position::ToString() const {
    // проверка корректности
    if (!IsValid()) {
        return "";
    }
    std::string str;
    int pos = col;
    // формируем буквенное представление строки
    while (pos >= 0) {
        str.insert(str.begin(), 'A' + pos % LETTERS);
        pos = pos / LETTERS - 1;
    }
    // формируем цифровое представление стролбца
    str += std::to_string(row + 1);
    return str;
}

Position Position::FromString(std::string_view str) {
    // ищем итератор на первую цифру во вводе
    auto it = std::find_if(str.begin(), str.end(),
        [](const unsigned char c) {
            return !(std::isalpha(c) && std::isupper(c));
        });

    // строковые представления
    std::string_view str_col = str.substr(0, it - str.begin());
    std::string_view str_row = str.substr(it - str.begin());

    // проверка корректности ввода
    if (str_col.empty() || str_row.empty()) {
        return Position::NONE;
    }

    if (str_col.size() > MAX_POS_LETTER_COUNT) {
        return Position::NONE;
    }

    int col = 0;
    for (const char ch : str_col) {
        col *= LETTERS;
        col += ch - 'A' + 1;
    }

    // на первой позиции может стоять знак
    if (!isdigit(str_row[0])) {
        return Position::NONE;
    }

    int row = 0;
    std::istringstream row_in{static_cast<std::string>(str_row)};
    if (!(row_in >> row) || !row_in.eof()) {
        return Position::NONE;
    }

    return Position{row-1, col-1};
}

bool Size::operator==(Size rhs) const {
    return (cols == rhs.cols) && (rows == rhs.rows);
}
