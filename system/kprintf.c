/**
 * @file kprintf.c
 */

/* Embedded Xinu, Copyright (C) 2009, 2013.  All rights reserved. */


/* TA-BOT:MAILTO thomas.panozzo@marquette.edu 
 *
 *
 */

//Version 1.0: Written by Thomas Panozzo and Shayne Burns


#include <xinu.h>

#define UNGETMAX 10             /* Can un-get at most 10 characters. */

static unsigned char ungetArray[UNGETMAX];

/**
 * Synchronously read a character from a UART.  This blocks until a character is
 * available.  The interrupt handler is not used.
 *
 * @return
 *      The character read from the UART as an <code>unsigned char</code> cast
 *      to an <code>int</code>.
 */
syscall kgetc(void)
{
    volatile struct pl011_uart_csreg *regptr;

    /* Pointer to the UART control and status registers.  */
    regptr = (struct pl011_uart_csreg *)0x3F201000;

	while(!kcheckc())
	{
		
	}

	if(ungetArray[0] != '\0') //If the ungetArray is not empty, the character is in there
	{
		unsigned char temp = ungetArray[0];
		int i;	

		//Stack
		for ( i = 1; i < UNGETMAX; i++)
		{
			if (ungetArray[i] == '\0')
			{
				temp = ungetArray[i-1];
				ungetArray[i-1] = '\0';
				return temp;
			}
		}
		if (ungetArray[UNGETMAX-1] != '\0')
		{
			temp = ungetArray[UNGETMAX-1];
			ungetArray[UNGETMAX-1] = '\0';
		}
	return temp; 
	}


	
	return (int) (regptr->dr);
	
    return SYSERR;
}

/**
 * kcheckc - check to see if a character is available.
 * @return true if a character is available, false otherwise.
 */
syscall kcheckc(void)
{
    volatile struct pl011_uart_csreg *regptr;
    regptr = (struct pl011_uart_csreg *)0x3F201000;

	if (ungetArray[0] != '\0' || !(regptr->fr & PL011_FR_RXFE) ) //Returns true if a character is waiting
	{
		return 1;
	}	
	else
	{
		return 0;
	}
    return SYSERR;
}

/**
 * kungetc - put a serial character "back" into a local buffer.
 * @param c character to unget.
 * @return c on success, SYSERR on failure.
 */
syscall kungetc(unsigned char c)
{
	int i;
	if ( ungetArray[UNGETMAX-1] !='\0')
	{	
		return (int)c;
	}
	for (i = UNGETMAX-1; i >=0; i--)
	{
		if( ungetArray[i] != '\0')
		{
			ungetArray[i+1] = c;
			return (int)c;
		}
		else if(ungetArray[0] == '\0')
		{
			ungetArray[0] = c;
			return (int)c;
		}
	}

    return SYSERR;
}


/**
 * Synchronously write a character to a UART.  This blocks until the character
 * has been written to the hardware.  The interrupt handler is not used.
 *
 * @param c
 *      The character to write.
 *
 * @return
 *      The character written to the UART as an <code>unsigned char</code> cast
 *      to an <code>int</code>.
 */
syscall kputc(uchar c)
{
    volatile struct pl011_uart_csreg *regptr;

    /* Pointer to the UART control and status registers.  */
    regptr = (struct pl011_uart_csreg *)0x3F201000;

			
	while(regptr->fr & PL011_FR_TXFF) //While the transmit FIFO is full, wait until it's not
	{

	}

	regptr->dr = (int)c; //Sets the values in the data register to the character inputted, c.
    return c;
}

/**
 * kernel printf: formatted, synchronous output to SERIAL0.
 *
 * @param format
 *      The format string.  Not all standard format specifiers are supported by
 *      this implementation.  See _doprnt() for a description of supported
 *      conversion specifications.
 * @param ...
 *      Arguments matching those in the format string.
 *
 * @return
 *      The number of characters written.
 */
syscall kprintf(const char *format, ...)
{
	//spinlock_t lock = lock_create();
    int retval;
    va_list ap;
//TODO: Surround the three lines below with a lock_acquire and lock_release thing

    lock_acquire(serial_lock);
	va_start(ap, format);
    retval = _doprnt(format, ap, (int (*)(int, int))kputc, 0);
    va_end(ap);
	lock_release(serial_lock);

    return retval;
}
