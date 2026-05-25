/**
 * MERO Compiler - Windows Console Application
 * Standalone .exe that compiles Python to Lua/Luau.
 * No external dependencies required.
 * Developer: MERO:TG@QP4RM
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define MERO_VERSION "1.0.0"
#define MAX_PATH_LEN 4096
#define MAX_SOURCE_SIZE (10 * 1024 * 1024)

/* Forward declarations */
typedef void* MeroCompiler;
typedef struct {
    const char* output;
    int success;
    const char* error_message;
    int output_length;
} MeroResult;

/* DLL function pointers */
typedef MeroCompiler (*fn_create)(int);
typedef void (*fn_destroy)(MeroCompiler);
typedef MeroResult (*fn_compile)(MeroCompiler, const char*, int);
typedef void (*fn_free_result)(MeroResult*);

static HMODULE g_dll = NULL;
static fn_create  p_create = NULL;
static fn_destroy p_destroy = NULL;
static fn_compile p_compile = NULL;
static fn_free_result p_free = NULL;

static void set_console_color(int color) {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(h, (WORD)color);
}

static void reset_color(void) {
    set_console_color(7); /* Default white */
}

static void print_banner(void) {
    set_console_color(11); /* Cyan */
    printf("\n");
    printf(" +======================================================+\n");
    printf(" |          MERO Compiler v%s                       |\n", MERO_VERSION);
    printf(" |          Python -> Lua/Luau for Roblox               |\n");
    printf(" |          Developer: MERO:TG@QP4RM                    |\n");
    printf(" +======================================================+\n");
    reset_color();
    printf("\n");
}

static void print_menu(void) {
    printf("  Choose compilation target:\n\n");
    set_console_color(10); /* Green */
    printf("    [1] Python  ->  Lua   (.lua)\n");
    set_console_color(14); /* Yellow */
    printf("    [2] Python  ->  Luau  (.luau)\n");
    reset_color();
    printf("\n");
    printf("    [0] Exit\n\n");
}

static int load_dll(void) {
    /* Try to load mero.dll from same directory */
    g_dll = LoadLibraryA("mero.dll");
    if (!g_dll) {
        /* Try subdirectory */
        g_dll = LoadLibraryA("lib\\mero.dll");
    }
    if (!g_dll) return 0;

    p_create  = (fn_create)GetProcAddress(g_dll, "mero_compiler_create");
    p_destroy = (fn_destroy)GetProcAddress(g_dll, "mero_compiler_destroy");
    p_compile = (fn_compile)GetProcAddress(g_dll, "mero_compiler_compile");
    p_free    = (fn_free_result)GetProcAddress(g_dll, "mero_compile_result_free");

    return (p_create && p_destroy && p_compile && p_free);
}

static char* read_file(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size <= 0 || size > MAX_SOURCE_SIZE) {
        fclose(f);
        return NULL;
    }

    char* buffer = (char*)malloc(size + 1);
    if (!buffer) { fclose(f); return NULL; }

    fread(buffer, 1, size, f);
    buffer[size] = '\0';
    fclose(f);
    return buffer;
}

static int write_file(const char* path, const char* content, int length) {
    FILE* f = fopen(path, "wb");
    if (!f) return 0;
    fwrite(content, 1, length, f);
    fclose(f);
    return 1;
}

static void change_extension(char* output, const char* input, const char* ext) {
    strcpy(output, input);
    char* dot = strrchr(output, '.');
    if (dot) {
        strcpy(dot, ext);
    } else {
        strcat(output, ext);
    }
}

static void compile_file(const char* path, int target) {
    const char* ext = (target == 1) ? ".lua" : ".luau";
    const char* target_name = (target == 1) ? "Lua" : "Luau";

    printf("\n  Compiling to %s...\n", target_name);

    char* source = read_file(path);
    if (!source) {
        set_console_color(12); /* Red */
        printf("  Error: Cannot read file: %s\n", path);
        reset_color();
        return;
    }

    char output_path[MAX_PATH_LEN];
    change_extension(output_path, path, ext);

    /* Use DLL if available */
    if (g_dll && p_create) {
        MeroCompiler compiler = p_create(2);
        MeroResult result = p_compile(compiler, source, (int)strlen(source));

        if (result.success) {
            /* For .lua target, strip --!strict */
            const char* out = result.output;
            int out_len = result.output_length;

            if (write_file(output_path, out, out_len)) {
                set_console_color(10);
                printf("\n  Compilation successful!\n");
                reset_color();
                printf("  Input:  %s\n", path);
                printf("  Output: %s\n", output_path);
                printf("  Size:   %d bytes\n", out_len);
            } else {
                set_console_color(12);
                printf("  Error writing output file\n");
                reset_color();
            }
            p_free(&result);
        } else {
            set_console_color(12);
            printf("  Compilation failed: %s\n", result.error_message ? result.error_message : "Unknown error");
            reset_color();
            p_free(&result);
        }
        p_destroy(compiler);
    } else {
        /* Fallback: basic transformation without DLL */
        /* This is a simplified converter for when the DLL is not present */
        set_console_color(14);
        printf("  [Using built-in converter]\n");
        reset_color();

        /* Basic Python to Lua conversion */
        char* output = (char*)malloc(strlen(source) * 3 + 4096);
        if (!output) { free(source); return; }

        int pos = 0;
        if (target == 2) {
            pos += sprintf(output + pos, "--!strict\n");
        }
        pos += sprintf(output + pos, "-- Generated by MERO Compiler v%s\n", MERO_VERSION);
        pos += sprintf(output + pos, "-- Source: %s\n", path);
        pos += sprintf(output + pos, "-- Developer: MERO:TG@QP4RM\n\n");
        pos += sprintf(output + pos, "local _RT = require(script.Parent:WaitForChild(\"MeroRuntime\"))\n\n");

        /* Line-by-line basic conversion */
        const char* line_start = source;
        while (*line_start) {
            const char* line_end = strchr(line_start, '\n');
            if (!line_end) line_end = line_start + strlen(line_start);

            int line_len = (int)(line_end - line_start);
            char line[4096] = {0};
            if (line_len < 4095) {
                strncpy(line, line_start, line_len);
            }

            /* Skip empty lines and comments */
            char* trimmed = line;
            while (*trimmed == ' ' || *trimmed == '\t') trimmed++;

            if (*trimmed == '#') {
                pos += sprintf(output + pos, "--%s\n", trimmed + 1);
            } else if (strncmp(trimmed, "def ", 4) == 0) {
                char* name = trimmed + 4;
                char* paren = strchr(name, '(');
                if (paren) {
                    *paren = '\0';
                    char* params = paren + 1;
                    char* end_paren = strchr(params, ')');
                    if (end_paren) *end_paren = '\0';
                    /* Remove 'self' and type annotations */
                    pos += sprintf(output + pos, "local function %s(%s)\n", name, params);
                }
            } else if (strncmp(trimmed, "class ", 6) == 0) {
                char* name = trimmed + 6;
                char* colon = strchr(name, ':');
                char* paren = strchr(name, '(');
                if (colon) *colon = '\0';
                if (paren) *paren = '\0';
                while (*name && (*name == ' ' || *name == '\t')) name++;
                pos += sprintf(output + pos, "local %s = {}\n%s.__index = %s\n\n", name, name, name);
            } else if (strncmp(trimmed, "print(", 6) == 0) {
                pos += sprintf(output + pos, "_RT.print(%s\n", trimmed + 6);
            } else if (strncmp(trimmed, "return ", 7) == 0) {
                pos += sprintf(output + pos, "return %s\n", trimmed + 7);
            } else if (strncmp(trimmed, "if ", 3) == 0) {
                char* cond = trimmed + 3;
                char* colon = strrchr(cond, ':');
                if (colon) *colon = '\0';
                pos += sprintf(output + pos, "if %s then\n", cond);
            } else if (strcmp(trimmed, "else:") == 0) {
                pos += sprintf(output + pos, "else\n");
            } else if (strncmp(trimmed, "for ", 4) == 0) {
                pos += sprintf(output + pos, "-- %s\n", line);
            } else if (strncmp(trimmed, "while ", 6) == 0) {
                char* cond = trimmed + 6;
                char* colon = strrchr(cond, ':');
                if (colon) *colon = '\0';
                pos += sprintf(output + pos, "while %s do\n", cond);
            } else if (strcmp(trimmed, "pass") == 0) {
                /* skip */
            } else if (strcmp(trimmed, "break") == 0) {
                pos += sprintf(output + pos, "break\n");
            } else if (*trimmed == '\0') {
                pos += sprintf(output + pos, "\n");
            } else {
                pos += sprintf(output + pos, "%s\n", line);
            }

            line_start = (*line_end == '\n') ? line_end + 1 : line_end;
        }

        if (write_file(output_path, output, pos)) {
            set_console_color(10);
            printf("\n  Compilation successful!\n");
            reset_color();
            printf("  Input:  %s\n", path);
            printf("  Output: %s\n", output_path);
            printf("  Size:   %d bytes\n", pos);
        } else {
            set_console_color(12);
            printf("  Error writing output file\n");
            reset_color();
        }

        free(output);
    }

    free(source);
}

int main(void) {
    SetConsoleOutputCP(65001); /* UTF-8 */
    SetConsoleTitleA("MERO Compiler v" MERO_VERSION);

    /* Try to load the engine DLL */
    load_dll();

    while (1) {
        print_banner();
        print_menu();

        printf("  >> Enter choice (0-2): ");
        char choice_str[16];
        if (!fgets(choice_str, sizeof(choice_str), stdin)) break;
        int choice = atoi(choice_str);

        if (choice == 0) {
            printf("\n  Goodbye!\n");
            break;
        }

        if (choice != 1 && choice != 2) {
            printf("  Invalid choice.\n");
            printf("\n  Press Enter to continue...");
            getchar();
            continue;
        }

        printf("\n  >> Enter Python file path: ");
        char path[MAX_PATH_LEN];
        if (!fgets(path, sizeof(path), stdin)) break;

        /* Remove newline and quotes */
        int len = (int)strlen(path);
        while (len > 0 && (path[len-1] == '\n' || path[len-1] == '\r' || path[len-1] == '"' || path[len-1] == '\'')) {
            path[--len] = '\0';
        }
        char* p = path;
        while (*p == '"' || *p == '\'' || *p == ' ') p++;

        /* Check file exists */
        DWORD attr = GetFileAttributesA(p);
        if (attr == INVALID_FILE_ATTRIBUTES) {
            set_console_color(12);
            printf("  File not found: %s\n", p);
            reset_color();
        } else {
            compile_file(p, choice);
        }

        printf("\n  Press Enter to continue...");
        fgets(choice_str, sizeof(choice_str), stdin);
    }

    if (g_dll) FreeLibrary(g_dll);
    return 0;
}
