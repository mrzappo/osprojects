/**
 * @file create.c
 * @provides create, newpid, userret
 *
 * COSC 3250 / COEN 4820 Assignment 4
 */
/* Embedded XINU, Copyright (C) 2008.  All rights reserved. */

//TA-BOT:MAILTO thomas.panozzo@marquette.edu shayne.burns@marquette.edu

#include <arm.h>
#include <xinu.h>

/* Assembly routine for atomic operations */
extern int _atomic_increment_post(int *);
extern int _atomic_increment_limit(int *, int);

static pid_typ newpid(void);
void userret(void);
void *getstk(ulong);

/**
 * Create a new process to start running a function.
 * @param funcaddr address of function that will begin in new process
 * @param ssize    stack size in bytes
 * @param name     name of the process, used for debugging
 * @param nargs    number of arguments that follow
 * @return the new process id
 */
syscall create(void *funcaddr, ulong ssize, char *name, ulong nargs, ...)
{
	ulong *saddr;               /* stack address                */
	ulong pid;                  /* stores new process id        */
	pcb *ppcb;                  /* pointer to proc control blk  */
	ulong i;
	va_list ap;                 /* points to list of var args   */
	ulong pads = 0;             /* padding entries in record.   */
	void INITRET(void);

	if (ssize < MINSTK)
		ssize = MINSTK;
	ssize = (ulong)(ssize + 3) & 0xFFFFFFFC;
	/* round up to even boundary    */
	saddr = (ulong *)getstk(ssize);     /* allocate new stack and pid   */
	pid = newpid();
	/* a little error checking      */
	if ((((ulong *)SYSERR) == saddr) || (SYSERR == pid))
	{
		return SYSERR;
	}

	_atomic_increment_post(&numproc);

	ppcb = &proctab[pid];
	/* setup PCB entry for new proc */
	ppcb->state = PRSUSP;

	// TODO: Setup PCB entry for new process.

	ppcb->stklen = ssize;
	ppcb->stkbase = (void *)saddr;
	ppcb->core_affinity = -1;
	strncpy(ppcb->name, name, PNMLEN);
	
	//Read initialize.c for examples
	/* Initialize stack with accounting block. */ //Used for debugging
	*saddr = STACKMAGIC;             //Top of the stack
	*--saddr = pid;                  //Pushes the PID to the stack
	*--saddr = ppcb->stklen;         //Pushes the stklen to the stack
	*--saddr = (ulong)ppcb->stkbase; //Pushes the stack base to the stack

	/* Handle variable number of arguments passed to starting function   */
	if (nargs)
	{
		pads = ((nargs - 1) / 4) * 4;
	}
	/* If more than 4 args, pad record size to multiple of native memory */
	/*  transfer size.  Reserve space for extra args                     */
	for (i = 0; i < pads; i++)
	{
		*--saddr = 0;
	}
	//Note: use contents of arm.h to get register numbers
	// TODO: Initialize process context.
	//
	//
	// TODO:  Place arguments into activation record.
	//        See K&R 7.3 for example using va_start, va_arg and
	//        va_end macros for variable argument functions.

	va_start(ap,nargs);
    int x;
    for (x = 0; x < 4; x++)
    {
		if (x < nargs)
        	ppcb->regs[x] = va_arg(ap,int);
    }

	if (nargs > 4)
	{
		for (x = 0; x < nargs-4; x++) //Changed from x =4 and x < NARGS
		{
			*(saddr+x) = va_arg(ap,int);
		}
	}
	va_end(ap);
	ppcb->regs[PREG_SP] = (int)saddr;
	ppcb->regs[PREG_LR] = (int)userret; //(int)userret
	ppcb->regs[PREG_PC] = (int)funcaddr;
	return pid;
}

/**
 * Obtain a new (free) process id.
 * @return a free process id, SYSERR if all ids are used
 */
static pid_typ newpid(void)
{
	pid_typ pid;                /* process id to return     */
	static pid_typ nextpid = 0;

	for (pid = 0; pid < NPROC; pid++)
	{                           /* check all NPROC slots    */
		//        nextpid = (nextpid + 1) % NPROC;
		_atomic_increment_limit(&nextpid, NPROC);
		if (PRFREE == proctab[nextpid].state)
		{
			return nextpid;
		}
	}
	return SYSERR;
}

/**
 * Entered when a process exits by return.
 */
void userret(void)
{
	uint cpuid = getcpuid();
	kill(currpid[cpuid]);
}
