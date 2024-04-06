#ifndef _ANSI_
#define _ANSI_

#define ESCAPE      ""
#define ANSI_NORMAL "[0m"
#define ANSI_BOLD   "[0;1m"
#define ANSI_UNDERL "[0;4m"
#define ANSI_BLINK  "[0;5m"
#define ANSI_INVERS "[0;7m"

#define ANSI_BLACK  "[0;30m"
#define ANSI_GREY   "[1;30m"
#define ANSI_RED    "[0;31m"
#define ANSI_GREEN  "[0;32m"
#define ANSI_YELLOW "[0;33m"
#define ANSI_BLUE   "[0;34m"
#define ANSI_PURPLE "[0;35m"
#define ANSI_CYAN   "[0;36m"
#define ANSI_WHITE  "[0;37m"

#define ANSI_BOLD_RED    "[0;1;31m"
#define ANSI_BOLD_GREEN  "[0;1;32m"
#define ANSI_BOLD_YELLOW "[0;1;33m"
#define ANSI_BOLD_BLUE   "[0;1;34m"
#define ANSI_BOLD_PURPLE "[0;1;35m"
#define ANSI_BOLD_CYAN   "[0;1;36m"
#define ANSI_BOLD_WHITE  "[0;1;37m"

#define ANSI_BLINK_GREY   "[0;5;30m"
#define ANSI_BLINK_RED    "[0;5;31m"
#define ANSI_BLINK_GREEN  "[0;5;32m"
#define ANSI_BLINK_YELLOW "[0;5;33m"
#define ANSI_BLINK_BLUE   "[0;5;34m"
#define ANSI_BLINK_PURPLE "[0;5;35m"
#define ANSI_BLINK_CYAN   "[0;5;36m"
#define ANSI_BLINK_WHITE  "[0;5;37m"

#define ANSI_INVERSE_GREY   "[0;7;30m"
#define ANSI_INVERSE_RED    "[0;7;31m"
#define ANSI_INVERSE_GREEN  "[0;7;32m"
#define ANSI_INVERSE_YELLOW "[0;7;33m"
#define ANSI_INVERSE_BLUE   "[0;7;34m"
#define ANSI_INVERSE_PURPLE "[0;7;35m"
#define ANSI_INVERSE_CYAN   "[0;7;36m"
#define ANSI_INVERSE_WHITE  "[0;7;37m"

#define ANSI_INVERSE_RED_W    "[0;35;44m[0;41m"
#define ANSI_INVERSE_GREEN_W  "[0;35;44m[0;42m"
#define ANSI_INVERSE_YELLOW_W "[0;35;44m[0;43m"
#define ANSI_INVERSE_BLUE_W   "[0;35;44m[0;44m"
#define ANSI_INVERSE_PURPLE_W "[0;35;44m[0;45m"
#define ANSI_INVERSE_CYAN_W   "[0;35;44m[0;46m"

#define ANSI_RED_BG "[41m"
#define ANSI_WHITE_BG "[47m"
#define ANSI_BLUE_BG "[44m"


#define ANSI_CLS    "[2J"
#define ANSI_HOME   "[1;1H"
#endif

char *color_table[] = {
  "[1;30m",			/* Grey 0 */
  "[0;31m",			/* Red 1 */
  "[0;32m",			/* Green 2 */
  "[0;33m",			/* Yellow 3 */
  "[0;34m",			/* Blue 4 */
  "[0;35m",			/* Purple 5 */
  "[0;36m",			/* Cyan 6 */
  "[0;37m",			/* White 7 */
  "[0;1;31m",			/* Bold_Red */
  "[0;1;32m",			/* Bold_Green */
  "[0;1;33m",			/* Bold_Yellow */
  "[0;1;34m",			/* Bold_Blue */
  "[0;1;35m",			/* Bold_Purple */
  "[0;1;36m",			/* Bold_Cyan */
  "[0;1;37m",			/* Bold White */
  "[0;5;30m",			/* Blink Grey */
  "[0;5;31m",			/* Blink Red */
  "[0;5;32m",			/* Blink Green */
  "[0;5;33m",			/* Blink Yellow */
  "[0;5;34m",			/* Blink Blue */
  "[0;5;35m",			/* Blink Purple */
  "[0;5;36m",			/* Blink Cyan */
  "[0;5;37m",			/* Blink White */
  "[0;7;31m",			/* Inverse Red */
  "[0;7;32m",			/* Inverse Green */
  "[0;7;33m",			/* Inverse Yellow */
  "[0;7;34m",			/* Inverse Blue */
  "[0;35;7m",			/* Inverse purple thing */
  "[0;7;36m",			/* Inverse Cyan */
  "[0;7;37m",			/* Inverse White */
  "[0;35;44m[0;41m",		/* White on Inverse Red */
  "[0;30m",			/* Black */
  "[47m",			/* White BG */
  "[0;35;44m[0;44m",		/* White on Inverse Blue */
  "[44m",			/* Blue BG */
  "[41m",			/* Red BG */
  "",				/* Bell */
  "[0m",			/* Ansi normal */
  0
};
