#include "main.hpp"
#include <lexer.hpp>

int main(int argc, char *argv[]) { 
    Lexer lexer("log(\"Hello World!\");");
    lexer.tokenize(true);
    return 0; 
}