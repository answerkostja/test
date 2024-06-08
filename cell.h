#pragma once

#include "common.h"
#include "formula.h"
#include "FormulaAST.h"

#include <functional>
#include <unordered_set>
#include <optional>


class Cell;


class Sheet : public SheetInterface {
public:

    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

    // Можете дополнить ваш класс нужными полями и методами

private:
    // Можете дополнить ваш класс нужными полями и методами
    std::vector<std::vector<std::shared_ptr<Cell>>> table;
};


    class Formula : public FormulaInterface {
    public:
        // Реализуйте следующие методы:
        Formula(std::string expression);
        
        Value Evaluate(const SheetInterface& sheet) const override;

        std::vector<Position> GetReferencedCells() const override;
            
        std::string GetExpression() const override;

    private:
        FormulaAST ast_;
    };

std::unique_ptr<Formula> ParseFormula(std::string expression);

class Cell : public CellInterface {
public:

    class CellHasher {
    public:
        size_t operator() (const Cell& cell) const {

            return static_cast<size_t>(hasher_(cell.GetText()));
        }
    private:
        std::hash<std::string> hasher_;
    };

    Cell(Sheet& sheet);
    ~Cell();

    bool operator==(const Cell& other) const {
        return GetText() == other.GetText();
    }

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;

    std::shared_ptr<std::unordered_set<Cell, CellHasher>> GetInRef(){
        return std::make_shared<std::unordered_set<Cell, CellHasher>>(incoming_refs_);
    }
    void AddOutRef(Cell& cell){
        outgoing_refs_.insert(cell);
    }
    void AddInRef(Cell& cell){
        cell.GetInRef()->insert(*this);
    }

    bool IsReferenced() const;
    bool IsEmpty();
    void FindСycle(std::unordered_set<Cell, CellHasher>& unset, Cell* beg){
       
        for (size_t j = 0; j < GetReferencedCells().size(); j++) {
            if (sheet_.GetCell(GetReferencedCells()[j]) == nullptr) {
                continue;
            }
            unset.insert(*static_cast<Cell*>(sheet_.GetCell(GetReferencedCells()[j])));
            if (!(static_cast<Cell*>(sheet_.GetCell(GetReferencedCells()[j])))->GetReferencedCells().empty()) {
                if (unset.find(*beg) != unset.end()) {
                    throw CircularDependencyException("CircularDependency");
                }
                static_cast<Cell*>(sheet_.GetCell(GetReferencedCells()[j]))->FindСycle(unset, beg);
                
            }
        }
        

    };

private:

    class Impl {
    public:
        virtual Value GetValue() const = 0;
        virtual std::string GetText() const = 0;
        virtual std::shared_ptr<std::unordered_set<Cell, CellHasher>> GetReffs() {
            return std::make_shared<std::unordered_set<Cell, CellHasher>>(std::unordered_set<Cell, CellHasher>{});
        }
        virtual std::vector<Position> GetReferencedCells() const {
            return std::vector<Position>{};
        }
        
        
    private:
    };
    class EmptyImpl : public Impl {
    public:
        Value GetValue() const override {
            return 0.;
        }
        std::string GetText() const override {
            return str_;
        }
    private:
        std::string str_;
    };
    class TextImpl : public Impl {
    public:
        TextImpl(std::string str)
            :str_(str) {
        }

        void Set(std::string text) {
            str_ = text;
        }
        Value GetValue() const override {
            if (str_[0] == ESCAPE_SIGN) {
                if (str_.size() == 1) {
                    return "";
                }
                std::string str(str_);
                str.erase(0, 1);
                return str;
            }
            try {
                double result = std::stod(str_);
                return result;
            }
            catch (...) {};
            return str_;
            
        }
        std::string GetText() const override {
            return str_;
        }

    private:
        std::string str_;
    };
    class FormulaImpl : public Impl {
    public:
        FormulaImpl(std::string str, Sheet& sheet_formula)
            :str_(str),
            sheet_formula_(std::make_shared<Sheet>(sheet_formula)) {
            Set(str);
        }
        void Set(std::string text) {
            str_ = text;
            text.erase(0, 1);
            formula = std::move(ParseFormula(text));
            for (size_t i = 0; i < formula->GetReferencedCells().size(); i++) {
                if (sheet_formula_->GetCell(formula->GetReferencedCells()[i]) != nullptr) {
                    outgoing_refs_formula_.insert(*static_cast<Cell*>(sheet_formula_->GetCell(formula->GetReferencedCells()[i])));
                }
                else {
                    sheet_formula_->SetCell(formula->GetReferencedCells()[i], "");
                    outgoing_refs_formula_.insert(*static_cast<Cell*>(sheet_formula_->GetCell(formula->GetReferencedCells()[i])));
                }
            }
        }

        Value GetValue() const override {
            if (cache_.has_value()) {
                if (std::holds_alternative<double>(cache_.value())) {
                    return std::get<double>(cache_.value());
                }
                else {
                    return std::get<FormulaError>(cache_.value());
                }

            }
            if (std::holds_alternative<double>(formula->Evaluate(*sheet_formula_))) {
                double result = std::get<double>(formula->Evaluate(*sheet_formula_));
                cache_ = result;
                return result;
            }
            cache_ = std::get<FormulaError>(formula->Evaluate(*sheet_formula_));
            return std::get<FormulaError>(formula->Evaluate(*sheet_formula_));
        }
        std::string GetText() const override {
            return '=' + formula->GetExpression();
        }

        void InvalidCache() {
            cache_ = std::nullopt;
        };
        

        std::shared_ptr<std::unordered_set<Cell, CellHasher>> GetReffs() override{
            return std::make_shared<std::unordered_set<Cell, CellHasher>>(outgoing_refs_formula_);
        };

        std::vector<Position> GetReferencedCells() const override {
            return formula->GetReferencedCells();
        };

    private:
        std::string str_;
        std::unique_ptr<Formula> formula;
        std::shared_ptr<Sheet> sheet_formula_;
        mutable std::optional<FormulaInterface::Value> cache_;
        std::unordered_set<Cell, CellHasher> outgoing_refs_formula_;
        
    };

    std::shared_ptr<Impl> impl_;
    Sheet& sheet_;
    std::unordered_set<Cell, CellHasher> outgoing_refs_;
    std::unordered_set<Cell, CellHasher> incoming_refs_;
    Cell* beg_ = this;
    
    

    // Добавьте поля и методы для связи с таблицей, проверки циклических 
    // зависимостей, графа зависимостей и т. д.
};

