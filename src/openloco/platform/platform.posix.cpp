#ifndef _WIN32

#include "../interop/interop.hpp"
#include "../openloco.h"
#include "platform.h"
#include <iostream>
#include <pwd.h>
#include <time.h>
#include <yaml.h>

#ifdef __linux__
#include <sys/types.h>
#include <unistd.h>
#endif

static openloco::interop::loco_global<char * [0xFFFF], 0x005183FC> _strings;

uint32_t read(uint8_t** string)
{
    uint32_t read = 0;

    uint8_t* ptr = *string;

    if ((ptr[0] & 0b10000000) == 0)
    {
        read = ptr[0];
        *string += 1;
    }
    else if ((ptr[0] & 0b11100000) == 0b11000000)
    {
        assert((ptr[1] & 0b11000000) == 0b10000000);

        read = (ptr[0] & 0b11111) << 6 | ((ptr[1] & 0b111111));
        //printf("%02X %02X %d %s\n", ptr[0], ptr[1], read, value);
        *string += 2;
    }
    else if ((ptr[0] & 0b11110000) == 0b11100000)
    {
        assert((ptr[1] & 0b11000000) == 0b10000000);
        assert((ptr[2] & 0b11000000) == 0b10000000);
        *string += 3;
    }
    else if ((ptr[0] & 0b11111000) == 0b11110000)
    {
        *string += 4;
    }

    return read;
}

static char* readString(const char* value, size_t size)
{

    char* str = (char*)malloc(size);
    char* out = str;

    uint8_t* ptr = (uint8_t*)value;
    while (true)
    {
        uint32_t readChar = read(&ptr);
        if (readChar == '{')
        {
            uint8_t* start = ptr;
            do
            {
                readChar = read(&ptr);
            } while (readChar != '}');

            int len = ptr - start;
            auto view = std::basic_string_view(start, len);

            printf("[%.*s] %d\n", len, view.data(), len);
            continue;
        }

        //        switch (read)
        //        {
        //            case '\0':
        //                break;
        //            case 'A' ... 'Z':
        //            case 'a' ... 'z':
        //            case '{':
        //            case ':':
        //            case '&':
        //            case ' ':
        //            case ',':
        //            case '/':
        //            case '%':
        //            case '}':
        //            case '-':
        //            case '?':
        //            case ')':
        //            case '(':
        //            case '!':
        //            case '+':
        //            case '[':
        //            case ']':
        //            case '*':
        //            case ';':
        //            case '#':
        //            case '=':
        //            case '\\':
        //            case '@':
        //            case '_':
        //            case '\'':
        //            case '0' ... '9':
        //            case '.':
        //            case 228: // ä
        //            case 252: // ü
        //            case 246: // ö
        //            case 223: // ß
        //            case 241: // ñ
        //            case 196: // Ä
        //            case 187: // »
        //            case 220: // Ü
        //            case 214: // Ö
        //            case 176: // °
        //            case 169: // ©
        //                break;
        //            default:
        //                printf("%5d %c\n", read, read);
        //        }

        *out = readChar;
        out++;

        if (readChar == '\0')
            break;
    }

    return str;
}

int main(int argc, const char** argv)
{
    FILE* file = fopen("/Users/Marijn/de-DE.yml", "rb");

    yaml_parser_t parser;
    yaml_document_t document;
    yaml_parser_initialize(&parser);
    yaml_parser_set_input_file(&parser, file);
    yaml_parser_load(&parser, &document);
    [[maybe_unused]] auto node = yaml_document_get_root_node(&document);
    assert(node->type == YAML_MAPPING_NODE);

    int i = 0;
    for (auto pair = node->data.mapping.pairs.start; pair < node->data.mapping.pairs.top; pair++)
    {
        [[maybe_unused]] auto key = yaml_document_get_node(&document, pair->key);
        [[maybe_unused]] auto value = yaml_document_get_node(&document, pair->value);
        int key2 = strtol(reinterpret_cast<const char*>(key->data.scalar.value), nullptr, 10);

        auto str = readString(reinterpret_cast<const char*>(value->data.scalar.value), value->data.scalar.length);

        if (str != nullptr)
        {
            _strings[key2] = str;
            printf("%4d |%s|\n", key2, str);
        }

        i++;
    }

    yaml_document_delete(&document);

    yaml_parser_delete(&parser);
    fclose(file);
    openloco::interop::load_sections();
    openloco::lpCmdLine((char*)argv[0]);
    openloco::main();
    return 0;
}

uint32_t openloco::platform::get_time()
{
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    return spec.tv_nsec / 1000000;
}

#if !(defined(__APPLE__) && defined(__MACH__))
static std::string GetEnvironmentVariable(const std::string& name)
{
    auto result = getenv(name.c_str());
    return result == nullptr ? std::string() : result;
}

static fs::path get_home_directory()
{
    auto pw = getpwuid(getuid());
    if (pw != nullptr)
    {
        return pw->pw_dir;
    }
    else
    {
        return GetEnvironmentVariable("HOME");
    }
}

fs::path openloco::platform::get_user_directory()
{
    auto path = fs::path(GetEnvironmentVariable("XDG_CONFIG_HOME"));
    if (path.empty())
    {
        path = get_home_directory();
        if (path.empty())
        {
            path = "/";
        }
        else
        {
            path = path / fs::path(".config");
        }
    }
    return path / fs::path("OpenLoco");
}
#endif

#if !(defined(__APPLE__) && defined(__MACH__))
std::string openloco::platform::prompt_directory(const std::string& title)
{
    std::string input;
    std::cout << "Type your Locomotion path: ";
    std::cin >> input;
    return input;
}
#endif

#endif
