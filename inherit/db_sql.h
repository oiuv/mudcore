#ifndef MUDCORE_DB_SQL_H
#define MUDCORE_DB_SQL_H

// 数据库查询构造器的内部实现，由 DB.c 包含，不单独加载。
private int mysqlDialect() {
#ifdef __USE_MYSQL__
    return db_type == __USE_MYSQL__;
#else
    return 0;
#endif
}

private string sqlIdentifier(string name) {
    string *parts;
    string quote;
    int i;

    if (!stringp(name) || !sizeof(regexp(({ name }),
        "^[A-Za-z_][A-Za-z0-9_]*(\\.[A-Za-z_][A-Za-z0-9_]*)*$")))
        error("Invalid SQL identifier.\n");
    quote = mysqlDialect() ? "`" : "\"";
    parts = explode(name, ".");
    for (i = 0; i < sizeof(parts); i++)
        parts[i] = quote + parts[i] + quote;
    return implode(parts, ".");
}

// 构造器只接受字段、通配符和常见聚合表达式；复杂 SQL 由 sql() 显式提供。
private string sqlColumn(string column, int allowAlias) {
    string name, alias, func, argument, expression;
    int split, open;

    if (!stringp(column)) error("SQL column must be a string.\n");
    name = trim(column);
    split = strsrch(upper_case(name), " AS ");
    if (split >= 0) {
        if (!allowAlias) error("SQL alias is not allowed here.\n");
        alias = sqlIdentifier(trim(name[split + 4..]));
        name = trim(name[0..split - 1]);
    }
    open = strsrch(name, "(");
    if (open > 0 && name[<1] == ')') {
        func = upper_case(trim(name[0..open - 1]));
        if (member_array(func, ({ "COUNT", "SUM", "MIN", "MAX", "AVG" })) < 0)
            error("Unsupported SQL aggregate expression.\n");
        argument = trim(name[open + 1..<2]);
        if (argument == "*" || argument == "1") {
            if (func != "COUNT") error("Only COUNT accepts * or 1.\n");
        } else if (strsrch(upper_case(argument), "DISTINCT ") == 0) {
            argument = "DISTINCT " + sqlIdentifier(trim(argument[9..]));
        } else {
            argument = sqlIdentifier(argument);
        }
        expression = func + "(" + argument + ")";
    } else if (name == "*") {
        expression = name;
    } else if (strlen(name) > 2 && name[<2..] == ".*") {
        expression = sqlIdentifier(name[0..<3]) + ".*";
    } else {
        expression = sqlIdentifier(name);
    }
    return expression + (alias ? " AS " + alias : "");
}

private string sqlOperator(string op) {
    string result;

    if (!stringp(op)) error("SQL operator must be a string.\n");
    result = upper_case(trim(op));
    if (member_array(result, ({ "=", "!=", "<>", ">", ">=", "<", "<=", "LIKE",
        "NOT LIKE", "IS", "IS NOT", "GLOB", "NOT GLOB", "REGEXP", "NOT REGEXP", "MATCH" })) < 0)
        error("Unsupported SQL comparison operator.\n");
    return result;
}

// 值编码与 SQL 模板分开。十六进制文本不受反斜杠转义或 SQL 模式影响。
private string sqlValue(mixed value) {
    buffer bytes;
    string *hexBytes;
    string encoded, number;
    int i;

    if (undefinedp(value)) return "NULL";
    if (intp(value)) return sprintf("%d", value);
    if (floatp(value)) {
        number = sprintf("%.17g", value);
        if (!sizeof(regexp(({ number }), "^[+-]?[0-9]+(\\.[0-9]+)?([eE][+-]?[0-9]+)?$")))
            error("SQL numeric values must be finite.\n");
        return number;
    }
    if (!stringp(value)) error("SQL values must be strings, numbers or undefined.\n");
    // 驱动部分后端按 C 字符串读取 TEXT，拒绝无法可靠往返的 NUL。
    bytes = string_encode(value, "UTF-8");
    hexBytes = allocate(sizeof(bytes));
    for (i = 0; i < sizeof(bytes); i++) {
        if (!bytes[i]) error("SQL text cannot contain NUL.\n");
        hexBytes[i] = sprintf("%02x", bytes[i]);
    }
    encoded = implode(hexBytes, "");
#ifdef __USE_SQLITE3__
    if (db_type == __USE_SQLITE3__) return "CAST(X'" + encoded + "' AS TEXT)";
#endif
#ifdef __USE_MYSQL__
    if (db_type == __USE_MYSQL__) return "CONVERT(X'" + encoded + "' USING utf8mb4)";
#endif
#ifdef __USE_POSTGRE__
    if (db_type == __USE_POSTGRE__) return "convert_from(decode('" + encoded + "','hex'),'UTF8')";
#endif
    error("SQL text encoding is not implemented for this database backend.\n");
}

private string sqlCondition(mixed *args) {
    string op;
    mixed value;

    if (sizeof(args) != 2 && sizeof(args) != 3)
        error("SQL comparison requires a column and value, with an optional operator.\n");
    op = sizeof(args) == 2 ? "=" : sqlOperator(args[1]);
    value = args[<1];
    if (undefinedp(value)) {
        if (op == "=") op = "IS";
        else if (op == "!=" || op == "<>") op = "IS NOT";
    }
    return sqlColumn(args[0], 0) + " " + op + " " + sqlValue(value);
}

private string sqlBoolean(string boolean) {
    string result;

    result = stringp(boolean) && trim(boolean) != "" ? upper_case(trim(boolean)) : "AND";
    if (result != "AND" && result != "OR") error("SQL boolean must be AND or OR.\n");
    return result;
}

private void appendWhere(string condition, string boolean) {
    string op;

    op = sqlBoolean(boolean);
    db_sql_where += (sizeof(db_sql_where) ? " " + op + " " : "") + condition;
}

private string sqlColumns(string *columns) {
    string *result;
    string column;

    result = ({});
    foreach (column in columns) result += ({ sqlColumn(column, 1) });
    return implode(result, ",");
}

// 仅替换引号和普通注释之外的匿名 ?；不猜测依赖 SQL 模式的语法。
private string bindSql(string query, mixed *bindings) {
    string result, ch, next;
    int i, quote, comment, index;

    result = "";
    for (i = 0; i < strlen(query); i++) {
        ch = query[i..i];
        next = i + 1 < strlen(query) ? query[i + 1..i + 1] : "";
        if (comment == 1) {
            result += ch;
            if (ch == "\n" || ch == "\r") comment = 0;
        } else if (comment == 2) {
            if (ch == "/" && next == "*") error("Nested SQL comments are not supported with bindings.\n");
            result += ch;
            if (ch == "*" && next == "/") {
                result += next;
                i++;
                comment = 0;
            }
        } else if (quote) {
            if (ch == "\\") error("Use a bound parameter instead of a backslash in a SQL quoted token.\n");
            result += ch;
            if (query[i] == quote) {
                if (i + 1 < strlen(query) && query[i + 1] == quote) {
                    result += next;
                    i++;
                } else quote = 0;
            }
        } else if (ch == "'" || ch == "\"" || ch == "`") {
            quote = query[i];
            result += ch;
        } else if (ch == "[") {
            quote = ']';
            result += ch;
        } else if (ch == "-" && next == "-") {
            comment = 1;
            result += ch + next;
            i++;
        } else if (ch == "#" && mysqlDialect()) {
            comment = 1;
            result += ch;
        } else if (ch == "/" && next == "*") {
            if (i + 2 < strlen(query) && (query[i + 2] == '!' ||
                (i + 3 < strlen(query) && upper_case(query[i + 2..i + 3]) == "M!")))
                error("Executable SQL comments are not supported with bindings.\n");
            comment = 2;
            result += ch + next;
            i++;
        } else if (ch == ":" && next == ":") {
            // 保留 PostgreSQL 类型转换，单冒号命名参数不接受。
            result += ch + next;
            i++;
        } else if (ch == ":" || ch == "@") {
            error("Named SQL parameters are not supported; use anonymous ?.\n");
        } else if (ch == "$") {
            error("Dollar quoting and named SQL parameters are not supported.\n");
        } else if (ch == "?") {
            if (next != "" && sizeof(regexp(({ next }), "^[0-9]$")))
                error("Use anonymous ? parameters, not numbered parameters.\n");
            if (index >= sizeof(bindings)) error("Not enough SQL parameter values.\n");
            // 参数保持独立表达式，避免 -? 与负数拼成 -- 注释。
            result += "(" + sqlValue(bindings[index++]) + ")";
        } else {
            result += ch;
        }
    }
    if (quote || comment == 2) error("Unterminated SQL quote or comment.\n");
    if (index != sizeof(bindings)) error("Too many SQL parameter values.\n");
    return result;
}

#endif
