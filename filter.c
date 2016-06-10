#include <stdio.h>
#include <stdlib.h>
#include <mysql/mysql.h>
#include <string.h>
#include "filter.h"

int filter_word(char *oriStr){

    /* 存储数据库取出的屏蔽关键词 */
    char *testURL[100];
    char *testWord[100];

    MYSQL mysql;//mysql连接
    MYSQL_RES *res;//这个结构代表返回行的一个查询结果
    MYSQL_ROW row;//一个数据的类型安全（type-safe）的表示
    char *query;//查询语句
    int t,r;
    mysql_init(&mysql);
    if(!mysql_real_connect(&mysql,"localhost","aaron","LS1234","filter",0,NULL,0)){
        printf("Error connecting to database:%s",mysql_error(&mysql));
    }else{
        printf("Connected...\n");
    }
    query="select * from fword";
    t=mysql_real_query(&mysql,query,(unsigned int)strlen(query));//执行指定为计数字符串的SQL查询
    if(t){
        printf("执行显示时出现异常",mysql_error(&mysql));
    }
    res = mysql_store_result(&mysql);
        if (res) {
            printf("Retrieved %lu rows\n", (unsigned long)mysql_num_rows(res));
            while ((row = mysql_fetch_row(res))) {
                printf("Fetched data...\n") ;
            }
            if (mysql_errno(&mysql)) {
                fprintf(stderr, "Retrive error: %s\n", mysql_error(&mysql));
            }
            mysql_free_result(res);
        }

    mysql_close(&mysql);
    return 0;
}
