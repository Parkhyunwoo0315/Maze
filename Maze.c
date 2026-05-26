#include "Maze.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 방향 벡터 층위아래dz 좌우dx 상하dy */
static const int dz[6] = { 1, -1, 0, 0, 0, 0 };
static const int dx[6] = { 0, 0, 1, -1, 0, 0 };
static const int dy[6] = { 0, 0, 0, 0, 1, -1 };

/* 3차원 좌표를 1차원 배열 인덱스로 변환 */
static inline int index_of(int z, int x, int y, int R, int C) {
    return z * (R * C) + x * C + y;
}

/* 초기화/해제 */
void init_maze(Maze* mz) {
    if (!mz) return;
    mz->L = mz->R = mz->C = 0;
    mz->map = NULL;
    mz->start.z = mz->start.x = mz->start.y = -1;
    mz->exitp.z = mz->exitp.x = mz->exitp.y = -1;
}

void free_maze(Maze* mz) {
    if (!mz) return;
    if (mz->map) { free(mz->map); mz->map = NULL; }
    mz->L = mz->R = mz->C = 0;
    mz->start.z = mz->start.x = mz->start.y = -1;
    mz->exitp.z = mz->exitp.x = mz->exitp.y = -1;
}

/* 파일에서 미로 로드 */
int load_maze(Maze* mz, const char* path) {
    if (!mz || !path) return 0;
    FILE* fp = fopen(path, "r");
    if (!fp) return 0;

    int L = 0, R = 0, C = 0;
    if (fscanf(fp, " %d %d %d", &L, &R, &C) != 3) { fclose(fp); return 0; }
    if (L == 0 && R == 0 && C == 0) { fclose(fp); return 0; }

    int ch = fgetc(fp);
    if (ch != '\n' && ch != EOF) {
        while (ch != '\n' && ch != EOF) ch = fgetc(fp);
    }

    int cells = L * R * C;
    char* map = (char*)malloc((size_t)cells);
    if (!map) { fclose(fp); return 0; }
    memset(map, '#', (size_t)cells);

    char linebuf[1024];
    Pos start = { -1, -1, -1 }, exitp = { -1, -1, -1 };

    for (int z = 0; z < L; ++z) {
        while (1) {
            if (!fgets(linebuf, sizeof(linebuf), fp)) { break; }
            size_t ln = strlen(linebuf);
            while (ln > 0 && (linebuf[ln - 1] == '\n' || linebuf[ln - 1] == '\r')) { linebuf[--ln] = '\0'; }
            if (ln == 0) continue;

            for (int x = 0; x < R; ++x) {
                char* row;
                if (x == 0) row = linebuf;
                else {
                    if (!fgets(linebuf, sizeof(linebuf), fp)) { linebuf[0] = '\0'; row = linebuf; }
                    else {
                        size_t rln = strlen(linebuf);
                        while (rln > 0 && (linebuf[rln - 1] == '\n' || linebuf[rln - 1] == '\r')) { linebuf[--rln] = '\0'; }
                        row = linebuf;
                    }
                }
                size_t rowlen = strlen(row);
                for (int y = 0; y < C; ++y) {
                    char c = (y < (int)rowlen) ? row[y] : '#';
                    int id = index_of(z, x, y, R, C);
                    map[id] = c;
                    if (c == 'S') { start.z = z; start.x = x; start.y = y; }
                    if (c == 'E') { exitp.z = z; exitp.x = x; exitp.y = y; }
                }
            }
            break;
        }
    }

    mz->L = L; mz->R = R; mz->C = C;
    mz->map = map;
    mz->start = start;
    mz->exitp = exitp;
    fclose(fp);
    return 1;
}

/* 맵 출력 */
void print_maze(const Maze* mz) {
    if (!mz || !mz->map) return;
    for (int z = 0; z < mz->L; ++z) {
        for (int x = 0; x < mz->R; ++x) {
            for (int y = 0; y < mz->C; ++y) {
                int id = index_of(z, x, y, mz->R, mz->C);
                putchar(mz->map[id]);
            }
            putchar('\n');
        }
        if (z + 1 < mz->L) putchar('\n');
    }
}

/* DFS(깊이 우선 탐색)로 출구 검색 및 경로 'o' 표시.
   반환: 도달 거리 또는 -1이면 출구 없음 */
static int dfs_helper(Maze* mz, int cz, int cx, int cy, char* visited, int count, int eid) {
    int cur_id = index_of(cz, cx, cy, mz->R, mz->C);

    /* 목적지('E')에 도달한 경우 */
    if (cur_id == eid) {
        return count;
    }

    /* 6방향(위, 아래, 동, 서, 남, 북) 탐색 */
    for (int d = 0; d < 6; ++d) {
        int nz = cz + dz[d];
        int nx = cx + dx[d];
        int ny = cy + dy[d];

        /* 1. 미로 범위 밖이면 패스 */
        if (nz < 0 || nz >= mz->L || nx < 0 || nx >= mz->R || ny < 0 || ny >= mz->C) continue;

        int nid = index_of(nz, nx, ny, mz->R, mz->C);

        /* 2. 이미 방문했거나 벽('#')이면 패스 */
        if (visited[nid] || mz->map[nid] == '#') continue;

        /* 3. 방문 표시 후 다음 칸으로 전진 (재귀 호출) */
        visited[nid] = 1;
        int res = dfs_helper(mz, nz, nx, ny, visited, count + 1, eid);

        /* 4. 길을 찾았다면? (탈출 성공 기록을 들고 돌아오는 중) */
        if (res >= 0) {
            /* 출발점과 출구 자리가 아니라면 통로에 'o' 표시를 남김 */
            if (mz->map[nid] != 'S' && mz->map[nid] != 'E') {
                mz->map[nid] = 'o';
            }
            return res; /* 최종 이동 거리 전달 */
        }

        /* 5. (선택 사항) 만약 최단 경로가 아닌 모든 경로를 탐색하는 백트래킹이 필요하다면
              여기서 visited[nid] = 0; 처리를 할 수 있으나, 단순 탈출이 목적이므로 유지합니다. */
    }

    return -1; /* 이 방향으로는 출구가 없음 */
}

/* DFS로 출구 검색 및 경로 'o' 표시.
   반환: 도달 거리 또는 -1이면 출구 없음 */
int search_exit(Maze* mz) {
    if (!mz || !mz->map) return -1;
    if (mz->start.z == -1 || mz->exitp.z == -1) return -1;

    int cells = mz->L * mz->R * mz->C;
    char* visited = (char*)calloc((size_t)cells, 1);
    if (!visited) return -1;

    int sid = index_of(mz->start.z, mz->start.x, mz->start.y, mz->R, mz->C);
    int eid = index_of(mz->exitp.z, mz->exitp.x, mz->exitp.y, mz->R, mz->C);

    /* 시작점 방문 표시 */
    visited[sid] = 1;

    /* 재귀 함수 호출 (시작 거리: 0) */
    int result = dfs_helper(mz, mz->start.z, mz->start.x, mz->start.y, visited, 0, eid);

    free(visited);
    return result;
}