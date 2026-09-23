# 数据库接口与授权

`CORE_DB`（`/mudcore/inherit/DB`）提供数据库连接和链式 SQL 构造，也可通过宿主可覆盖的 `_DB` 使用。支持哪些后端取决于 FluffOS 编译选项，不要求连接外部服务；SQLite 可以只使用宿主本地文件。

## 驱动与权限前提

驱动需要启用 `__PACKAGE_DB__` 和所选后端。创建对象或调用 `set_connection()` 只设置连接参数，执行查询时才实际连接；指定参数不等于获得 master 授权。

默认 master 的 `valid_database()` 要求数据库调用对象的 UID、EUID 均为 `ROOT_UID`，并校验显式配置的目标：

| 后端 | 默认授权条件 |
| --- | --- |
| SQLite | 配置非空 `DB_SQLITE_DATABASE`，请求路径完全一致，使用空 host/user |
| 其他后端 | database、host、user 与 `DB_DATABASE`、`DB_HOST`、`DB_USERNAME` 一致，且 `DB_PASSWORD` 为字符串 |

未配置目标时拒绝连接。SQLite 的允许结果为 `1`，表示允许无密码连接；整数 `0` 表示拒绝。其他后端允许显式配置空字符串密码，但不等同于缺少密码配置。

宿主有自己的 master 时，应按自己的 DAO 对象和目标授权。驱动传给连接检查的 `info` 是 `({ database, host, user })`，不含后端类型；由指定 DAO 固定后端，并在 master 限制调用者及目标，不要把 `set_connection()` 当作权限检查。

## 连接配置

### SQLite

在宿主 `DATA_DIR + ".env"` 中按需配置：

```text
DB_SQLITE_DATABASE : /data/game.sqlite
```

已获授权的服务对象可创建并配置组件。以下是函数体片段，`db` 供后续查询使用：

```c
object db;

db = new(CORE_DB);
db->set_connection(([
    "host": "",
    "database": "/data/game.sqlite",
    "user": "",
    "type": __USE_SQLITE3__
]));
```

`__USE_SQLITE3__` 仅在驱动启用对应后端时可用；可选代码需要相应条件编译。路径应符合宿主文件权限且父目录存在。`DB_SQLITE_DATABASE` 用于默认 master 授权，并不会自动替代组件的 `database` 参数。

### MySQL 等服务端数据库

宿主 `.env` 使用冒号格式，以下值为占位示例，应替换为自己的连接信息：

```text
DB_HOST : 127.0.0.1
DB_DATABASE : game
DB_USERNAME : game_user
DB_PASSWORD : 请替换为实际密码
```

`new(CORE_DB)` 默认读取前三项，并使用驱动 `__DEFAULT_DB__`。需要显式选择时使用 `set_connection()` 的 `type` 或构造参数：

```c
object db;

db = new(CORE_DB, "127.0.0.1", "game", "game_user", __USE_MYSQL__);
```

密码由 master 的 `valid_database()` 提供，不是 `set_connection()` 的字段。不要将实际凭据写入源码或提交 `.env`。模块只对 MySQL 执行 `set names utf8mb4` 初始化。

### 公共配置方法

| 方法 | 作用 |
| --- | --- |
| `create(host, database, user, type)` | 创建时设置连接信息，参数可省略 |
| `set_connection(mapping connection)` | 设置 `host`、`database`、`user`，可选 `type`；未指定类型时保留当前类型 |
| `set_auto_close(int flag)` | 控制高层查询结束后的自动关闭，默认开启 |
| `close(1)` | 强制关闭当前连接，下次查询再连接 |

`set_connection()` 会先关闭旧连接并清空查询状态，再应用新目标，避免继续使用旧连接或旧后端的参数编码。不同查询流程不要交叉修改同一个组件的构造状态。

## 查询

下面片段假定 `db` 已配置且获授权，数据表由宿主创建：

```c
mixed rows;

rows = db->table("players")->where("level", ">=", 10)->get("id", "name");
if (stringp(rows)) {
    // 处理数据库返回的错误文本。
    error(rows + "\n");
}
printf("%O\n", rows);
```

`table()` 和 `sql()` 开始一条新查询，会重置前一条查询的条件。同一个 `table()` 查询可继续追加条件并再次读取；每次按当前条件重新生成 SQL，聚合或写入不会变成后续读取的语句。`get()` / `first()` 省略列名时重新选择全部列，失败后重试不会沿用旧错误。

| 方法 | 结果或作用 |
| --- | --- |
| `get()` / `get("id", "name")` | 返回二维数组；无行时为空数组，常规数据库错误返回字符串 |
| `first()` / `first("id", "name")` | 取第一行，列名是可变参数，不是一个数组参数 |
| `find(int id)` | 按 `id` 列查找第一行 |
| `value(string column)` | 取首行指定列的值 |
| `pluck(string column)` | 返回指定列的值数组 |
| `count()`、`max()`、`min()`、`avg()`、`sum()` | 聚合查询 |
| `with("column")` | `get()` 的结果第一行包含列名 |

普通链式查询的 `first()`、`find()` 和 `value()` 在数据库端使用 `LIMIT 1`，保留分页偏移且不改变查询对象原有的 limit。显式 `limit(0)` 仍返回空结果；`in_random_order()` 保留原有候选集，原始 `sql()` 不自动改写。`pluck()` 只返回列值，不包含 `with("column")` 的表头。

`first()` 无记录时返回空数组，`value()` 无记录时沿用空字符串返回；数据库返回的错误字符串继续向上传递。字符串既可能是业务值，也可能是错误，因此需要严格区分结果时优先使用 `get()` / `first()` 检查数组结果。权限拒绝等驱动错误可能抛异常，在服务边界使用 `catch` 处理。

部分 SQLite 驱动在查询失败时返回 `0`；读取接口还会检查列元数据，缺失时返回错误，避免把失败当作空结果。此检查不能弥补驱动对建表、写入等无结果集语句的所有错误报告缺陷。

### 条件与排序

| 调用 | 含义 |
| --- | --- |
| `where("id", 1)` | 等号条件 |
| `where("level", ">=", 10)` | 指定运算符 |
| `where(({ ({ "level", ">=", 10 }), ({ "active", 1 }) }))` | 同时满足多项条件 |
| `or_where(...)` | 追加 OR 条件；作为第一个条件时不生成多余 OR |
| `where_between("level", ({ 10, 20 }))` | 区间条件 |
| `where_in("id", ({ 1, 2, 3 }))` | 集合条件，第三参数可省略，表示是否取反 |
| `where_null("deleted_at")` | NULL 条件 |
| `distinct()` | 查询去重；指定字段的聚合对该字段去重，如 `distinct()->count("team")` |
| `order_by("level", "desc")` | 排序，多次调用可追加字段 |
| `limit(10)->offset(20)` | 限制行数与偏移；均须非负，偏移必须配合 limit |
| `in_random_order()` | 对读取到的结果随机处理，不等同于数据库端随机查询 |

`count()` / `count("*")` 统计行数；需要统计不同值时明确提供字段，例如 `distinct()->count("team")`。

数组形式的 `where` / `or_where` 将数组内的条件以 AND 组合，并整体追加到已有条件，不覆盖先前的筛选。空 `IN` 集合恒假，空 `NOT IN` 集合恒真。

区间、集合和 NULL 条件还有 `or_where_*`、`where_not_*` 等对应方法，签名见 [实现](../../inherit/DB.c)。

## 分组与过滤

`group_by()` 支持多个字段及重复调用追加，`having()` 默认以 AND 追加，第四参数可指定 `"AND"` 或 `"OR"`；也可使用 `or_having()`。

```c
mixed rows;

rows = db->table("scores")
    ->where("active", 1)
    ->group_by("team")
    ->having("SUM(points)", ">=", 20)
    ->order_by("total", "desc")
    ->get("team", "SUM(points) AS total", "COUNT(*) AS members");
```

生成顺序为 `WHERE → GROUP BY → HAVING → ORDER BY → LIMIT/OFFSET`。`get()` 用于获取全部分组；`first()` 及标量聚合方法只取首个结果。分组条件只用于 SELECT，对带 GROUP BY/HAVING 的 `update()` 或 `delete()` 会报错，不静默忽略条件。

新一轮 `table()` / `sql()` 会清空之前的分组、过滤和排序。被拒绝的分组写入不会留下待执行的 UPDATE/DELETE。

## 写入与原生 SQL

```c
mixed result;

result = db->table("players")->insert(([ "id": 1, "name": "示例角色" ]));
result = db->table("players")->where("id", 1)->update(([ "name": "新名称" ]));
result = db->table("players")->where("id", 1)->delete();
```

这是三个独立操作的调用示例，实际代码应逐次检查结果。`insert()`、`update()`、`delete()` 的正常结果为 `1`，表示 SQL 执行成功，不是实际受影响行数；常规错误返回字符串。更新和删除不指定条件时会作用于整张表。

原生语句通过 `sql(...)->exec()` 执行：

```c
mixed result;

result = db->sql("CREATE TABLE IF NOT EXISTS sample (id INTEGER PRIMARY KEY, name TEXT)")->exec();
db->close(1);
```

`exec()` 返回驱动的执行结果，不能把建表等语句返回 `0` 一概当作失败；成功路径不会自动关闭连接，因此显式调用 `close(1)`。异常路径也应由调用方完成清理。高层 `get()`、`first()`、聚合及写入方法在默认设置下会结束连接。

`sql(...)->value("name")` / `pluck("name")` 按原始结果集的列名选择值；使用别名时传入对应别名，缺少列时返回错误。

`dump()` 返回当前连接与 SQL 状态，调试时使用 `printf("%s", db->dump())`。不要把可能包含业务数据的诊断信息直接展示给玩家。

## 占位参数

在 `sql()` 的第二个参数中传入数组，每个匿名 `?` 对应一个值：

```c
mixed rows;
string playerName;

playerName = "O'Reilly";
rows = db->sql("SELECT id, name FROM players WHERE name = ? AND level >= ?",
    ({ playerName, 10 }))->get();
```

框架只替换引号和普通注释之外的 `?`，支持 SQL 的重复引号转义；参数数量必须匹配。数组中的字符串、整数、有限浮点数和 `undefined` 分别编码为文本、数字和 SQL NULL；整数 `0` 与空字符串保持各自含义。mapping 缺失项属于 `undefined`，需要默认数值时应由调用方显式提供。

相同的值编码也用于 `where`、`having`、IN/BETWEEN、`insert` 和 `update`。字符串中的单引号、反斜杠、中文和注入样式文本都作为数据处理。SQL NULL 的 `where` 等号比较会生成 `IS NULL`，不等号生成 `IS NOT NULL`；也可直接使用 `where_null()`。

当前 FluffOS 的 `db_exec(handle, sql)` 没有向 LPC 暴露原生绑定接口。这里是**框架层的参数替换与值编码**，不是数据库服务端预处理语句，也不提供预处理缓存。SQLite 使用十六进制 BLOB 转 TEXT，MySQL 使用十六进制字面量转 utf8mb4 文本，PostgreSQL 使用 UTF-8 十六进制解码；不依赖字符串反斜杠转义模式。对应语法见 [SQLite](https://www.sqlite.org/lang_expr.html)、[MySQL](https://dev.mysql.com/doc/refman/8.4/en/hexadecimal-literals.html)、[PostgreSQL](https://www.postgresql.org/docs/current/functions-binarystring.html)。

绑定值不接受对象、mapping、数组或 buffer；文本中的 NUL 会被拒绝，避免部分驱动截断 TEXT。绑定模板仅支持匿名 `?`，不支持编号参数、命名参数、dollar 引号、可执行注释或嵌套块注释。引号内存在反斜杠的模板会报错，因为其含义受 SQL 模式影响；应将该文本改为参数传入。

省略第二个参数的 `sql(sqlText)` 保留原始 SQL 用法，不扫描问号，也不替调用方处理嵌入的值。原始 SQL 模板始终由程序控制；玩家输入应放在参数数组中。

## 字段、表达式与兼容

占位参数只能表示值，不能表示表名、列名、排序方向或 SQL 片段。构造器会验证结构参数：

- 表名和字段使用英文字母或下划线开头的名称，支持数字后缀和 `table.column`；由组件按后端引用。
- 查询字段支持 `*`、`table.*`、`AS` 别名，以及 `COUNT` / `SUM` / `MIN` / `MAX` / `AVG` 聚合；COUNT 支持 `*`、`1`，聚合支持 DISTINCT 字段。
- 比较运算符限定在实现的白名单中，HAVING 连接词只接受 AND/OR。动态字段仍应由宿主按业务权限选择。
- 需要算术表达式、JOIN、子查询、特殊引用名称或后端专有语法时，使用受控的 `sql(sqlText, params)` 模板，不再向字段名位置塞入任意 SQL。

数字与文本按传入类型编码；希望保持文本比较语义时传字符串。原来依赖任意字段表达式或隐式数值转换的调用方应按上述约定调整。

后端可用性与 SQL 方言仍由驱动和数据库决定，例如 UPDATE/DELETE 的 LIMIT 支持并不一致。本机真实 SQLite 已验证参数往返、分组、增删改查和权限拒绝；MySQL/PostgreSQL 的编码按相应语法实现，尚未执行真实服务端集成测试，不能据此宣称所有后端均已验收。
