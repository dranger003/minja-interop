# minja-interop

## Description

A minimal interop shim to use minja via P/Invoke in C#.

## Build

```bash
git clone https://github.com/google/minja.git google.minja
git clone https://github.com/nlohmann/json.git nlohmann.json
git clone https://github.com/dranger003/minja-interop.git
cd minja-interop

# For linux
./build.sh release
# For Windows
build.cmd release
```

## Use

Update the project file:

```xml
<PropertyGroup>
	<!-- ... -->
	<AllowUnsafeBlocks>True</AllowUnsafeBlocks>
</PropertyGroup>
<PropertyGroup>
	<DefineConstants Condition="$([MSBuild]::IsOSPlatform('Windows'))">WINDOWS</DefineConstants>
	<DefineConstants Condition="$([MSBuild]::IsOSPlatform('Linux'))">LINUX</DefineConstants>
</PropertyGroup>
```

Add the interop class:

```csharp
public partial class MinjaInterop
{
    private const string _LIBRARY_PATH = "../../../../minja-interop/build";

#if DEBUG
    private const string _CONFIG = "Debug";
#else
    private const string _CONFIG = "Release";
#endif

#if WINDOWS
    private const string _LIBRARY_NAME = "minja-interop.dll";
#elif LINUX
    private const string _LIBRARY_NAME = "libminja-interop.so";
#endif

    private const string LibraryName = $"{_LIBRARY_PATH}/{_CONFIG}/{_LIBRARY_NAME}";

    private static JsonSerializerOptions _serializerOptions = new()
    {
        Encoder = JavaScriptEncoder.UnsafeRelaxedJsonEscaping,
        PropertyNamingPolicy = JsonNamingPolicy.SnakeCaseLower,
    };

    [StructLayout(LayoutKind.Sequential)]
    private struct _ApplyResult
    {
        public readonly nint str;
        public readonly nint err;
    }

    [LibraryImport(LibraryName, EntryPoint = "Apply", StringMarshalling = StringMarshalling.Utf8)]
    private static partial _ApplyResult _Apply(
        string source,
        string messages,
        string tools,
        string extra_context,
        [MarshalAs(UnmanagedType.I1)] bool add_generation_prompt,
        string bos_token,
        string eos_token
    );

    [LibraryImport(LibraryName, EntryPoint = "FreeApplyResult")]
    private static partial void _FreeApplyResult(_ApplyResult res);

    public static string Apply(
        string source,
        dynamic messages,
        dynamic? tools = null,
        dynamic? extraContent = null,
        bool addGenerationPrompt = true,
        string bosToken = "",
        string eosToken = ""
    )
    {
        tools ??= new { };
        extraContent ??= new { };

        var messagesJson = JsonSerializer.Serialize((object?)messages, _serializerOptions) ?? "{}";
        var toolsJson = JsonSerializer.Serialize((object?)tools, _serializerOptions) ?? "{}";
        var extraContentJson = JsonSerializer.Serialize((object?)extraContent, _serializerOptions) ?? "{}";

        var result = _Apply(source, messagesJson, toolsJson, extraContentJson, addGenerationPrompt, bosToken, eosToken);
        var str = Marshal.PtrToStringAnsi(result.str) ?? String.Empty;
        var err = Marshal.PtrToStringAnsi(result.err) ?? String.Empty;
        _FreeApplyResult(result);

        if (!String.IsNullOrEmpty(err))
        {
            throw new Exception(err);
        }

        return str;
    }
}
```

Apply the template:

```csharp
var source =
    "{% for message in messages %}" +
    "{{ '<|' + message['role'] + '|>\\n' + message['content'] + '<|end|>' + '\\n' }}" +
    "{% endfor %}";

var messages = new[]
{
    new
    {
        Role = "user",
        Content = "Hello?",
    },
    new
    {
        Role = "assistant",
        Content = "Hello!",
    },
    new
    {
        Role = "user",
        Content = "How are you?",
    },
};

var prompt = MinjaInterop.Apply(source, messages);
Console.WriteLine(prompt);
```
