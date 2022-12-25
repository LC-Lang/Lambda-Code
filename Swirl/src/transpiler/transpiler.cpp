#include <iostream>
#include <exception>
#include <memory>
#include <map>
#include <fstream>

#include <parser/parser.h>
#include <definitions/definitions.h>

#define _debug true


void Transpile(AbstractSyntaxTree& _ast, const std::string& _buildFile) {
    bool                       is_scp;
    std::ifstream              bt_fstream;
    std::string                tmp_str_cnst;
    std::size_t                last_scp_order;
    std::string                compiled_source;
    std::array<const char*, 3> vld_scopes = {"CONDITION", "FUNC", "CLASS"};

    bt_fstream.open("Swirl/src/transpiler/builtins.cpp");
    compiled_source = {
        std::istreambuf_iterator<char>(bt_fstream),
        {}
    };
    bt_fstream.close();

    compiled_source += "int main() {\n";

    for (auto const& child : _ast.chl) {
        if (child.type == "BR_OPEN") {
            compiled_source += "{";
            continue;
        }
        if (child.type == "BR_CLOSE") {
            compiled_source += "}";
            continue;
        }

        if (child.type == "if" || child.type == "elif" || child.type == "else") {
            if (child.type == "else")
                tmp_str_cnst += "else";
            else
                tmp_str_cnst += child.type + " (" + child.condition + ")";
            compiled_source += tmp_str_cnst;
            tmp_str_cnst.clear();
            continue;
        }

        else if (child.type == "CALL") {
            tmp_str_cnst += child.ident + "(";
            int args_count = 0;

            for (const auto& arg : child.args) {
                if (arg.type == "STRING")
                    tmp_str_cnst += "\"" + arg.value + "\"";
                else
                    tmp_str_cnst += arg.value;

                args_count++;
                if (args_count != child.args.size()) tmp_str_cnst += ",";
            }

            tmp_str_cnst += ")";
            compiled_source += tmp_str_cnst + ";";
            tmp_str_cnst.clear();
        }
    }

    std::ofstream o_file_buf(_buildFile);
    compiled_source += "}";
    o_file_buf << compiled_source;
    o_file_buf.close();
}