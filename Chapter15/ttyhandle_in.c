#include "../inclusions.h"

local void erase1(struct ttycblk *, struct uart_csreg *);
local void echoch(char, struct ttycblk *, struct uart_csreg *);
local void eputc(char, struct ttycblk *, struct uart_csreg *);

void ttyhandle_in(struct ttycblk *typtr, struct uart_csreg *csrptr)
{
  char ch;
  uint32 avail;

  ch = csrptr->buffer;

  avail = semcount(typtr->tyisem);
  if (avail < 0)
    avail = 0;

  /* Handle raw mode */
  if (typtr->tyimode == TY_IMRAW) {

    /* No space, ignore input */
    if (avail >= TY_IBUFLEN)
      return;

    /* Place character in buffer with no editing */
    *typtr->tyitail++ = ch;

    /* Wrap buffer pointer */
    if (typtr->tyotail >= &typtr->tyobuff[TY_OBUFLEN])
      typtr->tyotail = typtr->tyobuff;

    signal(typtr->tyisem);
    return;
  }

  /* Handle common parts of cooked and cbreak modes */
  if ((ch == TY_RETURN) && typtr->tyicrlf)
    ch = TY_NEWLINE;

  /* If flow control is in effect, handle ^S and ^Q */
  if (typtr->tyoflow) {
    /* ^Q starts output */
    if (ch == typtr->tyostart) {
      typtr->tyoheld = FALSE;
      ttykickout(csrptr);
      return;
    /* ^S stops output */
    } else if (ch == typtr->tyostop) {
      typtr->tyoheld = TRUE;
      return;
    }
  }

  /* Any other character starts output */
  typtr->tyoheld = FALSE;

  if (typtr->tyimode == TY_IMCBREAK) {
    /* If input buffer is full, send bell to user */
    if (avail >= TY_IBUFLEN) {
      eputc(typtr->tyifullc, typtr, csrptr);
    } else {
      *typtr->tyitail++ = ch;

      /* Wrap around buffer */
      if (typtr->tyitail >= &typtr->tyibuff[TY_IBUFLEN])
        typtr->tyitail = typtr->tyibuff;

      if (typtr->tyiecho)
        echoch(ch, typtr, csrptr);
    }
    return;
  } else {
    /* Line kill character arrives - kill entire line */
    if (ch == typtr->tyikillc && typtr->tyikill) {

      if (typtr->tyitail < typtr->tyibuff)
        typtr->tyihead += TY_IBUFLEN;

      typtr->tyicursor = 0;
      eputc(TY_RETURN, typtr, csrptr);
      eputc(TY_NEWLINE, typtr, csrptr);
      return;
    }

    /* Erase (backspace) character */
    if (((ch == typtr->tyierasec) || (ch == typtr->tyierasec2)) && typtr->tyierase) {
      if (typtr->tyicursor > 0) {
        typtr->tyicursor--;
        erase1(typtr, csrptr);
      }
      return;
    }

    /* End of line */
    if ((ch == TY_NEWLINE) || (ch == TY_RETURN)) {
      if (typtr->tyiecho)
        echoch(ch, typtr, csrptr);

      *typtr->tyitail++ = ch;
      if (typtr->tyitail >= &typtr->tyibuff[TY_IBUFLEN])
        typtr->tyitail = typtr->tyibuff;

      /* Make entire line (plus \n or \r) available */
      signaln(typtr->tyisem, typtr->tyicursor + 1);
      /* Reset for next line */
      typtr->tyicursor = 0;
      return;
    }

    /* Character to be placed in buffer - send bell if buffer overflowed */
    avail = semcount(typtr->tyisem);
    if (avail < 0)
      avail = 0;

    if ((avail + typtr->tyicursor) >= TY_IBUFLEN) {
      eputc(typtr->tyifullc, typtr, csrptr);
      return;
    }

    /* EOF character: recognize at beginning of line, but print
     * and ignore otherwise
     */
    if (ch == typtr->tyeofch && typtr->tyeof) {
      if (typtr->tyieho)
        echoch(ch, typtr, csrptr);

      if (typtr->tyicursor != 0)
        return;

      *typtr->tyitail++ = ch;
      signal(typtr->tyisem);
      return;
    }

    /* Echo the character */
    if (typtr->tyiecho)
      echoch(ch, typtr, csrptr);

    /* Insert in the input buffer */
    typtr->tyicursor++;
    *typtr->tyitail++ = ch;

    /* Wrap around if needed */
    if (typtr->tyitail >= &typtr->tyibuff[TY_IBUFLEN])
      typtr->tyitail = typtr->tyibuff;

    return;
  }
}

/* --------------------------------------------------------------------
 *   erase1() - Erase one character honoring erasing backspace
 * --------------------------------------------------------------------
 */
local void erase1(struct ttycblk *typtr, struct uart_csreg *csrptr)
{
  char ch;

  if ((--typtr->tyitail) < typtr->tyibuff)
    typtr->tyitail += TY_IBUFLEN;

  /* Pickup character to erase */
  ch = *typtr->tyitail;

  /* Echoing */
  if (typtr->tyiecho) {
    /* Non printable */
    if (ch < TY_BLANK || ch == 0177) {
      /* Visual cntl characters */
      if (typtr->tyevis) {
        eputc(TY_BACKSP, typtr, csrptr);
        /* Erase character */
        if (typtr->tyieback) {
          eputc(TY_BLANK, typtr, csrptr);
          eputc(TY_BACKSP, typtr, csrptr);
        }
      }
      /* Bypass up arrow */
      eputc(TY_BACKSP, typtr, csrptr);
      if (typtr->tyieback) {
        eputc(TY_BLANK, typtr, csrptr);
        eputc(TY_BACKSP, typtr, csrptr);
      }
    /* Normal character that is printable */
    } else {
      eputc(TY_BACKSP, typtr, csrptr);
      /* Erase the character */
      if (typtr->tyieback) {
        eputc(TY_BLANK, typtr, csrptr);
        eputc(TY_BACKSP, typtr, csrptr);
      }
    }
  }
  return;
}

/* ----------------------------------------------------------------------------
 *   echoch() - Echo a character with visual and output crlf options
 * ----------------------------------------------------------------------------
 */
local void echoch(char ch, struct ttycblk *typtr, struct uart_csreg *csrptr)
{
  if ((ch == TY_NEWLINE || ch == TY_RETURN) && typtr->tyecrlf) {
    eputc(TY_RETURN, typtr, csrptr);
    eputc(TY_NEWLINE, typtr, csrptr);
  } else if ((ch < TY_BLANK || ch == 0177) && typtr->tyevis) {
    /* Print ^X */
    eputc(TY_UPARROW, typtr, csrptr);
    /* Make it printable */
    eputc(ch + 0100, typtr, csrptr);
  } else {
    eputc(ch, typtr, csrptr);
  }
}

/* -----------------------------------------------------------------------------
 *   eputc() - Put one character in the echo queue
 * -----------------------------------------------------------------------------
 */
local void eputc(char ch, struct ttycblk *typtr, struct uart_csreg *csrptr)
{
  *typtr->tyetail++ = ch;

  /* Wrap around buffer, if needed */
  if (typtr->tyetail >= &typtr->tyebuff[TY_EBUFLEN])
    typtr->tyetail = typtr->tyebuff;

  ttykickout(csrptr);
  return;
}
