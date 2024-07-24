#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <optional>
#include <string>
#include <vector>
#include "./nlulyhapvu.hpp"
#include "./whyzly.hpp"
#include "./avrlupghapvu.hpp"
using namespace std;

const string KEYWORD_COLOR = "\033[1;34m"; // Blue
const string NUMBER_COLOR = "\033[1;32m";  // Green
const string OPERATOR_COLOR = "\033[1;31m";// Red
const string IDENTIFIER_COLOR = "\033[1;33m";// Yellow
const string COMMENT_COLOR = "\033[90m";    // Grey
const string RESET = "\033[0m";
const string ERROR = "\033[1;31m";
const string GREEN = "\033[1;32m";
const string GREY = "\033[90m";
const string BLUE = "\033[34m";
const string PINK = "\033[95m";

void highlightWord(string& text, const string& word, const string& highlightColor) {
    size_t pos = 0;
    string highlightStart = highlightColor;
    string highlightEnd = "\033[0m";  // Reset color code

    while ((pos = text.find(word, pos)) != string::npos) {
        bool inLineComment = false;
        bool inBlockComment = false;

        // Check if 'pos' is within a line comment
        size_t lineCommentPos = text.rfind('#', pos);
        if (lineCommentPos != string::npos) {
            size_t newlinePos = text.find('\n', lineCommentPos);
            if (newlinePos == string::npos) newlinePos = text.length();
            if (pos > lineCommentPos && pos < newlinePos) {
                inLineComment = true;
            }
        }

        // Check if 'pos' is within a block comment
        size_t blockCommentStart = text.rfind("/*", pos);
        if (blockCommentStart != string::npos) {
            size_t blockCommentEnd = text.find("*/", blockCommentStart);
            if (blockCommentEnd == string::npos) blockCommentEnd = text.length();
            if (pos > blockCommentStart && pos < blockCommentEnd) {
                inBlockComment = true;
            }
        }

        // If the word is within a comment, skip it
        if (!inLineComment && !inBlockComment) {
            text.insert(pos, highlightStart);
            pos += highlightStart.length() + word.length();
            text.insert(pos, highlightEnd);
            pos += highlightEnd.length();
        } else {
            pos += word.length();
        }
    }
}

void highlightAndPrintTokens(string& code) { // this is a god awful function, will update it when I am able to correctly utilize the tokenization into this
    highlightWord(code, "elif", RED);
    highlightWord(code, "if", RED);
    highlightWord(code, "else", RED);
    highlightWord(code, "exit", RED);
    highlightWord(code, "out", RED);
    highlightWord(code, "may", RED);
    highlightWord(code, "(", BLUE);
    highlightWord(code, ")", BLUE);
    highlightWord(code, ";", BLUE);
    highlightWord(code, "{", BLUE);
    highlightWord(code, "}", BLUE);
    highlightWord(code, "=", GREEN);
    highlightWord(code, "+", GREEN);
    highlightWord(code, "-", GREEN);
}

void highlightComments(string& text) {
    size_t pos = 0;
    while ((pos = text.find('#', pos)) != string::npos) {
        size_t endPos = text.find('\n', pos);
        if (endPos == string::npos) endPos = text.length();
        string comment = text.substr(pos, endPos - pos);
        text.replace(pos, endPos - pos, COMMENT_COLOR + comment + RESET);
        pos = endPos + COMMENT_COLOR.length() + RESET.length();
    }

    pos = 0;
    while ((pos = text.find("/*", pos)) != string::npos) {
        size_t endPos = text.find("*/", pos);
        if (endPos == string::npos) endPos = text.length();
        string comment = text.substr(pos, endPos - pos + 2);
        text.replace(pos, endPos - pos + 2, COMMENT_COLOR + comment + RESET);
        pos = endPos + 2 + COMMENT_COLOR.length() + RESET.length();
    }
}

bool hasExtension(const string& filename) {
    // Check if the file name ends with ".cal"
    const string extension = ".42";
    if (filename.length() >= extension.length()) {
        return (0 == filename.compare(filename.length() - extension.length(), extension.length(), extension));
    }
    return false;
}

void dcom_helper(string error_msg, char* argv[]) {
    cerr << ERROR << error_msg << endl << endl;
    cerr << PINK << "KVVTZ JVTWPSLY (dcom) has 2 options: " << endl;
    cerr << PINK << "Usage: " << argv[0] << " <input.42>" << " run" << endl;
    cerr << PINK << "See Code: " << argv[0] << " <input.42>" << " code" << endl;
    exit(EXIT_FAILURE);
}

int main(int argc, char* argv[]) {
  if (argc < 3) {
    dcom_helper("[Invalid Compile]", argv);
  }

  
  string filename = argv[1];

    if (!hasExtension(filename)) {
        dcom_helper("[Invalid File]", argv);
        // Handle the error accordingly
    }

    std::string contents;
    {
        std::stringstream contents_stream;
        std::fstream input(argv[1], std::ios::in);
        contents_stream << input.rdbuf();
        contents = contents_stream.str();
    }

    string code = contents;

    Tokenizer tokenizer(std::move(contents));
    std::vector<Token> tokens = tokenizer.tokenize();
    
    if (string(argv[2]) == "code") {
        highlightComments(code);
        highlightAndPrintTokens(code);

        cout << code << endl;
        exit(EXIT_SUCCESS);
    } 
    else if (string(argv[2]) == "run") {
        cout << GREY << "Compiling.." << endl;
    }
    else if (string(argv[2]) == "help") {
        cout << PINK << "KVVTZ JVTWPSLY (dcom) has 2 options: " << endl;
        cout << PINK << "Usage: " << argv[0] << " <input.42>" << " run" << endl;
        cout << PINK << "See Code: " << argv[0] << " <input.42>" << " code" << endl;
        exit(EXIT_SUCCESS);
    }
    else {
        cout << IDENTIFIER_COLOR << "Unknown argument\nCompiling anyways..." << endl;
    }

    Parser parser(std::move(tokens));
    std::optional<NodeProg> prog = parser.parse_prog();

    if (!prog.has_value()) {
        std::cerr << ERROR << "Invalid program" << std::endl;
        exit(EXIT_FAILURE);
    }

    Generator generator(prog.value());
    {
        std::fstream file("out.asm", std::ios::out);
        file << generator.gen_prog();
    }

    system("nasm -felf64 out.asm");
    system("ld -o Kvvtz_JVtwpsly out.o");

    cout << GREEN << "[Compile Success] Kvvtz JVtwpsly has been leaked!" << endl;
    

    return EXIT_SUCCESS;
}
