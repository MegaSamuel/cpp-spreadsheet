#pragma once

#include "cell.h"
#include "common.h"

#include <functional>

class Sheet : public SheetInterface {
public:
    ~Sheet();

    // установить ячейку в позицию pos
    void SetCell(Position pos, std::string text) override;

    // получить ячейку в позиции pos
    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    // очистить ячейку в позиции pos
    void ClearCell(Position pos) override;

    // получить размер печатной области
    Size GetPrintableSize() const override;

    // напечатать значение ячейки
    void PrintValues(std::ostream& output) const override;

    // напечатать текст ячейки
    void PrintTexts(std::ostream& output) const override;

private:
    // проверка ячейки на зациклическую зависимость за О(1)
    bool IsCircularDependency(const CellInterface* cell, const Position& pos) const;

    // получить самый левый верхний и самый правый нижний углы зоны с ячейками
    std::pair<Position, Position> GetArea() const;

    // структура хэшера для таблицы
    struct PosHasher { 
        size_t operator()(const Position& p) const;
    };

    // структура для печати содержимого ячейки
    struct Visitor {
        std::string operator() (std::string val) const;
        std::string operator() (double val) const;
        std::string operator() (FormulaError val) const;
    };

    // используем хэш-таблицу т.к. получать ячейку должны за О(1)
    std::unordered_map<Position, std::unique_ptr<CellInterface>, PosHasher> cell_map_;
};
