#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <filesystem>
#include <unordered_map>

#include <cli/cli.h>
#include <pre-processor/pre-processor.h>
#include <swirl.typedefs/swirl_t.h>
#include <tokenizer/InputStream.h>
#include <tokenizer/Tokenizer.h>
#include <transpiler/transpiler.h>
#include <parser/parser.h>
#include <include/SwirlConfig.h>

bool SW_DEBUG = false;
std::string SW_FED_FILE_PATH;
std::string SW_OUTPUT;
std::string SW_FED_FILE_SOURCE;
std::vector<std::string> lines_rec{};


const std::vector<Argument> application_flags = {
    {{"-h","--help"}, "Show the help message", false, {}},
    {{"-o", "--output"}, "Output file name", true, {}},
    {{"-d", "--debug"}, "Log the steps of compilation", false, {}},
    {{"-v", "--version"}, "Show the version of Swirl", false, {}}
};

std::unordered_map<std::string, uint8_t> valid_expr_bin_ops = {
    {"+", 0},   {"-", 1},  {"*", 2},  {"/", 3},  {"%", 4},   {"==", 5},
    {"!=", 6},  {">", 7},  {"<", 8},  {">=", 9}, {"<=", 10}, {"&&", 11},
    {"||", 12}, {"&", 13}, {"|", 14}, {"^", 15}, {"<<", 16}, {">>", 17},
};

std::unordered_map<std::string, uint8_t> keywords = {
    {"func", 0},      {"return", 1},  {"if", 2},      {"else", 3},
    {"for", 4},       {"while", 5},   {"is", 6},      {"in", 7},
    {"or", 8},        {"and", 9},     {"class", 10},  {"public", 11},
    {"private", 12},  {"const", 15},  {"static", 16}, {"break", 17},
    {"continue", 18}, {"elif", 19},   {"global", 20}, {"importc", 21},
    {"typedef", 22},  {"import", 23}, {"export", 24}, {"from", 25},
    {"var", 26},      {"fn", 27}};

std::unordered_map<std::string, uint8_t> operators = {
    {"+", 0},   {"-", 1},   {"*", 2},   {"/", 3},    {"%", 4},    {"==", 5},
    {"!=", 6},  {">", 7},   {"<", 8},   {">=", 9},   {"<=", 10},  {"&&", 11},
    {"||", 12}, {"&", 13},  {"|", 14},  {"^", 15},   {"<<", 16},  {">>", 17},
    {"=", 18},  {"+=", 19}, {"-=", 20}, {"*=", 21},  {"/=", 22},  {"%=", 23},
    {"&=", 25}, {"|=", 26}, {"^=", 27}, {"<<=", 28}, {">>=", 29}, {"++", 30},
    {"--", 31}, {"~", 32},  {"//", 33}};

std::optional<std::unordered_map<std::string, std::string>> compile(
        std::string& _source,
        const std::string& _cacheDir,
        bool symt = false) {

    InputStream chrinp_stream(_source);
    TokenStream tk(chrinp_stream);
    // preProcess(_source, tk, _cacheDir);
    Parser parser(tk);
    // return Transpile(
    //         {},
    //         _cacheDir,
    //         compiled_source,
    //         true,
    //         symt
    //         );
}

int main(int argc, const char** const argv) {
    cli app(argc, argv, application_flags);

    if (app.contains_flag("-h")) {
        std::cout << USAGE << app.generate_help() << '\n';
        return 0;
    }

    if (app.contains_flag("-v")) {
        std::cout << "Swirl v" << swirl_VERSION_MAJOR << "." << swirl_VERSION_MINOR << "." << swirl_VERSION_PATCH << "\n";
        return 0;
    }

    std::optional<std::string> _file = app.get_file();

    if (!_file.has_value()) {
        std::cerr << "No Input file\n";
        return 1;
    }

    SW_FED_FILE_PATH = *app.get_file();

    if (!std::filesystem::exists(SW_FED_FILE_PATH)) {
        std::cerr << "File '" << SW_FED_FILE_PATH << "' not found!" << std::endl;
        return 1;
    }

    std::ifstream fed_file_src_buf(SW_FED_FILE_PATH);
    SW_FED_FILE_SOURCE = {
            std::istreambuf_iterator<char>(fed_file_src_buf),
            {}
    };
    fed_file_src_buf.close();

    std::string cache_dir = getWorkingDirectory(SW_FED_FILE_PATH) + PATH_SEP + "__swirl_cache__" + PATH_SEP;

    bool _debug = app.contains_flag("-d");

    std::string file_name = SW_FED_FILE_PATH.substr(SW_FED_FILE_PATH.find_last_of("/\\") + 1);
    std::string out_dir = SW_FED_FILE_PATH.replace(SW_FED_FILE_PATH.find(file_name),file_name.length(),"");
    file_name = file_name.substr(0, file_name.find_last_of('.'));

    SW_OUTPUT = file_name;

    if (app.contains_flag("-o"))
        SW_OUTPUT = app.get_flag_value("-o");

    SW_FED_FILE_SOURCE += "\n";

    if ( !SW_FED_FILE_SOURCE.empty() ) {
        InputStream is(SW_FED_FILE_SOURCE);
        TokenStream tk(is, _debug);
        // preProcess(SW_FED_FILE_SOURCE, tk, cache_dir);

        Parser parser(tk);
        parser.dispatch();
        // Transpile(parser.m_AST->chl, cache_dir + SW_OUTPUT + ".cpp", compiled_source);
    }
}
