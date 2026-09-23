// Base64 字符串接口：按 UTF-8 字节编码，不依赖外部服务。
string base64encode(string source) {
    string alphabet;
    buffer input, output;
    int i, j, size, value;

    alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    input = string_encode(source, "UTF-8");
    size = sizeof(input);
    output = allocate_buffer((size + 2) / 3 * 4);
    for (i = 0, j = 0; i < size; i += 3, j += 4) {
        value = input[i] << 16;
        if (i + 1 < size) value |= input[i + 1] << 8;
        if (i + 2 < size) value |= input[i + 2];
        output[j] = alphabet[(value >> 18) & 63];
        output[j + 1] = alphabet[(value >> 12) & 63];
        output[j + 2] = i + 1 < size ? alphabet[(value >> 6) & 63] : '=';
        output[j + 3] = i + 2 < size ? alphabet[value & 63] : '=';
    }
    return string_decode(output, "UTF-8");
}

private int base64_digit(int ch) {
    if (ch >= 'A' && ch <= 'Z') return ch - 'A';
    if (ch >= 'a' && ch <= 'z') return ch - 'a' + 26;
    if (ch >= '0' && ch <= '9') return ch - '0' + 52;
    if (ch == '+') return 62;
    if (ch == '/') return 63;
    error("base64decode: invalid character.\n");
}

string base64decode(string source) {
    string text;
    buffer input, compact, output;
    int i, j, size, padding, a, b, c, d, value;

    input = string_encode(source, "UTF-8");
    compact = allocate_buffer(sizeof(input));
    for (i = 0; i < sizeof(input); i++) {
        if (input[i] == ' ' || input[i] == '\t' || input[i] == '\r' || input[i] == '\n')
            continue;
        compact[size++] = input[i];
    }
    if (!size) return "";
    if (size % 4) error("base64decode: length must be a multiple of four.\n");
    if (compact[size - 1] == '=') padding++;
    if (compact[size - 2] == '=') padding++;
    output = allocate_buffer(size / 4 * 3 - padding);
    for (i = 0, j = 0; i < size; i += 4) {
        a = base64_digit(compact[i]);
        b = base64_digit(compact[i + 1]);
        c = i == size - 4 && padding == 2 ? 0 : base64_digit(compact[i + 2]);
        d = i == size - 4 && padding ? 0 : base64_digit(compact[i + 3]);
        if (i == size - 4 && ((padding == 2 && (b & 15)) || (padding == 1 && (c & 3))))
            error("base64decode: nonzero padding bits.\n");
        value = (a << 18) | (b << 12) | (c << 6) | d;
        output[j++] = (value >> 16) & 255;
        if (j < sizeof(output)) output[j++] = (value >> 8) & 255;
        if (j < sizeof(output)) output[j++] = value & 255;
    }
    // string_decode strips trailing NUL bytes; a sentinel preserves text exactly.
    output += string_encode(" ", "UTF-8");
    text = string_decode(output, "UTF-8");
    input = string_encode(text, "UTF-8");
    if (sizeof(input) != sizeof(output)) error("base64decode: result is not valid UTF-8.\n");
    for (i = 0; i < sizeof(output); i++)
        if (input[i] != output[i]) error("base64decode: result is not valid UTF-8.\n");
    return text[0..<2];
}
