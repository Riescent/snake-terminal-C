#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>

#define SNAKE_ICON 'O'
#define POINT '*'
#define WAIT_TIME 2000

struct winsize grid_size;

void	create_grid(char (*grid)[grid_size.ws_row][grid_size.ws_col]);
void	add_point_on_grid(char (*grid)[grid_size.ws_row][grid_size.ws_col]);
void	create_snake(char (*grid)[grid_size.ws_row][grid_size.ws_col]);
void	wait_for_user_to_start(void);
void	print_grid(char (*grid)[grid_size.ws_row][grid_size.ws_col]);
void	clear_terminal(void);
void	is_it_game_over(char (*grid)[grid_size.ws_row][grid_size.ws_col]);
void	move_snake(char (*grid)[grid_size.ws_row][grid_size.ws_col]);
void	grow_snake(int y, int x);
void	get_direction(void);
bool	eat_point(char (*grid)[grid_size.ws_row][grid_size.ws_col]);
char	get_char(void);

typedef struct snake {
	int y;
	int x;
	struct snake *next;
} snake;

snake *head = NULL;
snake *tail = NULL;
char direction;
char previous_direction;
int score;
bool point_eaten;

int		main(void) {
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &grid_size);
	grid_size.ws_row -= 5;
	grid_size.ws_col -= 2;
	char grid[grid_size.ws_row][grid_size.ws_col];
	srand(time(NULL));
	score = 0;
	head = NULL;
	tail = NULL;
	point_eaten = false;
	direction = '\0';

	create_grid(&grid);
	create_snake(&grid);
	add_point_on_grid(&grid);
	print_grid(&grid);
	wait_for_user_to_start();
	//direction = 'w';
	while(true) {
		move_snake(&grid);
		previous_direction = direction;
		if (point_eaten)
			add_point_on_grid(&grid);
		print_grid(&grid);
		for (unsigned int i = 0; i < 16000 - (score * 500); i++)
			get_direction();
	}
}

bool	eat_point(char (*grid)[grid_size.ws_row][grid_size.ws_col]) {
	switch (direction)
	{
	case 'w':
		if ((*grid)[head->y - 1][head->x] == POINT) {
			score++;
			return true;
		}
		break;
	case 'a':
		if ((*grid)[head->y][head->x - 1] == POINT || (*grid)[head->y][head->x - 2] == POINT) {
			score++;
			return true;
		}
		break;
	case 's':
		if ((*grid)[head->y + 1][head->x] == POINT) {
			score++;
			return true;
		}
		break;
	case 'd':
		if ((*grid)[head->y][head->x + 1] == POINT || (*grid)[head->y][head->x + 2] == POINT) {
			score++;
			return true;
		}
		break;
	default:
		printf("ERROR line %i", __LINE__);
		exit(1);
	}
	return false;
}

void delete_coordinates(int y, int x, char (*grid)[grid_size.ws_row][grid_size.ws_col]) {
	if ((*grid)[y][x] == 'O')
		(*grid)[y][x] = ' ';
}

void	move_snake(char (*grid)[grid_size.ws_row][grid_size.ws_col]) {
	//Update linked list
	point_eaten = eat_point(grid);
	snake *next = tail->next;
	if (point_eaten)
		grow_snake(tail->y, tail->x);
	else {
		delete_coordinates(tail->y, tail->x, grid);
		if (direction == 'd' || previous_direction == 'd')
			delete_coordinates(tail->y, tail->x - 1, grid);
		else if (direction == 'a' || previous_direction == 'a')
			delete_coordinates(tail->y, tail->x + 1, grid);
		if (next != NULL)
			if (next->y == tail->y) {
				if (next->x > tail->x)
					delete_coordinates(tail->y, tail->x + 1, grid);
				else
					delete_coordinates(tail->y, tail->x - 1, grid);
			}
	}

	snake *current = tail;
	next = tail->next;

	if (score != 0) {
		current->y = next->y;
		current->x = next->x;

		for (int i = 0; i < score - 1; i++) {
			current = next;
			next = next->next;
			current->y = next->y;
			current->x = next->x;
		}
	}

	switch (direction)
	{
	case 'w':
		head->y--;
		break;
	case 'a':
		head->x -= 2;
		break;
	case 's':
		head->y++;
		break;
	case 'd':
		head->x += 2;
		break;
	}

	is_it_game_over(grid);
		if (direction == 'd')
			(*grid)[head->y][head->x - 1] = SNAKE_ICON;
		else if (direction == 'a')
			(*grid)[head->y][head->x + 1] = SNAKE_ICON;
		(*grid)[head->y][head->x] = SNAKE_ICON;

}

void	grow_snake(int y, int x) {
	snake *new_element = (snake*) malloc(sizeof(snake));

	new_element->y = y;
	new_element->x = x;

	new_element->next = tail;
	tail = new_element;
}

void	is_it_game_over(char (*grid)[grid_size.ws_row][grid_size.ws_col]) {
	bool game_over = false;

	if (head->y > grid_size.ws_row - 1 || head->y < 0
		|| head->x > grid_size.ws_col - 1 || head->x < 0)
		game_over = true;
	else if ((*grid)[head->y][head->x] == SNAKE_ICON)
		game_over = true;

	if (game_over) {
		char character = '\0';
		clear_terminal();
		printf("GAME OVER\nScore: %i\nType r to restart, anything else to stop\n", score);
		sleep(1);
		printf(" ");
		while (character == '\0')
			character = get_char();
			usleep(WAIT_TIME);
		if (character == 'r') {
			if (score < 1) {
				free(tail);
				tail = NULL;
				head = NULL;
			} else {
				snake *next = tail->next;
				snake *remove_me = tail;

				for (int i = 0; i < score + 1; i++) {
					free(remove_me);
					remove_me = next;
					if (next->next == NULL)
						break;
					next = next->next;
				}
			}
			main();
		}
		clear_terminal();
		exit(0);
	}
}

void	clear_terminal(void) {
	printf("\e[1;1H\e[2J");
}

void	print_grid(char (*grid)[grid_size.ws_row][grid_size.ws_col]) {

	clear_terminal();
	for (int x = 0; x < grid_size.ws_col; x++)
		printf("_");
	printf("\n");
	for (int y = 0; y < grid_size.ws_row; y++) {
		printf("|");
		for (int x = 0; x < grid_size.ws_col; x++)
			printf("%c", (*grid)[y][x]);
		printf("|\n");
	}
	for (int x = 0; x < grid_size.ws_col; x++)
		printf("_");
	printf("\n");
	for (int i = 0; i < grid_size.ws_col / 2 - 4; i++)
		printf(" ");
	printf("Score: %i\n", score);
	for (int i = 0; i < grid_size.ws_col / 2 - 20; i++)
		printf(" ");
	printf("W -> up, S -> down, A -> left, D -> right\n");
}

// returns character if a key is being pressed, else returns '\0'
char	get_char(void) {
	struct termios oldt, newt;
	int oldf;
	char ch;

	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

	ch = getchar();

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);

	if (ch != EOF)
		return (ch);
	else
		return ('\0');
}

void	get_direction(void) {
	char c = get_char();

	if ((c == 'w' && previous_direction != 's') || (c == 'a' && previous_direction != 'd')
		|| (c == 's' && previous_direction != 'w') || (c == 'd' && previous_direction != 'a')) {
		direction = c;
	}
}

void	wait_for_user_to_start(void) {
	while (direction != 'w' && direction != 'a' && direction != 's' && direction != 'd')
		get_direction();
		usleep(WAIT_TIME);
}

void	create_snake(char (*grid)[grid_size.ws_row][grid_size.ws_col]) {
	int x = grid_size.ws_col / 2;
	if (x % 2 == 1)
		x++;
	grow_snake(grid_size.ws_row / 2 , x);
	head = tail;
	(*grid)[head->y][head->x] = SNAKE_ICON;
}

void	add_point_on_grid(char (*grid)[grid_size.ws_row][grid_size.ws_col]) {
	int y = rand() % grid_size.ws_row;
	int x = rand() % grid_size.ws_col;

	while ((*grid)[y][x] != ' ' || x % 2 == 1) {
		y = rand() % grid_size.ws_row;
		x = rand() % grid_size.ws_col;
	}
	(*grid)[y][x] = POINT;
}

void	create_grid(char (*grid)[grid_size.ws_row][grid_size.ws_col]) {
	for (int y = 0; y < grid_size.ws_row; y++)
		for (int x = 0; x < grid_size.ws_col; x++)
			(*grid)[y][x] = ' ';
}
