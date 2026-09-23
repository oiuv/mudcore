// Internal migration helpers. New APIs use snake_case; see docs/function-naming.md.
#ifndef MUDCORE_FUNCTION_COMPAT_H
#define MUDCORE_FUNCTION_COMPAT_H

private string _mudcore_program_path(string path) {
    if (!stringp(path) || path == "") return "";
    if (path[0] != '/') path = "/" + path;
    if (sizeof(path) > 4 && path[<4..] == ".lpc") return path[0..<5];
    if (sizeof(path) > 2 && path[<2..] == ".c") return path[0..<3];
    return path;
}

// A base entry forwards only when it has not itself been overridden. This
// preserves both old/new overrides and explicit ::parent calls without recursion.
private int _mudcore_forward_name(string current, string alternate, string owner) {
    string currentSource, alternateSource;

    owner = _mudcore_program_path(owner);
    currentSource = _mudcore_program_path(efun::function_exists(current, this_object(), 1));
    if (currentSource != owner) return 0;
    alternateSource = _mudcore_program_path(efun::function_exists(alternate, this_object(), 1));
    return alternateSource != "" && alternateSource != owner;
}

// Host-supplied quest/camp/pattern/verb objects may implement only the old API.
// Prefer the new entry when present. Never catch a real provider error as fallback.
private mixed _mudcore_call_named(mixed target, string current, string legacy, mixed *args...) {
    object provider;

    provider = stringp(target) ? load_object(target) : target;
    if (!objectp(provider)) return call_other(provider, current, args...);
    return call_other(
        provider,
        efun::function_exists(current, provider) ? current : legacy,
        args...
    );
}

#endif
