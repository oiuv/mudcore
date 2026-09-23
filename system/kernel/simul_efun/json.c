/**
 * json.c
 *
 * LPC support functions for JSON serialization and deserialization.
 * Attempts to be compatible with reasonably current FluffOS and LDMud
 * drivers, with at least a gesture or two toward compatibility with
 * older drivers.
 *
 *
 * mixed json_decode(mixed text)
 *     Deserializes JSON into an LPC value.
 *
 * string json_encode(mixed value)
 *     Serializes an LPC value into JSON text.
 *
 * v1.0: initial release
 * v1.0.1: fix for handling of \uXXXX on FLUFFOS
 * v1.0.2: define array keyword for LDMud & use it consistently
 * v1.0.3: fix for empty data structures
 * v1.0.4: Removed array keyword. (Yucong Sun)
 * v1.0.5: Fix decoding number 0.
 *
 * LICENSE
 *
 * The MIT License (MIT)
 *
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __STD_JSON_H
#define __STD_JSON_H

private mixed jsonParseValue(mixed *parse);

// parse: ({ UTF-8 buffer, byte offset }); buffers are only read, never changed.
private int jsonPeek(mixed *parse) {
    return parse[1] < sizeof(parse[0]) ? parse[0][parse[1]] : -1;
}

private void jsonError(mixed *parse, string message) {
    error(sprintf("json_decode: %s at byte %d.\n", message, parse[1]));
}

private void jsonWhitespace(mixed *parse) {
    int ch;

    ch = jsonPeek(parse);
    while (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n') {
        parse[1]++;
        ch = jsonPeek(parse);
    }
}

// Grow fragment arrays geometrically to avoid repeatedly copying long strings.
private void jsonAppend(mixed *parts, string text) {
    if (!sizeof(text)) return;
    if (parts[1] == sizeof(parts[0])) parts[0] += allocate(sizeof(parts[0]));
    parts[0][parts[1]++] = text;
}

private string jsonJoin(mixed *parts) {
    return parts[1] ? implode(parts[0][0..parts[1] - 1], "") : "";
}

private int jsonHex(mixed *parse) {
    int i, ch, value;

    for (i = 0; i < 4; i++) {
        ch = jsonPeek(parse);
        if (ch >= '0' && ch <= '9') ch -= '0';
        else if (ch >= 'a' && ch <= 'f') ch -= 'a' - 10;
        else if (ch >= 'A' && ch <= 'F') ch -= 'A' - 10;
        else jsonError(parse, "invalid Unicode escape");
        value = (value << 4) | ch;
        parse[1]++;
    }
    return value;
}

// ICU conversion may replace malformed UTF-8, so check the byte round trip.
private string jsonUtf8(buffer input) {
    string text;
    buffer encoded;
    int i;

    text = string_decode(input, "UTF-8");
    encoded = string_encode(text, "UTF-8");
    if (sizeof(encoded) != sizeof(input)) error("json_decode: invalid UTF-8.\n");
    for (i = 0; i < sizeof(input); i++)
        if (encoded[i] != input[i]) error("json_decode: invalid UTF-8.\n");
    return text;
}

// sprintf("%c") rejects some valid Unicode code points on older drivers.
private string jsonCodePoint(int code) {
    buffer bytes;

    if (code < 0x80) {
        bytes = allocate_buffer(1);
        bytes[0] = code;
    } else if (code < 0x800) {
        bytes = allocate_buffer(2);
        bytes[0] = 0xc0 | (code >> 6);
        bytes[1] = 0x80 | (code & 63);
    } else if (code < 0x10000) {
        bytes = allocate_buffer(3);
        bytes[0] = 0xe0 | (code >> 12);
        bytes[1] = 0x80 | ((code >> 6) & 63);
        bytes[2] = 0x80 | (code & 63);
    } else {
        bytes = allocate_buffer(4);
        bytes[0] = 0xf0 | (code >> 18);
        bytes[1] = 0x80 | ((code >> 12) & 63);
        bytes[2] = 0x80 | ((code >> 6) & 63);
        bytes[3] = 0x80 | (code & 63);
    }
    return string_decode(bytes, "UTF-8");
}

private string jsonParseString(mixed *parse) {
    mixed *parts;
    string escaped;
    int start, ch, code, low;

    if (jsonPeek(parse) != '"') jsonError(parse, "expected a quoted string");
    parse[1]++;
    parts = ({ allocate(16), 0 });
    start = parse[1];
    for (;;) {
        ch = jsonPeek(parse);
        if (ch < 32) jsonError(parse, "unterminated string or unescaped control character");
        if (ch != '"' && ch != '\\') {
            parse[1]++;
            continue;
        }
        if (parse[1] > start)
            jsonAppend(parts, jsonUtf8(parse[0][start..parse[1] - 1]));
        parse[1]++;
        if (ch == '"') return jsonJoin(parts);
        ch = jsonPeek(parse);
        parse[1]++;
        switch (ch) {
            case '"': escaped = "\""; break;
            case '\\': escaped = "\\"; break;
            case '/': escaped = "/"; break;
            case 'b': escaped = "\b"; break;
            case 'f': escaped = "\x0c"; break;
            case 'n': escaped = "\n"; break;
            case 'r': escaped = "\r"; break;
            case 't': escaped = "\t"; break;
            case 'u':
                code = jsonHex(parse);
                if (code >= 0xd800 && code <= 0xdbff) {
                    if (jsonPeek(parse) != '\\') jsonError(parse, "missing low surrogate");
                    parse[1]++;
                    if (jsonPeek(parse) != 'u') jsonError(parse, "missing low surrogate");
                    parse[1]++;
                    low = jsonHex(parse);
                    if (low < 0xdc00 || low > 0xdfff) jsonError(parse, "invalid low surrogate");
                    code = 0x10000 + ((code - 0xd800) << 10) + low - 0xdc00;
                } else if (code >= 0xdc00 && code <= 0xdfff) {
                    jsonError(parse, "unexpected low surrogate");
                }
                escaped = jsonCodePoint(code);
                break;
            default: jsonError(parse, "invalid string escape");
        }
        jsonAppend(parts, escaped);
        start = parse[1];
    }
}

private mixed jsonParseNumber(mixed *parse) {
    int start, digits, floating, ch;
    string number;

    start = parse[1];
    if (jsonPeek(parse) == '-') parse[1]++;
    ch = jsonPeek(parse);
    if (ch == '0') {
        parse[1]++;
    } else {
        if (ch < '1' || ch > '9') jsonError(parse, "expected a digit");
        while (jsonPeek(parse) >= '0' && jsonPeek(parse) <= '9') parse[1]++;
    }
    if (jsonPeek(parse) == '.') {
        floating = 1;
        parse[1]++;
        digits = parse[1];
        while (jsonPeek(parse) >= '0' && jsonPeek(parse) <= '9') parse[1]++;
        if (parse[1] == digits) jsonError(parse, "expected a fractional digit");
    }
    ch = jsonPeek(parse);
    if (ch == 'e' || ch == 'E') {
        floating = 1;
        parse[1]++;
        ch = jsonPeek(parse);
        if (ch == '+' || ch == '-') parse[1]++;
        digits = parse[1];
        while (jsonPeek(parse) >= '0' && jsonPeek(parse) <= '9') parse[1]++;
        if (parse[1] == digits) jsonError(parse, "expected an exponent digit");
    }
    number = string_decode(parse[0][start..parse[1] - 1], "UTF-8");
    return floating ? to_float(number) : to_int(number);
}

private mixed jsonParseValue(mixed *parse) {
    mapping fields;
    mixed *items;
    mixed value;
    string key, token;
    int ch, count, i;

    jsonWhitespace(parse);
    ch = jsonPeek(parse);
    switch (ch) {
        case '"': return jsonParseString(parse);
        case '{':
            fields = ([]);
            parse[1]++;
            jsonWhitespace(parse);
            if (jsonPeek(parse) == '}') {
                parse[1]++;
                return fields;
            }
            for (;;) {
                key = jsonParseString(parse);
                jsonWhitespace(parse);
                if (jsonPeek(parse) != ':') jsonError(parse, "expected ':'");
                parse[1]++;
                fields[key] = jsonParseValue(parse);
                jsonWhitespace(parse);
                ch = jsonPeek(parse);
                parse[1]++;
                if (ch == '}') return fields;
                if (ch != ',') jsonError(parse, "expected ',' or '}'");
                jsonWhitespace(parse);
            }
        case '[':
            items = allocate(16);
            parse[1]++;
            jsonWhitespace(parse);
            if (jsonPeek(parse) == ']') {
                parse[1]++;
                return ({});
            }
            for (;;) {
                value = jsonParseValue(parse);
                if (count == sizeof(items)) items += allocate(sizeof(items));
                items[count++] = value;
                jsonWhitespace(parse);
                ch = jsonPeek(parse);
                parse[1]++;
                if (ch == ']') return items[0..count - 1];
                if (ch != ',') jsonError(parse, "expected ',' or ']'");
            }
        case 't': token = "true"; value = 1; break;
        case 'f': token = "false"; value = 0; break;
        case 'n': token = "null"; value = 0; break;
        default:
            if (ch == '-' || (ch >= '0' && ch <= '9')) return jsonParseNumber(parse);
            jsonError(parse, "expected a JSON value");
    }
    for (i = 0; i < sizeof(token); i++) {
        if (jsonPeek(parse) != token[i]) jsonError(parse, "invalid literal");
        parse[1]++;
    }
    return value;
}

mixed json_decode(mixed text) {
    mixed *parse;
    mixed value;

    if (intp(text) && text == 0) return 0;
    if (stringp(text)) text = string_encode(text, "UTF-8");
    else if (!bufferp(text)) error("json_decode: expected a string or buffer.\n");
    parse = ({ text, 0 });
    value = jsonParseValue(parse);
    jsonWhitespace(parse);
    if (parse[1] != sizeof(text)) jsonError(parse, "unexpected trailing data");
    return value;
}

private string jsonQuote(string value) {
    buffer input;
    mixed *parts;
    string escaped;
    int i, start, ch;

    input = string_encode(value, "UTF-8");
    parts = ({ allocate(16), 0 });
    jsonAppend(parts, "\"");
    for (i = 0; i < sizeof(input); i++) {
        ch = input[i];
        if (ch >= 32 && ch != '"' && ch != '\\') continue;
        if (i > start) jsonAppend(parts, string_decode(input[start..i - 1], "UTF-8"));
        switch (ch) {
            case '"': escaped = "\\\""; break;
            case '\\': escaped = "\\\\"; break;
            case '\b': escaped = "\\b"; break;
            case 12: escaped = "\\f"; break;
            case '\n': escaped = "\\n"; break;
            case '\r': escaped = "\\r"; break;
            case '\t': escaped = "\\t"; break;
            default: escaped = sprintf("\\u%04x", ch);
        }
        jsonAppend(parts, escaped);
        start = i + 1;
    }
    if (start < sizeof(input)) jsonAppend(parts, string_decode(input[start..], "UTF-8"));
    jsonAppend(parts, "\"");
    return jsonJoin(parts);
}

varargs string json_encode(mixed value, mixed *pointers) {
    string *parts;
    mixed key, item;
    int count;

    if (undefinedp(value)) return "null";
    if (intp(value) || floatp(value)) return "" + value;
    if (stringp(value)) return jsonQuote(value);
    if (!mapp(value) && !arrayp(value)) return "null";
    // Only ancestors count as cycles; repeated non-cyclic values are preserved.
    if (pointers && member_array(value, pointers) != -1) return "null";
    pointers = pointers ? pointers + ({ value }) : ({ value });
    parts = allocate(sizeof(value));
    if (mapp(value)) {
        foreach (key, item in value) {
            // JSON object keys must be strings, as in the existing interface.
            if (!stringp(key)) continue;
            parts[count++] = jsonQuote(key) + ":" + json_encode(item, pointers);
        }
        return count ? "{" + implode(parts[0..count - 1], ",") + "}" : "{}";
    }
    foreach (item in value) parts[count++] = json_encode(item, pointers);
    return "[" + implode(parts, ",") + "]";
}

#endif /* __STD_JSON_H */
