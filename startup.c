/* ***** BEGIN LICENSE BLOCK *****
 * Version: BSD License
 * 
 * Copyright (c) 2006, Diego Casorran <dcasorran@gmail.com>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 ** Redistributions of source code must retain the above copyright notice,
 *  this list of conditions and the following disclaimer.
 *  
 ** Redistributions in binary form must reproduce the above copyright notice,
 *  this list of conditions and the following disclaimer in the documentation
 *  and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * ***** END LICENSE BLOCK ***** */

/**
 * $Id: startup.c,v 0.1 2006/07/06 23:39:43 diegocr Exp $
 */

#include <proto/exec.h>
#include <dos/dosextens.h>

/***************************************************************************/

#define STACK_SIZE	(32*1024)

GLOBAL int __swap_stack_and_call(struct StackSwapStruct * stk,APTR function);
GLOBAL int __main ( VOID );

/***************************************************************************/

int __startup_function_which_calls_main_without_arguments ( void )
{
	unsigned int stack_size, __stack_size = STACK_SIZE ;
	struct WBStartup * startup_message;
	struct Process * this_process;
	struct StackSwapStruct *stk;
	APTR old_window_pointer;
	struct Task *this_task;
	APTR new_stack;
	int ret = 1;
	
	SysBase = *(struct ExecBase **) 4L;
	
	this_process = (struct Process *)(this_task = FindTask(NULL));
	
	if(!this_process->pr_CLI)
	{
		struct MsgPort * mp = &this_process->pr_MsgPort;
		
		WaitPort(mp);
		
		startup_message = (struct WBStartup *)GetMsg(mp);
	}
	else	startup_message = NULL;
	
	old_window_pointer = this_process->pr_WindowPtr;
	
//	_WBenchMsg = startup_message;
	
	__stack_size += ((ULONG)this_task->tc_SPUpper-(ULONG)this_task->tc_SPLower);
	
	/* Make the stack size a multiple of 32 bytes. */
	stack_size = 32 + ((__stack_size + 31UL) & ~31UL);
	
	/* Allocate the stack swapping data structure
	   and the stack space separately. */
	stk = AllocVec( sizeof(*stk), MEMF_PUBLIC|MEMF_ANY );
	if(stk != NULL)
	{
		new_stack = AllocMem(stack_size,MEMF_PUBLIC|MEMF_ANY);
		if(new_stack != NULL)
		{
			/* Fill in the lower and upper bounds, then
			   take care of the stack pointer itself. */
			
			stk->stk_Lower	= new_stack;
			stk->stk_Upper	= (ULONG)(new_stack)+stack_size;
			stk->stk_Pointer= (APTR)(stk->stk_Upper - 32);
			
			ret = __swap_stack_and_call(stk,(APTR)__main);
			
			FreeMem(new_stack, stack_size);
		}
		
		FreeVec(stk);
	}
	
	this_process->pr_WindowPtr = old_window_pointer;
	
	if(startup_message != NULL)
	{
		Forbid();
		
		ReplyMsg((struct Message *)startup_message);
	}
	
	return ret;
}

