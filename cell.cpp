#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>
#include <typeinfo>

std::unique_ptr<Formula> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}

// Реализуйте следующие методы
Cell::Cell(Sheet& sheet)
    :impl_(std::make_unique<EmptyImpl>(EmptyImpl{})),
    sheet_(sheet)
{
};

Cell::~Cell() = default;

void Cell::Set(std::string text) {
    if (text == "") {
        impl_ = std::move(std::make_unique<EmptyImpl>(EmptyImpl{}));
    }else if (text[0] == FORMULA_SIGN && text.size() != 1) {
        std::shared_ptr<Cell::Impl>impl_cache = std::move(impl_);
        impl_ = std::move(std::make_unique<FormulaImpl>(FormulaImpl(text, sheet_)));
        outgoing_refs_ = std::move(*impl_->GetReffs());
        for (const Cell& iter: outgoing_refs_) {
            AddInRef(*const_cast<Cell*>(&iter));
        }
        try {
            FindСycle(outgoing_refs_, beg_);
        }catch(...){
            impl_ = std::move(impl_cache);
            throw CircularDependencyException("CircularDependency");
        }
    }
    else {
        impl_ = std::move(std::make_unique<TextImpl>(TextImpl(text)));
    }
    
    
};

void Cell::Clear() {
    impl_ = std::move(std::make_unique<EmptyImpl>(EmptyImpl{}));
};

Cell::Value Cell::GetValue() const {
    return impl_->GetValue();
};

std::string Cell::GetText() const {
    return impl_->GetText();
};

std::vector<Position> Cell::GetReferencedCells() const {
    return impl_->GetReferencedCells();
};

bool Cell::IsEmpty() {
    if (impl_ == nullptr) {
        throw CircularDependencyException("CircularDependency");
    }
    if ((*impl_).GetText() == "") {
        return true;
    }
    return false;
}

bool Cell::IsReferenced() const {
    if (!outgoing_refs_.empty()) {
        return true;
    }
    return false;
};




