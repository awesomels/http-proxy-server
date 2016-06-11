#include <stdio.h>
#include <stdlib.h>
#include <mysql/mysql.h>
#include <string.h>
#include "filter.h"

int filter_word(char *oriStr){

    /* 存储数据库取出的屏蔽关键词 */
    char *testURL[100];
    char *testWord[100];
    int wpnum=0;

    MYSQL mysql;//mysql连接
    MYSQL_RES *res;//这个结构代表返回行的一个查询结果
    MYSQL_ROW row;//一个数据的类型安全（type-safe）的表示
    char *query;//查询语句
    int t,r,i=0;
    mysql_init(&mysql);
    if(!mysql_real_connect(&mysql,"localhost","aaron","LS1234","filter",0,NULL,0)){
        printf("Error connecting to database:%s",mysql_error(&mysql));
    }else{
        printf("Connected...\n");
    }
    /* 设置数据库默认字符集 */
    if ( mysql_set_character_set( & mysql, "utf8" ) ) {
        fprintf ( stderr , "错误, %s/n" , mysql_error( & mysql) ) ;
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
                printf("Fetched data...%s\n",row[1]) ;
                testWord[i]=calloc(strlen(row[1])+1, sizeof (char));
                strcpy(testWord[i], row[1]);
                i++;
                wpnum++;
            }
            if (mysql_errno(&mysql)) {
                fprintf(stderr, "Retrive error: %s\n", mysql_error(&mysql));
            }
            mysql_free_result(res);
        }

    for(i=0 ; i<wpnum; i++){
        //printf("%s\n",testWord[i]);
        if( strstr(oriStr, testWord[i]) )
            printf("识别到关键字：%s\n",testWord[i]);
        free(testWord[i]);
        testWord[i]=NULL;
    }
    mysql_close(&mysql);
    return 0;
}
