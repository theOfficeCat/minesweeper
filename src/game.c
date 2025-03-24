#include "game.h"
#include "data.h"
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>

struct game_data
create_board(int rows, int columns)
{
	struct game_data data;
	u_int8_t *board = malloc(sizeof(u_int8_t)*columns*rows);

	data.rows = rows;
	data.columns = columns;

	int qtt = (columns*rows)*15/100 + 1; /* proporción de 15% de casillas
					      * con mina + 1 extra en caso de
					      * que hayan menos de 15 casillas
					      */

	for (int i = 0; i < columns*rows; ++i) {
		board[i] = 0;
	}

	data.intern_board = board;

	for (int i = 0; i < qtt; ++i) {
		add_mine(&data);
	}


	return data;
}

void
add_mine(struct game_data *data)
{
	/* generar posición aleatoria en el mapa */
	int pos_y = rand()%(*data).rows;
	int pos_x = rand()%(*data).columns;

	if (data->intern_board[pos_y*data->columns + pos_x] & 0b10000000) {
		add_mine(data);
		return;
	}

	data->intern_board[pos_y*data->columns + pos_x] = 0b10000000;

	/* relleno de casillas contiguas a la mina */
	for (int i = -1; i < 2; ++i) {
		/* recorrido de filas contiguas */
		for (int j = -1; j < 2; ++j) {
			/* recorrido de columnas contiguas */
			if (pos_y + i < 0 || pos_y + i >= data->rows ||
			    pos_x + j < 0 || pos_x + j >= data->columns ||
			    (data->intern_board[(pos_y + i)*data->columns +
			    pos_x + j] & 0b10000000) == 1) {
				continue;
			}

			data->intern_board[(pos_y + i)*data->columns + pos_x
			 		     + j] += 0b00000001;
		}
	}
}

int
click(int x, int y, struct game_data *data)
{
	if (x < 0 || x >= data->columns || y < 0 || y >= data->rows) {
		//printf("posición no válida\n");
		return 1;
	}

	if (data->intern_board[y*data->columns + x] & 0b00100000 ||
	    data->intern_board[y*data->columns + x] & 0b01000000) {

		int qtty_flags = 0;

		for (int i = -1; i < 2; ++i) {
		    for (int j = -1; j < 2; ++j) {
			    if (y + i < 0 || y + i >= data->rows ||
                    x + j < 0 || x + j >= data->columns) {
                    continue;
                }

				if (i == 0 && j == 0) {
				    continue;
				}

				if (data->intern_board[(y + i)*data->columns + x + j] & 0b01000000) {
                    qtty_flags++;
                }
			}
		}

		if (qtty_flags >= (data->intern_board[y*data->columns + x] & 0b00001111)) {
            for (int i = -1; i < 2; ++i) {
                for (int j = -1; j < 2; ++j) {
                    if (y + i < 0 || y + i >= data->rows ||
                        x + j < 0 || x + j >= data->columns) {
                        continue;
                    }

                    if (i == 0 && j == 0) {
                        continue;
                    }

                    if (!(data->intern_board[(y + i)*data->columns + x + j] & 0b01000000) && !(data->intern_board[(y + i)*data->columns + x + j] & 0b00100000)) {
                        if (data->intern_board[(y + i)*data->columns + x + j] & 0b10000000) {
                            return 2;
                        }

                        click(x + j, y + i, data);
                    }

                }
            }
        }

	}

	if (data->intern_board[y*data->columns + x] & 0b10000000) {
		data->intern_board[y*data->columns + x] |= 0b00100000;
		return 2;
	}

	struct queue next_cells = create_queue();

	add_element_queue(&next_cells, x, y);

	while (!q_empty(&next_cells)) {
		struct q_data *cell = read_element(&next_cells);

		if (data->intern_board[cell->y*data->columns + cell->x] == 0b00000000) {
			if (cell->x + 1 >= 0 && cell->x + 1 < data->columns)
				add_element_queue(&next_cells, cell->x+1, cell->y);

			if (cell->x - 1 >= 0 && cell->x - 1 < data->columns)
				add_element_queue(&next_cells, cell->x-1, cell->y);

			if (cell->y + 1 >= 0 && cell->y + 1 < data->rows)
				add_element_queue(&next_cells, cell->x, cell->y+1);

			if (cell->y - 1 >= 0 && cell->y - 1 < data->rows)
				add_element_queue(&next_cells, cell->x, cell->y-1);

			if (cell->x + 1 >= 0 && cell->x + 1 < data->columns &&
			    cell->y + 1 >= 0 && cell->y + 1 < data->rows)
				add_element_queue(&next_cells, cell->x+1, cell->y+1);

			if (cell->x + 1 >= 0 && cell->x + 1 < data->columns &&
			    cell->y - 1 >= 0 && cell->y - 1 < data->rows)
				add_element_queue(&next_cells, cell->x+1, cell->y-1);


			if (cell->x - 1 >= 0 && cell->x - 1 < data->columns &&
			    cell->y + 1 >= 0 && cell->y + 1 < data->rows)
				add_element_queue(&next_cells, cell->x-1, cell->y+1);


			if (cell->x - 1 >= 0 && cell->x - 1 < data->columns &&
			    cell->y - 1 >= 0 && cell->y - 1 < data->rows)
				add_element_queue(&next_cells, cell->x-1, cell->y-1);


		}

		data->intern_board[cell->y*data->columns + cell->x] |= 0b00100000;

		delete_element(&next_cells);

	}

	return 0;
}


void
flag(int x, int y, struct game_data *data)
{
	if (data->intern_board[y*data->columns + x] & 0b00100000) {
		//printf("casilla ya despejada, no se puede colocar bandera\n");
		return;
	}

	data->intern_board[y*data->columns + x] |= 0b01000000;
}
