#include <iostream>
#include <sstream>
#include <string>
#include <cmath>
#include <stdexcept>
#include "calc.h"
#include "../util.h"

void calc_help() {
    c_B("Usage: calc <function>\n");
    c_B("Example: calc \"2+2\" \n");
    c_B("Supported operators: +, -, *, /, ^ \n");
    c_B("Parentheses are not supported.\n");
    c_B("Function must be a valid mathematical expression. \n");
    c_B("Example: calc \"2+2*3\" = 8 \n");
}

double evaluateExpression(const std::string& expression) {
    std::istringstream iss(expression);
    double result = 0;
    double currentNumber = 0;
    char operation = '+';

    while (iss >> currentNumber) {
        switch (operation) {
            case '+':
                result += currentNumber;
                break;
            case '-':
                result -= currentNumber;
                break;
            case '*':
                result *= currentNumber;
                break;
            case '/':
                if (currentNumber != 0) {
                    result /= currentNumber;
                } else {
                    throw std::runtime_error("Division by zero");
                }
                break;
            case '^':
                result = std::pow(result, currentNumber);
                break;
        }
        iss >> operation;
    }

    return result;
}

double eval(const char *func) {
    std::string expression(func);

    std::istringstream iss(expression);
    std::ostringstream intermediateExpression;
    double currentNumber = 0;
    char operation = 0;
    double tempResult = 0;
    bool inExponentiation = false;

    while (iss >> currentNumber) {
        if (inExponentiation) {
            tempResult = std::pow(tempResult, currentNumber);
            iss >> operation;
            if (operation != '^') {
                intermediateExpression << tempResult << ' ' << operation << ' ';
                inExponentiation = false;
            }
        } else {
            tempResult = currentNumber;
            iss >> operation;
            if (operation == '^') {
                inExponentiation = true;
            } else {
                intermediateExpression << tempResult << ' ' << operation << ' ';
            }
        }
    }

    if (inExponentiation) {
        intermediateExpression << tempResult;
    }

    std::string intermediateExpressionStr = intermediateExpression.str();
    iss.clear();
    iss.str(intermediateExpressionStr);
    intermediateExpression.str("");
    double tempResult2 = 1;
    bool inMultiplicationOrDivision = false;

    while (iss >> currentNumber) {
        if (inMultiplicationOrDivision) {
            switch (operation) {
                case '*':
                    tempResult2 *= currentNumber;
                    break;
                case '/':
                    if (currentNumber != 0) {
                        tempResult2 /= currentNumber;
                    } else {
                        throw std::runtime_error("Division by zero");
                    }
                    break;
            }
            iss >> operation;
            if (operation == '+' || operation == '-') {
                intermediateExpression << tempResult2 << ' ' << operation << ' ';
                inMultiplicationOrDivision = false;
            }
        } else {
            iss >> operation;
            if (operation == '*' || operation == '/') {
                tempResult2 = currentNumber;
                inMultiplicationOrDivision = true;
            } else {
                intermediateExpression << currentNumber << ' ' << operation << ' ';
            }
        }
    }

    if (inMultiplicationOrDivision) {
        intermediateExpression << tempResult2;
    }

    return evaluateExpression(intermediateExpression.str());
}

void calcp(const char *func) {
    double y = eval(func);
    c_G("Ans. %0.5f", y);
}