/*
+----------------------------------------------------------------------+
  | Copyright (c) The PHP Group                                          |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | https://www.php.net/license/3_01.txt                                 |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Edmond                                                       |
  +----------------------------------------------------------------------+
*/
#include "scheduler.h"

static void execute_microtasks(void)
{

}

static void handle_callbacks(void)
{

}

static void resume_next_fiber(void)
{

}

/**
 * Handlers for the scheduler.
 * This functions pointer will be set to the actual functions.
 */
static void (*h_execute_microtasks)(void)	= execute_microtasks;
static void (*h_handle_callbacks)(void)		= handle_callbacks;
static void (*h_resume_next_fiber)(void)	= resume_next_fiber;

zend_result async_scheduler_fiber_resume()
{
    return SUCCESS;
}

zend_result async_scheduler_yield()
{
	h_execute_microtasks();
	h_handle_callbacks();
	h_execute_microtasks();
	h_resume_next_fiber();
	return SUCCESS;
}
