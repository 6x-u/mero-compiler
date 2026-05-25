/**
 * MERO Compiler - Diagnostics System
 * Formatted error reporting with source code context.
 * Developer: MERO:TG@QP4RM
 */
#ifndef MERO_DIAGNOSTICS_H
#define MERO_DIAGNOSTICS_H

#include <string>
#include <vector>

extern "C" {
#include "mero/error.h"
}

namespace mero {

class Diagnostics {
public:
    struct DiagMessage {
        MeroErrorLevel level;
        std::string file;
        uint32_t line;
        uint32_t column;
        std::string message;
        std::string source_line;
        std::string suggestion;
    };

    Diagnostics() = default;

    void set_source(const std::string& source, const std::string& filename);
    void add_from_context(const MeroErrorContext& ctx);
    void add_error(uint32_t line, uint32_t col, const std::string& msg);
    void add_warning(uint32_t line, uint32_t col, const std::string& msg);
    void add_note(uint32_t line, uint32_t col, const std::string& msg);

    std::string format_all() const;
    std::string format_message(const DiagMessage& msg) const;
    bool has_errors() const;
    int error_count() const;
    int warning_count() const;
    const std::vector<DiagMessage>& messages() const { return messages_; }

private:
    std::vector<std::string> source_lines_;
    std::string filename_;
    std::vector<DiagMessage> messages_;

    std::string get_source_line(uint32_t line) const;
    std::string make_pointer(uint32_t column) const;
};

} // namespace mero

#endif // MERO_DIAGNOSTICS_H
