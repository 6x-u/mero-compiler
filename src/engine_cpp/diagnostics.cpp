/**
 * MERO Compiler - Diagnostics Implementation
 * Developer: MERO:TG@QP4RM
 */
#include "diagnostics.h"
#include <sstream>
#include <algorithm>

namespace mero {

void Diagnostics::set_source(const std::string& source, const std::string& filename) {
    filename_ = filename;
    source_lines_.clear();
    std::istringstream stream(source);
    std::string line;
    while (std::getline(stream, line)) {
        source_lines_.push_back(line);
    }
}

void Diagnostics::add_from_context(const MeroErrorContext& ctx) {
    MeroError* err = ctx.head;
    while (err) {
        DiagMessage msg;
        msg.level = err->level;
        msg.file = err->location.file ? err->location.file : filename_;
        msg.line = err->location.line;
        msg.column = err->location.column;
        msg.message = err->message;
        msg.source_line = get_source_line(err->location.line);
        messages_.push_back(std::move(msg));
        err = err->next;
    }
}

void Diagnostics::add_error(uint32_t line, uint32_t col, const std::string& msg) {
    DiagMessage m;
    m.level = MERO_ERROR_ERROR;
    m.file = filename_;
    m.line = line;
    m.column = col;
    m.message = msg;
    m.source_line = get_source_line(line);
    messages_.push_back(std::move(m));
}

void Diagnostics::add_warning(uint32_t line, uint32_t col, const std::string& msg) {
    DiagMessage m;
    m.level = MERO_ERROR_WARNING;
    m.file = filename_;
    m.line = line;
    m.column = col;
    m.message = msg;
    m.source_line = get_source_line(line);
    messages_.push_back(std::move(m));
}

void Diagnostics::add_note(uint32_t line, uint32_t col, const std::string& msg) {
    DiagMessage m;
    m.level = MERO_ERROR_NOTE;
    m.file = filename_;
    m.line = line;
    m.column = col;
    m.message = msg;
    m.source_line = get_source_line(line);
    messages_.push_back(std::move(m));
}

std::string Diagnostics::format_all() const {
    std::string output;
    for (auto& msg : messages_) {
        output += format_message(msg);
        output += "\n";
    }
    int errors = error_count();
    int warns = warning_count();
    if (errors > 0 || warns > 0) {
        output += "\n";
        output += std::to_string(errors) + " error(s), " +
                  std::to_string(warns) + " warning(s)\n";
    }
    return output;
}

std::string Diagnostics::format_message(const DiagMessage& msg) const {
    std::string output;
    const char* level_str = mero_error_level_str(msg.level);

    // Header: file:line:col: level: message
    output += "\033[1m" + msg.file + ":" +
              std::to_string(msg.line) + ":" +
              std::to_string(msg.column) + ": ";

    switch (msg.level) {
        case MERO_ERROR_ERROR:
        case MERO_ERROR_FATAL:
            output += "\033[31m" + std::string(level_str) + "\033[0m"; break;
        case MERO_ERROR_WARNING:
            output += "\033[33m" + std::string(level_str) + "\033[0m"; break;
        default:
            output += "\033[36m" + std::string(level_str) + "\033[0m"; break;
    }

    output += "\033[1m: " + msg.message + "\033[0m\n";

    // Source line with pointer
    if (!msg.source_line.empty()) {
        std::string line_num = std::to_string(msg.line);
        std::string padding(line_num.size(), ' ');

        output += " " + padding + " |\n";
        output += " " + line_num + " | " + msg.source_line + "\n";
        output += " " + padding + " | " + make_pointer(msg.column) + "\n";
    }

    if (!msg.suggestion.empty()) {
        output += "  = suggestion: " + msg.suggestion + "\n";
    }

    return output;
}

bool Diagnostics::has_errors() const {
    return std::any_of(messages_.begin(), messages_.end(),
        [](const DiagMessage& m) {
            return m.level == MERO_ERROR_ERROR || m.level == MERO_ERROR_FATAL;
        });
}

int Diagnostics::error_count() const {
    return static_cast<int>(std::count_if(messages_.begin(), messages_.end(),
        [](const DiagMessage& m) {
            return m.level == MERO_ERROR_ERROR || m.level == MERO_ERROR_FATAL;
        }));
}

int Diagnostics::warning_count() const {
    return static_cast<int>(std::count_if(messages_.begin(), messages_.end(),
        [](const DiagMessage& m) { return m.level == MERO_ERROR_WARNING; }));
}

std::string Diagnostics::get_source_line(uint32_t line) const {
    if (line == 0 || line > source_lines_.size()) return "";
    return source_lines_[line - 1];
}

std::string Diagnostics::make_pointer(uint32_t column) const {
    if (column == 0) return "^";
    return std::string(column - 1, ' ') + "^";
}

} // namespace mero
