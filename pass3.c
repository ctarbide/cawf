/*
 *	pass3.c - cawf(1) pass 3 function
 */

/*
 *	Copyright (c) 1991 Purdue University Research Foundation,
 *	West Lafayette, Indiana 47907.  All rights reserved.
 *
 *	Written by Victor A. Abell <abe@cc.purdue.edu>,  Purdue	University
 *	Computing Center.  Not derived from licensed software; derived from
 *	awf(1) by Henry Spencer of the University of Toronto.
 *
 *	Permission is granted to anyone to use this software for any
 *	purpose on any computer system, and to alter it and redistribute
 *	it freely, subject to the following restrictions:
 *
 *	1. The author is not responsible for any consequences of use of
 *	   this software, even if they arise from flaws in it.
 *
 *	2. The origin of this software must not be misrepresented, either
 *	   by explicit claim or by omission.  Credits must appear in the
 *	   documentation.
 *
 *	3. Altered versions must be plainly marked as such, and must not
 *	   be misrepresented as being the original software.  Credits must
 *	   appear in the documentation.
 *
 *	4. This notice may not be removed or altered.
 */

#include "cawf.h"

void
Pass3(len, word, wl, sarg, narg)
	int len;			/* output length -- negative is
					 * special */
	unsigned char *word;		/* word */
	int wl;				/* real word length */
	unsigned char *sarg;		/* string argument */
	int narg;			/* numeric argument */
{
	int addto;			/* spaces to add to all words */
	static int fp = 1;		/* first page print status */
	int i, j, k;			/* temporary index */
	int n;				/* temporary number */
	int sp = 0;			/* no-break spacing switch */
	int sp_Outll;			/* sp-saved Outll */
	char sp_Outln;			/* sp-saved Outln[0] */
	int sp_Outlx;			/* sp-saved Outlx */
	int sp_Padx;			/* sp-saved Padx */
	int sp_Tind;			/* sp-saved Tind */
	int xsp;			/* extra spaces to add */
	int vsp;			/* vertical spacing status */

	vsp = 0;
    /*
     * If not a special command, process a word.
     */
	if (len >= 0 && Outll < 0) {
	/*
	 * Enter first word.
	 */
		(void) memcpy((void *)Outln, (void *)word, (size_t)wl);
		Outlx = wl;
		Outll = len;
		Padx = 0;
	} else if (len >= 0
	       && (Outll+Contlen+len+narg) <= (LL-Pgoff-Ind-Tind)) {
	/*
	 * The word fits, so enter it.
	 */
		if ((Contlen + len) > 0) {
line_too_big:
			if ((Outlx + Contlen + wl) >= MAXOLL) {
				Error3(len, (char *)word, (char *)sarg, narg,
					"output line too big");
				return;
			} else {
				if (Contlen > 0 && Cont != NULL) {
					if (Contlen == 1 && *Cont == ' ') {
						Padchar[Padx++] = Outlx;
						Outln[Outlx++] = ' ';
					} else {
					    (void) strcpy((char *)&Outln[Outlx],
						(char *)Cont);
					    Outlx += Contlen;
					}
				}
				if (len > 0) {
				    (void) memcpy((void *)&Outln[Outlx],
					(void *)word, (size_t)wl);
				    Outlx += wl;
				}
			}
		}
		Outll += Contlen + len;
	} else if (len == NOBREAK || len == MESSAGE) {
		/*
		 * Do nothing (equivalent to break)
		 */
	} else if (len == DOBREAK && strcmp((char *)word, "need") == 0
	       &&  (Nxtln + narg) < (Pglen + 1 - Botmarg)) {
		/*
		 * Do nothing, because there is room on the page.
		 */
	} else if (len == DOBREAK && strcmp((char *)word, "toindent") == 0
	       &&  (Ind + Tind + Outll) < Ind) {
	/*
	 * Move to indent position with line - there is room.
	 */
		n = Ind - (Ind + Tind + Outll);
		Outll += n;
		if ((Outlx + n) >= MAXOLL)
			goto line_too_big;
		for (i = n; i; i--)
			Outln[Outlx++] = ' ';
		Padx = 0;
		Free(&Cont);
		Contlen = 0;
	} else if (Outll >= 0
	       || (len == DOBREAK && strcmp((char *)word, "need") == 0)
	       || (len == RAWLINE)) {
	/*
	 * A non-empty line or a "need" or a RAWLINE forces output.
	 */
		vsp = 0;

print_line:
		if (Nxtln == 1) {
	    /*
	     * We're at the top of the page.
	     *
	     * Determine if the page is in the printing range.
	     *
	     * Issue the header.
	     */
			j = fp;
			if ((Pageprt = PageInRange(Thispg)) == 1) {
				if (!fp)
					Charput("\f");
				fp = 0;
			}
			for (i = (Topmarg - 1)/2; i > 0; i--) {
				Charput("\n");
				Nxtln++;
			}
		    /*
		     * Print the page header, as required.
		     */
			if (Fph || !j) {
				i = LenprtHF(Hdc, Thispg, 0, NULL, 0)
				  + LenprtHF(Hdl, Thispg, 0, NULL, 0)
				  + LenprtHF(Hdr, Thispg, 0, NULL, 0) + 2;
				j = (LL - i - Pgoff) / 2 + 1;
				n = LL - Pgoff - i - j + 2;
				for (k = 0; k < Pgoff; k++)
					Charput(" ");
				if (Hdl)
				    (void) LenprtHF(Hdl, Thispg, 1, NULL, 0);
				while (j--)
					Charput(" ");
				if (Hdc)
				    (void) LenprtHF(Hdc, Thispg, 1, NULL, 0);
				while (n--)
					Charput(" ");
				if (Hdr)
				    (void) LenprtHF(Hdr, Thispg, 1, NULL, 0);
				Charput("\n");
			} else
				Charput("\n");
			Nxtln++;
			while(Nxtln <= Topmarg) {
				Charput("\n");
				Nxtln++;
			}
		}
	    /*
	     * If this is a raw line, simply output the string argument.
	     */
		if (len == RAWLINE) {
			Stringput(sarg, strlen((char *)sarg));
			Charput("\n");
			Nxtln++;
		} else {

	    /*
	     * Print a normal output line.
	     */
		/*
		 *  Add a trailing hyphen, if mecessary.
		 */
		    if (vsp == 0 && Eollen > 0 && Eol != NULL) {
			i = strlen((char *)Eol);
			if ((Outlx + i) >= MAXOLL)
			    goto line_too_big;
			(void) memcpy((void *)&Outln[Outlx], (void *)Eol, i);
			Outlx += i;
			Outll += Eollen;
		    }
		/*
		 * Trim trailing spaces from the output line.
		 */
     		    while (Outlx > 0) {
			if (Outln[Outlx - 1] != ' ')
			    break;
			if (Padx > 0 && (Outlx - 1) == Padchar[Padx - 1])
			    Padx--;
			Outlx--;
			Outll--;
		    }
		    if (Outlx == 0) {
			Charput("\n");
		    } else if (len == DOBREAK
			 && strcmp((char *)word, "center") == 0) {
		    /*
		     * Center the output line.
		     */
			i = (LL - Pgoff - Outll) / 2;
			if (i < 0)
			    i = 0;
			for (j = (Pgoff + Ind + Tind + i); j; j--)
			    Charput(" ");
			Stringput(Outln, Outlx);
			Charput("\n");
		    } else if (Adj == LEFTADJ
		           || (Adj == BOTHADJ && (len < 0 || Padx == 0))) {
		    /*
		     * No right margin adjustment - disabled, inappropriate
		     * (line ended by break) or impossible.
		     */
			for (i = 0; i < (Pgoff + Ind + Tind); i++)
			    Charput(" ");
			Stringput(Outln, Outlx);
			Charput("\n");
		    } else if (Adj == BOTHADJ) {
		    /*
		     * Adjust right margin.
		     */
			for (i = 0; i < (Pgoff + Ind + Tind); i++)
			    Charput(" ");
			i = LL - (Pgoff + Ind + Tind);
			j = i - Outll;
			addto = Padx ? (j / Padx) : 0;
			xsp = j - (Padx * addto);
			for (i = k = 0; i < Padx; i++) {
			    while (k < Outlx && k <= Padchar[i]) {
				Charput(&Outln[k]);
				k++;
			    }
			    if (k >= Outlx)
				break;
			    j = addto;
			    if (Padfrom == PADLEFT) {
				if (i < xsp)
				    j++;
			    } else if (i >= (Padx - xsp))
				j++;
			    while (j--)
				Charput(" ");
			}
			if ((Outlx - k) > 0)
				Stringput(&Outln[k], Outlx - k);
			Charput("\n");
			Padfrom = (Padfrom == PADLEFT) ? PADRIGHT : PADLEFT;
		    }
		/*
		 * End of line housekeeping
		 */
		    Nxtln++;
		    Outll = -1;
		    Outlx = 0;
		    Padx = 0;
		    Tind = 0;
		    Nospmode = 0;
		    if (vsp == 0 && len == DOBREAK
		    &&  strcmp((char *)word, "need") == 0) {
		    /*
		     * Break caused by "need" - satisfy it.
		     */
			while (Nxtln < (Pglen + 1 - Botmarg)) {
			    Charput("\n");
			    Nxtln++;
			}
		    }
		}
		if (Nxtln >= (Pglen + 1 - Botmarg)) {
	    /*
	     * Footer required
	     */
			for (i = (Botmarg - 1)/2; i > 0; i--) {
				Charput("\n");
				Nxtln++;
			}
			i = LenprtHF(Ftl, Thispg, 0, NULL, 0)
			  + LenprtHF(Ftc, Thispg, 0, NULL, 0)
			  + LenprtHF(Ftr, Thispg, 0, NULL, 0) + 2;
			j = (LL - i - Pgoff) / 2 + 1;
			n = LL - Pgoff - i - j + 2;
			for (k = 0; k < Pgoff; k++)
				Charput(" ");
			if (Ftl)
				(void) LenprtHF(Ftl, Thispg, 1, NULL, 0);
			while (j--)
				Charput(" ");
			if (Ftc)
				(void) LenprtHF(Ftc, Thispg, 1, NULL, 0);
			while (n--)
				Charput(" ");
			if (Ftr)
				(void) LenprtHF(Ftr, Thispg, 1, NULL, 0);
			Charput("\n");
			Nxtln++;
		/*
		 * The last blank line on the page is suppressed to assist
		 * printers that can't look ahead to the following FF.
		 */
			while (Nxtln < Pglen) {
				Charput("\n");
				Nxtln++;
			}
			Nxtln = 1;
			Thispg++;
			Nospmode = 1;
			Padfrom = PADRIGHT;
		}
	    /*
	     * Initiate any extra vertical spacing.
	     */
		if (++vsp < Vspace)
			goto print_line;
	    /*
	     * Save any input word that might have forced output.
	     */
		if (len >= 0) {
			(void) memcpy((void *)Outln, (void *)word, (size_t)wl);
			Outlx = wl;
			Outll = len;
			Padx = 0;
		} else if (len == RAWLINE)
			return;
	}
    /*
     * A break causes padding reversal.
     */
	if (len == DOBREAK)
		Padfrom = PADRIGHT;
	if (len >= 0 || strcmp((char *)word, "nohyphen") == 0) {
    /*
     * Reset continuation and hyphenation.
     */
		if (Contlen != 1 || Cont[0] != ' ') {
			Free(&Cont);
			Cont = Newstr((unsigned char *)" ");
			Contlen = 1;
		}
		if (Eollen > 0) {
			Free(&Eol);
			Eollen = 0;
		}
		return;
	}
    /*
     * Now post-process any special commands.
     */
	if (len == MESSAGE) {
		Error3(len, (char *)word, (char *)sarg, narg, NULL);
		return;
	}

	switch (*word) {

	case 'b':				/* both */
	    /*
	     * Adjust on both margins.
	     */
		Adj = BOTHADJ;
		return;

	case 'c':				/* center */
		return;

	case 'e':				/* errsto */
	    /*
	     * "errsto" comes from awf.
	     */
		return;

	case 'f':				/* flush and fph */
		if (word[1] == 'l')
			return;
		else if (word[1] == 'p') {
	    /*
	     * First page header status
	     */
			Fph = narg;
			return;
		}
		break;

	case 'g':				/* gap */
	    /*
	     * Increase word gap.  (Space is not paddable.)
	     */
		if (Outll >= 0) {
			if ((Outlx + narg - 1) >= MAXOLL)
				goto line_too_big;
			for (i = 0; i < (narg - 1); i++) {
				Outln[Outlx++] = ' ';
				Outll++;
			}
		}
		return;

	case 'h':				/* hyphen */
	    /*
	     * Set discretionary hyphen.
	     */
		Free(&Cont);
		Contlen = 0;
		Free(&Eol);
		Eol = (sarg != NULL) ? Newstr(sarg) : NULL;
		Eollen = narg;
		return;

	case 'i':				/* indent */
	    /*
	     * Set indentation.
	     */
		Ind = narg;
		return;

	case 'l':				/* left or linelen */
		if (word[1] == 'e') {
	    /*
	     * Adjust on left margin.
	     */
			Adj = LEFTADJ;
			return;
		} else if (word[1] == 'i') {
	    /*
	     * Set line length.
	     */
			LL = narg;
			return;
		}
		break;

	case 'n':				/* need or nospace */
		if (word[1] == 'e')
			return;			/* need */
		else if (word[1] == 'o') {
	    /*
	     * Set no space mode.
	     */
			Nospmode = 1;
			return;
		}
		break;

	case 'p':				/* pagelen or pageoffset */
		if (strncmp((char *)&word[1], "age", 3) != 0)
			break;
		if (word[4] == 'l') {
	    /*
	     * Set page length.
	     */
			Pglen = narg;
			return;
		} else if (word[4] == 'o') {
	    /*
	     * Set page offset.
	     */
			Pgoff = narg;
			return;
		}
		break;

	case 's':				/* space */
		if (sp) {

		/*
		 * Restore values after NOBREAK spacing ("^'sp").
		 */

#if	defined(_BCC)
#pragma warn -def
#endif	/* defined(_BCC) */
			Outlx = sp_Outlx;
			Outln[0] = sp_Outln;
			Padx = sp_Padx;
			Outll = sp_Outll;
			Tind = sp_Tind;

#if	defined(_BCC)
#pragma warn +def
#endif	/* defined(_BCC) */

			return;
		}
		if (Nospmode == 0) {
			if (len == NOBREAK) {
		
			/*
			 * Set up for NOBREAK spacing.
			 */
				sp_Outlx = Outlx;
				sp_Outln = Outln[0];
				sp_Padx = Padx;
				sp_Outll = Outll;
				sp_Tind = Tind;
				vsp = Vspace + 1;
				sp = 1;
			}
		/*
		 * Generate a blank line.
		 */
			Outlx = 0;
			Padx = 0;
			Outll = LL - 1;
			if (sp)
				goto print_line;
		}
		return;

	case 't':				/* tabto, tempindent, toindent,
						 * or ttllen */
		if (word[1] == 'a') {
	    /*
	     * Move to TAB stop.
	     */
			if (Outll < 0)
				Outll = 0;
			for (i = 0; i < Ntabs; i++) {
				if ((n = Tabs[i] - Outll) > 0) {
					if ((Outlx + n) >= MAXOLL)
						goto line_too_big;
					Outll += n;
					for (i = n; i > 0; i--)
						Outln[Outlx++] = ' ';
					Free(&Cont);
					Contlen = 0;
					Padx = 0;
					break;
				}
			}
			return;
		} else if (word[1] == 'e') {
	    /*
	     * Set temporary indentation.
	     */
			if (*sarg == '\0' && narg >= 0)
				Tind = narg - Ind;
			else
				Tind = ((Ind + narg) >= 0) ? narg : -Ind;
			return;
		} else if (word[1] == 'o') {
			return;				/* toindent */

	    /*
	     * Set 3-part title line length.
	     */
		} else if (word[1] == 't') {
			LT = narg;
			return;
		}
		break;

	case 'u':					/* userhyphen */
	    /*
	     * Set continuation and end-of-line values for hyphenation.
	     */
		Free(&Cont);
		Free(&Eol);
		Contlen = Eollen = narg;
		Cont = (sarg == NULL) ? NULL : Newstr(sarg);
		Eol = (sarg == NULL) ? NULL : Newstr(sarg);
		return;

	case 'v':					/* vspace */
	    /*
	     * Set vertical spacing.
	     */
		Vspace = (narg == 0) ? 1 : narg;
		return;

	case 'y':					/* yesspace */
	    /*
	     * Set space mode.
	     */
		Nospmode = 0;
		return;
	}				/* end of switch(*word) */
    /*
     * Locate header and footer defintions.
     */
	if (regexec(Pat[14].pat, word)) {
		if (strcmp((char *)word, "LH") == 0) {
		    /*
		     * Left header
		     */
			Free(&Hdl);
			if (sarg != NULL)
				Hdl = Newstr(sarg);
			return;
		}
		if (strcmp((char *)word, "CH") == 0) {
		    /*
		     * Center header
		     */
			Free(&Hdc);
			if (sarg != NULL)
				Hdc = Newstr(sarg);
			return;
		}
		if (strcmp((char *)word, "RH") == 0) {
		    /*
		     * Right header
		     */
			Free(&Hdr);
			if (sarg != NULL)
				Hdr = Newstr(sarg);
			return;
		}
		if (strcmp((char *)word, "LF") == 0) {
		    /*
		     * Left footer
		     */
			Free(&Ftl);
			if (sarg != NULL)
				Ftl = Newstr(sarg);
			return;
		}
		if (strcmp((char *)word, "CF") == 0) {
		    /*
		     * Center footer
		     */
			Free(&Ftc);
			if (sarg != NULL)
				Ftc = Newstr(sarg);
			return;
		}
		if (strcmp((char *)word, "RF") == 0) {
		    /*
		     * Right footer
		     */
			Free(&Ftr);
			if (sarg != NULL)
				Ftr = Newstr(sarg);
			return;
		}
	}
    /*
     * Error on unknown arguments
     */
	Error3(len, (char *)word, (char *)sarg, narg, "unknown request");
}
