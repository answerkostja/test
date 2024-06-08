#include "formula.h"
#include "cell.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << fe.ToString();
}


// Реализуйте следующие методы:
    Formula::Formula(std::string expression) try
        :ast_(ParseFormulaAST(expression)) {
    }
    catch(const std::exception& exc){
        throw FormulaException("#ARITHM!");
    }

    
    Formula::Value Formula::Evaluate(const SheetInterface& sheet) const{
            if (std::holds_alternative<double>(ast_.Execute(sheet))) {
                return std::get<double>(ast_.Execute(sheet));
            }
            else if (std::holds_alternative<std::string>(ast_.Execute(sheet))) {
                return FormulaError{ FormulaError::Category::Value };
            } else {
                return std::get<FormulaError>(ast_.Execute(sheet));
            }
            
    }

    std::vector<Position> Formula::GetReferencedCells() const {
        std::vector<Position> result;
        std::set<Position> result_set;
        for (auto iter = ast_.GetCells().begin(); iter != ast_.GetCells().end(); iter++) {
            result_set.insert(*iter);
        }
        for (auto iter = result_set.begin(); iter != result_set.end(); iter++) {
            result.push_back(*iter);
        }
        return result;
    }
    std::string Formula::GetExpression() const{
        
        std::ostringstream out;
        ast_.PrintFormula(out);
        std::string line(out.str());
        return line;
    }


