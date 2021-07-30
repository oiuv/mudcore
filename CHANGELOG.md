## 更新日志

### 2021年7月

* 增加CORE_INTERMUD_D，方便使用游戏列表互联

### 2021年6月

* 优化CORE_ROOM，增加光线相关方法接口
* 增加HTTP功能示例模块（智能机器人和QQ聊天机器人）

### 2021年5月

* 增加部分sefun和默认指令
* 增加CORE_DB接口，方便数据库操作

### 2021年4月

* 增加CORE_CONFIG_D，方便游戏自定义运行配置，配置文件位置`/data/config`
* 为了数据安全调整数据库密码配置也保存在自定义配置文件中

### 2021年3月

* 调整CORE_LOGIN_D中 init_new_player 方法为`private void init_new_player(object user, object ob)`
* 新增CORE_NAME_D，用户注册校验是否重名

### 2021年1月

* MOVE 模块优化，增加可控制是否自动 look
* DBASE 模块增加默认对象设置
* ROOM 模块优化，增加函数别名
* 增加完善部分守护进程
* 更新指令系统，增加指令缓存和文件形式的别名功能(在指令目录新建 `别名.alias` 文件，内容为别名对应指令然后 rehash 即可生效)
* 更新系统自带指令使用说明，补充框架文档

### 2020年

* 发布MUDCORE测试版
