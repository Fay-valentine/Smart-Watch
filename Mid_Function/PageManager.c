#include "PageManager.h"
#include "Pages.h"
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
/**
 * @brief 加载首页
 */
void Page_Init(void)
{
    //主题初始化
    lv_disp_t * dispp = lv_disp_get_default();
    lv_theme_t * theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED),
                                               true, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);

    //栈页面初始化
    page_stack_init(&page_stack);
    page_stack_push(&page_stack,&HomePage);
    HomePage.init();
    lv_disp_load_scr(*HomePage.page_obj);
}

/**
 * @brief 获取当前页面（栈顶页面）的指针。
 *
 * @param NULL
 * @return 返回当前页面的指针，如果栈为空则返回NULL。
 */
Page_t* Page_Get_NowPage(void)
{
    return page_stack_top(&page_stack);
}

/**
 * @brief 加载新界面
 * @param newPage 新界面
 */
void Page_Load(Page_t* newPage)
{
    //栈满，则不执行
    if (page_stack_is_full(&page_stack))
    {
        return;
    }

    //栈非空，则deinit当前界面，再接着加载界面
    if (!page_stack_is_empty(&page_stack))
    {
        page_stack.pages[page_stack.top]->deinit();
    }

    //加载新界面
    page_stack_push(&page_stack,newPage);//将新页面入栈
    newPage->init();//初始化
    //覆盖滑动效果，并删除旧屏幕
    lv_scr_load_anim(*newPage->page_obj,LV_SCR_LOAD_ANIM_OVER_RIGHT,150,0,true);
}

/**
 * @brief 回退页面，回退到主页面不执行
 */
void Page_Back(void)
{
    //栈空，不执行
    if (page_stack_is_empty(&page_stack))
    {
        return;
    }

    //如果在首页，则也不执行
    if (Page_Get_NowPage()==&HomePage)
    {
        return;
    }

    page_stack.pages[page_stack.top]->deinit();//先deinit当前页面（进行一些退出操作，不删除页面）

    page_stack_pop(&page_stack);//再弹出当前页面

    //最后切换到上一个页面
    page_stack.pages[page_stack.top]->init();
    lv_scr_load_anim(*page_stack.pages[page_stack.top]->page_obj,LV_SCR_LOAD_ANIM_OVER_RIGHT,150,0,true);
}

void Page_Back_to_Bottom(void)
{
    //栈空，不执行
    if (page_stack_is_empty(&page_stack))
    {
        return;
    }

    //如果在首页，则也不执行
    if (Page_Get_NowPage()==&HomePage)
    {
        return;
    }

    //弹出主页面以外的页面
    while (page_stack.top>0)
    {
        page_stack_pop(&page_stack);
    }
    page_stack.pages[page_stack.top]->init();//初始化主页
    lv_scr_load_anim(*page_stack.pages[page_stack.top]->page_obj,LV_SCR_LOAD_ANIM_NONE,0,0,true);
}





