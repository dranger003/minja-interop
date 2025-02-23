#include <chat-template.hpp>

#ifdef _WIN32
    #define EXPORT __declspec(dllexport)
#else
    #define EXPORT __attribute__((visibility("default")))
#endif

extern "C"
{
    EXPORT const char* apply_str(
        const char* source,
        const char* messages,
        const char* tools,
        const char* extra_context,
        bool add_generation_prompt,
        const char* bos_token,
        const char* eos_token)
    {
        auto tmpl = minja::chat_template(source, bos_token, eos_token);

        auto inputs = minja::chat_template_inputs{
            json::parse(messages),
            json::parse(tools),
            add_generation_prompt,
            json::parse(extra_context),
        };

        auto prompt = tmpl.apply(inputs);

        auto size = prompt.size() + 1;
        auto str = new char[size];
        std::memset(str, 0, size);
        std::memcpy(str, prompt.c_str(), size - 1);

        return str;
    }

    EXPORT void free_str(const char* str) {
        if (str) delete [] str;
    }
}
