#pragma once

#include "common.h"
#include "formula.h"

class Cell : public CellInterface {
public:
    Cell(const SheetInterface& sheet);
    ~Cell();

    // установить текст ячейки
    void Set(std::string text);

    // очистить ячейку
    void Clear();

    // получить значение ячейки за О(К) впервые и за О(1) повторно
    Value GetValue() const override;

    // получить текст ячейки за О(1)
    std::string GetText() const override;

    // получить ячейки от которых зависим
    std::vector<Position> GetReferencedCells() const override;

private:
    class Impl;        // базовая ячейка
    class EmptyImpl;   // пустая ячейка
    class TextImpl;    // ячейка ч текстом
    class FormulaImpl; // ячейка с формулой

    // указатель на реализацию ячейки
    std::unique_ptr<Impl> impl_;

    // ссылка на лист к которому принадлежит ячейка
    SheetInterface& sheet_;
};
