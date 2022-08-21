#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

//-----------------------------------------------------------------------------

Sheet::~Sheet() {
}

void Sheet::SetCell(Position pos, std::string text) {
    if(!pos.IsValid()) {
        throw InvalidPositionException("Wrong position "s + std::to_string(pos.row) + 
                                       ", "s + std::to_string(pos.col));
    }
    // если ячейка новая - создаем
    if (!cell_map_.count(pos)) {
        cell_map_.emplace(pos, std::make_unique<Cell>(*this));
    }
    // добавляем текст
    cell_map_.at(pos)->Set(std::move(text));
}

const CellInterface* Sheet::GetCell(Position pos) const {
    return getCell(pos);
}

CellInterface* Sheet::GetCell(Position pos) {
    return getCell(pos);
}

void Sheet::ClearCell(Position pos) {
    if(!pos.IsValid()) {
        throw InvalidPositionException("Wrong position "s + std::to_string(pos.row) + 
                                       ", "s + std::to_string(pos.col));
    }
    cell_map_.erase(pos);
}

Size Sheet::GetPrintableSize() const {
    if(cell_map_.empty()) {
        return Size{0, 0};
    }
    auto [left_top, right_bottom] = getArea();
    return Size{right_bottom.row-left_top.row+1,
                right_bottom.col-left_top.col+1};
}

void Sheet::PrintValues(std::ostream& output) const {
    if(cell_map_.empty()) {
        return;
    }
    // берем область с ячейками
    auto [left_top, right_bottom] = getArea();
    // проходим по строкам
    for(int row = left_top.row; row <= right_bottom.row; ++row) {
        // проходим по столбцам
        for(int col = left_top.col; col <= right_bottom.col; ++col) {
            if(col > 0) {
                output << '\t';
            }
            auto ptr_cell = GetCell(Position{row, col});
            if(nullptr != ptr_cell) {
                // если ячейка есть - печатаем
                std::visit([&](const auto value) {output << value;}, ptr_cell->GetValue());
            }
        }
        output << '\n';
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
    if(cell_map_.empty()) {
        return;
    }
    // берем область с ячейками
    auto [left_top, right_bottom] = getArea();
    // проходим по строкам
    for(int row = left_top.row; row <= right_bottom.row; ++row) {
        // проходим по столбцам
        for(int col = left_top.col; col <= right_bottom.col; ++col) {
            if(col > 0) {
                output << '\t';
            }
            auto ptr_cell = GetCell(Position{row, col});
            if(nullptr != ptr_cell) {
                // если ячейка есть - печатаем
                output << ptr_cell->GetText();
            }
        }
        output << '\n';
    }
}

//-----------------------------------------------------------------------------

std::pair<Position, Position> Sheet::getArea() const {
    // Position LeftTop{Position::MAX_ROWS, Position::MAX_COLS};
    Position LeftTop{0, 0};
    Position RightBottom{0, 0};

    // если есть таблица
    if(!cell_map_.empty()) {
        // ищем самый левый верхний и самый правый нижний уголы
        for (const auto& [key, ptr_cell] : cell_map_) {
            if (nullptr != ptr_cell) {
                // LeftTop.row = std::min(LeftTop.row, key.row);
                // LeftTop.col = std::min(LeftTop.col, key.col);
                RightBottom.row = std::max(RightBottom.row, key.row);
                RightBottom.col = std::max(RightBottom.col, key.col);
            }
        }
    }
    return std::make_pair(LeftTop, RightBottom);
}

// собственный хэшер для мапы
size_t Sheet::PosHasher::operator() (const Position& p) const {
    return std::hash<int>()(p.row)+Position::MAX_COLS*std::hash<int>()(p.col);
}

CellInterface* Sheet::getCell(Position pos) const {
    if(!pos.IsValid()) {
        throw InvalidPositionException("Wrong position "s + std::to_string(pos.row) + 
                                       ", "s + std::to_string(pos.col));
    }
    // возвращаем указатель на ячейку в позиции pos
    return cell_map_.count(pos) ? cell_map_.at(pos).get() : nullptr;
}

//-----------------------------------------------------------------------------

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}
