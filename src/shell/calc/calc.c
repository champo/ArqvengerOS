#include "shell/calc/calc.h"
#include "library/stdio.h"
#include "mcurses/mcurses.h"
#define cleanbuffer() while(getchar()!='\n')

void manCalc(void) {
    setBold(1);
    printf("Usage:\n\t calc\n");
    setBold(0);

    printf("Once inside the calc program you can do simple math calculations "
            "between integer numbers with the format \"X symb Y\", X and Y "
            "representing numbers and symb being a +. -, * or /. Also, you can "
            "use quit or exit to finish the program.");
}

void calc(void) {
    int num1;
    int num2;
    int ans;
    char c;
    
    printf("(Calc) > ");
    while(scanf("quit") == EOF && scanf("exit")) {
        printf("\n(Calc) > ");
        if(scanf("help") != EOF) {
            printf("Inside the calc program you can do simple math calculations "
            "between integer numbers with the format \"X symb Y\", X and Y "
            "representing numbers and symb being a +. -, * or /. Also, you can "
            "use quit or exit to finish the program.");
        } else {
            if(scanf("%d%c%d", &num1, &c, &num2) == EOF) {
                printf("Wrong expression, Use help command for instructions.");
                
            } else {
                switch(c){
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
                        ans = num1 / num2;
                        printf("%d", ans);
                        break;
                    default:
                        printf("Wrong operator, use only +, -, * and /");
                        break;
                }
            }         
        }
        printf("\n(Calc) > ");
        cleanbuffer();
    }
    cleanbuffer();
}
