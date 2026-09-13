#include "PageManager.h"
#include <stdbool.h>
PageStack_t page_stack;

//init
static void page_stack_init(PageStack_t* stack)
{
    stack->top=PAGE_STACK_EMPTY;
}

//empty
static bool page_stack_is_empty(const PageStack_t* stack)
{
    return stack->top<0;
}

//full
static bool page_stack_is_full(const PageStack_t* stack)
{
    return stack->top>=MAX_DEPTH-1;
}

//push

/**
 * @return true：入栈成功  false：栈满
 */
static bool page_stack_push(PageStack_t* stack,Page_t* page)
{
    //is full
    if (page_stack_is_full(stack))
    {
        return false;
    }
    stack->pages[++stack->top]=page;
    return true;
}

//top
/**
 * @brief 获取当前页面
 * @return NULL:栈空
 */
static Page_t* page_stack_top(const PageStack_t* stack)
{
    //is empty
    if (page_stack_is_empty(stack))
    {
        return NULL;
    }
    return stack->pages[stack->top];
}

//pop
/**
 * @brief 弹出当前页面并deinit
 * @return true：成功弹出  false：栈空
 */
static bool page_stack_pop(PageStack_t* stack)
{
    //is empty
    if (page_stack_is_empty(stack))
    {
        return false;
    }
    stack->pages[stack->top--]->deinit();
    return true;
}

/*-----------------------------------------------------------------------------------*/
Page_t* Page_Get_NowPage(void)
{
    return page_stack_top(&page_stack);
}



