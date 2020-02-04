### CORE_ROOM

房间环境对象继承，实现的核心方法如下：

```c
void setup();
string coordinate();
varargs void setArea(int area, int x, int y, int z);
void removeExit(string dir);
void removeRandomExit();
void addExit(string dir, string dest);
int valid_leave(object me, string dir);
mapping query_doors();
string look_door(string dir);
varargs void set_door(string dir, mixed data, string other_side_dir, int status);
varargs int open_door(string dir, int from_other_side);
varargs int close_door(string dir, int from_other_side);
mixed query_door(string dir, string prop);
```
