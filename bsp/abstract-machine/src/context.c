#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <rtthread.h>
#ifdef __riscv
# include "arch/riscv.h"
#endif

volatile rt_ubase_t g_from = 0;
volatile rt_ubase_t g_to = 0;


static Context* ev_handler(Event e, Context *c) {
  switch (e.event) {
    case EVENT_YIELD: {
        if (0 != g_from) {
          *((Context **)g_from) = c;
        }

        assert(0 != g_to);
        c = *((Context **)g_to);
        assert(NULL != c);
      }
      break;
    case EVENT_IRQ_TIMER:
      /* 什么都不用做, 直接返回 */
      break;

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
  g_from = 0;
  g_to = to;

  yield();
} /* rt_hw_context_switch_to */

/**
 * @brief 上下文的切换并保存
 * @param from: 当前的上下文
 * @param to:   切换到的上下文
 */
void rt_hw_context_switch(rt_ubase_t from, rt_ubase_t to) {
  g_from = from;
  g_to = to;

  yield();
} /* rt_hw_context_switch */

void rt_hw_context_switch_interrupt(void *context, rt_ubase_t from, rt_ubase_t to, struct rt_thread *to_thread) {
  assert(0);
}


typedef void (*entry_func_t)(void *);
typedef void (*exit_func_t)();
struct entry_arg {
  entry_func_t entry;
  exit_func_t exit;
  void *arg;
};

// 包装函数处理 exit
void entry_warp(void *param) {
  struct entry_arg *arg = (struct entry_arg *)param;

  arg->entry(arg->arg);
  arg->exit();

  panic("Should not reach here!\n");
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
  rt_uint8_t *stack_end = NULL;
  Context *ctx  = NULL;
  struct entry_arg *arg = NULL;

  // NOTE: 使用栈传递结构体
  arg = (struct entry_arg *)rt_malloc_align(sizeof(struct entry_arg), 4);
  arg->entry = tentry;
  arg->arg = parameter;
  arg->exit = texit;

  // 对齐 stack_addr 到 sizeof(uintptr_t)
  stack_end = stack_addr + sizeof(rt_ubase_t);
  stack_end = (rt_uint8_t *)RT_ALIGN_DOWN((uintptr_t)stack_end, (sizeof(uintptr_t)/4U));

  ctx = kcontext(RANGE(0, stack_end), entry_warp, (void *)arg);

  return (rt_uint8_t *)ctx;
} /* rt_hw_stack_init */
