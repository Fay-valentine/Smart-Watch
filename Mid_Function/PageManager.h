#ifndef INC_10_WATCH_PAGEMANAGER_H
#define INC_10_WATCH_PAGEMANAGER_H

#include "lvgl.h"

#define MAX_DEPTH   6
#define PAGE_STACK_EMPTY    ((int8_t)-1)

//为什么page_obj用二级指针，因为一开始lv_obj_t* pageObj还没有指向具体的页面对象，为NULL，后面分配了，指针变量就会变，后续就可能访问不到了
//使用二级指针，就能保证一直可以访问，因为指针变量的地址不变
typedef struct
{
    void (*init)(void);
    void (*deinit)(void);
    lv_obj_t** page_obj;
}Page_t;

//top指向栈顶元素，top=-1表示空栈,top=0表示第一个，top=1表示第二个
typedef struct
{
    Page_t* pages[MAX_DEPTH];
    int8_t top;
}PageStack_t;

extern PageStack_t page_stack;

void Page_Init(void);
Page_t* Page_Get_NowPage(void);
void Page_Load(Page_t* newPage);
void Page_Back(void);
void Page_Back_to_Bottom(void);

#endif //INC_10_WATCH_PAGEMANAGER_H