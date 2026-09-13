#ifndef INC_10_WATCH_PAGEMANAGER_H
#define INC_10_WATCH_PAGEMANAGER_H

#include "ui.h"

#define MAX_DEPTH   6
#define PAGE_STACK_EMPTY    ((int8_t)-1)
typedef struct
{
    void (*init)(void);
    void (*deinit)(void);
    lv_obj_t** page_obj;
}Page_t;

//top指向栈顶元素，top=-1表示空栈
typedef struct
{
    Page_t* pages[MAX_DEPTH];
    int8_t top;
}PageStack_t;

extern PageStack_t page_stack;

void page_init(void);
void page_push(Page_t* page);
void page_pop(void);
Page_t* page_top(void);

#endif //INC_10_WATCH_PAGEMANAGER_H