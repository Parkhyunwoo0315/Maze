#ifndef MAZE_H
#define MAZE_H

#include <stddef.h>

/* 위치 */
typedef struct { int z, x, y; } Pos;

typedef struct {
    int L, R, C;
    char* map; /* size = L*R*C, NULL if not loaded */
    Pos start, exitp;
} Maze;

/* 초기화/해제 */
void init_maze(Maze* mz);
void free_maze(Maze* mz);

/* 파일에서 미로 로드
   반환: 성공 1, 실패 0 */
int load_maze(Maze* mz, const char* path);

/* 맵 출력 */
void print_maze(const Maze* mz);

/* BFS로 출구 검색 및 경로 'o' 표시.
   반환: 최단 거리 또는 -1이면 출구 없음 */
int search_exit(Maze* mz);

#endif /* MAZE_H */

