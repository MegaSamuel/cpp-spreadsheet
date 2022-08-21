#include "cell.h"
#include "sheet.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>
#include <stack>

//-----------------------------------------------------------------------------

// базовая ячейка
class Cell::Impl {
public:
    virtual ~Impl() = default;

    virtual Value GetValue() const = 0;

    virtual std::string GetText() const = 0;

    virtual std::vector<Position> GetReferencedCells() const {
        return {};
    }

    virtual void CacheInvalidate() {}
};

//-----------------------------------------------------------------------------

// пустая ячейка
class Cell::EmptyImpl : public Impl {
public:
    Value GetValue() const override {
        return "";
    }

    std::string GetText() const override {
        return "";
    }
};

//-----------------------------------------------------------------------------

// ячейка с текстом
class Cell::TextImpl : public Impl {
public:
    TextImpl(std::string text) : text_(std::move(text)) {
        if (text_.empty()) {
            throw std::logic_error("");
        }
    }

    Cell::Value GetValue() const override {
        if (text_[0] == ESCAPE_SIGN) {
            return text_.substr(1);
        }
        return text_;
    }
    
    std::string GetText() const override {
        return text_;
    }

private:
    std::string text_;
};

//-----------------------------------------------------------------------------

// ячейка с формулой
class Cell::FormulaImpl : public Impl {
public:
    explicit FormulaImpl(std::string expression, const SheetInterface& sheet)
        : sheet_(sheet) {
        if (expression.empty() || expression[0] != FORMULA_SIGN) {
            throw std::logic_error("");
        }
        formula_ptr_ = ParseFormula(expression.substr(1));
    }
    
    Value GetValue() const override {
        FormulaInterface::Value value;
        if (!cache_.has_value()) {
            cache_ = formula_ptr_->Evaluate(sheet_);
        }
        value = cache_.value();
        if (std::holds_alternative<double>(value)) {
            return std::get<double>(value);
        }
        return std::get<FormulaError>(value);
    }
    
    std::string GetText() const override {
        return FORMULA_SIGN + formula_ptr_->GetExpression();
    }

    std::vector<Position> GetReferencedCells() const override {
        return formula_ptr_->GetReferencedCells();
    }

    void CacheInvalidate() override {
        cache_.reset();
    }

private:
    std::unique_ptr<FormulaInterface> formula_ptr_;
    const SheetInterface& sheet_;
    mutable std::optional<FormulaInterface::Value> cache_;
};

//-----------------------------------------------------------------------------

Cell::Cell(Sheet& sheet) : impl_(std::make_unique<EmptyImpl>()),
                           sheet_(sheet) {
}

Cell::~Cell() {
}

void Cell::Set(std::string text) {
    std::unique_ptr<Impl> impl;

    if (text.empty()) {
        impl = std::make_unique<EmptyImpl>();
    }
    else if (text.size() > 1 && text[0] == FORMULA_SIGN) {
        impl = std::make_unique<FormulaImpl>(std::move(text), sheet_);
    }
    else {
        impl = std::make_unique<TextImpl>(std::move(text));
    }

    if (IsCircularDependency(*impl)) {
        // новая ячейка создает цикл
        throw CircularDependencyException("");
    }

    impl_ = std::move(impl);

    for (const auto& pos : impl_->GetReferencedCells()) {
        Cell* cell = static_cast<Cell*>(sheet_.GetCell(pos));
        
        if (nullptr == cell) {
            sheet_.SetCell(pos, "");
            cell = static_cast<Cell*>(sheet_.GetCell(pos));
        }

        cell->children_.insert(this);
    }

    CacheInvalidate();
}

void Cell::Clear() {
    impl_ = std::make_unique<EmptyImpl>();
}

Cell::Value Cell::GetValue() const {
    return impl_->GetValue();
}

std::string Cell::GetText() const {
    return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
    return impl_->GetReferencedCells();
}

//-----------------------------------------------------------------------------


void Cell::CacheInvalidate() {
    impl_->CacheInvalidate();

    for (Cell* cell : children_) {
        cell->CacheInvalidate();
    }
}

bool Cell::IsCircularDependency(const Impl& new_impl) const {
    // перекладываем зависимые ячейки в множество для обеспечения поиска за О(1)
    std::unordered_set<const Cell*> referenced_cells;
    for (const auto& pos : new_impl.GetReferencedCells()) {
        const Cell* cell = static_cast<Cell*>(sheet_.GetCell(pos));
        referenced_cells.insert(cell);
    }

    // стек для обхода ячеек
    std::stack<const Cell*> to_visit;
    // добавляем себя
    to_visit.push(this);

    // множество посещенных ячеек
    std::unordered_set<const Cell*> already_visited;

    while (!to_visit.empty()) {
        // берем ячейку из стека
        const Cell* curr = to_visit.top();
        to_visit.pop();

        // нашли себя - цикл
        if (referenced_cells.count(curr)) {
            return true;
        }

        // добавляем в посещенные
        already_visited.insert(curr);

        // добавляем в стек потомков текущей ячейки
        for (const Cell* cell : curr->children_) {
            if (!already_visited.count(cell)) {
                to_visit.push(cell);
            }
        }
    }

    return false;
}

//-----------------------------------------------------------------------------
