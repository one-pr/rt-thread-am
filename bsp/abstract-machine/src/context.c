#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <rtthread.h>


static Context* ev_handler(Event e, Context *c) {
  switch (e.event) {
    default: printf("Unhandled event ID = %d\n", e.event); assert(0);
  }
  return c;
} /* ev_handler */

void __am_cte_init() {
  cte_init(ev_handler);
}

/**
 * @brief 上下文的切换
 * @param to: 切换到的上下文
 */
void rt_hw_context_switch_to(rt_ubase_t to) {
  assert(0);
} /* rt_hw_context_switch_to */

/**
 * @brief 上下文的切换并保存
 * @param from: 当前的上下文
 * @param to:   切换到的上下文
 */
void rt_hw_context_switch(rt_ubase_t from, rt_ubase_t to) {
  assert(0);
} /* rt_hw_context_switch */

void rt_hw_context_switch_interrupt(void *context, rt_ubase_t from, rt_ubase_t to, struct rt_thread *to_thread) {
  assert(0);
}


typedef void (*entry_f)(void *);
typedef void (*exit_f)();
// [全局变量] entry_warp 的参数
entry_f g_tentry = NULL;
exit_f g_texit = NULL;
void *g_parameter = NULL;

static void entry_warp(void *arg) {
  assert(NULL != g_tentry);
  g_tentry(g_parameter);

  assert(NULL != g_texit);
  g_texit();
} /* entry_warp */

/**
 * @brief 上下文的创建
 * @param tentry:     上下文 入口
 * @param parameter:  上下文 参数
 * @param stack_addr: 栈底
 * @param texit:      若上下文对应的内核线程从 tentry 返回, 则调用 texit
 * @return rt_uint8_t *:  创建完成的上下文
 */
rt_uint8_t *rt_hw_stack_init(void *tentry, void *parameter, rt_uint8_t *stack_addr, void *texit) {
  g_tentry = tentry;
  g_parameter = parameter;
  g_texit = texit;

  // 对齐 stack_addr 到 sizeof(uintptr_t)
	rt_uint8_t *stack_addr_alig = (rt_uint8_t *)RT_ALIGN_DOWN((uintptr_t)stack_addr, sizeof(uintptr_t));

  Context *ctx = kcontext(RANGE(0, stack_addr_alig), entry_warp, parameter);

  return (rt_uint8_t *)ctx;
} /* rt_hw_stack_init */
