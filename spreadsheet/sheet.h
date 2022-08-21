#pragma once

#include "cell.h"
#include "common.h"

#include <unordered_map>
#include <functional>

class Sheet : public SheetInterface {
public:
    ~Sheet();

    // установить ячейку в позицию pos за О(1)
    void SetCell(Position pos, std::string text) override;

    // получить ячейку в позиции pos за О(1)
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
    // получить самый левый верхний и самый правый нижний углы зоны с ячейками
    std::pair<Position, Position> getArea() const;

    // структура хэшера для таблицы
    struct PosHasher { 
        size_t operator()(const Position& p) const;
    };

    // используем хэш-таблицу т.к. получать ячейку должны за О(1)
    std::unordered_map<Position, std::unique_ptr<Cell>, PosHasher> cell_map_;

    CellInterface* getCell(Position pos) const;
};
