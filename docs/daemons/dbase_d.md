### CORE_DBASE_D

`DBASE_D` 保存对象的公共数据，使用 LPC 对象存档，不是 SQL 数据库。默认存档为 `DATA_DIR + "dbase_d.o"`（默认 `/data/dbase_d.o`）。SQL 接口见 [DB](../inherit/DB.md)。

### 核心方法

```c
mixed query_data();
int set_data(mixed data);
mixed query_object_data(mixed ob);
int set_object_data(mixed ob, mixed data);
mapping query_save_dbase();
string *query_saved_object();
int clear_object(mixed ob);
int cleanup_all_save_object(int raw);
varargs int remove(string euid);
void mud_shutdown();
```

- `query_data()` / `set_data()` 操作调用对象的记录。`query_object_data()` / `set_object_data()` 接受对象或已加载对象的路径。
- 记录以 `base_name(ob)` 为键，同一源文件的克隆对象共用一条记录，不适合直接区分多个玩家或物品实例的存档。
- 写入非零数据时会调用守护进程的 `save()` 落盘；传入 `0` 则删除内存记录。如需保存数值 `0`，可包装为映射，例如 `([ "count": 0 ])`。
- `query_save_dbase()` 返回记录映射，`query_saved_object()` 返回已记录的对象路径。
- `clear_object()` 会先销毁对应的已加载对象，再删除记录；删除操作本身不立即落盘，需要随后调用 `DBASE_D->save()`。
- `cleanup_all_save_object(raw)` 检查源文件已不存在的记录：`raw = 0` 只记录待清理项，非零时删除，最后保存。
- `remove()`、`mud_shutdown()` 用于对象销毁和停机流程，普通读写使用前述接口。

### 使用 CORE_DBSAVE

需要 `save()` / `restore()` 便利接口的对象可继承 `CORE_DBSAVE`，并实现两个钩子：

```c
mixed save_dbase_data();
int receive_dbase_data(mixed data);
```

`save_dbase_data()` 返回要保存的数据；`receive_dbase_data()` 接收恢复的数据，首次使用时应能处理 `0`。由宿主在初始化时调用 `restore()`，在数据变更后调用 `save()`。这两个便利方法当前固定返回 `1`，不能把返回值当作底层存档成功的确认。

直接调用 `DBASE_D` 不要求继承此组件。源码见 [dbsave.c](../../inherit/dbsave.c) 和 [dbase_d.c](../../system/daemons/dbase_d.c)。
