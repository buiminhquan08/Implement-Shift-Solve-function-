#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define MAX_ITER 100
#define MAX 100
#define EPSILON 1e-4 // Sai so 10^-4

/*
Trang thai cua bo xu ly bieu thuc:
S_START: Trang thai bat dau
S_OPERAND: Trang thai nhan toan hang
S_OPERATOR: Trang thai nhan toan tu
S_OPEN: Trang thai nhan dau mo ngoac
S_CLOSE: Trang thai nhan dau dong ngoac
S_ERROR: Trang thai loi
S_END: Trang thai ket thuc
*/

// Dinh nghia cac trang thai trong qua trinh phan tich bieu thuc
typedef enum { S_START, S_OPERAND, S_OPERATOR, S_OPEN, S_CLOSE, S_ERROR, S_END } state_t;

// Dinh nghia loai token trong bieu thuc
typedef enum {
    OPERAND,  // So hang
    OPERATOR, // Toan tu
    VARIABLE  // Bien (x)
} TokenType;

// Cau truc luu tru token
typedef struct {
    TokenType type;
    union {
        float operand;   // Gia tri so
        char operator_;  // Toan tu
        float variable;  // Bien x
    } value;
} Token;

// Kiem tra ky tu co phai la toan tu hay khong
int isOperator(char c) {
    return (c == '+' || c == '-' || c == '*' || c == '/' || c == '^');
}

// Xac dinh do uu tien cua toan tu
int precedence(char op) {
    switch (op) {
        case '+':
        case '-': return 1;
        case '*':
        case '/': return 2;
        case '^': return 3;
        default: return 0;
    }
}
// Tr? v? 1 n?u k?t qu? l� duong, -1 n?u l� �m
int getSign(char **expr) {
    int sign = 1;
    while (**expr == '+' || **expr == '-') {
        if (**expr == '-') sign *= -1;
        (*expr)++;  // Di chuy?n sang k� t? ti?p theo
    }
    return sign;
}

// Tach phuong trinh thanh hai phan trai va phai cua dau "="
void splitEquation(char *expr, char *left, char *right) {
    char *equalSign = strchr(expr, '=');
    if (equalSign) {
        *equalSign = '\0';
        strcpy(left, expr);
        strcpy(right, equalSign + 1);
    } else {
        strcpy(left, expr);
        right[0] = '0';
        right[1] = '\0';
    }
}

// Chuyen bieu thuc trung to sang hau to
Token *infixToPostfix(char* myFunction){
    state_t current_state = S_START;
    Token *output = (Token *)malloc(MAX * sizeof(Token));
    int outputIndex = 0;
    char stack[MAX]; // Stack luu toan tu
    int stackTop = -1;

    while (1) {
        switch (current_state) {
           case S_START:
    if (*myFunction == '-' || *myFunction == '+') {
        int sign = getSign(&myFunction);
        if (isdigit(*myFunction) || *myFunction == 'x' || *myFunction == '(') {
            if (*myFunction == '(') {
                stack[++stackTop] = '(';
                myFunction++;
                if (*myFunction == '-' || *myFunction == '+') {
                    int signInner = getSign(&myFunction);
                    if (isdigit(*myFunction) || *myFunction == 'x') {
                        if (*myFunction == 'x') {
                            output[outputIndex].type = VARIABLE;
                            output[outputIndex].value.variable = 0;
                            if (signInner == -1)
                                output[outputIndex].value.operand = -1; // optional
                            outputIndex++;
                            myFunction++;
                        } else {
                            float operand = 0.0;
                            int decimal_flag = 0;
                            float decimal_divisor = 1.0;
                            while (isdigit(*myFunction) || *myFunction == '.') {
                                if (*myFunction == '.') {
                                    decimal_flag = 1;
                                } else {
                                    if (decimal_flag == 0) {
                                        operand = operand * 10 + (*myFunction - '0');
                                    } else {
                                        decimal_divisor *= 10;
                                        operand = operand + (*myFunction - '0') / decimal_divisor;
                                    }
                                }
                                myFunction++;
                            }
                            output[outputIndex].type = OPERAND;
                            output[outputIndex].value.operand = signInner * operand;
                            outputIndex++;
                        }
                    }
                    current_state = S_OPERATOR;
                } else {
                    current_state = S_START;
                }
            } else if (isdigit(*myFunction)) {
                current_state = S_OPERAND;
                float operand = 0.0;
                int decimal_flag = 0;
                float decimal_divisor = 1.0;
                while (isdigit(*myFunction) || *myFunction == '.') {
                    if (*myFunction == '.') {
                        decimal_flag = 1;
                    } else {
                        if (decimal_flag == 0) {
                            operand = operand * 10 + (*myFunction - '0');
                        } else {
                            decimal_divisor *= 10;
                            operand = operand + (*myFunction - '0') / decimal_divisor;
                        }
                    }
                    myFunction++;
                }
                output[outputIndex].type = OPERAND;
                output[outputIndex].value.operand = sign * operand;
                outputIndex++;

                if (isOperator(*myFunction)) {
                    current_state = S_OPERATOR;
                } else if (*myFunction == ')') {
                    current_state = S_CLOSE;
                } else if (*myFunction == 0) {
                    current_state = S_END;
                } else {
                    current_state = S_ERROR;
                }
            } else if (*myFunction == 'x') {
    if (sign == -1) {
        // Push -1 operand
        output[outputIndex].type = OPERAND;
        output[outputIndex].value.operand = -1;
        outputIndex++;
        
        // Push x variable
        output[outputIndex].type = VARIABLE;
        output[outputIndex].value.variable = 0;
        outputIndex++;

        // Push multiplication operator
        output[outputIndex].type = OPERATOR;
        output[outputIndex].value.operator_ = '*';
        outputIndex++;
    } else {
        output[outputIndex].type = VARIABLE;
        output[outputIndex].value.variable = 0;
        outputIndex++;
    }
    myFunction++;
    current_state = S_OPERATOR;
}
        } else {
            current_state = S_ERROR;
        }
    } else if (isdigit(*myFunction) || *myFunction == '.' || *myFunction == 'x') {
        current_state = S_OPERAND;
    } else if (*myFunction == '(') {
        current_state = S_OPEN;
    } else if (*myFunction == 0) {
        current_state = S_END;
    } else {
        current_state = S_ERROR;
    }
    break;
            case S_OPERAND:
    if (*myFunction == 'x') {
        output[outputIndex].type = VARIABLE;
        output[outputIndex].value.variable = 0;
        outputIndex++;
        myFunction++;  
    } else {
        float operand = 0.0;
        int decimal_flag = 0;
        float decimal_divisor = 1.0;
        while (isdigit(*myFunction) || *myFunction == '.') {
            if (*myFunction == '.') {
                decimal_flag = 1;
            } else {
                if (decimal_flag == 0) {
                    operand = operand * 10 + (*myFunction - '0');
                } else {
                    decimal_divisor *= 10;
                    operand = operand + (*myFunction - '0') / decimal_divisor;
                }
            }
            myFunction++;
        }
        output[outputIndex].type = OPERAND;
        output[outputIndex].value.operand = operand;
        outputIndex++;
    }

    if (isOperator(*myFunction) || *myFunction == '-') {
        current_state = S_OPERATOR;
    } else if (*myFunction == ')') {
        current_state = S_CLOSE;
    } else if (*myFunction == 0) {
        current_state = S_END;
    } else {
        current_state = S_ERROR;
    }
    break;

            case S_OPERATOR:
    while (stackTop >= 0 && isOperator(stack[stackTop]) &&
           ((precedence(stack[stackTop]) > precedence(*myFunction)) ||
            (precedence(stack[stackTop]) == precedence(*myFunction) && *myFunction != '^'))) {  
        output[outputIndex].type = OPERATOR;
        output[outputIndex].value.operator_ = stack[stackTop];
        outputIndex++;
        stackTop--;
    }
    stack[++stackTop] = *myFunction;
    myFunction++;
    current_state = S_START;
    break;

            case S_OPEN:
    stack[++stackTop] = *myFunction;
    myFunction++;

    if (isdigit(*myFunction) || *myFunction == 'x' || *myFunction == '(') {  
        current_state = S_START;
    } else if (*myFunction == '-') {  
        output[outputIndex].type = OPERAND;
        output[outputIndex].value.operand = 0;
        outputIndex++;
        current_state = S_OPERATOR;
    } else {
        current_state = S_ERROR;
    }
    break;

            case S_CLOSE:
    while (stackTop >= 0 && stack[stackTop] != '(') {
        output[outputIndex].type = OPERATOR;
        output[outputIndex].value.operator_ = stack[stackTop];
        outputIndex++;
        stackTop--;
    }
    if (stackTop >= 0) stackTop--; // Bat dau '(' khoi stack )
    myFunction++;

    if (isOperator(*myFunction)) {
        current_state = S_OPERATOR;
    } else if (*myFunction == ')') {
        current_state = S_CLOSE; // Neu c� ngoac dong tiep, xu ly tiep
    } else if (*myFunction == 0) {
        current_state = S_END;
    } else {
        current_state = S_ERROR; // Neu gap so hoac bien ngay sau ')', loi
    }
    break;


            case S_END:
                while (stackTop >= 0) {
                    output[outputIndex].type = OPERATOR;
                    output[outputIndex].value.operator_ = stack[stackTop];
                    outputIndex++;
                    stackTop--;
                }
                output[outputIndex].type = OPERATOR;
                output[outputIndex].value.operator_ = 'E'; // Ky hieu ket thuc
                outputIndex++;
                return output;

            case S_ERROR:
                printf("Error in expression!\n");
                free(output);
                return NULL;
        }
    }
}

// Hàm xử lý cơ số âm và mũ không nguyên
float handlePower(float base, float exponent) {
    int integerPart = (int)exponent;
    float decimalPart = exponent - integerPart;

    if (base >0) {
        // Nếu cơ số dương, tính bình thường
        return pow(base, exponent);
    } 
    else if (base == 0) {
        // Nếu cơ số bằng 0, trả về 0 nếu mũ dương
        return (exponent > 0) ? 0 : NAN; // Không xác định với mũ âm
    }
    // Nếu cơ số âm, xử lý phần nguyên và phần thập phân của mũ
    else {
        // Xử lý phần nguyên của mũ
        float result = pow(base, integerPart);

        if (decimalPart != 0) {
            int n = round(decimalPart*10+1);  // N giả định phần mũ có dạng 1/n

            // Kiểm tra căn chẵn hay lẻ
            if (n % 2 == 0) {
                // Căn chẵn của số âm không xác định
                return NAN;
            } else {
                float root = pow(fabs(base), decimalPart); // Tính căn
                result *= (base < 0 && n % 2 != 0) ? -root: root; // Nhân với kết quả trước đó
            }
        }
    return result;
    }
}

float evaluatePostfix(Token *postfix, float x_value) {
    float stack[MAX];
    int top = -1;
    for (int i = 0; postfix[i].type != OPERATOR || postfix[i].value.operator_ != 'E'; i++) {
        if (postfix[i].type == OPERAND) {
            stack[++top] = postfix[i].value.operand;
        } else if (postfix[i].type == VARIABLE) {
            stack[++top] = x_value;
        } else {
            float b = stack[top--];
            float a = stack[top--];
            switch (postfix[i].value.operator_) {
                case '+': stack[++top] = a + b; break;
                case '-': stack[++top] = a - b; break;
                case '*': stack[++top] = a * b; break;
                case '/':
                    if (fabs(b) < EPSILON) {
                        printf("Loi: chia cho 0 tai x = %f\n", x_value);
                        return NAN;
                    }
                    stack[++top] = a / b;
                    break;
                case '^': stack[++top] = handlePower(a, b); break;
            }
        }
    }
    return stack[top];
}

// Phuong phap chia doi (Bisection)
float bisection(Token *postfixLeft, Token *postfixRight, float a, float b) {
    float fa = evaluatePostfix(postfixLeft, a) - evaluatePostfix(postfixRight, a);
    float fb = evaluatePostfix(postfixLeft, b) - evaluatePostfix(postfixRight, b);
    
    if (fabs(fa) < EPSILON) return a;
    if (fabs(fb) < EPSILON) return b;

    if (fa * fb > 0) {
        printf("Loi: Ham khong doi dau trong doan [%f, %f]\n", a, b);
        return NAN;
    }

    for (int i = 0; i < MAX_ITER; i++) {
        float c = (a + b) / 2;
        float fc = evaluatePostfix(postfixLeft, c) - evaluatePostfix(postfixRight, c);

        if (fabs(fc) < EPSILON)
            return c;

        if (fa * fc < 0) {
            b = c;
            fb = fc;
        } else {
            a = c;
            fa = fc;
        }
    }
    return (a + b) / 2;  
}

float hybridMethod(Token *postfixLeft, Token *postfixRight, float a, float b, float x0) {
    float x = x0;
    for (int iter = 0; iter < MAX_ITER; iter++) {
        float f_left = evaluatePostfix(postfixLeft, x);
        float f_right = evaluatePostfix(postfixRight, x);
        float fx = f_left - f_right;

        if (fabs(fx) < EPSILON) return x;

        float dfx = ((evaluatePostfix(postfixLeft, x + EPSILON) - evaluatePostfix(postfixRight, x + EPSILON)) - fx) / EPSILON;
        
        if (fabs(dfx) < EPSILON) {
            printf("Dao ham gan bang 0, chuyen sang Bisection.\n");
            return bisection(postfixLeft, postfixRight, a, b);
        }
        
        float x_new = x - fx / dfx;

        if (x_new < a || x_new > b) {
            printf("Newton-Raphson ra ngoai khoang [%f, %f], dung Bisection.\n", a, b);
            return bisection(postfixLeft, postfixRight, a, b);
        }

        x = x_new;
    }
    
    printf("Newton-Raphson khong hoi tu, chuyen sang Bisection.\n");
    return bisection(postfixLeft, postfixRight, a, b);
}

void replaceSqrtWithPower(char *expr) {
    char temp[MAX];
    int i = 0, j = 0;

    while (expr[i] != '\0') {
        if (strncmp(&expr[i], "sqrt(", 5) == 0) { // Neu gap sqrt(
            i += 5; // Bo qua "sqrt("
            temp[j++] = '('; // Mo ngoac cho can
            while (expr[i] != '\0' && expr[i] != ')') { // Sao chep noi dung trong can
                temp[j++] = expr[i++];
            }
            temp[j++] = ')'; // �ong ngoac lai
            temp[j++] = '^'; // Them dau mu
            temp[j++] = '0';
            temp[j++] = '.';
            temp[j++] = '5'; // Them 0.5
        } else {
            temp[j++] = expr[i++];
        }
    }
    temp[j] = '\0';
    strcpy(expr, temp); // Cap nhat loi bieu thuc goc
}

int main() {
    char leftExpr[MAX], rightExpr[MAX], str[MAX];
    int nhapLai = 1; // Bien kiem soat viec nhap lai bieu thuc

    while (1) {
        if (nhapLai) { // Chi nhap bieu thuc moi neu nhapLai == 1
            printf("Nhap bieu thuc hoac phuong trinh: ");
            fgets(str, MAX, stdin);
            str[strcspn(str, "\n")] = 0;

            splitEquation(str, leftExpr, rightExpr);
            replaceSqrtWithPower(leftExpr);
            replaceSqrtWithPower(rightExpr);
        }

        Token *outputLeft = infixToPostfix(leftExpr);
        Token *outputRight = infixToPostfix(rightExpr);

        if (!outputLeft || !outputRight) {
            printf("Loi nhap bieu thuc!\n");
            nhapLai = 1; // Bat buoc nhap lai neu bieu thuc loi
            continue;
        }

        int choice;
        printf("\n1. Tinh gia tri x\n2. Giai phuong trinh\n3.Tinh gia tri bieu thuc\n4. Reset\nChon: ");
        scanf("%d", &choice);
        getchar();

        if (choice == 1) {
            float x;
            printf("Nhap gia tri x: ");
            scanf("%f", &x);
            printf("Gia tri bieu thuc: %.2f\n", evaluatePostfix(outputLeft, x));
            nhapLai = 0; // Khong yeu cau nhap lai bieu thuc
            continue;
        } else if (choice == 2) {
    double start = -50, end = 50, step = 0.5;
    bool found = false;

    for (double a = start; a < end; a += step) {
        double b = a + step;
        double fa = evaluatePostfix(outputLeft, a) - evaluatePostfix(outputRight, a);
        double fb = evaluatePostfix(outputLeft, b) - evaluatePostfix(outputRight, b);

        if (isnan(fa) || isnan(fb) || isinf(fa) || isinf(fb)) {
            continue; // B? qua kho?ng l?i
        }

        if (fa * fb < 0) {
            double x0 = (a + b) / 2;
            double root = hybridMethod(outputLeft, outputRight, a, b, x0);

            if (!isnan(root)) {
                printf("Nghi?m t�m du?c: x = %.2f\n", root);
                found = true;
                break; // T�m th?y nghi?m, tho�t
            }
        }
    }

    // Neu khong tim thay nghiem trong [-50, 50], mo rong pham vi tim kiem
    if (!found) {
        double start = -1e6, end = 1e6, step = 1000; // Bo sung khai bao bien

        for (double a = start; a < end; a += step) {
            double b = a + step;
            double fa = evaluatePostfix(outputLeft, a) - evaluatePostfix(outputRight, a);
            double fb = evaluatePostfix(outputLeft, b) - evaluatePostfix(outputRight, b);

            if (isnan(fa) || isnan(fb) || isinf(fa) || isinf(fb)) {
                continue; // Bo qua khoang loi
            }

            if (fa * fb < 0) {
               double x0 = (a + b) / 2;
                double root = hybridMethod(outputLeft, outputRight, a, b, x0);

                if (!isnan(root)) {
                    printf("Nghiem tim duoc: x = %.2f\n", root);
                    break; // Thoat ngay neu tim thay nghiem
                }
            }
        }
    }

    nhapLai = 0;
    continue;

        } else if (choice == 3) {
            printf("Gia tri bieu thuc: %.2f\n", evaluatePostfix(outputLeft, 2));
            nhapLai = 0; // Khong yeu cau nhap lai bieu thuc
            continue;
        } else if (choice == 4) {
            free(outputLeft);
            free(outputRight);
            nhapLai = 1; // Reset, yeu cau nhap lai bieu thuc
            continue;
        }

        free(outputLeft);
        free(outputRight);
    }

    return 0;
}


