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

/* 1차원 인덱스 → 3차원 좌표 역변환 */
static inline void coords_of(int id, int R, int C, int* z, int* x, int* y) {
    *z = id / (R * C);
    int rem = id % (R * C);
    *x = rem / C;
    *y = rem % C;
}

/* 개행 제거 헬퍼 - load_maze 내 중복 통합 */
static void strip_newline(char* s) {
    size_t n = strlen(s);
    while (n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r')) s[--n] = '\0';
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
            strip_newline(linebuf); 
            if (linebuf[0] == '\0') continue;

            for (int x = 0; x < R; ++x) {
                if (x > 0) {
                    if (!fgets(linebuf, sizeof(linebuf), fp)) linebuf[0] = '\0';
                    else strip_newline(linebuf);
                }
                size_t rowlen = strlen(linebuf);
                for (int y = 0; y < C; ++y) {
                    char c = (y < (int)rowlen) ? linebuf[y] : '#';
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

/* 최대 숫자로 칸 너비 계산 */
static int calc_width(const int* order, int cells) {
    int maxval = 0;
    for (int i = 0; i < cells; ++i)
        if (order[i] > maxval) maxval = order[i];
    int width = 1, tmp = maxval;
    while (tmp >= 10) { tmp /= 10; ++width; }
    return width;
}

/* 탐색 순서(왼쪽)와 최종 경로(오른쪽)를 나란히 출력 */
void print_maze_order(const Maze* mz, const int* order) {
    if (!mz || !mz->map || !order) return;
    int cells = mz->L * mz->R * mz->C;
    int width = calc_width(order, cells);
    int gap = 4;

    for (int z = 0; z < mz->L; ++z) {
        for (int x = 0; x < mz->R; ++x) {
            /* 왼쪽: 탐색 순서 */
            for (int y = 0; y < mz->C; ++y) {
                int id = index_of(z, x, y, mz->R, mz->C);
                if (order[id] > 0) printf("%*d ", width, order[id]);
                else               printf("%*c ", width, mz->map[id]);
            }
            for (int g = 0; g < gap; ++g) putchar(' ');
            /* 오른쪽: 최종 경로 (같은 너비로 맞춤) */
            for (int y = 0; y < mz->C; ++y) {
                int id = index_of(z, x, y, mz->R, mz->C);
                printf("%*c ", width, mz->map[id]);
            }
            putchar('\n');
        }
        if (z + 1 < mz->L) putchar('\n');
    }
}

/* ───────────────────────────────────────────
   BFS
─────────────────────────────────────────── */
int search_exit_bfs(Maze* mz) {
    if (!mz || !mz->map) return -1;
    if (mz->start.z == -1 || mz->exitp.z == -1) return -1;

    int cells = mz->L * mz->R * mz->C;

    int* prev = (int*)malloc(sizeof(int) * (size_t)cells);
    int* queue = (int*)malloc(sizeof(int) * (size_t)cells);
    int* distarr = (int*)malloc(sizeof(int) * (size_t)cells);
    int* order = (int*)malloc(sizeof(int) * (size_t)cells);

    if (!prev || !queue || !distarr || !order) {
        free(prev); free(queue); free(distarr); free(order);
        return -1;
    }

    for (int i = 0; i < cells; ++i) { prev[i] = -1; distarr[i] = -1; order[i] = 0; }

    int qh = 0, qt = 0;
    int sid = index_of(mz->start.z, mz->start.x, mz->start.y, mz->R, mz->C);
    int eid = index_of(mz->exitp.z, mz->exitp.x, mz->exitp.y, mz->R, mz->C);

    queue[qt++] = sid;
    distarr[sid] = 0;

    int step = 1;
    while (qh < qt) {
        int cur = queue[qh++];
        order[cur] = step++;
        if (cur == eid) break;

        int cz, cx, cy;
        coords_of(cur, mz->R, mz->C, &cz, &cx, &cy);

        for (int d = 0; d < 6; ++d) {
            int nz = cz + dz[d];
            int nx = cx + dx[d];
            int ny = cy + dy[d];
            if (nz < 0 || nz >= mz->L || nx < 0 || nx >= mz->R || ny < 0 || ny >= mz->C) continue;
            int nid = index_of(nz, nx, ny, mz->R, mz->C);
            if (distarr[nid] != -1 || mz->map[nid] == '#') continue; 
            prev[nid] = cur;
            distarr[nid] = distarr[cur] + 1;
            queue[qt++] = nid;
        }
    }

    int result = -1;
    if (distarr[eid] != -1) {
        result = distarr[eid];
        /* 경로 역추적 후 'o' 표시 */
        int cur = eid;
        while (cur != -1) {
            if (cur != sid && cur != eid) mz->map[cur] = 'o';
            cur = prev[cur];
        }
        printf("\n \t 탐색 순서 \t\t\t    최종 경로\n");
        print_maze_order(mz, order);
    }

    free(prev); free(queue); free(distarr); free(order);
    return result;
}

/* ───────────────────────────────────────────
   DFS
─────────────────────────────────────────── */
static int dfs_helper(Maze* mz, int cz, int cx, int cy, char* visited, int count, int eid, int* order, int* step) {
    int cur_id = index_of(cz, cx, cy, mz->R, mz->C);
    order[cur_id] = (*step)++;

    if (cur_id == eid) return count;

    for (int d = 0; d < 6; ++d) {
        int nz = cz + dz[d];
        int nx = cx + dx[d];
        int ny = cy + dy[d];

        if (nz < 0 || nz >= mz->L || nx < 0 || nx >= mz->R || ny < 0 || ny >= mz->C) continue;

        int nid = index_of(nz, nx, ny, mz->R, mz->C);

        if (visited[nid] || mz->map[nid] == '#') continue;

        visited[nid] = 1;
        int res = dfs_helper(mz, nz, nx, ny, visited, count + 1, eid, order, step);

        if (res >= 0) {
            if (mz->map[nid] != 'S' && mz->map[nid] != 'E') mz->map[nid] = 'o';
            return res;
        }
    }

    return -1;
}

int search_exit_dfs(Maze* mz) {
    if (!mz || !mz->map) return -1;
    if (mz->start.z == -1 || mz->exitp.z == -1) return -1;

    int cells = mz->L * mz->R * mz->C;

    char* visited = (char*)malloc((size_t)cells);
    int* order = (int*)malloc(sizeof(int) * (size_t)cells);

    if (!visited || !order) { free(visited); free(order); return -1; }
    for (int i = 0; i < cells; ++i) { visited[i] = 0; order[i] = 0; }

    int sid = index_of(mz->start.z, mz->start.x, mz->start.y, mz->R, mz->C);
    int eid = index_of(mz->exitp.z, mz->exitp.x, mz->exitp.y, mz->R, mz->C);

    visited[sid] = 1;

    int step = 1;
    int result = dfs_helper(mz, mz->start.z, mz->start.x, mz->start.y, visited, 0, eid, order, &step);

    if (result >= 0) {
        printf("\n \t 탐색 순서 \t\t\t    최종 경로\n");
        print_maze_order(mz, order);
    }

    free(visited); free(order);
    return result;
}
