/*****************************************************************************
Copyright: 2019, Mud.Ren
File name: mysql.h
Description: mysql 数据库配置文件
Author: xuefeng
Version: v1.0
Date: 2019-04-01
*****************************************************************************/
#ifndef MYSQL_H
#define MYSQL_H

#define DB_CONFIG   read_lines(DATA_DIR "DB_CONFIG")
#define DB_HOST     DB_CONFIG[0]
#define DB_NAME     DB_CONFIG[1]
#define DB_USER     DB_CONFIG[2]
#define DB_PASSWD   DB_CONFIG[3]

#endif
