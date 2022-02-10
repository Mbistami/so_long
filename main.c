#include <fcntl.h>
#include <mlx.h>
#include <stdio.h>
#include "./get_next_line/get_next_line.h"

typedef struct  s_point {
    int x;
    int y;
}   t_point;

typedef struct s_vars
{
    void    *mlx;
    void    *win;
}   t_vars;


typedef struct	s_data {
	void	*img;
	char	*addr;
	int		bits_per_pixel;
	int		line_length;
	int		endian;
    t_vars  vars;
}				t_data;

typedef struct s_game_data
{
    int     min_line_len;
    int     exit_count;
    int     collectible_count;
    int     starting_pos_count;
    int     error;
    int     lines_count;
    char    *map;
    int     square_size;
    int player;
    t_data  img;
    t_vars vars;
} t_game_data;

void    new_point(int x, int y, t_point *point)
{
    point->x = x;
    point->y = y;
}

void	my_mlx_pixel_put(t_data *data, int x, int y, int color)
{
	char	*dst;

	dst = data->addr + (y * data->line_length + x * (data->bits_per_pixel / 8));
	*(unsigned int*)dst = color;
}

void save_game_data (char c, t_game_data *data, int col, int lines)
{
    if (lines >= 0)
    {
        if (c == 'C')
        data->collectible_count += 1;
        else if (c == 'E')
            data->exit_count += 1;
        else if (c == 'P')
        {
            data->starting_pos_count+= 1;
            if (lines >= 0)
                data->player = lines * (data->min_line_len + 1) + col;
            printf("player pos :%d\n", data->player);
        }
        else if ((c != '0' && c != '1') && c != 10) // replace 13 && 10 with 13 (NEW LINE WINDOWS/MAC)
            data->error = 1;
    }
    else if (c == 'C')
        data->collectible_count--;
}

int validate_char(char* readed_line, t_game_data *data, int col, int line)
{
    if (ft_strlen(readed_line) - 1 != data->min_line_len) //
    {
        if (readed_line[ft_strlen(readed_line) - 1] == '\n')
            return 0;
        else if (ft_strlen(readed_line) != data->min_line_len)
            return 0;
    }
            
    if ((line == 0) || readed_line[ft_strlen(readed_line) - 1] != '\n')
    {
        if ((readed_line[col] != '1' && (readed_line[col] != 10)))
            return 0;
    }
    else
    {
        if (readed_line[0] != '1' || readed_line[ft_strlen(readed_line) - 2] != '1') // replace 3 with 2 since new line in ubuntu = 2 chars
            return 0;
        save_game_data(readed_line[col], data, col, line);
    }
    return (1);
}

int parse_map(t_game_data *data, int fd)
{
    char    *container;
    char    *readed_line;
    int     line;
    int     col;

    readed_line = get_next_line(fd);
    data->min_line_len = ft_strlen(readed_line) - 1;
    container = (char *)malloc(sizeof(char));
    line = 0;
    container[0] = '\0';
    while (readed_line && !data->error)
    {
        col = 0;
        while (readed_line[col])
        {
            if (!validate_char(readed_line, data, col, line))
                return (0);
            col++;
        }   
        container = ft_strjoin(container, readed_line);
        readed_line = get_next_line(fd);
        line++;
    }
    data->map = container;
    data->lines_count = line;
    return 1;
}

void init_game_data(t_game_data *data)
{
    data->min_line_len = 0;
    data->exit_count = 0;
    data->collectible_count = 0;
    data->starting_pos_count = 0;
    data->error = 0;
    data->map = 0;
}

int get_color (char type)
{
    if (type == '1')
        return 0x00FF0000;
    else if (type == 'C')
        return 0xB001BD;
    else if (type == 'P')
        return 0xD5F12C;
    else if (type == 'E')
        return 0x3EF568;
    else
        return 0x00000;
}

void draw_square (t_point *point, t_data *img, t_game_data *data, char type)
{
    int heading;
    int square_size;
    int color;
    int x = point->x;
    int y = point->y;

    square_size = data->square_size;
    heading = 0;
    color = get_color(type);
    
    while (heading != square_size)
    {
        my_mlx_pixel_put(img, x + heading, y, 0x00FF0000);
        my_mlx_pixel_put(img, x , y + heading, 0x00FF0000);
        my_mlx_pixel_put(img, x + square_size - heading, y + square_size, 0x00FF0000);
        my_mlx_pixel_put(img, x + square_size, y + square_size - heading, 0x00FF0000);
        printf("%d\n", data->collectible_count);
        if (type == 'E' && data->collectible_count != 0)
        {
            heading++;
            continue;
        }
        my_mlx_pixel_put(img, x + (square_size / 2), y + heading, color);
        my_mlx_pixel_put(img, x + (heading), y + (square_size / 2), color);   
        heading++;
    }
}

void draw_map (char *map, t_data *img, t_game_data *data)
{
    int x = 0;
    int y = 0;
    int i = 0;
    int square_size = data->square_size;
    t_point point; 
    // my_mlx_pixel_put(img, 5, 5, 0x00FF0000);
    // printf("%i", square_size);
    new_point(x, y, &point);
    while(map[i])
    {
        if (map[i] != 10)
            draw_square(&point, img, data, map[i]);
        if (map[i] != 10)
            x += square_size;
        else 
        {
            x = 0;
            y += square_size;
        }
        new_point(x, y, &point);
        i++;
    }
}

void make_moves(t_game_data *data, int keycode)
{
    if (keycode == 126 && data->map[data->player - (data->min_line_len + 1)] != '1')
    {
        data->map[data->player] = '0';
        data->map[data->player - (data->min_line_len + 1)] = 'P';
        data->player = data->player - (data->min_line_len + 1);
    }
    else if (keycode == 125 && data->map[data->player + (data->min_line_len + 1)] != '1')
    {
        data->map[data->player] = '0';
        data->map[data->player + (data->min_line_len + 1)] = 'P';
        data->player = data->player + (data->min_line_len + 1);
    }
    else if (keycode == 124 && data->map[data->player + 1] != '1')
    {
        data->map[data->player] = '0';
        data->map[data->player + 1] = 'P';
        data->player++;
    }
    else if (keycode == 123 && data->map[data->player - 1] != '1')
    {
        data->map[data->player] = '0';
        data->map[data->player - 1] = 'P';
        data->player--;
    }
}

void update_game_data (t_game_data *data)
{
    int i;

    i =0;
    data->collectible_count = 0;
    data->exit_count = 0;
    while (data->map[i])
    {
        if (data->map[i] == 'C')
            data->collectible_count++;
        else if (data->map[i] == 'E')
            data->exit_count++;
        i++;
    }
    if (data->exit_count == 0)
        mlx_string_put(data->vars.mlx, data->vars.win, 1200 / 2, (data->square_size * data->lines_count) / 2, 0x556E86, "YOU WON!");
}

int key_hook (int keycode, t_game_data *data)
{
    make_moves(data, keycode);
    draw_map(data->map, &data->img, data);
    mlx_put_image_to_window(data->vars.mlx, data->vars.win, data->img.img, 0, 0);
    update_game_data(data);
    printf("Map width: %d exits: %d collectibles: %d starting position: %d lines: %d\n", data->min_line_len, data->exit_count, data->collectible_count, data->starting_pos_count, data->lines_count);
    return (0);
}

int main(void)
{
    int fd;
    t_game_data data;
    void *mlx;
    void *mlx_win;
    t_data img;
    t_vars vars;

    init_game_data(&data);
    fd = open("./file", O_RDONLY);
    if (!parse_map(&data, fd) || (((!data.min_line_len || !data.exit_count) || (!data.starting_pos_count || !data.collectible_count)) || data.error == 1))
        {
            printf("ERROR ON MAP PARSING");
            return 0;
        }
    else
        printf("Map width: %d exits: %d collectibles: %d starting position: %d lines: %d", data.min_line_len, data.exit_count, data.collectible_count, data.starting_pos_count, data.lines_count);
    printf("\n%s\n%d", data.map, 1080 / data.min_line_len);
    
    data.square_size = 1080 / data.min_line_len;
    printf("%d\n", data.square_size);
    vars.mlx = mlx_init();
    vars.win = mlx_new_window(vars.mlx, (data.square_size * data.min_line_len) + 2, data.square_size * data.lines_count, "./so_long");

    img.img = mlx_new_image(vars.mlx, 1080, data.square_size * data.lines_count + 1);
	img.addr = mlx_get_data_addr(img.img, &img.bits_per_pixel, &img.line_length,
								&img.endian);
	draw_map(data.map, &img, &data);
    data.img = img;
    data.vars = vars;
	mlx_put_image_to_window(vars.mlx, vars.win, img.img, 0, 0);
    printf("%c\n", data.map[43]);
    mlx_hook(vars.win, 2, 0, key_hook, &data);
    mlx_loop(vars.mlx);
}