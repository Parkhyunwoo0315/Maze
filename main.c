/*
입력파일에서 3차원 미로를 읽어 포인터 기반 선형 배열로 저장하고
BFS로 최단 경로를 찾은 뒤 경로만('o') 표시한 맵을 출력.

입력 파일: input.txt
*/
#include <stdio.h>
#include "Maze.h"

int main(int argc, char** argv) {
    const char* path = (argc > 1) ? argv[1] : "succ.txt";
    /*const char* path = (argc > 1) ? argv[1] : "fail.txt";*/

    /* 탐색 방법 선택 */
    char method[16];
    printf("탐색 방법을 선택하세요 (bfs/dfs): ");
    if (scanf("%15s", method) != 1) {
        fprintf(stderr, "입력 오류\n");
        return 1;
    }

    Maze maze;
    init_maze(&maze);
    if (!load_maze(&maze, path)) {
        fprintf(stderr, "파일을 열거나 파싱하지 못했습니다: %s\n", path);
        return 1;
    }
	
    /* 원본 맵 출력 */
    printf("\n원본 맵:\n");
    print_maze(&maze);

    /* 탐색 */
    int res;
    if (strcmp(method, "bfs") == 0) {
        printf("\n[BFS] 탐색 중...\n");
        res = search_exit_bfs(&maze);
    }
    else if (strcmp(method, "dfs") == 0) {
        printf("\n[DFS] 탐색 중...\n");
        res = search_exit_dfs(&maze);
    }
    else {
        fprintf(stderr, "알 수 없는 방법: %s (bfs 또는 dfs 입력)\n", method);
        free_maze(&maze);
        return 1;
    }

    /* 결과 출력 */
    if (res >= 0)
        printf("\n출구 발견! 이동한 거리: %d\n", res);
    else
        printf("\n탈출 실패!\n");

    free_maze(&maze);
    return 0;
}

