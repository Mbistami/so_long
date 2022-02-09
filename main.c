#include <fcntl.h>
#include "./mlx/mlx.h"
#include "./get_next_line/get_next_line.h"

typedef struct s_game_data
{
    int     min_line_len;
    int     exit_count;
    int     collectible_count;
    int     starting_pos_count;
    int     error;
    char    *map;
} t_game_data;

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

int main(void)
{
    int fd;
    t_game_data data;
    void *mlx;

    init_game_data(&data);
    fd = open("./file", O_RDONLY);
    if (!parse_map(&data, fd) || (((!data.min_line_len || !data.exit_count) || (!data.starting_pos_count || !data.collectible_count)) || data.error == 1))
        printf("ERROR ON MAP PARSING");
    else
        printf("Map width: %d exits: %d collectibles: %d starting position: %d", data.min_line_len, data.exit_count, data.collectible_count, data.starting_pos_count);
    printf("\n%s\n", data.map);
    printf("PERFECT SQUARE SIZE %d", 1600 / data.min_line_len);
    mlx = mlx_init();
    
}