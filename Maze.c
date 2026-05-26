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

/* 1차원 인덱스 3차원으로 */
static inline void coords_of(int id, int R, int C, int* z, int* x, int* y) {
    *z = id / (R * C);
    int rem = id % (R * C);
    *x = rem / C;
    *y = rem % C;
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

/* 파일에서 미로 로드
   반환: 성공 1, 실패 0 */
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

/* BFS로 출구 검색 및 경로 'o' 표시.
   반환: 최단 거리 또는 -1이면 출구 없음 */
int search_exit(Maze* mz) {
    if (!mz || !mz->map) return -1;
    if (mz->start.z == -1 || mz->exitp.z == -1) return -1;

    int cells = mz->L * mz->R * mz->C;

    int* prev = (int*)malloc(sizeof(int) * (size_t)cells);
    char* visited = (char*)calloc((size_t)cells, 1);
    int* queue = (int*)malloc(sizeof(int) * (size_t)cells);
    int* distarr = (int*)malloc(sizeof(int) * (size_t)cells);

    if (!prev || !visited || !queue || !distarr) {
        free(prev); free(visited); free(queue); free(distarr);
        return -1;
    }

    for (int i = 0; i < cells; ++i) { prev[i] = -1; distarr[i] = -1; }

    int qh = 0, qt = 0;
    int sid = index_of(mz->start.z, mz->start.x, mz->start.y, mz->R, mz->C);
    int eid = index_of(mz->exitp.z, mz->exitp.x, mz->exitp.y, mz->R, mz->C);

    queue[qt++] = sid;
    visited[sid] = 1;
    distarr[sid] = 0;

    while (qh < qt) {
        int cur = queue[qh++];
        if (cur == eid) break;

        int cz, cx, cy;
        coords_of(cur, mz->R, mz->C, &cz, &cx, &cy);

        for (int d = 0; d < 6; ++d) {
            int nz = cz + dz[d];
            int nx = cx + dx[d];
            int ny = cy + dy[d];
            if (nz < 0 || nz >= mz->L || nx < 0 || nx >= mz->R || ny < 0 || ny >= mz->C) continue;
            int nid = index_of(nz, nx, ny, mz->R, mz->C);
            if (visited[nid] || mz->map[nid] == '#') continue;
            visited[nid] = 1;
            prev[nid] = cur;
            distarr[nid] = distarr[cur] + 1;
            queue[qt++] = nid;
        }
    }

    /* 찾음 */
    int result = -1;
    if (distarr[eid] != -1) {
        result = distarr[eid];
        int cur = eid;
        while (cur != -1) {
            if (cur != sid && cur != eid) mz->map[cur] = 'o';
            cur = prev[cur];
        }
    }

    free(prev); free(visited); free(queue); free(distarr);
    return result;
}