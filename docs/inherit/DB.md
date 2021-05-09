虽然MUD游戏多数只使用到`save_object()`存档数据，但总有一些情况我们需要使用到mysql，存一些必要的数据到数据库中，LPC中提供了数据库操作的核心函数：`db_connect`、`db_exec`、`db_fetch`、`db_close`、`db_status`，可以满足我们所有数据库操作的需求，比如，像在游戏中读取bbs.mud.ren论坛的帖子，可以用以下代码实现：

```c
// 读取论坛内容
varargs string content(int id, string host, string db, string user)
{
#ifdef __PACKAGE_DB__
    if (id)
    {
        mixed handle;

        if (stringp(host) && stringp(db) && stringp(user))
        {
            handle = db_connect(host, db, user);
        }
        else
        {
            handle = db_connect("127.0.0.1", "mudren", "root");
        }

        if (stringp(handle)) /* error */
            return handle;
        else
        {
            mixed rows;

            db_exec(handle, "set names utf8"); // 防止乱码
            rows = db_exec(handle, "SELECT markdown FROM contents WHERE contentable_type LIKE '%Thread' AND contentable_id='" + id + "'");
            if (!rows)
            {
                db_close(handle);
                return "";
            }
            else if (stringp(rows)) /* error */
            {
                db_close(handle);
                return rows;
            }
            else
            {
                mixed *res;
                string content = "";

                for (int i = 1; i <= rows; i++)
                {
                    res = db_fetch(handle, i);
                    // write(res[0] + "\n");
                    content += res[0];
                }
                db_close(handle);
                return content;
            }
        }
    }
    else
    {
        return "请指定主题ID~";
    }
#else
    return "注意：数据库功能被禁用，无法读取论坛内容。";
#endif
}
```

代码看起来有一点多，想不想更优雅的操作数据库？看看以下实现相同功能的代码：

```c
inherit CORE_DB;
// 读取论坛内容
varargs string content(int id, string host, string db, string user)
{
#ifdef __PACKAGE_DB__
    if (id)
    {
        string content;

        if (stringp(host) && stringp(db) && stringp(user))
        {
            DB::setConnection(([
                "host":host,
                "database":db,
                "user":user,
            ]));
        }
        content = DB::table("contents")->where(({({"contentable_type", "LIKE", "%Thread"}), ({"contentable_id", id})}))->value("markdown");

        return content;
    }
    else
    {
        return "请指定主题ID~";
    }
#else
    return "注意：数据库功能被禁用，无法读取论坛内容。";
#endif
}
```

泥芯框架v1.3版开始提供更优雅的数据库操作方式，使用方式介绍如下。

----

## 基础配置

当你需要使用数据库增删改查功能时，可直接使用`CORE_DB`(`/mudcore/inherit/DB`)：

首先，在你的`/data/`目录中新增配置文件`config`（如果你用git做版本管理，这个文件一定要加到.gitignore中），文件内容增加以下配置：

```
DB_HOST : 127.0.0.1
DB_DATABASE : mud
DB_USERNAME : root
DB_PASSWORD : root
```

注意，`config`文件中只是指定了数据库默认配置，如果不使用文件配置，也可以在初始化时配置。

```c
void test()
{
    /**
     * @brief 初始化数据库连接
     *
     */
    object DB = new(CORE_DB, host, db, user);

    // ...
}
```

或者使用 `setConnection()` 方法配置数据库：

```c
void test()
{
    object DB = new(CORE_DB);
    mapping db = ([
        "host":"127.0.0.1",
        "database":"mud",
        "user":"root",
    ]);
    DB->setConnection(db);
}
```

上以配置使用以的是系统默认数据库类型`__DEFAULT_DB__`，如果要指定数据库类型可以使用以下方式：

```c
void test()
{
    // 初始化时指定type
    object DB = new(CORE_DB, host, db, user, type);
    // 通过配置指定type(使用SQLITE3)
    mapping db = ([
        "host":"",
        "database":"/data/mudlite.db",
        "user":"",
        "type":__USE_SQLITE3__
    ]);
    DB->setConnection(db);
}
```

数据库类型可以使用宏定义：`__USE_MYSQL__`、`__USE_SQLITE3__`、`__USE_POSTGRE__`或`__DEFAULT_DB__`。

----

## 查询

### 从数据表中查询所有行

使用`DB`对象的`table`方法指定要查询的数据表并返回一个查询对象，使用`get`方法获取所有结果，返回值为结果二维数组，示例代码：

```c
inherit CORE_DB;

void test()
{
    mixed res;

    // 获取 users 表的所有结果
    res = DB->table("users")->get();
    printf("%O\n", res);
}
```

### 从数据表中获取单行或单列

使用`first`方法可以获取查询结果的第一行，返回值为一维数组结果，如果指定数组做为参数，只查询指定的列，另外你可以使用`where`方法限制查询条件，核心代码：

```c
// 查询用户表中的第一条数据
res = DB->table("users")->first();
// 查询用户表中的第一条数据的name和email
res = DB->table("users")->first("name", "email");
// 查询用户表中用户名为 ivy 的数据
res = DB->table("users")->where("name", "ivy")->first();
```

如果不需要整行数据，则可以使用`value`方法从记录中获取单个值。该方法将直接返回该字段的值：

```c
// 获得用户 ivy 的 email
email = DB->table("users")->where("name", "ivy")->value("email");
// 获取随机用户的 email
email = DB->table("users")->inRandomOrder()->value("email");
```

如果是通过 id 字段值获取一行数据，可以使用 find 方法：

```c
// 获取 ID 为 1 的用户数据
res = DB->table("users")->find(1);
```

### 获取一列的值

如果你想获取单列数据的集合，则可以使用 pluck 方法。在下面的例子中，我们将获取角色表中所有邮箱：
```c
// 返回所有用户的邮箱数组
res = DB->table("users")->pluck("email");
```

### 聚合

查询构造器还提供了各种聚合方法，比如 `count`，`max`，`min`，`avg`，还有 `sum`。你可以在构造查询后调用任何方法：

```c
// 获取用户总数
users = DB->table("users")->count();
// 获取等级最高的用户
level = DB->table("users")->max("level");
```

同样，你可以通过条件查询限制聚合查询：

```c
// 返回ID为9的用户发贴的访问量之和
res = DB->table("topics")->where("user_id", 9)->sum("view_count");
// 判断用户mudren是否存在
count = DB->table("users")->where("name", "mudren")->count();
```

### Where Clauses

#### where 语句

在构造 `where` 查询实例中，你可以使用 `where` 方法。调用 `where` 最基本的方式是需要传递三个参数：第一个参数是列名，第二个参数是任意一个数据库系统支持的运算符，第三个是该列要比较的值。如：

```c
user = DB->table("users")->where("name", "=", "mudren")->get();
```

为了方便，如果你只是简单比较列值和给定数值是否相等，可以将数值直接作为 where 方法的第二个参数：

```c
user = DB->table("users")->where("name", "mudren")->get();
```

当然，你也可以使用其他的运算符来编写 `where` 子句：

```c
users = DB->table("users")
                ->where("id", ">=", 100)
                ->get();

users = DB->table("users")
                ->where("id", "<>", 100)
                ->get();

users = DB->table("users")
                ->where("email", "like", "%@mud.ren")
                ->get();
```

你还可以传递条件数组到 where 函数中实现 `AND` 查询：

```c
// 相当于 WHERE user_id>7 AND category_id=4
res = DB->table("topics")->where(({ ({"user_id", ">", 7}), ({"category_id", 4}) }))->get();
```

你也可以对 `where` 函数链式调用，以上查询可以使用以下方式：
```c
// 相当于 WHERE user_id>7 AND category_id=4
res = DB->table("topics")->where("user_id", ">", 7)->where("category_id", 4)->get();
```

#### orWhere 语句
你可以一起链式调用 `where` 约束，也可以在查询中添加 or 字句。 `orWhere` 方法和 `where` 方法接收的参数一样：

```c
// 相当于 WHERE user_id>7 OR category_id=4
res = DB->table("topics")->where("user_id", ">", 7)->orWhere("category_id", 4)->get();
```

请注意：`orWhere`方法之前必须至少有一次`where`调用，否则报错。

### 附加 Where 语句

除了基础的 `where` 和 `orWhere`，还有以下语句可用：

#### whereBetween / orWhereBetween / whereNotBetween / orWhereNotBetween

验证字段值是否在给定的两个值之间或之外：

```c
users = DB->table("users")
           ->whereBetween("votes", ({1, 100}))
           ->get();
```

#### whereIn / whereNotIn / orWhereIn / orWhereNotIn

验证字段值是否在给定的数组中：

```c
users = DB->table("users")
           ->whereIn("id", ({1, 3, 5, 7, 9}))
           ->get();
```

#### whereNull / orWhereNull / whereNotNull / orWhereNotNull

验证指定的字段是否是 `NULL`：

```c
users = DB->table("users")
           ->whereNull("updated_at")
           ->get();
```

### Distinct, Ordering, Grouping, Limit & Offset

#### distinct

`distinct` 方法会强制让查询返回的结果不重复：

```c
users = DB->table("users")
                ->distinct()
                ->get("name");
```

#### orderBy

`orderBy` 方法允许你通过给定字段对结果集进行排序。 `orderBy` 的第一个参数应该是你希望排序的字段，第二个参数控制排序的方向，可以是 `asc` 或 `desc`：

```c
users = DB->table("users")
                ->orderBy("name", "desc")
                ->get();
```

如果你需要使用多个字段进行排序，你可以多次引用 `orderBy`：

```c
users = DB->table("users")
                ->orderBy("name", "desc")
                ->orderBy("email", "asc")
                ->get();
```

#### inRandomOrder

`inRandomOrder` 方法被用来将结果进行随机排序。例如，你可以使用此方法随机找到一个用户：

```c
randomUser = DB->table("users")
                ->inRandomOrder()
                ->first();
```

#### limit / offset

要限制结果的返回数量，或跳过指定数量的结果，你可以使用 `limit` 和 `offset` 方法：

```c
users = DB->table("users")
                ->offset(10)
                ->limit(5)
                ->get();
```

----

## 插入

查询构造器还提供了 `insert` 方法用于插入记录到数据库中。 `insert` 方法接收映射形式的字段名和字段值进行插入操作：

```c
DB->table("users")->insert((["name":"test", "email":"test@mud.ren"]));
```

## 更新

查询构造器也可以通过 `update` 方法更新已有的记录。 `update` 方法和 `insert` 方法一样，接受包含要更新的字段及值的映射。你可以通过 `where` 子句对 `update` 查询进行约束：

```c
DB->table("users")->where("id", ">", 120)->limit(5)->update((["level":777]));
```

## 删除

查询构造器也可以使用 `delete` 方法从表中删除记录。 在使用 `delete` 前，可以添加 `where` 子句来约束 `delete` 语法：

```c
DB->table("migrations")->where("id",">", 55)->limit(5)->delete();
```

----

## 原生语句

除了使用封装好的方法外，查询构造器还提供了原生SQL语句的使用，通过使用 `sql` 方法执行，注意对 `SELECT` 语句可以使用 `get`、`first`、`pluck`、`value`方法获取查询结果，而 `INSERT`、`UPDATE`、`DELETE` 语句使用 `exec` 执行。

```c
// 查询
res = DB->sql("select * from users")->get();
res = DB->sql("select * from users")->pluck("name");
res = DB->sql("select * from users where name='mudren'")->value("email");
// 删除
res = DB->sql("delete from users where id>130 limit 5")->exec();
```

----

## 长连接

需要注意的是，`CORE_DB` 默认使用的是短连接，所有数据库操作完成后自动释放 db_handle ，如果是大量重复查询，最好改为长连接，并自己在必要时操作释放 db_handle 。

如需长连接，请使用 `setAutoClose(0)` 方法禁用自动释放，并在执行结束时调用 `close(1)` 方法释放 db_handle

```c
void test()
{
    // 初始化数据库连接
    object DB = new(CORE_DB, host, db, user);
    // 禁用自动关闭连接
    DB->setAutoClose(0);

    // ...数据库操作

    // 关闭数据库连接
    DB->close(1);
}
```

## 调试

在绑定查询的时候，您可以使用 `dump` 方法来输出最近一次查询SQL：

```c
printf(DB->dump());
```

调试输出结果类似以下：

    -*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*-
    db_host = 127.0.0.1
    db_db = mud
    db_user = root
    db_handle = 1
    db_error = 0
    db_table = topics
    db_table_column = ({ /* sizeof() == 2 */
    "user_id",
    "title"
    })
    db_sql = SELECT user_id,title FROM topics WHERE user_id > '7' AND category_id='4'
    -*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*--*-

----

如果你只是简单查询，可以通过继承`CORE_DB`的方式，而不需要用 `new()` 方法生成数据库连接对象，示例如下：

```c
inherit CORE_DB;

void test()
{
    mixed res;

    /**
     * @brief 配置数据库连接
     *
     */
     mapping db = ([
     	"host":"127.0.0.1",
        "database":"mud",
        "user":"root"
     ]);
    DB::setConnection(db);

    // 获取 users 表的所有结果
    res = DB::table("users")->get();
    printf("%O", res);
}
```
