#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::~Sheet() = default;

void Sheet::SetCell(Position pos, std::string text) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("");
    }
    size_t table_size_row = table.size();

    for (int i = table_size_row; i <= pos.row; i++) {
        table.push_back(std::vector<std::shared_ptr<Cell>>{});
    }

    int table_size_col = table.at(0).size();

    for (int i = 0; i < static_cast<int>(table_size_row); i++) {
        for (int j = table_size_col; j <= pos.col; j++) {
            table.at(i).push_back(std::shared_ptr<Cell> { new Cell{*this}});
        }
    }
    table_size_col = std::max(static_cast<int>(table.at(0).size() - 1), pos.col);

    for (int i = table_size_row; i < static_cast<int>(table.size()); i++) {
        for (int j = 0; j <= table_size_col; j++) {
            table.at(i).push_back(std::shared_ptr<Cell> { new Cell{ *this }});
        }
    }

    table.at(pos.row).at(pos.col)->Set(text);
}

const CellInterface* Sheet::GetCell(Position pos) const {
    if (!pos.IsValid()) {
        throw InvalidPositionException("");
    }
    if (pos.row >= static_cast<int>(table.size()) || pos.col >= static_cast<int>(table[0].size())) {
        return nullptr;
    }
    if (table[pos.row][pos.col]->IsEmpty()) {
        return table[pos.row][pos.col].get();
    }
    return table[pos.row][pos.col].get();
}
CellInterface* Sheet::GetCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("");
    }
    if (pos.row >= static_cast<int>(this->table.size())) {
        return nullptr;
    }else{
        if (this->table.size() == 0) {
            return nullptr;
        }
        else {
            if (pos.col >= static_cast<int>(this->table[0].size())) {
                return nullptr;
            }
        }
    }
    if (table[pos.row][pos.col]->IsEmpty()) {
        return table[pos.row][pos.col].get();
    }
    return table[pos.row][pos.col].get();
}

void Sheet::ClearCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("");
    }
    if (pos.row >= static_cast<int>(this->table.size())) {
        return;
    }
    if (this->table.size() == 0) {
        return;
    }
    else {
        if (pos.col >= static_cast<int>(this->table[0].size())) {
            return;
        }
    }

    bool table_null = table.size() == 0;
    if (static_cast<int>(table.size()) < pos.row + 1 || table_null) {
        this->SetCell(pos, "");
    }
    if (!table_null) {
        if (static_cast<int>(table.at(0).size()) < pos.col) {
            this->SetCell(pos, "");
        }
    }
    table[pos.row][pos.col]->Clear();
    if (pos.row != static_cast<int>(table.size()) - 1 || pos.col != static_cast<int>(table[0].size()) - 1) {
        return;
    }
    bool empty = true;
    if (pos.row != 0) {
        for (int i = pos.row; i >= 0; i--) {
            for (int j = 0; j < pos.col; j++) {
                if (!table[i][j]->IsEmpty()) {
                    empty = false;
                }
            }
            if (empty) {
                table.erase(table.begin() + table.size() - 1);
            }
            else {
                break;
            }
        }
    }
    empty = true;
    if (pos.col != 0 && table.size() != 0) {
        for (int j = pos.col; j >= 0; j--) {
            for (int i = table.size() - 1; i >= 0; i--) {
                if (!table[i][j]->IsEmpty()) {
                    empty = false;
                }
            }
            if (empty) {
                for (int i = table.size() - 1; i >= 0; i--) {
                    table[i].erase(table[i].begin() + table[i].size() - 1);
                }
            }
            else {
                break;
            }
        }
    }
}

Size Sheet::GetPrintableSize() const {
    if (table.size() == 0) {
        return Size{ 0,0 };
    }
    int rows = table[0].size();
    int cows = table.size();
    if (table.size() == 1 && table[0].size() == 1) {
        if (table[0][0]->GetText() == "") {
            return Size{ 0 , 0 };
        }
    }
    return Size{ cows , rows };
}

void Sheet::PrintValues(std::ostream& output) const {
    if (this->table.size() == 1 && this->table[0].size() == 1) {
        if (this->table[0][0]->IsEmpty()) {
            return;
        }
    }
    bool is_null = true;
    for (size_t i = 0; i < table.size(); i++) {
        for (size_t j = 0; j < table[0].size(); j++) {
            
            if (std::holds_alternative<double>(table[i][j]->GetValue())) {
                double result = std::get<double>(table[i][j]->GetValue());
                if (result != 0.) {
                        if (is_null) {
                            output << result;
                            is_null = false;
                        }
                        else {
                            output << '\t' << result;
                        }
                }
                else if (table[i][j]->GetText() == "0") {
                    if (is_null) {
                        output << result;
                        is_null = false;
                    }
                    else {
                        output << '\t' << result;
                    }
                } 
                else {
                    if (is_null) {
                            is_null = false;
                        }
                        else {
                            output << '\t';
                        }
                    }
                }
                else if (std::holds_alternative<std::string>(table[i][j]->GetValue())) {
                    std::string result = std::get<std::string>(table[i][j]->GetValue());
                    if (is_null) {
                        output << result;
                        is_null = false;
                    }
                    else {
                        output << '\t' << result;
                    }
                }
                else {
                    if (is_null) {
                        output << std::get<FormulaError>(table[i][j]->GetValue()).ToString();
                        is_null = false;
                    }
                    else {
                        output << '\t' << std::get<FormulaError>(table[i][j]->GetValue()).ToString();
                    }
                }
        }
        output << '\n';
        is_null = true;
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
    if (this->table.size() == 1 && this->table[0].size() == 1) {
        if (this->table[0][0]->IsEmpty()) {
            return;
        }
    }
    bool is_null = true;
    for (size_t i = 0; i < table.size(); i++) {
        for (size_t j = 0; j < table[0].size(); j++) {
            if (is_null) {
                output << table[i][j]->GetText();
                is_null = false;
            }
            else {
                output << '\t' << table[i][j]->GetText();
            }
        }
        output << '\n';
        is_null = true;
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}