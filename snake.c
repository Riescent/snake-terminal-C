#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#define MAX_Y 55
#define MAX_X 202
#define SNAKE_ICON 'O'
#define POINT '*'

void	create_grid(void);
void	add_point_on_grid(void);
void	create_snake(void);
void	wait_for_user_to_start(void);
void	print_grid(void);
void	clear_terminal(void);
void	is_it_game_over(void);
void	move_snake(void);
void	grow_snake(int y, int x);
void	get_direction(void);
bool	eat_point(void);
char	get_char(void);

typedef struct snake {
	int y;
	int x;
	struct snake *next;
} snake;

char grid[MAX_Y][MAX_X];
snake *head = NULL;
snake *tail = NULL;
char direction;
char previous_direction;
int score;
bool point_eaten;

int		main(void) {
	srand(time(NULL));
	score = 0;
	head = NULL;
	tail = NULL;
	point_eaten = false;
	direction = '\0';

	create_grid();
	create_snake();
	add_point_on_grid();
	print_grid();
	wait_for_user_to_start();
	//direction = 'w';
	while(true) {
		move_snake();
		previous_direction = direction;
		if (point_eaten)
			add_point_on_grid();
		print_grid();
		for (unsigned int i = 0; i < 16000; i++)
			get_direction();
	}
}

bool	eat_point(void) {
	switch (direction)
	{
	case 'w':
		if (grid[head->y - 1][head->x] == POINT) {
			score++;
			return true;
		}
		break;
	case 'a':
		if (grid[head->y][head->x - 1] == POINT || grid[head->y][head->x - 2] == POINT) {
			score++;
			return true;
		}
		break;
	case 's':
		if (grid[head->y + 1][head->x] == POINT) {
			score++;
			return true;
		}
		break;
	case 'd':
		if (grid[head->y][head->x + 1] == POINT || grid[head->y][head->x + 2] == POINT) {
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

void delete_coordinates(int y, int x) {
	if (grid[y][x] == 'O')
		grid[y][x] = ' ';
}

void	move_snake(void) {
	//Update linked list
	point_eaten = eat_point();
	snake *next = tail->next;
	if (point_eaten)
		grow_snake(tail->y, tail->x);
	else {
		delete_coordinates(tail->y, tail->x);
		if (direction == 'd' || previous_direction == 'd')
			delete_coordinates(tail->y, tail->x - 1);
		else if (direction == 'a' || previous_direction == 'a')
			delete_coordinates(tail->y, tail->x + 1);
		if (next != NULL)
			if (next->y == tail->y) {
				if (next->x > tail->x)
					delete_coordinates(tail->y, tail->x + 1);
				else
					delete_coordinates(tail->y, tail->x - 1);
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

	is_it_game_over();
		if (direction == 'd')
			grid[head->y][head->x - 1] = SNAKE_ICON;
		else if (direction == 'a')
			grid[head->y][head->x + 1] = SNAKE_ICON;
		grid[head->y][head->x] = SNAKE_ICON;

}

void	grow_snake(int y, int x) {
	snake *new_element = (snake*) malloc(sizeof(snake));

	new_element->y = y;
	new_element->x = x;

	new_element->next = tail;
	tail = new_element;
}

void	is_it_game_over(void) {
	bool game_over = false;

	if (head->y > MAX_Y - 1 || head->y < 0
		|| head->x > MAX_X - 1 || head->x < 0)
		game_over = true;
	else if (grid[head->y][head->x] == SNAKE_ICON)
		game_over = true;
	/*else if (grid[head->y][head->x - 1] == SNAKE_ICON && direction == 'd')
		game_over = true;
	else if (grid[head->y][head->x + 1] == SNAKE_ICON && direction == 'a')
		game_over = true;*/

	if (game_over) {
		char character = '\0';
		clear_terminal();
		printf("GAME OVER\nScore: %i\nType r to restart, anything else to stop\n", score);
		while (character == '\0')
			character = get_char();
			usleep(2000);
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

void	print_grid(void) {

	clear_terminal();
	for (int x = 0; x < MAX_X; x++)
		printf("_");
	printf("\n");
	for (int y = 0; y < MAX_Y; y++) {
		printf("|");
		for (int x = 0; x < MAX_X; x++)
			printf("%c", grid[y][x]);
		printf("|\n");
	}
	for (int x = 0; x < MAX_X; x++)
		printf("_");
	printf("\n");
	for (int i = 0; i < MAX_X / 2 - 4; i++)
		printf(" ");
	printf("Score: %i\nW -> up\nS -> down\nA -> left\nD -> right\n", score);
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
		usleep(2000);
}

void	create_snake(void) {
	grow_snake(MAX_Y / 2 , MAX_X / 2);
	head = tail;
	grid[head->y][head->x] = SNAKE_ICON;
}

void	add_point_on_grid(void) {
	int y = rand() % MAX_Y;
	int x = rand() % MAX_X;

	while (grid[y][x] != ' ') {
		y = rand() % MAX_Y;
		x = rand() % MAX_X;
	}
	grid[y][x] = POINT;
}

void	create_grid(void) {
	for (int y = 0; y < MAX_Y; y++)
		for (int x = 0; x < MAX_X; x++)
			grid[y][x] = ' ';
}
