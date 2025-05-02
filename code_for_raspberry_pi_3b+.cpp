#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#define MAX_ITER 100
#define MAX 100
#define EPSILON 1e-4 // Sai so 10^-4

#define LCD_ADDR 0x27  // �?a ch? I2C c?a LCD, n?u kh�ng ho?t d?ng th? 0x3F
#define LCD_BACKLIGHT 0x08
#define LCD_ENABLE 0x04
#define ACTION_SOLVE '!'   // k� hi?u d?c bi?t cho Solve
#define ACTION_RESET '@'   // k� hi?u d?c bi?t cho Reset

#define SDA_PIN 8  // Ch�n SDA
#define SCL_PIN 9  // Ch�n SCL

const int rowPins[4] = {5, 6, 7, 26};; // GPIO24, GPIO25, GPIO4,GPIO12
const int colPins[6] = {0, 1, 2, 3, 4,24};  // GPIO17, GPIO18, GPIO27, GPIO22, GPIO23, GPIO19

// Ma tr?n k� t? c?a keypad
char keys[4][6] = {
    {'=','7','4','1','+','D'},
    {'0','8','5','2','-','A'},
    {'^','9','6','3','*','S'},
    {'x','.','(',')','/','O'}
};

void i2c_start() {
    pinMode(SDA_PIN, OUTPUT);
    pinMode(SCL_PIN, OUTPUT);
    digitalWrite(SDA_PIN, LOW);
    delay(1);  // Thay th? usleep(5)
    digitalWrite(SCL_PIN, LOW);
}

void i2c_stop() {
    pinMode(SDA_PIN, OUTPUT);
    digitalWrite(SDA_PIN, LOW);
    digitalWrite(SCL_PIN, HIGH);
    delay(1);  // Thay th? usleep(5)
    digitalWrite(SDA_PIN, HIGH);
}

void i2c_write_byte(unsigned char data) {
    for (int i = 0; i < 8; i++) {
        digitalWrite(SDA_PIN, (data & 0x80) ? HIGH : LOW);
        delay(1);  // Thay th? usleep(5)
        digitalWrite(SCL_PIN, HIGH);
        delay(1);  // Thay th? usleep(5)
        digitalWrite(SCL_PIN, LOW);
        data <<= 1;
    }
    pinMode(SDA_PIN, INPUT); // Nh?n ACK
    delay(1);  // Thay th? usleep(5)
    digitalWrite(SCL_PIN, HIGH);
    delay(1);  // Thay th? usleep(5)
    digitalWrite(SCL_PIN, LOW);
    pinMode(SDA_PIN, OUTPUT);
}

void lcd_send_cmd(char cmd) {
    char data_high = cmd & 0xF0;
    char data_low = (cmd << 4) & 0xF0;
    
    i2c_start();
    i2c_write_byte(LCD_ADDR << 1);
    i2c_write_byte(data_high | LCD_BACKLIGHT | LCD_ENABLE);
    i2c_write_byte(data_high | LCD_BACKLIGHT);
    i2c_write_byte(data_low | LCD_BACKLIGHT | LCD_ENABLE);
    i2c_write_byte(data_low | LCD_BACKLIGHT);
    i2c_stop();
    delay(1);  // Thay th? usleep(500)
}

void lcd_send_data(char data) {
    char data_high = (data & 0xF0) | 0x01;
    char data_low = ((data << 4) & 0xF0) | 0x01;
    
    i2c_start();
    i2c_write_byte(LCD_ADDR << 1);
    i2c_write_byte(data_high | LCD_BACKLIGHT | LCD_ENABLE);
    i2c_write_byte(data_high | LCD_BACKLIGHT);
    i2c_write_byte(data_low | LCD_BACKLIGHT | LCD_ENABLE);
    i2c_write_byte(data_low | LCD_BACKLIGHT);
    i2c_stop();
    delay(1);  // Thay th? usleep(500)
}

void lcd_init() {
    lcd_send_cmd(0x33); // Kh?i d?ng LCD 4-bit mode
    lcd_send_cmd(0x32);
    lcd_send_cmd(0x28); // Ch? d? 2 d�ng, 5x8 font
    lcd_send_cmd(0x0C); // B?t m�n h�nh, t?t con tr?
    lcd_send_cmd(0x06); // Ch? d? nh?p t? d?ng tang
    lcd_send_cmd(0x01); // X�a m�n h�nh
    delay(5);
}

void lcd_set_cursor(int row, int col) {
    int offsets[] = {0x80, 0xC0};
    lcd_send_cmd(offsets[row] + col);
}

void lcd_print(const char *str) {
    while (*str) {
        lcd_send_data(*str++);
    }
}
void keypad_init() {
    for (int i = 0; i < 4; i++) { // 4 h�ng
        pinMode(rowPins[i], OUTPUT);
        digitalWrite(rowPins[i], HIGH); // �?t m?c d?nh HIGH
    }
    for (int j = 0; j < 6; j++) { // 5 c?t
        pinMode(colPins[j], INPUT);
        pullUpDnControl(colPins[j], PUD_UP); // D�ng pull-up thay v� pull-down
    }
}

// Qu�t b�n ph�m v� tr? v? k� t? du?c nh?n
char keypad_get_key() {
    for (int row = 0; row < 4; row++) { 
        for (int i = 0; i < 4; i++) 
            digitalWrite(rowPins[i], HIGH); // �?t t?t c? h�ng v? HIGH
        
        digitalWrite(rowPins[row], LOW); // K�ch ho?t h�ng c?n ki?m tra
        
        for (int col = 0; col < 6; col++) { 
            if (digitalRead(colPins[col]) == LOW) { // N?u ph�t hi?n nh?n ph�m
                delay(20); // Ch?ng rung ph�m (debounce)
                if (digitalRead(colPins[col]) == LOW) { // Ki?m tra l?i
                    while (digitalRead(colPins[col]) == LOW); // Ch? th? ph�m
                    digitalWrite(rowPins[row], HIGH); // Reset h�ng
                    return keys[row][col]; // Tr? v? k� t? nh?n
                }
            }
        }
    }
    return '\0'; // Kh�ng c� ph�m n�o du?c nh?n
}
char convert_key_to_char(char key) {
    switch (key) {
        case '0': return '0';
        case '1': return '1';
        case '2': return '2';
        case '3': return '3';
        case '4': return '4';
        case '5': return '5';
        case '6': return '6';
        case '7': return '7';
        case '8': return '8';
        case '9': return '9';
        case '+': return '+';
        case '-': return '-';
        case '*': return '*';
        case '/': return '/';
        case '=': return '=';
        case '(': return '(';
        case ')': return ')';
        case '.': return '.';
        case '^': return '^';
        case 'x': return 'x';
        default: return '\0'; // K� t? kh�ng h?p l?
    }
}

void keypad_get_string(char *str, int maxLen) {
    int idx = 0;
    int col = 0;
    char displayBuffer[17] = {0}; // luu d�ng dang hi?n th?

    lcd_set_cursor(1, 0);

    while (idx < maxLen - 1) {
        char key = keypad_get_key();

        if (key != '\0') {
            // X? l� c�c ph�m ch?c nang d?c bi?t
            if (key == 'D') { // X�a 1 k� t?
                if (idx > 0) {
                    idx--;
                    col = col > 0 ? col - 1 : 0;
                    displayBuffer[col] = '\0';
                    str[idx] = '\0';

                    lcd_set_cursor(1, 0);
                    for (int i = 0; i < 16; i++) {
                        lcd_send_data(displayBuffer[i] ? displayBuffer[i] : ' ');
                    }
                    lcd_set_cursor(1, col);
                }
            } else if (key == 'A') { // X�a t?t c?
                idx = 0;
                col = 0;
                str[0] = '\0';
                memset(displayBuffer, 0, sizeof(displayBuffer));
                lcd_set_cursor(1, 0);
                for (int i = 0; i < 16; i++) lcd_send_data(' ');
                lcd_set_cursor(1, 0);
            } else if (key == 'S') { // Gi?i phuong tr�nh
                str[idx] = ACTION_SOLVE;
                str[idx + 1] = '\0';
                return;
            } else if (key == 'O') { // Reset b? nh?
                str[idx] = ACTION_RESET;
                str[idx + 1] = '\0';
                return;
            } else {
                // Th�m k� t? h?p l?
                char convertedKey = convert_key_to_char(key);
                if (convertedKey != '\0') {
                    if (col >= 16) {
                        for (int i = 0; i < 15; i++) {
                            displayBuffer[i] = displayBuffer[i + 1];
                        }
                        displayBuffer[15] = convertedKey;
                    } else {
                        displayBuffer[col++] = convertedKey;
                    }

                    str[idx++] = convertedKey;
                    str[idx] = '\0';

                    lcd_set_cursor(1, 0);
                    for (int i = 0; i < 16; i++) {
                        lcd_send_data(displayBuffer[i] ? displayBuffer[i] : ' ');
                    }

                    lcd_set_cursor(1, col < 16 ? col : 15);
                }
            }
        }

        delay(100); // ch?ng rung
    }

    str[idx] = '\0';
}

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

Token *infixToPostfix(char* myFunction){
    state_t current_state = S_START;
    Token *output = (Token *)malloc(MAX * sizeof(Token));
    int outputIndex = 0;
    char stack[MAX]; // Stack luu toan tu
    int stackTop = -1;

    while (1) {
        switch (current_state) {
           case S_START:
    if (*myFunction == '-') {  
        output[outputIndex].type = OPERAND;
        output[outputIndex].value.operand = 0;  // Them so 0 truoc dau tru
        outputIndex++;
        current_state = S_OPERATOR;
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
                printf("Loi nhap bieu thuc!!!\n");
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
            int n = round(1.0 / decimalPart);  // N giả định phần mũ có dạng 1/n

            // Kiểm tra căn chẵn hay lẻ
            if (n % 2 == 0) {
                return NAN;
            } else {
                float root = pow(fabs(base), decimalPart); // Tính căn
                result *= (base < 0 && n % 2 != 0) ? -root: root; // Nhân với kết quả trước đó
            }
        }
    return result;
    }
}

// Tinh gia tri cua bieu thuc hau to
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
                case '/': stack[++top] = a / b; break;
                case '^': stack[++top] = handlePower(a, b); break;
            }
        }
    }
    return stack[top];
}

// Phuong ph�p chia d�i (Bisection)
float bisection(Token *postfixLeft, Token *postfixRight, float a, float b) {
    float fa = evaluatePostfix(postfixLeft, a) - evaluatePostfix(postfixRight, a);
    float fb = evaluatePostfix(postfixLeft, b) - evaluatePostfix(postfixRight, b);
    
    if (fabs(fa) < EPSILON) return a;
    if (fabs(fb) < EPSILON) return b;

    if (fa * fb > 0) {
        return NAN; // Tr? v? gi� tr? l?i thay v� NAN
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

// Phuong ph�p lai (Hybrid Method)
float hybridMethod(Token *postfixLeft, Token *postfixRight, float a, float b, float x0) {
    float x = x0;
    for (int iter = 0; iter < MAX_ITER; iter++) {
        float f_left = evaluatePostfix(postfixLeft, x);
        float f_right = evaluatePostfix(postfixRight, x);
        float fx = f_left - f_right;

        if (fabs(fx) < EPSILON) return x;

        // T�nh d?o h�m x?p x?
        float dfx = ((evaluatePostfix(postfixLeft, x + EPSILON) - evaluatePostfix(postfixRight, x + EPSILON)) - fx) / EPSILON;
        
        if (fabs(dfx) < EPSILON) {
            return bisection(postfixLeft, postfixRight, a, b);
        }
        
        float x_new = x - fx / dfx;

        if (x_new < a || x_new > b) {
            return bisection(postfixLeft, postfixRight, a, b);
        }

        x = x_new;
    }
    return bisection(postfixLeft, postfixRight, a, b);
}

int main() {
    char leftExpr[MAX] = "", rightExpr[MAX] = "", str[MAX];
    Token *outputLeft = NULL;
    Token *outputRight = NULL;

    if (wiringPiSetup() == -1) {
        printf("Khong the khoi tao wiringPi\n");
        return 1;
    }

    lcd_init();
    keypad_init();

    while (1) {
    lcd_send_cmd(0x01);
    lcd_set_cursor(0, 0);
    char *msg = "Nhap bieu thuc:";
    for (int i = 0; msg[i] != '\0'; i++) {
        lcd_send_data(msg[i]);
    }

    keypad_get_string(str, MAX);

    if (str[0] == ACTION_RESET) {
        if (outputLeft) {
            free(outputLeft);
            outputLeft = NULL;
        }
        if (outputRight) {
            free(outputRight);
            outputRight = NULL;
        }
        continue; // Quay l?i nh?p t? d?u
    }

    if (str[0] == ACTION_SOLVE || strchr(str, ACTION_SOLVE)) {
        char exprOnly[MAX];
        strcpy(exprOnly, str);

        // B? ACTION_SOLVE d?u chu?i
        if (exprOnly[0] == ACTION_SOLVE) {
            memmove(exprOnly, exprOnly + 1, strlen(exprOnly));
        }

        // Lo?i b? t?t c? ACTION_SOLVE trong chu?i
        int j = 0;
        for (int i = 0; exprOnly[i] != '\0'; i++) {
            if (exprOnly[i] != ACTION_SOLVE)
                exprOnly[j++] = exprOnly[i];
        }
        exprOnly[j] = '\0';

        // T�ch bi?u th?c
        splitEquation(exprOnly, leftExpr, rightExpr);

        if (outputLeft) free(outputLeft);
        if (outputRight) free(outputRight);
        outputLeft = infixToPostfix(leftExpr);
        outputRight = infixToPostfix(rightExpr);

        if (!outputLeft || !outputRight) {
            lcd_send_cmd(0x01);
            lcd_set_cursor(0, 0);
            char *errMsg = "Bieu thuc sai!";
            for (int i = 0; errMsg[i] != '\0'; i++) {
                lcd_send_data(errMsg[i]);
            }
            delay(2000);
            continue;
        }

        lcd_send_cmd(0x01);
        lcd_set_cursor(0, 0);
        char *gptMsg = "GPT";
        for (int i = 0; gptMsg[i] != '\0'; i++) {
            lcd_send_data(gptMsg[i]);
        }
        double a = -50, b = 50, x0;
        bool found = false;
        clock_t start_time = clock(); // B?t d?u do th?i gian
        double root = hybridMethod(outputLeft, outputRight, a, b, x0);
		clock_t end_time = clock(); // K?t th�c do th?i gian
		int elapsed_time = (int)((end_time - start_time) * 1000 / CLOCKS_PER_SEC); // Th?i gian ms	
        
		if (!isnan(root)) {
            lcd_set_cursor(0, 5);
            char resStr[16];
            snprintf(resStr, 16, "x=%.2f", root);
            for (int i = 0; resStr[i] != '\0'; i++) {
                lcd_send_data(resStr[i]);
            }
             lcd_set_cursor(1, 0);
    		snprintf(resStr, 16, "Time: %.5dms", elapsed_time);
    		for (int i = 0; resStr[i] != '\0'; i++) {
            lcd_send_data(resStr[i]);
    }

            found = true;
            
        } 
        if (!found) {
            double start = -1e6, end = 1e6, step = 1000;
            for (double a = start; a < end; a += step) {
                double b = a + step;
                double fa = evaluatePostfix(outputLeft, a) - evaluatePostfix(outputRight, a);
                double fb = evaluatePostfix(outputLeft, b) - evaluatePostfix(outputRight, b);
                if (isnan(fa) || isnan(fb) || isinf(fa) || isinf(fb))
                    continue;
                if (fa * fb < 0) {
                    x0 = (a + b) / 2;
                    // B?t d?u do th?i gian
    				clock_t start_time = clock();
                    root = hybridMethod(outputLeft, outputRight, a, b, x0);
                    // K?t th�c do th?i gian
    				clock_t end_time = clock();
    				int elapsed_time = (int)((end_time - start_time) * 1000 / CLOCKS_PER_SEC);

                    if (!isnan(root)) {
                        lcd_set_cursor(0, 5);
                        char resStr[16];
                        snprintf(resStr, 16, "x=%.2f", root);
                        for (int i = 0; resStr[i] != '\0'; i++) {
                            lcd_send_data(resStr[i]);
                        }
                        // D�ng 2: hi?n th? th?i gian
				lcd_set_cursor(1, 0);
				snprintf(resStr, 16, "Time: %.5dms", (int)elapsed_time);
				for (int i = 0; resStr[i] != '\0'; i++) {
    			lcd_send_data(resStr[i]);
}
                        found = true;
                        break;
                    }
                }
            }
        }

        
    
        // Ch? nh?n RESET m?i quay l?i nh?p bi?u th?c
        while (1) {
            keypad_get_string(str, MAX);
            if (str[0] == ACTION_RESET) {
                if (outputLeft) {
                    free(outputLeft);
                    outputLeft = NULL;
                }
                if (outputRight) {
                    free(outputRight);
                    outputRight = NULL;
                }
                break; // Tho�t v�ng l?p ch? reset -> quay l?i nh?p bi?u th?c
            }
        }
    }
}
}
