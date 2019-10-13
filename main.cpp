#include <iostream>
#include <fstream>
#include <string>
#include <array>
#include <vector>
#include <bitset>
#include <optional>
#include "Arguments.h"

const int SIZE = 9;
const int SECTION = 3;

using SudokuArray = std::array<std::array<int, SIZE>, SIZE>;
using AllowedNumbers = std::array<std::array<std::bitset<SIZE>, SIZE>, SIZE>;

std::optional<SudokuArray> ParseSudoku(const std::string &input) {
    if (input.size() != SIZE * SIZE) {
        throw std::invalid_argument("Sudoku does not have correct number of cells\n");
    }

    SudokuArray sudoku;
    for (size_t i = 0; i < SIZE * SIZE; ++i) {
        if (input[i] < '0' || input[i] > '9') {
            std::cerr << "Invalid character in input data!\n";
            return {};
        }
        size_t j = i / 9;
        sudoku[j][i - (j * 9)] = input[i] - '0';
    }

    return {sudoku};
}

void Print(std::ostream &out, const SudokuArray &sudoku) {
    for (size_t i = 0; i < SIZE; ++i) {
        for (size_t j = 0; j < SIZE; ++j) {
            out << sudoku[i][j];
        }
    }
    out << '\n';
}

AllowedNumbers InitAllowedNumbers() {
    AllowedNumbers allowedNumbers;
    for (size_t i = 0; i < SIZE; ++i)
        for (size_t j = 0; j < SIZE; ++j)
            allowedNumbers[i][j].flip();
    return allowedNumbers;
}

void UpdateAllowedNumbers(const SudokuArray &sudoku, AllowedNumbers &allowedNumbers, size_t x, size_t y) {
    // Section
    size_t sectionX = x - (x % SECTION);
    size_t sectionY = y - (y % SECTION);
    for (size_t i = sectionX; i < sectionX + SECTION; ++i)
        for (size_t j = sectionY; j < sectionY + SECTION; ++j)
            allowedNumbers[i][j][sudoku[x][y] - 1ULL] = false;

    // Rows and Columns
    for (size_t i = 0; i < SIZE; ++i)
        allowedNumbers[x][i][sudoku[x][y] - 1ULL] = false;
    for (size_t i = 0; i < SIZE; ++i)
        allowedNumbers[i][y][sudoku[x][y] - 1ULL] = false;
}

int PopulateAllowedNumbers(const SudokuArray &sudoku, AllowedNumbers &allowedNumbers) {
    int numbersPlaced = 0;
    for (size_t i = 0; i < SIZE; ++i) {
        for (size_t j = 0; j < SIZE; ++j) {
            if (sudoku[i][j] > 0) {
                allowedNumbers[i][j].reset();
                UpdateAllowedNumbers(sudoku, allowedNumbers, i, j);
                ++numbersPlaced;
            }
        }
    }
    return numbersPlaced;
}

bool IsValid(const SudokuArray &sudoku) {
    for (size_t i = 0; i < SIZE; ++i) {
        std::bitset<SIZE> rows, columns;
        for (size_t j = 0; j < SIZE; ++j) {
            rows.set(sudoku[i][j] - 1);
            columns.set(sudoku[j][i] - 1);
        }
        if (!rows.all() || !columns.all()) return false;
    }

    for (size_t i = 0; i < SIZE; i += 3) {
        for (size_t j = 0; j < SIZE; j += 3) {
            std::bitset<SIZE> section;
            for (size_t x = i; x < i + 3; ++x) {
                for (size_t y = j; y < j + 3; ++y) {
                    section.set(sudoku[x][y] - 1);
                }
            }
            if (!section.all()) return false;
        }
    }

    return true;
}

void PlaceNumber(SudokuArray &sudoku, AllowedNumbers &allowedNumbers, size_t x, size_t y, int value) {
    sudoku[x][y] = value;
    allowedNumbers[x][y].reset();
    UpdateAllowedNumbers(sudoku, allowedNumbers, x, y);
}

int GetLastAllowedNumber(const AllowedNumbers &allowedNumbers, size_t x, size_t y) {
    for (int i = SIZE - 1; i >= 0; --i)
        if (allowedNumbers[x][y][i])
            return i + 1;
    return -1;
}

size_t CountAllowedNumbers(const AllowedNumbers &allowedNumbers, size_t x, size_t y) {
    size_t count = 0;
    for (size_t i = 0; i < SIZE; ++i)
        if (allowedNumbers[x][y][i])
            count++;
    return count;
}

int PlaceOnlyOneAllowed(SudokuArray &sudoku, AllowedNumbers &allowedNumbers) {
    int numbersPlaced = 0;
    for (size_t i = 0; i < SIZE; ++i) {
        for (size_t j = 0; j < SIZE; ++j) {
            if (CountAllowedNumbers(allowedNumbers, i, j) == 1) {
                PlaceNumber(sudoku, allowedNumbers, i, j, GetLastAllowedNumber(allowedNumbers, i, j));
                ++numbersPlaced;
            }
        }
    }
    return numbersPlaced;
}

int PlaceOneSectionAllowed(SudokuArray &sudoku, AllowedNumbers &allowedNumbers) {
    int numbersPlaced = 0;
    for (size_t sectionX = 0; sectionX < SIZE; sectionX += SECTION) {
        for (size_t sectionY = 0; sectionY < SIZE; sectionY += SECTION) {
            for (int k = 1; k <= 9; ++k) {
                int allowedX = -1;
                int allowedY = -1;
                bool skip = false;
                for (size_t cellX = sectionX; !skip && cellX < sectionX + SECTION; ++cellX) {
                    for (size_t cellY = sectionY; cellY < sectionY + SECTION; ++cellY) {
                        if (allowedNumbers[cellX][cellY][k - 1]) {
                            if (allowedX == -1) {
                                allowedX = cellX;
                                allowedY = cellY;
                            } else {
                                allowedX = -1;
                                skip = true;
                                break;
                            }
                        }
                    }
                }
                if (allowedX >= 0) {
                    PlaceNumber(sudoku, allowedNumbers, allowedX, allowedY, k);
                    ++numbersPlaced;
                }
            }
        }
    }
    return numbersPlaced;
}

int PlaceOneRowOrColumnAllowed(SudokuArray &sudoku, AllowedNumbers &allowedNumbers) {
    int numbersPlaced = 0;
    for (int value = 1; value <= 9; ++value) {
        // Rows
        for (size_t i = 0; i < SIZE; ++i) {
            int allowedY = -1;
            for (size_t j = 0; j < SIZE; ++j) {
                if (allowedNumbers[i][j][value - 1]) {
                    if (allowedY == -1) {
                        allowedY = j;
                    } else {
                        allowedY = -1;
                        break;
                    }
                }
            }
            if (allowedY >= 0) {
                PlaceNumber(sudoku, allowedNumbers, i, allowedY, value);
                ++numbersPlaced;
            }
        }

        // Columns
        for (size_t j = 0; j < SIZE; ++j) {
            int allowedX = -1;
            for (size_t i = 0; i < SIZE; ++i) {
                if (allowedNumbers[i][j][value - 1]) {
                    if (allowedX == -1) {
                        allowedX = i;
                    } else {
                        allowedX = -1;
                        break;
                    }
                }
            }
            if (allowedX >= 0) {
                PlaceNumber(sudoku, allowedNumbers, allowedX, j, value);
                ++numbersPlaced;
            }
        }
    }
    return numbersPlaced;
}

std::optional<SudokuArray> AttemptBruteForce(SudokuArray &sudoku, AllowedNumbers &allowedNumbers, int numbersPlaced);

int SolveSudoku(SudokuArray &sudoku, AllowedNumbers &allowedNumbers, int numbersPlaced) {
    int lastNumbersPlaced = 0;
    while (numbersPlaced - lastNumbersPlaced > 3) {
        lastNumbersPlaced = numbersPlaced;
        numbersPlaced += PlaceOnlyOneAllowed(sudoku, allowedNumbers);
        numbersPlaced += PlaceOneSectionAllowed(sudoku, allowedNumbers);
        numbersPlaced += PlaceOneRowOrColumnAllowed(sudoku, allowedNumbers);
    }

    if (numbersPlaced < 81) {
        std::optional<SudokuArray> bruteForcedSudoku = AttemptBruteForce(sudoku, allowedNumbers, numbersPlaced);

        if (bruteForcedSudoku.has_value()) {
            numbersPlaced = 0;
            for (size_t i = 0; i < SIZE; ++i) {
                for (size_t j = 0; j < SIZE; ++j) {
                    sudoku[i][j] = (*bruteForcedSudoku)[i][j];

                    if ((*bruteForcedSudoku)[i][j] > 0) {
                        ++numbersPlaced;
                    }
                }
            }
        }
    }

    return numbersPlaced;
}

std::optional<SudokuArray> AttemptBruteForce(SudokuArray &sudoku, AllowedNumbers &allowedNumbers, int numbersPlaced) {
    for (size_t i = 0; i < SIZE; ++i) {
        for (size_t j = 0; j < SIZE; ++j) {
            if (sudoku[i][j] == 0) {
                for (int value = 1; value <= 9; ++value) {
                    if (allowedNumbers[i][j][value - 1]) {
                        SudokuArray sudokuCopy = sudoku;
                        AllowedNumbers allowedNumbersCopy = allowedNumbers;
                        PlaceNumber(sudokuCopy, allowedNumbersCopy, i, j, value);
                        int placedNumbers = SolveSudoku(sudokuCopy, allowedNumbersCopy, numbersPlaced + 1);
                        if (placedNumbers == 81) {
                            return {sudokuCopy};
                        }
                    }
                }
                return {};
            }
        }
    }
    return {};
}

bool SolveSudoku(SudokuArray &sudoku) {
    AllowedNumbers allowedNumbers = InitAllowedNumbers();

    int numbersPlaced = PopulateAllowedNumbers(sudoku, allowedNumbers);

    int placed = SolveSudoku(sudoku, allowedNumbers, numbersPlaced);
    return placed == 81 && IsValid(sudoku);
}

void ReplaceAll(std::string &string, const std::string &from, const std::string &to) {
    size_t startPosition = 0;
    while ((startPosition = string.find(from, startPosition)) != std::string::npos) {
        string.replace(startPosition, from.length(), to);
        startPosition += to.length();
    }
}

void ProcessLines(std::istream &in, std::ostream &out) {
    std::string line;
    while (std::getline(in, line)) {
        ReplaceAll(line, ".", "0");
        std::optional<SudokuArray> sudoku = ParseSudoku(line);
        if (sudoku.has_value()) {
            if (SolveSudoku(*sudoku)) {
                Print(out, *sudoku);
            } else {
                out << '\n';
            }
        }
    }
}

bool MoveByOne(int &x, int &y) {
    ++y;
    if (y >= 9) {
        y = 0;
        ++x;
        if (x >= 9) return false;
    }
    return true;
}

void SolveCheckSudoku(SudokuArray &sudoku, AllowedNumbers &allowedNumbers, int numbersPlaced) {
    int placed = SolveSudoku(sudoku, allowedNumbers, numbersPlaced);
    if (placed == 81 && IsValid(sudoku)) {
        std::cout << "YES\n";
    } else {
        std::cout << "NO\n";
    }
}

void CheckSudoku(std::ifstream &inputFile) {
    SudokuArray sudoku;
    AllowedNumbers allowedNumbers;
    int numbersPlaced = 0;

    int x = 0, y = 0;
    std::string cell;
    while (inputFile >> cell) {
        if (cell.length() == 1) {
            sudoku[x][y] = std::stoi(cell);
            ++numbersPlaced;
        } else if (cell[0] == 'u') {
            std::string userCell = cell.substr(1);
            sudoku[x][y] = std::stoi(userCell);
            ++numbersPlaced;
        } else {
            for (const char &character : cell) {
                allowedNumbers[x][y].set((character - '0') - 1);
            }
            sudoku[x][y] = 0;
        }

        if (!MoveByOne(x, y)) break;
    }

    SolveCheckSudoku(sudoku, allowedNumbers, numbersPlaced);
}

int main(int argc, char *argv[]) {
    Arguments args;
    args.SetOption("-i");
    args.SetOption("-o");
    args.SetOption("--check");
    try {
        args.ParseArguments(argc, argv);
    } catch (const std::invalid_argument &error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
    if (args.HasOption("--check")) {
        if (args.GetParsedArgumentsCount() != 1) {
            std::cerr << "Invalid combination of options, when using \"--check\", other options cannot be specified!";
            return 1;
        }

        std::ifstream inputFile(args.GetOption("--check"));
        if (!inputFile.is_open()) {
            std::cerr << "Cannot open file to check!\n";
            return 3;
        }

        CheckSudoku(inputFile);
    } else {
        std::ifstream inputFile;
        std::ofstream outputFile;
        if (args.HasOption("-i")) {
            inputFile.open(args.GetOption("-i"));
            if (!inputFile.is_open()) {
                std::cerr << "Error: Cannot open input file for reading!\n";
                return 3;
            }
        }
        if (args.HasOption("-o")) {
            outputFile.open(args.GetOption("-o"));
            if (!outputFile.is_open()) {
                std::cerr << "Error: Cannot open output file for writing!\n";
                return 3;
            }
        }

        std::istream &in = inputFile.is_open() ? inputFile : std::cin;
        std::ostream &out = outputFile.is_open() ? outputFile : std::cout;
        ProcessLines(in, out);
    }
}
