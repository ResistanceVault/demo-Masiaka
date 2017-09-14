#include "genesis.h"
#include "string_parser.h"

u16 inline computeStringLen(char *str)
{
	u16 l = 0;
	while(str[l])
		l++;

	return l;
}

u16 inline charToTileIndex(char c)
{
	if (c >= 'A' && c <= 'Z')
		return (u16)(c - 'A'); 

	if (c >= '0' && c <= '9')
		return (u16)((c  + 26) - '0');

	switch(c)
	{
		case '!':
			return FONT_PUNCT_OFFSET;
		case '.':
			return FONT_PUNCT_OFFSET + 1;
		case '\'':
			return FONT_PUNCT_OFFSET + 2;
		case '$':
			return FONT_PUNCT_OFFSET + 3;
		case ',':
			return FONT_PUNCT_OFFSET + 4;
		case '(':
			return FONT_PUNCT_OFFSET + 5;
		case ')':
			return FONT_PUNCT_OFFSET + 6;
		case '?':
			return FONT_PUNCT_OFFSET + 7;
		case '-':
			return FONT_PUNCT_OFFSET + 8;
		case '@':
			return FONT_PUNCT_OFFSET + 9;
		case ':':
			return FONT_PUNCT_OFFSET + 10;
		case '=':
			return FONT_PUNCT_OFFSET + 11;
		case '+':
			return FONT_PUNCT_OFFSET + 12;
		case 'c':
			return FONT_PUNCT_OFFSET + 13;
		case '[':
			return FONT_PUNCT_OFFSET + 14;
		case ']':
			return FONT_PUNCT_OFFSET + 15;		

	};

	/* if no character was found,
		we return a special code */
	return 0xFF;
}