#include <fcntl.h>
#include "./mlx/mlx.h"
#include "./get_next_line/get_next_line.h"

typedef struct	s_data {
	void	*img;
	char	*addr;
	int		bits_per_pixel;
	int		line_length;
	int		endian;
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
    t_data  mlx_data;
} t_game_data;

void	my_mlx_pixel_put(t_data *data, int x, int y, int color)
{
	char	*dst;

	dst = data->addr + (y * data->line_length + x * (data->bits_per_pixel / 8));
	*(unsigned int*)dst = color;
}

void save_game_data (char c, t_game_data *data)
{
    if (c == 'C')
        data->collectible_count += 1;
    else if (c == 'E')
        data->exit_count += 1;
    else if (c == 'P')
        data->starting_pos_count+= 1;
    else if ((c != '0' && c != '1') && (c != 13 && c != 10)) // replace 13 && 10 with 13 (NEW LINE WINDOWS/MAC)
        data->error = 1;
}

int validate_char(char* readed_line, t_game_data *data, int col, int line)
{
    if (ft_strlen(readed_line) - 2 != data->min_line_len) //
    {
        if (readed_line[ft_strlen(readed_line) - 1] == '\n')
            return 0;
        else if (ft_strlen(readed_line) != data->min_line_len)
            return 0;
    }
            
    if ((line == 0) || readed_line[ft_strlen(readed_line) - 1] != '\n')
    {
        if ((readed_line[col] != '1' && (readed_line[col] != 13 && readed_line[col] != 10)))
            return 0;
    }
    else
    {
        if (readed_line[0] != '1' || readed_line[ft_strlen(readed_line) - 3] != '1') // replace 3 with 2 since new line in ubuntu = 2 chars
            return 0;
        save_game_data(readed_line[col], data);
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
    data->min_line_len = ft_strlen(readed_line) - 2;
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

void draw_square (int x, int y, t_data *img, int square_size)
{
    int heading;

    heading = 0;
    while (heading != square_size)
    {
        my_mlx_pixel_put(img, x + heading, y, 0x00FF0000);
        my_mlx_pixel_put(img, x , y + heading, 0x00FF0000);
        my_mlx_pixel_put(img, x + square_size - heading, y + square_size, 0x00FF0000);
        my_mlx_pixel_put(img, x + square_size, y + square_size - heading, 0x00FF0000);
        heading++;
    }
}

void draw_map (char *map, t_data *img, int square_size)
{
    int x = 0;
    int y = 0;
    int i = 0;
    // my_mlx_pixel_put(img, 5, 5, 0x00FF0000);
    printf("%i", square_size);
    while(map[i])
    {
        if (map[i] != 10 && map[i] != 13)
            draw_square(x, y, img, square_size);
        if (map[i] != 13 && map[i] != 10)
            x += square_size;
        else if (map[i] != 10)
        {
            x = 0;
            y += square_size;
        }
        i++;
    }
}

int main(void)
{
    int fd;
    t_game_data data;
    void *mlx;
    void *mlx_win;
    t_data img;

    init_game_data(&data);
    fd = open("./file", O_RDONLY);
    if (!parse_map(&data, fd) || (((!data.min_line_len || !data.exit_count) || (!data.starting_pos_count || !data.collectible_count)) || data.error == 1))
        printf("ERROR ON MAP PARSING");
    else
        printf("Map width: %d exits: %d collectibles: %d starting position: %d lines: %d", data.min_line_len, data.exit_count, data.collectible_count, data.starting_pos_count, data.lines_count);
    printf("\n%s\n%d", data.map, 1080 / data.min_line_len);
    
    data.square_size = 1080 / data.min_line_len;
    printf("%d", data.square_size);
    mlx = mlx_init();
    mlx_win = mlx_new_window(mlx, 1200, data.square_size * data.lines_count, "./so_long");
    img.img = mlx_new_image(mlx, 1200, data.square_size * data.lines_count);
	img.addr = mlx_get_data_addr(img.img, &img.bits_per_pixel, &img.line_length,
								&img.endian);
	draw_map(data.map, &img, data.square_size);
	mlx_put_image_to_window(mlx, mlx_win, img.img, 0, 0);
    mlx_loop(mlx);
}