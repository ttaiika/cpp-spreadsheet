#pragma once

#include "common.h"
#include "cell.h"

#include <functional>
#include <vector>

class Cell;

class Sheet : public SheetInterface {
public:
    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    CellInterface* GetCell(Position pos) override;
    const CellInterface* GetCell(Position pos) const override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

private:
    // Таблица
    std::vector<std::vector<std::unique_ptr<Cell>>> cells_;
};
