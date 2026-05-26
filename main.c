/*
GitHub Copilot

입력파일에서 3차원 미로를 읽어 포인터 기반 선형 배열로 저장하고
BFS로 최단 경로를 찾은 뒤 경로만('o') 표시한 맵을 출력합니다.

사용법:
  - 기본 입력 파일: input.txt
  - 또는 실행 인수로 파일 경로 지정: program.exe myinput.txt
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Maze.h"

int main(int argc, char** argv) {
    const char* path = (argc > 1) ? argv[1] : "succ.txt";
    /*const char* path = (argc > 1) ? argv[1] : "fail.txt";*/

    Maze maze;
    init_maze(&maze); /* 초기화 */

    if (!load_maze(&maze, path)) {
        fprintf(stderr, "파일을 열거나 파싱하지 못했습니다: %s\n", path);
        return 1;
    }

    /* 원본 맵 출력 */
	printf("원본 맵:\n");
    print_maze(&maze);

    /* 경로 탐색 res는 거리 */
    int res = search_exit(&maze);
    if (res >= 0) {
		printf("\n출구 발견! 이동한 거리: %d\n",res);
        /* 경로가 표시된 맵 출력 */
        print_maze(&maze);
    }
    else {
        printf("\n탈출 실패!\n");
    }

    free_maze(&maze);
    return 0;
}

