#include "sheet.h"
#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>
#include <memory>

using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
    if (!pos.IsValid()) throw InvalidPositionException("Invalid position");

    // Создаем строку, если нужно
    if (static_cast<size_t>(pos.row) >= cells_.size()) {
        cells_.resize(pos.row + 1);
    }

    // Создаем столбец, если нужно
    if (static_cast<size_t>(pos.col) >= cells_[pos.row].size()) {
        cells_[pos.row].resize(pos.col + 1);
    }

    // Создаем ячейку, если еще нет
    if (!cells_[pos.row][pos.col]) {
        cells_[pos.row][pos.col] = std::make_unique<Cell>(*this);
    }

    cells_[pos.row][pos.col]->Set(std::move(text));
}

const CellInterface* Sheet::GetCell(Position pos) const {
    if (!pos.IsValid()) throw InvalidPositionException("Invalid position");

    if (static_cast<size_t>(pos.row) < cells_.size() &&
        static_cast<size_t>(pos.col) < cells_[pos.row].size()) {
        return cells_[pos.row][pos.col].get();
    }
    return nullptr;
}

CellInterface* Sheet::GetCell(Position pos) {
    return const_cast<CellInterface*>(
        static_cast<const Sheet&>(*this).GetCell(pos)
        );
}

void Sheet::ClearCell(Position pos) {
    if (!pos.IsValid()) throw InvalidPositionException("Invalid cell position in ClearCell");
    if (pos.row < int(cells_.size()) && pos.col < int(cells_[pos.row].size()) && cells_[pos.row][pos.col]) {
        cells_[pos.row][pos.col]->Clear();
        if (!cells_[pos.row][pos.col]->IsReferenced()) {
            cells_[pos.row][pos.col].reset();
        }
    }
}

Size Sheet::GetPrintableSize() const {
    Size result{ 0, 0 };
    for (size_t i = 0; i < cells_.size(); ++i) {
        for (size_t j = 0; j < cells_[i].size(); ++j) {
            if (cells_[i][j]) {
                result.rows = std::max(result.rows, static_cast<int>(i + 1));
                result.cols = std::max(result.cols, static_cast<int>(j + 1));
            }
        }
    }
    return result;
}

void Sheet::PrintValues(std::ostream& output) const {
    Size size = GetPrintableSize();
    for (int row = 0; row < size.rows; ++row) {
        for (int col = 0; col < size.cols; ++col) {
            if (col > 0) output << '\t';
            if (row < int(cells_.size()) && col < int(cells_[row].size()) && cells_[row][col]) {
                std::visit([&output](const auto& value) {
                    using T = std::decay_t<decltype(value)>;
                    if constexpr (std::is_same_v<T, std::string>) output << value;
                    else if constexpr (std::is_same_v<T, double>) output << value;
                    else if constexpr (std::is_same_v<T, FormulaError>) output << value.ToString();
                    }, cells_[row][col]->GetValue());
            }
        }
        output << '\n';
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
    Size size = GetPrintableSize();
    for (int row = 0; row < size.rows; ++row) {
        for (int col = 0; col < size.cols; ++col) {
            if (col > 0) output << '\t';
            if (row < int(cells_.size()) && col < int(cells_[row].size()) && cells_[row][col]) {
                output << cells_[row][col]->GetText();
            }
        }
        output << '\n';
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}
