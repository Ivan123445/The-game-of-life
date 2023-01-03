#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>
#include <ctype.h>

// поле
#define WIDE_FIELD 80
#define HEIGHT_FIELD 25
#define FLOOR_SYMB '-'
#define ELEM_SYMB '@'
#define WALL_SYMB "|"
#define SPASE_SYMB " "
// настройки рождения и смерти
#define COL_TO_BORN 3
#define COL_TO_LIFE_MIN 2
#define COL_TO_LIFE_MAX 3
// скорость игры
#define TIME_ONE_STEP 20000
#define SPEED_CHANGE 500
#define MAX_SPEED 500
#define MIN_SPEED 100000
// управление
#define EXIT_SYMB 'q'
#define SPEED_INC_SYMB 'a'
#define SPEED_DEC_SYMB 'z'
// флаги
#define ERROR 0

// память
int **array_array_alloc(int rows, int cols);
void array_array_free(int **ptr, int rows);
int **copy_field(int **origin, int **copy);

// основные
void render(int **field, int speed);
void render_floor();
void render_data(int speed);
void render_error(int flag);
int find_neighbour(int **field, int col, int row);
int update_field(int **field, int **temp_field);
int **init_field(int *flag);
int read_symb(int *speed, int *flag);


int main() {
    int **field, **temp_field, **just_pointer;
    int speed = TIME_ONE_STEP, flag = 1;

    initscr();
    nodelay(stdscr, 1);

    field = init_field(&flag);
    render_error(flag);
    temp_field = array_array_alloc(HEIGHT_FIELD, WIDE_FIELD);
    if (freopen("/dev/tty", "r", stdin) == 0 ) { flag = 0; }


    while (flag) {
        // вывод
        clear();
        render(field, speed);
        refresh();
        // изменение поля
        update_field(field, temp_field);
        just_pointer = field;
        field = temp_field;
        temp_field = just_pointer;
        // чтение символов
        for (int i = 0; i < 10; ++i) {
            read_symb(&speed, &flag);
            usleep(speed);
        }
    }

    array_array_free(field, HEIGHT_FIELD);
    array_array_free(temp_field, HEIGHT_FIELD);
    endwin();
    return 0;
}

int **init_field(int *flag) {
    int temp;
    int **field = array_array_alloc(HEIGHT_FIELD, WIDE_FIELD);
    for (int i = 0; i < HEIGHT_FIELD; ++i) {
        for (int j = 0; j < WIDE_FIELD; ++j) {
            if (scanf("%d", &temp) != 1 || (temp != 1 && temp != 0)) {
                *flag = ERROR;
                i = HEIGHT_FIELD;
                j = WIDE_FIELD;
            } else {
                field[i][j] = temp;
            }
        }
    }
    return field;
}

void render(int **field, int speed) {
    char point = ELEM_SYMB;
    render_floor();
    for (int i = 0; i < HEIGHT_FIELD; ++i) {
        printw(WALL_SYMB);
        for (int j = 0; j < WIDE_FIELD; ++j) {
            if (field[i][j] == 1) {
                printw("%1c", point);
                continue;
            }
            printw(SPASE_SYMB);
        }
        printw(WALL_SYMB);
        if (i < HEIGHT_FIELD - 1) { printw("\n"); }
    }
    render_floor();
    render_data(speed);
}

void render_floor() {
    int floor = FLOOR_SYMB;
    printw("\n");
    for (int i = 0; i < WIDE_FIELD; ++i) { printw("%1c", floor); }
    printw("\n");
}

void render_data(int speed) {
    char exit = EXIT_SYMB, speed_inc = SPEED_INC_SYMB, speed_dec = SPEED_DEC_SYMB;
    printw("Speed: %d\n", ((MIN_SPEED - speed) / SPEED_CHANGE));
    printw("Cintrol: \n Exit - %c\n Speed increase - %c\n Speed decrease - %c\n", exit, speed_inc, speed_dec);
}

void render_error(int flag) {
    if (flag == 0) {
        printw("n/a");
        refresh();
    }
}

int read_symb(int *speed, int *flag) {
    switch (tolower(getch())) {
        case SPEED_INC_SYMB:
            *speed > SPEED_CHANGE && *speed - SPEED_CHANGE > MAX_SPEED ? *speed -= SPEED_CHANGE : 0;
            break;
        case SPEED_DEC_SYMB:
            *speed < MIN_SPEED ? *speed += SPEED_CHANGE : 0;
            break;
        case EXIT_SYMB:
            *flag = 0;
            break;
    }
    return 0;
}

int find_neighbour(int **field, int point_col, int point_row) {
    int check_col, check_row;  // проверяемые в цикле точки
    int sum = 0;

    field[point_col][point_row] == 1 ? sum -= 1 : 0;         // если сама точка 1, то учитываем это
    for (int i = point_col - 1; i < point_col + 2; ++i) {
        for (int j = point_row - 1; j < point_row + 2; ++j) {
            check_col = i;
            check_row = j;
            check_col == HEIGHT_FIELD ? check_col = 0 : 1;  // 4 строчки для перехода через край поля
            check_row == WIDE_FIELD ? check_row = 0 : 1;
            check_col == -1 ? check_col = HEIGHT_FIELD - 1 : 1;
            check_row == -1 ? check_row = WIDE_FIELD - 1 : 1;

            field[check_col][check_row] == 1 ? sum += 1 : 1;  // если нашли соседа, увеличиваем счетчик
        }
    }

    return sum;
}

int update_field(int **field, int **temp_field) {
    int neighbours;
    temp_field = copy_field(field, temp_field);

    for (int i = 0; i < HEIGHT_FIELD; ++i) {
        for (int j = 0; j < WIDE_FIELD; ++j) {
            neighbours = find_neighbour(field, i, j);

            neighbours == COL_TO_BORN ? temp_field[i][j] = 1 : 0;  // рождение
            (neighbours < COL_TO_LIFE_MIN || neighbours > COL_TO_LIFE_MAX)
            ? temp_field[i][j] = 0 : 1;  // смерть
        }
    }

    return 0;
}

int **copy_field(int **origin, int **copy) {
    for (int i = 0; i < HEIGHT_FIELD; ++i)
        for (int j = 0; j < WIDE_FIELD; ++j)
            copy[i][j] = origin[i][j];

    return copy;
}

int **array_array_alloc(int rows, int cols) {
    int **ptr = calloc(rows, sizeof(int *));
    for (int i = 0; i < rows; ++i) {
        ptr[i] = calloc(cols, sizeof(int));
    }
    return ptr;
}

void array_array_free(int **ptr, int rows) {
    if (ptr != NULL) {
        for (int i = 0; i < rows; ++i) {
            if (ptr[i] != NULL) {
                free(ptr[i]);
            }
        }
        free(ptr);
    }
}
