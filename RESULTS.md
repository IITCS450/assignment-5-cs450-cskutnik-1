Context Switching Approach

Context switching is handled in uswtch.S (provided by the instructor). When a thread yields, the four callee-saved registers are pushed onto the current stack and esp is saved into the ougoing thread's context pointer. The incoming thread's saved esp is then loaded and the same registers are popped. A final jump to the return address on the new stack marks the thread ZOMBIE and yields. The scheduler is a cooperatve round robin as per instruction and threads must called thread_yield() voluntarily. 

Limitations

Some of the limitations are that there is a maximum of 8 threads supported. Additionally, each thread has a fixed 4096 byte stack with no overflow detection. Mutex also has no re entry, as in a thread cannot relock a mutex it already holds.
