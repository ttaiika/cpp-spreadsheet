#include "formula.h"
 
#include "FormulaAST.h"
 
#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>
 
using namespace std::literals;
 
std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << "#ARITHM!";
}
 
namespace {
class Formula : public FormulaInterface {
public:
    explicit Formula(std::string expression) try : ast_(ParseFormulaAST(expression)) {}
    catch (...) {
        throw FormulaException("The formula is syntactically incorrect");
    }
    
    Value Evaluate(const SheetInterface& sheet) const override {
        try {
        std::function<double(Position)> args = [&sheet](const Position pos) -> double {
            if (!pos.IsValid()) {
                throw FormulaError(FormulaError::Category::Ref);
            }

            const auto* cell = sheet.GetCell(pos);
            if (!cell) {
                return 0.0;  // пустая ячейка
            }

            const auto& value = cell->GetValue();

            if (std::holds_alternative<double>(value)) {
                return std::get<double>(value);
            }
            else if (std::holds_alternative<std::string>(value)) {
                const auto& str = std::get<std::string>(value);
                if (str.empty()) {
                    return 0.0;
                }

                char* end = nullptr;
                double num = std::strtod(str.c_str(), &end);
                if (end == str.c_str() || *end != '\0') {
                    throw FormulaError(FormulaError::Category::Value);
                }
                return num;
            }
            else {
                throw std::get<FormulaError>(value);
            }
        };
        return ast_.Execute(args);
        } catch (const FormulaError& err) {
            return err;
        }
    }
    
    std::string GetExpression() const override {
        std::ostringstream out;
        ast_.PrintFormula(out);
        
        return out.str();
    }
    
    std::vector<Position> GetReferencedCells() const override {
        std::vector<Position> cells;
        for (const auto& cell : ast_.GetCells()) {         
            if (cell.IsValid()) {
                cells.push_back(cell);
            } else {
                continue;
            }
        }
        return cells;
    }
 
private:
    FormulaAST ast_;
};
    
}//end namespace
 
std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}
