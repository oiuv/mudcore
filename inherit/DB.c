/**
 * @brief LPMUD数据库操作模块，使用链式调用优雅的增删改查
 * @author 雪风@mud.ren
 * @copyright mudcore@mud.ren
 */
#ifdef __PACKAGE_DB__
#include <mysql.h>
// 数据库配置
nosave string db_host = DB_HOST;
nosave string db_db = DB_DATABASE;
nosave string db_user = DB_USERNAME;
nosave int db_type = __DEFAULT_DB__;
// 数据库查询
nosave mixed db_handle;
// 错误消息
nosave string db_error;
/**
 * @brief The table which the query is targeting.
 *
 */
nosave string db_table;
// 数据表头
nosave string *db_table_column = ({});

/**
 * @brief SQL语句
 *
 * SELECT
 * [DISTINCT] <select_list>
 * FROM <left_table>
 * [<join_type> JOIN <right_table>]
 * [ON <join_condition>]
 * [WHERE <where_condition>]
 * [GROUP BY <group_by_list>]
 * [HAVING <having_condition>]
 * [ORDER BY <order_by_condition>]
 * [LIMIT <limit_number>]
 * [OFFSET <offset_number>]
 * [UNION [ALL]]
 * [SELECT ***]
 */
nosave string db_sql;
// 显式 sql() 模板与最近执行的语句分开，链式查询每次重新构造。
nosave private string db_rawSql;
/**
 * @brief The columns that should be returned.
 *
 */
nosave string db_sql_columns = "*";
/**
 * @brief The where constraints for the query.
 *
 */
nosave string db_sql_where;
nosave string *db_sql_wheres = ({});
/**
 * @brief The groupings for the query.
 *
 */
nosave string db_sql_groups;
/**
 * @brief The having constraints for the query.
 *
 */
nosave string db_sql_havings;
/**
 * @brief The orderings for the query.
 *
 */
nosave string db_sql_orders;
/**
 * @brief The maximum number of records to return.
 *
 */
nosave int db_sql_limit = -1;
/**
 * @brief The number of records to skip.
 *
 */
nosave int db_sql_offset;

// 状态和条件
nosave int db_distinct;
nosave int db_inRandomOrder;
nosave int db_withColumn;
nosave int db_autoClose = 1;

varargs mixed close(int flag);
void resetSql();

// SQL 构造与参数处理。
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


/**
 * @brief 数据库连接初始化
 *
 * @param host
 * @param db
 * @param user
 * @return void
 */
varargs void create(string host, string db, string user, int type) {
    if (host) {
        db_host = host;
    }
    if (db) {
        db_db = db;
    }
    if (user) {
        db_user = user;
    }
    if (type) {
        db_type = type;
    }
}
/**
 * @brief 重置数据库链接配置
 *
 * @param db
 */
void setConnection(mapping db) {
    close(1);
    resetSql();
    db_host = db["host"];
    db_db = db["database"];
    db_user = db["user"];
    if (db["type"]) {
        db_type = db["type"];
    }
}
/**
 * @brief 是否自动关闭数据库连接
 *
 * @param flag
 */
void setAutoClose(int flag) {
    db_autoClose = flag;
}
// 重置查询
void resetSql() {
    db_error = 0;
    db_withColumn = 0;
    db_distinct = 0;
    db_sql = "";
    db_rawSql = 0;
    db_table = 0;
    db_table_column = ({});
    db_sql_columns = "*";
    db_sql_where = "";
    db_sql_wheres = ({});
    db_sql_groups = "";
    db_sql_havings = "";
    db_sql_orders = "";
    db_sql_limit = -1;
    db_sql_offset = 0;
    db_inRandomOrder = 0;
}
/**
 * @brief 原生SQL调用
 *
 * @return this_object()
 */
varargs object sql(string sql, mixed *bindings) {
    string prepared;

    if (!stringp(sql)) error("SQL template must be a string.\n");
    if (bindings && !arrayp(bindings)) error("SQL bindings must be an array.\n");
    prepared = arrayp(bindings) ? bindSql(sql, bindings) : sql;
    resetSql();
    db_rawSql = prepared;
    db_sql = prepared;

    return this_object();
}
/**
 * @brief 构造数据表调用
 *
 * @param table
 * @return object
 */
object table(string table) {
    resetSql();
    db_table = sqlIdentifier(table);

    return this_object();
}
/**
 * @brief 使用DISTINCT过滤重复数据
 *
 * @return object
 */
object distinct() {
    db_distinct = 1;

    return this_object();
}
/**
 * @brief 针对SQL展开数组的处理
 *
 * @param arr 需处理的数组
 * @param del 分隔符
 * @return string
 */
string implodeX(mixed *arr, string del) {
    string *result;
    mixed value;

    result = ({});
    foreach (value in arr) result += ({ sqlValue(value) });
    return implode(result, del);
}

// 数组条件作为一组追加，保留已有条件，避免多次调用互相覆盖。
void addArrayOfWheres(mixed *where, string boolean) {
    string *conditions;
    mixed item;

    if (!sizeof(where)) error("SQL condition group cannot be empty.\n");
    conditions = ({});
    foreach (item in where) {
        if (!arrayp(item)) error("SQL condition group must contain arrays.\n");
        conditions += ({ sqlCondition(item) });
    }
    appendWhere("(" + implode(conditions, " AND ") + ")", boolean);
}

object where(mixed *x...) {
    if (sizeof(x) == 1 && arrayp(x[0])) addArrayOfWheres(x[0], "AND");
    else appendWhere(sqlCondition(x), "AND");
    return this_object();
}

object orWhere(mixed *x...) {
    if (sizeof(x) == 1 && arrayp(x[0])) addArrayOfWheres(x[0], "OR");
    else appendWhere(sqlCondition(x), "OR");
    return this_object();
}

private object addBetween(string column, mixed *values, int not, string boolean) {
    string condition;

    if (!arrayp(values) || sizeof(values) != 2) error("BETWEEN requires two values.\n");
    condition = sqlColumn(
        column,
        0
    ) + (not ? " NOT BETWEEN " : " BETWEEN ") + sqlValue(values[0]) + " AND " + sqlValue(values[1]);
    appendWhere(condition, boolean);
    return this_object();
}

varargs object whereBetween(string column, mixed *x, int not) {
    return addBetween(column, x, not, "AND");
}
object whereNotBetween(string column, mixed *x) { return addBetween(column, x, 1, "AND"); }
varargs object orWhereBetween(string column, mixed *x, int not) {
    return addBetween(column, x, not, "OR");
}
object orWhereNotBetween(string column, mixed *x) { return addBetween(column, x, 1, "OR"); }

private object addNull(string column, int not, string boolean) {
    appendWhere(sqlColumn(column, 0) + (not ? " IS NOT NULL" : " IS NULL"), boolean);
    return this_object();
}

varargs object whereNull(string column, int not) { return addNull(column, not, "AND"); }
object whereNotNull(string column) { return addNull(column, 1, "AND"); }
varargs object orWhereNull(string column, int not) { return addNull(column, not, "OR"); }
object orWhereNotNull(string column) { return addNull(column, 1, "OR"); }

private object addIn(string column, mixed *values, int not, string boolean) {
    string field, condition;

    field = sqlColumn(column, 0);
    if (!arrayp(values)) error("IN requires an array.\n");
    // 空集合不产生无效 SQL，IN 恒假，NOT IN 恒真。
    condition = sizeof(values) ? field + (not ? " NOT IN (" : " IN (") + implodeX(
        values,
        ","
    ) + ")" : (not ? "1=1" : "1=0");
    appendWhere(condition, boolean);
    return this_object();
}

varargs object whereIn(string column, mixed *x, int not) { return addIn(column, x, not, "AND"); }
object whereNotIn(string column, mixed *x) { return addIn(column, x, 1, "AND"); }
varargs object orWhereIn(string column, mixed *x, int not) { return addIn(column, x, not, "OR"); }
object orWhereNotIn(string column, mixed *x) { return addIn(column, x, 1, "OR"); }

object groupBy(string *column...) {
    string *groups;
    string field;

    if (!sizeof(column)) error("GROUP BY requires at least one column.\n");
    groups = ({});
    foreach (field in column) groups += ({ sqlIdentifier(field) });
    db_sql_groups += (sizeof(db_sql_groups) ? "," : "") + implode(groups, ",");
    return this_object();
}

varargs object having(string column, string operator, mixed value, string boolean) {
    string condition, op;

    condition = sqlCondition(({ column, operator, value }));
    op = sqlBoolean(boolean);
    db_sql_havings += (sizeof(db_sql_havings) ? " " + op + " " : "") + condition;
    return this_object();
}

object orHaving(string column, string operator, mixed value) {
    return having(column, operator, value, "OR");
}
/**
 * @brief 排序
 *
 * @param column
 * @param order asc / desc
 * @return object
 */
varargs object orderBy(string column, string order) {
    string field;

    if (!nullp(column)) {
        field = sqlColumn(column, 0);
        if (!stringp(order) || member_array(lower_case(order), ({ "asc", "desc" })) < 0) {
            order = "ASC";
        }
        if (sizeof(db_sql_orders)) {
            db_sql_orders += ",";
        }

        db_sql_orders += field + " " + upper_case(order);
    }

    return this_object();
}

object inRandomOrder() {
    db_inRandomOrder = 1;

    return this_object();
}
/**
 * @brief limit处理
 *
 * @param n
 * @return object
 */
object limit(int n) {
    if (n < 0) error("LIMIT must not be negative.\n");
    db_sql_limit = n;

    return this_object();
}
/**
 * @brief offset处理
 *
 * @param n
 * @return object
 */
object offset(int n) {
    if (n < 0) error("OFFSET must not be negative.\n");
    db_sql_offset = n;

    return this_object();
}

object with(string str) {
    switch (str) {
        case "column":
            db_withColumn = 1;
            break;

        default:
            break;
    }

    return this_object();
}

// 构造完整语句后再执行；校验失败不能留下半条 UPDATE/DELETE。
private varargs string db_sql_bindings(string query, int writeQuery, int firstOnly) {
    int rowLimit;

    rowLimit = db_sql_limit;
    if (firstOnly && (rowLimit < 0 || rowLimit > 1)) rowLimit = 1;
    if (db_sql_offset && rowLimit < 0) error("OFFSET requires LIMIT.\n");
    if (writeQuery && (sizeof(db_sql_groups) || sizeof(db_sql_havings)))
        error("GROUP BY and HAVING are only supported for SELECT.\n");
    if (sizeof(db_sql_where)) query += " WHERE " + db_sql_where;
    if (!writeQuery && sizeof(db_sql_groups)) query += " GROUP BY " + db_sql_groups;
    if (!writeQuery && sizeof(db_sql_havings)) query += " HAVING " + db_sql_havings;
    if (sizeof(db_sql_orders)) query += " ORDER BY " + db_sql_orders;
    if (rowLimit >= 0) {
        query += " LIMIT " + rowLimit;
        if (db_sql_offset) query += " OFFSET " + db_sql_offset;
    }
    return query;
}

// 原始 SQL 保持原样；构造器不复用上一次执行的 SQL。
private varargs string db_sql(int firstOnly) {
    if (stringp(db_rawSql)) return db_rawSql;
    if (!stringp(db_table)) error("Call table() or sql() before executing a query.\n");
    return db_sql_bindings("SELECT " + (db_distinct ? "DISTINCT " : "") +
        db_sql_columns + " FROM " + db_table, 0, firstOnly);
}
/**
 * @brief 连接数据库并返回handle
 *
 */
private mixed connect() {
    // 连接数据库
    if (!db_handle || stringp(db_handle)) {
        db_handle = db_connect(db_host, db_db, db_user, db_type);
        if (!db_handle)
            return db_error = "Database connection failed";
        /* error */
        if (stringp(db_handle))
            return db_error = db_handle;
        else {
            // 字符集初始化只适用于 MySQL，SQLite 等后端不能执行此语句。
#ifdef __USE_MYSQL__
            if (db_type == __USE_MYSQL__)
                db_exec(db_handle, "set names utf8mb4");
#endif
        }
    }
    return db_handle;
}
/**
 * @brief 关闭数据库连接
 *
 */
varargs mixed close(int flag) {
    if ((flag || db_autoClose) && intp(db_handle) && db_handle && db_close(db_handle)) {
        db_handle = 0;
    }

    return db_handle;
}
/**
 * @brief 执行SQL语句并返回结果行数
 *
 */
private mixed executeQuery(string query) {
    mixed rows;

    db_error = 0;
    db_table_column = ({});
    db_sql = query;
    // 连接数据库
    if (stringp(connect())) {
        return db_error;
    }
    // 执行SQL语句
    rows = db_exec(db_handle, query);
    /* error */
    if (stringp(rows)) {
        close();
        return db_error = rows;
    }
    // 保存数据表头
    db_table_column = db_fetch(db_handle, 0);

    return rows;
}

varargs mixed exec() {
    return executeQuery(db_sql());
}

// 某些 SQLite 驱动把执行失败也返回为 0，读取接口还必须确认列元数据。
private int hasResultColumns() {
    if (arrayp(db_table_column) && sizeof(db_table_column)) return 1;
    close();
    db_error = "Database query returned no column metadata.";
    return 0;
}

varargs mixed get(string *columns...) {
    mixed rows, *res;
    int i;

    db_sql_columns = sizeof(columns) ? sqlColumns(columns) : "*";

    rows = exec();
    /* error */
    if (stringp(db_error) || !hasResultColumns()) {
        return db_error;
    }

    res = allocate(rows);
    for (i = 1; i <= rows; i++) {
        res[i - 1] = db_fetch(db_handle, i);
    }
    close();

    if (db_inRandomOrder) {
        res = shuffle(res);
    }
    if (db_withColumn) {
        res = ({ db_table_column }) + res;
    }

    return res;
}

mixed pluck(string column) {
    mixed res, *arr;
    int index, start, i;

    res = get(column);
    if (stringp(db_error)) return db_error;
    index = stringp(db_rawSql) ? member_array(column, db_table_column) : 0;
    if (index < 0) return db_error = "Requested column is not in the query result.";
    start = db_withColumn ? 1 : 0;
    arr = allocate(sizeof(res) - start);
    for (i = start; i < sizeof(res); i++) arr[i - start] = res[i][index];
    return arr;
}

varargs mixed first(string *columns...) {
    mixed rows, *res;
    int i = 1;

    db_sql_columns = sizeof(columns) ? sqlColumns(columns) : "*";

    rows = executeQuery(db_sql(!db_inRandomOrder));
    /* error */
    if (stringp(db_error) || !hasResultColumns()) {
        return db_error;
    }

    if (!rows) {
        close();
        return ({});
    }
    if (db_inRandomOrder) {
        i = random(rows) + 1;
    }
    res = db_fetch(db_handle, i);
    close();

    return res;
}

mixed find(int id) {
    return where("id", "=", id)->first();
}

mixed value(string column) {
    mixed result;
    int index;

    result = first(column);
    if (stringp(result)) return result;
    index = stringp(db_rawSql) ? member_array(column, db_table_column) : 0;
    if (index < 0) return db_error = "Requested column is not in the query result.";
    return sizeof(result) ? result[index] : "";
}

/**
 * @brief 数据库聚合函数
 *
 * @param func
 * @param column
 * @return private
 */
private mixed aggregate(string func, mixed column) {
    mixed rows, *res;
    string query, expression;

    if (!column) return "";
    if (!stringp(db_table)) error("Call table() before an aggregate query.\n");
    expression = intp(column) ? sprintf("%d", column) : sqlColumn(column, 0);
    if (db_distinct && stringp(column) && column != "*") expression = "DISTINCT " + expression;
    query = "SELECT " + func + "(" + expression + ") FROM " + db_table;
    rows = executeQuery(db_sql_bindings(query));
    if (stringp(db_error) || !hasResultColumns()) return db_error;
    if (!rows) {
        close();
        return 0;
    }
    res = db_fetch(db_handle, 1);
    close();
    return arrayp(res) && sizeof(res) ? res[0] : 0;
}

varargs mixed count(mixed column) {
    if (nullp(column)) {
        column = 1;
    }

    return aggregate("COUNT", column);
}

mixed max(string column) {
    return aggregate("MAX", column);
}

mixed min(string column) {
    return aggregate("MIN", column);
}

mixed avg(string column) {
    return aggregate("AVG", column);
}

mixed sum(string column) {
    return aggregate("SUM", column);
}

/**
 * @brief 插入
 *
 */
mixed insert(mapping m) {
    string *columns, *fields, *encoded;
    string column, query;

    if (!stringp(db_table)) error("Call table() before INSERT.\n");
    if (!sizeof(m)) error("INSERT requires at least one field.\n");
    columns = keys(m);
    fields = ({});
    encoded = ({});
    foreach (column in columns) {
        fields += ({ sqlIdentifier(column) });
        encoded += ({ sqlValue(m[column]) });
    }
    query = "INSERT INTO " + db_table + " (" + implode(
        fields,
        ","
    ) + ") VALUES (" + implode(encoded, ",") + ")";
    executeQuery(query);
    if (stringp(db_error)) return db_error;
    close();
    return 1;
}

mixed update(mapping m) {
    string *assignments;
    string query;
    mixed key, value;

    if (!stringp(db_table)) error("Call table() before UPDATE.\n");
    if (!sizeof(m)) error("UPDATE requires at least one field.\n");
    assignments = ({});
    foreach (key, value in m)
        assignments += ({ sqlIdentifier(key) + "=" + sqlValue(value) });
    query = "UPDATE " + db_table + " SET " + implode(assignments, ",");
    executeQuery(db_sql_bindings(query, 1));
    if (stringp(db_error)) return db_error;
    close();
    return 1;
}

mixed delete() {
    if (!stringp(db_table)) error("Call table() before DELETE.\n");
    executeQuery(db_sql_bindings("DELETE FROM " + db_table, 1));
    if (stringp(db_error)) return db_error;
    close();
    return 1;
}

/**
 * @brief 调试
 *
 */
string dump() {
    string line = repeat_string("-*-", 20);
    return sprintf(
        "\n%s\ndb_host = %s\ndb_db = %s\ndb_user = %s\ndb_handle = %d\ndb_error = %O\ndb_table = %s\ndb_table_column = %O\ndb_sql = %s\ndb_status = %s\n%s\n",
        line,
        db_host,
        db_db,
        db_user,
        db_handle,
        db_error,
        db_table,
        db_table_column,
        db_sql,
        db_status(),
        line
    );
}

#endif
