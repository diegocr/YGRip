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


#ifdef __RCS_ID
static const char TablaID[666] =
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"@(#) $Id: HTMLCharSet.h,v 0.1 2003/05/16 16:36:29 dcr8520 Exp $"
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"ISO 8859-1 Latin 1 and Unicode characters currently supported";
#endif

static const unsigned char * htmlcodes[] =
{
	"&lsquo;",  /* [1] (`) left single quote */
	"&rsquo;",  /* [2] (') right single quote */
	"&sbquo;",  /* [3] (,) single low-9 quote */
	"&ldquo;",  /* [4] (") left double quote */
	"&rdquo;",  /* [5] (") right double quote */
	"&bdquo;",  /* [6] (") double low-9 quote */
	"&dagger;",  /* [7] (+) dagger */
	"&Dagger;",  /* [8] (+) double dagger */
	"&permil;",  /* [9] (%) per mill sign */
	"&lsaquo;",  /* [10] (<) single left-pointing angle quote */
	"&rsaquo;",  /* [11] (>) single right-pointing angle quote */
	"&oline;",  /* [12] (¯) overline, = spacing overscore */
	"&trade;",  /* [13] (®) trademark sign */
	"&quot;",  /* [14] (") double quotation mark */
	"&amp;",  /* [15] (&) ampersand */
	"&frasl;",  /* [16] (/) slash */
	"&lt;",  /* [17] (<) less-than sign */
	"&gt;",  /* [18] (>) greater-than sign */
	"&ndash;",  /* [19] (-) en dash */
	"&mdash;",  /* [20] (-) em dash */
	"&nbsp;",  /* [21] () nonbreaking space */
	"&iexcl;",  /* [22] (¡) inverted exclamation */
	"&cent;",  /* [23] (¢) cent sign */
	"&pound;",  /* [24] (£) pound sterling */
	"&curren;",  /* [25] (¤) general currency sign */
	"&yen;",  /* [26] (¥) yen sign */
	"&brvbar;",  /* [27] (¦) broken vertical bar */
	"&brkbar;",  /* [28] (¦) broken vertical bar */
	"&sect;",  /* [29] (§) section sign */
	"&uml;",  /* [30] (¨) umlaut */
	"&die;",  /* [31] (¨) umlaut */
	"&copy;",  /* [32] (©) copyright */
	"&ordf;",  /* [33] (ª) feminine ordinal */
	"&laquo;",  /* [34] («) left angle quote */
	"&not;",  /* [35] (¬) not sign */
	"&shy;",  /* [36] (­) soft hyphen */
	"&reg;",  /* [37] (®) registered trademark */
	"&macr;",  /* [38] (¯) macron accent */
	"&hibar;",  /* [39] (¯) macron accent */
	"&deg;",  /* [40] (°) degree sign */
	"&plusmn;",  /* [41] (±) plus or minus */
	"&sup2;",  /* [42] (²) superscript two */
	"&sup3;",  /* [43] (³) superscript three */
	"&acute;",  /* [44] (´) acute accent */
	"&micro;",  /* [45] (µ) micro sign */
	"&para;",  /* [46] (¶) paragraph sign */
	"&middot;",  /* [47] (·) middle dot */
	"&cedil;",  /* [48] (¸) cedilla */
	"&sup1;",  /* [49] (¹) superscript one */
	"&ordm;",  /* [50] (º) masculine ordinal */
	"&raquo;",  /* [51] (») right angle quote */
	"&frac14;",  /* [52] (¼) one-fourth */
	"&frac12;",  /* [53] (½) one-half */
	"&frac34;",  /* [54] (¾) three-fourths */
	"&iquest;",  /* [55] (¿) inverted question mark */
	"&Agrave;",  /* [56] (À) uppercase A, grave accent */
	"&Aacute;",  /* [57] (Á) uppercase A, acute accent */
	"&Acirc;",  /* [58] (Â) uppercase A, circumflex accent */
	"&Atilde;",  /* [59] (Ã) uppercase A, tilde */
	"&Auml;",  /* [60] (Ä) uppercase A, umlaut */
	"&Aring;",  /* [61] (Å) uppercase A, ring */
	"&AElig;",  /* [62] (Æ) uppercase AE */
	"&Ccedil;",  /* [63] (Ç) uppercase C, cedilla */
	"&Egrave;",  /* [64] (È) uppercase E, grave accent */
	"&Eacute;",  /* [65] (É) uppercase E, acute accent */
	"&Ecirc;",  /* [66] (Ê) uppercase E, circumflex accent */
	"&Euml;",  /* [67] (Ë) uppercase E, umlaut */
	"&Igrave;",  /* [68] (Ì) uppercase I, grave accent */
	"&Iacute;",  /* [69] (Í) uppercase I, acute accent */
	"&Icirc;",  /* [70] (Î) uppercase I, circumflex accent */
	"&Iuml;",  /* [71] (Ï) uppercase I, umlaut */
	"&ETH;",  /* [72] (Ð) uppercase Eth, Icelandic */
	"&Ntilde;",  /* [73] (Ñ) uppercase N, tilde */
	"&Ograve;",  /* [74] (Ò) uppercase O, grave accent */
	"&Oacute;",  /* [75] (Ó) uppercase O, acute accent */
	"&Ocirc;",  /* [76] (Ô) uppercase O, circumflex accent */
	"&Otilde;",  /* [77] (Õ) uppercase O, tilde */
	"&Ouml;",  /* [78] (Ö) uppercase O, umlaut */
	"&times;",  /* [79] (×) multiplication sign */
	"&Oslash;",  /* [80] (Ø) uppercase O, slash */
	"&Ugrave;",  /* [81] (Ù) uppercase U, grave accent */
	"&Uacute;",  /* [82] (Ú) uppercase U, acute accent */
	"&Ucirc;",  /* [83] (Û) uppercase U, circumflex accent */
	"&Uuml;",  /* [84] (Ü) uppercase U, umlaut */
	"&Yacute;",  /* [85] (Ý) uppercase Y, acute accent */
	"&THORN;",  /* [86] (Þ) uppercase THORN, Icelandic */
	"&szlig;",  /* [87] (ß) lowercase sharps, German */
	"&agrave;",  /* [88] (à) lowercase a, grave accent */
	"&aacute;",  /* [89] (á) lowercase a, acute accent */
	"&acirc;",  /* [90] (â) lowercase a, circumflex accent */
	"&atilde;",  /* [91] (ã) lowercase a, tilde */
	"&auml;",  /* [92] (ä) lowercase a, umlaut */
	"&aring;",  /* [93] (å) lowercase a, ring */
	"&aelig;",  /* [94] (æ) lowercase ae */
	"&ccedil;",  /* [95] (ç) lowercase c, cedilla */
	"&egrave;",  /* [96] (è) lowercase e, grave accent */
	"&eacute;",  /* [97] (é) lowercase e, acute accent */
	"&ecirc;",  /* [98] (ê) lowercase e, circumflex accent */
	"&euml;",  /* [99] (ë) lowercase e, umlaut */
	"&igrave;",  /* [100] (ì) lowercase i, grave accent */
	"&iacute;",  /* [101] (í) lowercase i, acute accent */
	"&icirc;",  /* [102] (î) lowercase i, circumflex accent */
	"&iuml;",  /* [103] (ï) lowercase i, umlaut */
	"&eth;",  /* [104] (ð) lowercase eth, Icelandic */
	"&ntilde;",  /* [105] (ñ) lowercase n, tilde */
	"&ograve;",  /* [106] (ò) lowercase o, grave accent */
	"&oacute;",  /* [107] (ó) lowercase o, acute accent */
	"&ocirc;",  /* [108] (ô) lowercase o, circumflex accent */
	"&otilde;",  /* [109] (õ) lowercase o, tilde */
	"&ouml;",  /* [110] (ö) lowercase o, umlaut */
	"&divide;",  /* [111] (÷) division sign */
	"&oslash;",  /* [112] (ø) lowercase o, slash */
	"&ugrave;",  /* [113] (ù) lowercase u, grave accent */
	"&uacute;",  /* [114] (ú) lowercase u, acute accent */
	"&ucirc;",  /* [115] (û) lowercase u, circumflex accent */
	"&uuml;",  /* [116] (ü) lowercase u, umlaut */
	"&yacute;",  /* [117] (ý) lowercase y, acute accent */
	"&thorn;",  /* [118] (þ) lowercase thorn, Icelandic */
	"&yuml;",  /* [119] (ÿ) lowercase y, umlaut */
	NULL
};

static const unsigned char asciicodes[] =
{
	'`',  /* [1] (&lsquo;) left single quote */
	'\'',  /* [2] (&rsquo;) right single quote */
	',',  /* [3] (&sbquo;) single low-9 quote */
	'\"',  /* [4] (&ldquo;) left double quote */
	'\"',  /* [5] (&rdquo;) right double quote */
	'\"',  /* [6] (&bdquo;) double low-9 quote */
	'+',  /* [7] (&dagger;) dagger */
	'+',  /* [8] (&Dagger;) double dagger */
	'%',  /* [9] (&permil;) per mill sign */
	'<',  /* [10] (&lsaquo;) single left-pointing angle quote */
	'>',  /* [11] (&rsaquo;) single right-pointing angle quote */
	'¯',  /* [12] (&oline;) overline, = spacing overscore */
	'®',  /* [13] (&trade;) trademark sign */
	'\"',  /* [14] (&quot;) double quotation mark */
	'&',  /* [15] (&amp;) ampersand */
	'/',  /* [16] (&frasl;) slash */
	'<',  /* [17] (&lt;) less-than sign */
	'>',  /* [18] (&gt;) greater-than sign */
	'-',  /* [19] (&ndash;) en dash */
	'-',  /* [20] (&mdash;) em dash */
	160,  /* [21] (&nbsp;) nonbreaking space */
	'¡',  /* [22] (&iexcl;) inverted exclamation */
	'¢',  /* [23] (&cent;) cent sign */
	'£',  /* [24] (&pound;) pound sterling */
	'¤',  /* [25] (&curren;) general currency sign */
	'¥',  /* [26] (&yen;) yen sign */
	'¦',  /* [27] (&brvbar;) broken vertical bar */
	'¦',  /* [28] (&brkbar;) broken vertical bar */
	'§',  /* [29] (&sect;) section sign */
	'¨',  /* [30] (&uml;) umlaut */
	'¨',  /* [31] (&die;) umlaut */
	'©',  /* [32] (&copy;) copyright */
	'ª',  /* [33] (&ordf;) feminine ordinal */
	'«',  /* [34] (&laquo;) left angle quote */
	'¬',  /* [35] (&not;) not sign */
	'­',  /* [36] (&shy;) soft hyphen */
	'®',  /* [37] (&reg;) registered trademark */
	'¯',  /* [38] (&macr;) macron accent */
	'¯',  /* [39] (&hibar;) macron accent */
	'°',  /* [40] (&deg;) degree sign */
	'±',  /* [41] (&plusmn;) plus or minus */
	'²',  /* [42] (&sup2;) superscript two */
	'³',  /* [43] (&sup3;) superscript three */
	'´',  /* [44] (&acute;) acute accent */
	'µ',  /* [45] (&micro;) micro sign */
	'¶',  /* [46] (&para;) paragraph sign */
	'·',  /* [47] (&middot;) middle dot */
	'¸',  /* [48] (&cedil;) cedilla */
	'¹',  /* [49] (&sup1;) superscript one */
	'º',  /* [50] (&ordm;) masculine ordinal */
	'»',  /* [51] (&raquo;) right angle quote */
	'¼',  /* [52] (&frac14;) one-fourth */
	'½',  /* [53] (&frac12;) one-half */
	'¾',  /* [54] (&frac34;) three-fourths */
	'¿',  /* [55] (&iquest;) inverted question mark */
	'À',  /* [56] (&Agrave;) uppercase A, grave accent */
	'Á',  /* [57] (&Aacute;) uppercase A, acute accent */
	'Â',  /* [58] (&Acirc;) uppercase A, circumflex accent */
	'Ã',  /* [59] (&Atilde;) uppercase A, tilde */
	'Ä',  /* [60] (&Auml;) uppercase A, umlaut */
	'Å',  /* [61] (&Aring;) uppercase A, ring */
	'Æ',  /* [62] (&AElig;) uppercase AE */
	'Ç',  /* [63] (&Ccedil;) uppercase C, cedilla */
	'È',  /* [64] (&Egrave;) uppercase E, grave accent */
	'É',  /* [65] (&Eacute;) uppercase E, acute accent */
	'Ê',  /* [66] (&Ecirc;) uppercase E, circumflex accent */
	'Ë',  /* [67] (&Euml;) uppercase E, umlaut */
	'Ì',  /* [68] (&Igrave;) uppercase I, grave accent */
	'Í',  /* [69] (&Iacute;) uppercase I, acute accent */
	'Î',  /* [70] (&Icirc;) uppercase I, circumflex accent */
	'Ï',  /* [71] (&Iuml;) uppercase I, umlaut */
	'Ð',  /* [72] (&ETH;) uppercase Eth, Icelandic */
	'Ñ',  /* [73] (&Ntilde;) uppercase N, tilde */
	'Ò',  /* [74] (&Ograve;) uppercase O, grave accent */
	'Ó',  /* [75] (&Oacute;) uppercase O, acute accent */
	'Ô',  /* [76] (&Ocirc;) uppercase O, circumflex accent */
	'Õ',  /* [77] (&Otilde;) uppercase O, tilde */
	'Ö',  /* [78] (&Ouml;) uppercase O, umlaut */
	'×',  /* [79] (&times;) multiplication sign */
	'Ø',  /* [80] (&Oslash;) uppercase O, slash */
	'Ù',  /* [81] (&Ugrave;) uppercase U, grave accent */
	'Ú',  /* [82] (&Uacute;) uppercase U, acute accent */
	'Û',  /* [83] (&Ucirc;) uppercase U, circumflex accent */
	'Ü',  /* [84] (&Uuml;) uppercase U, umlaut */
	'Ý',  /* [85] (&Yacute;) uppercase Y, acute accent */
	'Þ',  /* [86] (&THORN;) uppercase THORN, Icelandic */
	'ß',  /* [87] (&szlig;) lowercase sharps, German */
	'à',  /* [88] (&agrave;) lowercase a, grave accent */
	'á',  /* [89] (&aacute;) lowercase a, acute accent */
	'â',  /* [90] (&acirc;) lowercase a, circumflex accent */
	'ã',  /* [91] (&atilde;) lowercase a, tilde */
	'ä',  /* [92] (&auml;) lowercase a, umlaut */
	'å',  /* [93] (&aring;) lowercase a, ring */
	'æ',  /* [94] (&aelig;) lowercase ae */
	'ç',  /* [95] (&ccedil;) lowercase c, cedilla */
	'è',  /* [96] (&egrave;) lowercase e, grave accent */
	'é',  /* [97] (&eacute;) lowercase e, acute accent */
	'ê',  /* [98] (&ecirc;) lowercase e, circumflex accent */
	'ë',  /* [99] (&euml;) lowercase e, umlaut */
	'ì',  /* [100] (&igrave;) lowercase i, grave accent */
	'í',  /* [101] (&iacute;) lowercase i, acute accent */
	'î',  /* [102] (&icirc;) lowercase i, circumflex accent */
	'ï',  /* [103] (&iuml;) lowercase i, umlaut */
	'ð',  /* [104] (&eth;) lowercase eth, Icelandic */
	'ñ',  /* [105] (&ntilde;) lowercase n, tilde */
	'ò',  /* [106] (&ograve;) lowercase o, grave accent */
	'ó',  /* [107] (&oacute;) lowercase o, acute accent */
	'ô',  /* [108] (&ocirc;) lowercase o, circumflex accent */
	'õ',  /* [109] (&otilde;) lowercase o, tilde */
	'ö',  /* [110] (&ouml;) lowercase o, umlaut */
	'÷',  /* [111] (&divide;) division sign */
	'ø',  /* [112] (&oslash;) lowercase o, slash */
	'ù',  /* [113] (&ugrave;) lowercase u, grave accent */
	'ú',  /* [114] (&uacute;) lowercase u, acute accent */
	'û',  /* [115] (&ucirc;) lowercase u, circumflex accent */
	'ü',  /* [116] (&uuml;) lowercase u, umlaut */
	'ý',  /* [117] (&yacute;) lowercase y, acute accent */
	'þ',  /* [118] (&thorn;) lowercase thorn, Icelandic */
	'ÿ',  /* [119] (&yuml;) lowercase y, umlaut */
	NULL
};


#if 0
char *MiscTagsS[] = { "<script", "<style", NULL };
char *MiscTagsE[] = { "</script>", "</style>", NULL };
#endif

