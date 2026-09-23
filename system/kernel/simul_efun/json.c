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

private mixed json_parse_value(mixed *parse);

// parse: ({ UTF-8 buffer, byte offset }); buffers are only read, never changed.
private int json_peek(mixed *parse) {
    return parse[1] < sizeof(parse[0]) ? parse[0][parse[1]] : -1;
}

private void json_error(mixed *parse, string message) {
    error(sprintf("json_decode: %s at byte %d.\n", message, parse[1]));
}

private void json_whitespace(mixed *parse) {
    int ch;

    ch = json_peek(parse);
    while (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n') {
        parse[1]++;
        ch = json_peek(parse);
    }
}

// Grow fragment arrays geometrically to avoid repeatedly copying long strings.
private void json_append(mixed *parts, string text) {
    if (!sizeof(text)) return;
    if (parts[1] == sizeof(parts[0])) parts[0] += allocate(sizeof(parts[0]));
    parts[0][parts[1]++] = text;
}

private string json_join(mixed *parts) {
    return parts[1] ? implode(parts[0][0..parts[1] - 1], "") : "";
}

private int json_hex(mixed *parse) {
    int i, ch, value;

    for (i = 0; i < 4; i++) {
        ch = json_peek(parse);
        if (ch >= '0' && ch <= '9') ch -= '0';
        else if (ch >= 'a' && ch <= 'f') ch -= 'a' - 10;
        else if (ch >= 'A' && ch <= 'F') ch -= 'A' - 10;
        else json_error(parse, "invalid Unicode escape");
        value = (value << 4) | ch;
        parse[1]++;
    }
    return value;
}

// ICU conversion may replace malformed UTF-8, so check the byte round trip.
private string json_utf8(buffer input) {
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
private string json_code_point(int code) {
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

private string json_parse_string(mixed *parse) {
    mixed *parts;
    string escaped;
    int start, ch, code, low;

    if (json_peek(parse) != '"') json_error(parse, "expected a quoted string");
    parse[1]++;
    parts = ({ allocate(16), 0 });
    start = parse[1];
    for (;;) {
        ch = json_peek(parse);
        if (ch < 32) json_error(parse, "unterminated string or unescaped control character");
        if (ch != '"' && ch != '\\') {
            parse[1]++;
            continue;
        }
        if (parse[1] > start)
            json_append(parts, json_utf8(parse[0][start..parse[1] - 1]));
        parse[1]++;
        if (ch == '"') return json_join(parts);
        ch = json_peek(parse);
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
                code = json_hex(parse);
                if (code >= 0xd800 && code <= 0xdbff) {
                    if (json_peek(parse) != '\\') json_error(parse, "missing low surrogate");
                    parse[1]++;
                    if (json_peek(parse) != 'u') json_error(parse, "missing low surrogate");
                    parse[1]++;
                    low = json_hex(parse);
                    if (low < 0xdc00 || low > 0xdfff) json_error(parse, "invalid low surrogate");
                    code = 0x10000 + ((code - 0xd800) << 10) + low - 0xdc00;
                } else if (code >= 0xdc00 && code <= 0xdfff) {
                    json_error(parse, "unexpected low surrogate");
                }
                escaped = json_code_point(code);
                break;
            default: json_error(parse, "invalid string escape");
        }
        json_append(parts, escaped);
        start = parse[1];
    }
}

private mixed json_parse_number(mixed *parse) {
    int start, digits, floating, ch;
    string number;

    start = parse[1];
    if (json_peek(parse) == '-') parse[1]++;
    ch = json_peek(parse);
    if (ch == '0') {
        parse[1]++;
    } else {
        if (ch < '1' || ch > '9') json_error(parse, "expected a digit");
        while (json_peek(parse) >= '0' && json_peek(parse) <= '9') parse[1]++;
    }
    if (json_peek(parse) == '.') {
        floating = 1;
        parse[1]++;
        digits = parse[1];
        while (json_peek(parse) >= '0' && json_peek(parse) <= '9') parse[1]++;
        if (parse[1] == digits) json_error(parse, "expected a fractional digit");
    }
    ch = json_peek(parse);
    if (ch == 'e' || ch == 'E') {
        floating = 1;
        parse[1]++;
        ch = json_peek(parse);
        if (ch == '+' || ch == '-') parse[1]++;
        digits = parse[1];
        while (json_peek(parse) >= '0' && json_peek(parse) <= '9') parse[1]++;
        if (parse[1] == digits) json_error(parse, "expected an exponent digit");
    }
    number = string_decode(parse[0][start..parse[1] - 1], "UTF-8");
    return floating ? to_float(number) : to_int(number);
}

private mixed json_parse_value(mixed *parse) {
    mapping fields;
    mixed *items;
    mixed value;
    string key, token;
    int ch, count, i;

    json_whitespace(parse);
    ch = json_peek(parse);
    switch (ch) {
        case '"': return json_parse_string(parse);
        case '{':
            fields = ([]);
            parse[1]++;
            json_whitespace(parse);
            if (json_peek(parse) == '}') {
                parse[1]++;
                return fields;
            }
            for (;;) {
                key = json_parse_string(parse);
                json_whitespace(parse);
                if (json_peek(parse) != ':') json_error(parse, "expected ':'");
                parse[1]++;
                fields[key] = json_parse_value(parse);
                json_whitespace(parse);
                ch = json_peek(parse);
                parse[1]++;
                if (ch == '}') return fields;
                if (ch != ',') json_error(parse, "expected ',' or '}'");
                json_whitespace(parse);
            }
        case '[':
            items = allocate(16);
            parse[1]++;
            json_whitespace(parse);
            if (json_peek(parse) == ']') {
                parse[1]++;
                return ({});
            }
            for (;;) {
                value = json_parse_value(parse);
                if (count == sizeof(items)) items += allocate(sizeof(items));
                items[count++] = value;
                json_whitespace(parse);
                ch = json_peek(parse);
                parse[1]++;
                if (ch == ']') return items[0..count - 1];
                if (ch != ',') json_error(parse, "expected ',' or ']'");
            }
        case 't': token = "true"; value = 1; break;
        case 'f': token = "false"; value = 0; break;
        case 'n': token = "null"; value = 0; break;
        default:
            if (ch == '-' || (ch >= '0' && ch <= '9')) return json_parse_number(parse);
            json_error(parse, "expected a JSON value");
    }
    for (i = 0; i < sizeof(token); i++) {
        if (json_peek(parse) != token[i]) json_error(parse, "invalid literal");
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
    value = json_parse_value(parse);
    json_whitespace(parse);
    if (parse[1] != sizeof(text)) json_error(parse, "unexpected trailing data");
    return value;
}

private string json_quote(string value) {
    buffer input;
    mixed *parts;
    string escaped;
    int i, start, ch;

    input = string_encode(value, "UTF-8");
    parts = ({ allocate(16), 0 });
    json_append(parts, "\"");
    for (i = 0; i < sizeof(input); i++) {
        ch = input[i];
        if (ch >= 32 && ch != '"' && ch != '\\') continue;
        if (i > start) json_append(parts, string_decode(input[start..i - 1], "UTF-8"));
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
        json_append(parts, escaped);
        start = i + 1;
    }
    if (start < sizeof(input)) json_append(parts, string_decode(input[start..], "UTF-8"));
    json_append(parts, "\"");
    return json_join(parts);
}

varargs string json_encode(mixed value, mixed *pointers) {
    string *parts;
    mixed key, item;
    int count;

    if (undefinedp(value)) return "null";
    if (intp(value) || floatp(value)) return "" + value;
    if (stringp(value)) return json_quote(value);
    if (!mapp(value) && !arrayp(value)) return "null";
    // Only ancestors count as cycles; repeated non-cyclic values are preserved.
    if (pointers && member_array(value, pointers) != -1) return "null";
    pointers = pointers ? pointers + ({ value }) : ({ value });
    parts = allocate(sizeof(value));
    if (mapp(value)) {
        foreach (key, item in value) {
            // JSON object keys must be strings, as in the existing interface.
            if (!stringp(key)) continue;
            parts[count++] = json_quote(key) + ":" + json_encode(item, pointers);
        }
        return count ? "{" + implode(parts[0..count - 1], ",") + "}" : "{}";
    }
    foreach (item in value) parts[count++] = json_encode(item, pointers);
    return "[" + implode(parts, ",") + "]";
}

#endif /* __STD_JSON_H */
