#include <chat-template.hpp>

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#define API __stdcall
#else
#define EXPORT __attribute__((visibility("default")))
#define API
#endif

const char* __strdup(const std::string& s) {
#ifdef _WIN32
	auto copy = _strdup(s.c_str());
#else
	auto copy = strdup(s.c_str());
#endif
	return copy;
}

struct ApplyResult {
	const char* str;
	const char* err;
};

extern "C"
{
	EXPORT ApplyResult API Apply(
		const char* source,
		const char* messages,
		const char* tools,
		const char* extra_context,
		bool add_generation_prompt,
		const char* bos_token,
		const char* eos_token)
	{
		try {
			auto tmpl = minja::chat_template(source, bos_token, eos_token);

			auto inputs = minja::chat_template_inputs{
				json::parse(messages),
				json::parse(tools),
				add_generation_prompt,
				json::parse(extra_context),
			};

			auto prompt = tmpl.apply(inputs);
			auto str = __strdup(prompt);

			if (!str) {
				return ApplyResult{ nullptr, "Memory allocation failed." };
			}

			return ApplyResult{ str, nullptr };
		}
		catch (const std::exception& e) {
			return ApplyResult{ nullptr, __strdup(e.what()) };
		}
		catch (...) {
			return ApplyResult{ nullptr, "Unknown error occurred." };
		}
	}

	EXPORT void API FreeApplyResult(ApplyResult res)
	{
		if (res.str) std::free((void*)res.str);
	}
}
