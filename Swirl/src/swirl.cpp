#include <iostream>
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
        {{"-c", "--compiler"}, "C++ compiler to use", true, {}},
        {{"-d", "--debug"}, "Log the steps of compilation", false, {}},
        {{"-v", "--version"}, "Show the version of Swirl", false, {}}
};


/* Lookup table for registered types and their visibility. */
std::unordered_map<std::string, const char*> type_registry = {
        // Pre-registered default types.
        {"int",     "global"},
        {"string",  "global"},
        {"bool",    "global"},
        {"float",   "global"},
        {"var",     "global"},
        {"function","global"}
};

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

    std::string cxx;

    if (app.contains_flag("-c"))
        cxx = app.get_flag_value("-c");
    else cxx = "g++";

    std::optional<std::string> _file = app.get_file();

    if (!_file.has_value()) { 
        std::cerr << "No Input file\n"; return 1; 
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
    file_name = file_name.substr(0, file_name.find_last_of("."));

    SW_OUTPUT = file_name;

    if (app.contains_flag("-o"))
        SW_OUTPUT = app.get_flag_value("-o");

    SW_FED_FILE_SOURCE += "\n";

    if ( !SW_FED_FILE_SOURCE.empty() ) {
        InputStream is(SW_FED_FILE_SOURCE);
        TokenStream tk(is, _debug);
        preProcess(SW_FED_FILE_SOURCE, tk, cache_dir);

        Parser parser(tk);
        parser.dispatch();
        Transpile(parser.m_AST->chl, cache_dir + SW_OUTPUT + ".cpp", compiled_source);
    }
 
    std::string compile_cmd = cxx + " " + cache_dir + SW_OUTPUT + ".cpp" + " -o " + out_dir + SW_OUTPUT;
       
    system(compile_cmd.c_str());
}
