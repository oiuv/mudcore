## 更新日志

### 2021年4月4日

* 增加CORE_CONFIG_D，方便游戏自定义运行配置，配置文件位置`/data/config`
* 为了数据安全调整数据库密码配置也保存在自定义配置文件中

### 2021年3月27日

* 调整CORE_LOGIN_D中 init_new_player 方法为`private void init_new_player(object user, object ob)`

### 2021年3月10日

* 新增CORE_NAME_D，用户注册校验是否重名

### 2020年11月22日

* MOVE 模块优化，增加可控制是否自动 look
* DBASE 模块增加默认对象设置
* ROOM 模块优化，增加函数别名
* 补充框架文档

### 2020年11月19日

* 更新指令系统，增加指令缓存和文件形式的别名功能(在指令目录新建 `别名.alias` 文件，内容为别名对应指令然后 rehash 即可生效)
* 更新系统自带指令使用说明
