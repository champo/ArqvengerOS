#include "shell/calc/calc.h"
#include "library/stdio.h"
#include "library/string.h"
#include "mcurses/mcurses.h"
#define cleanbuffer() while(getchar()!='\n')
#define MAXLINE 500

/**
 * Manual page for the calc command.
 */
void manCalc(void) {
    setBold(1);
    printf("Usage:\n\t calc\n");
    setBold(0);

    printf("Once inside the calc program you can do simple math calculations "
            "between integer numbers with the format \"X symb Y\", X and Y "
            "representing numbers and symb being a +. -, * or /. Also, you can "
            "use quit or exit to finish the program.\n");
}

/**
 * Command that executes a simple calculator.
 * Allows to make equations with the format X op Y.
 */
void calc(char* unused) {
    int num1;
    int num2;
    int ans = 0;
    char c;
    int flag = 1;
    char string[MAXLINE];

    printf("(Calc) > ");
    while (flag) {
        if (scanf("%d%c%d", &num1, &c, &num2) == 3) {
            switch (c) {
                case '+':
                    ans = num1 + num2;
                    printf("%d", ans);
                    break;
                case '-':
                    ans = num1 - num2;
                    printf("%d", ans);
                    break;
                case '*':
                    ans = num1 * num2;
                    printf("%d", ans);
                    break;
                case '/':
                    if(num2 != 0) {
                        ans = num1 / num2;
                        printf("%d", ans);
                    } else {
                        printf("Exception, cannot divide by 0");
                    }
                    break;
                default:
                    printf("Wrong operator, use only +, -, * and /");
                    break;
            }
        } else {
            scanf("%s", string);
            if (!strcmp("quit", string) || !strcmp("exit", string)) {
                flag = 0;
            } else {
                if (!strcmp("help", string)) {
                    printf("Inside the calc program you can do simple math calculations "
                    "between integer numbers with the format \"X symb Y\", X and Y "
                    "representing numbers and symb being a +. -, * or /. Also, you can "
                    "use quit or exit to finish the program.");
                } else {
                    printf("Wrong expression, Use help command for instructions.");
                }
            }
        }
        cleanbuffer();
        if (flag) {
            printf("\n(Calc) > ");
        }
    }
}
