#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#define _USE_MATH_DEFINES
#include <math.h>

typedef long long li;
typedef unsigned long long uli;

typedef struct Stop { //정류장 정보
    double y, x;        //위도,경도
    char* id;       
    char* fullname;     
} Stop;

double toRad(double deg) {
    return (deg / 180) * M_PI;
}       //각도를 라디안으로

void initStop(Stop* stop, char stopinfo[]) { //Stop 구조체를 초기화하는 함수 문자열(stopinfo[])을 파싱하여 구조체의 각 필드에 할당
    int idx = 0;
    while (stopinfo[++idx] != ',') {}
    stop->id = (char*)malloc(++idx);
    memcpy(stop->id, stopinfo, idx - 1);
    stop->id[idx - 1] = 0;
    int end = ++idx;
    while (stopinfo[++end] != '"') {}
    stop->fullname = (char*)malloc(++end - idx);
    memcpy(stop->fullname, stopinfo + idx, end - idx - 1);
    stop->fullname[end - idx - 1] = 0;
    idx = ++end;
    while (stopinfo[idx] != ',') { idx++; }
    end = ++idx;
    while (stopinfo[++end] != ',') {}

    uli mult = 1;
    int cnt = 0;
    uli pos = 0;
    uli expo = 1;
    bool sign = false;
    for (int r = end - 1; r >= idx; r--) {
        if (stopinfo[r] == '.') {
            expo = mult;
            continue;
        }
        else if (r == idx && stopinfo[r] == '-') {
            sign = true;
        }
        else {
            if (cnt > 18) pos /= 10;
            pos += mult * ((uli)stopinfo[r] - '0');
            if (cnt < 18) mult *= 10;
            cnt++;
        }
    }
    cnt--;
    if (cnt > 18) {
        expo = 1e17;
    }
    stop->y = (double)pos / expo;
    if (sign) stop->y *= -1;
    stop->y = toRad(stop->y);

    idx = ++end;
    while (stopinfo[++end] != ',') {}
    mult = 1;
    pos = 0;
    expo = 1;
    cnt = 0;
    sign = false;
    for (int r = end - 1; r >= idx; r--) {
        if (stopinfo[r] == '.') {
            expo = mult;
            continue;
        }
        else if (r == idx && stopinfo[r] == '-') {
            sign = true;
        }
        else {
            if (cnt > 18) pos /= 10;
            pos += mult * ((uli)stopinfo[r] - '0');
            if (cnt < 18) mult *= 10;
            cnt++;
        }
    }
    cnt--;
    if (cnt > 18) {
        expo = 1e17;
    }
    stop->x = (double)pos / expo;
    if (sign) stop->x *= -1;
    stop->x = toRad(stop->x);
}

int cmp(const void* _lhs, const void* _rhs) {
    Stop* lhs = (Stop*)_lhs, * rhs = (Stop*)_rhs;
    return strcmp(lhs->id, rhs->id) > 0;//qsort 함수에서 사용하는 비교 함수 Stop 구조체의 id를 기준으로 오름차순 정렬
}

int bicmp(const void* _lhs, const void* _rhs) {
    Stop* lhs = (Stop*)_lhs, * rhs = (Stop*)_rhs;
    return strcmp(lhs->id, rhs->id);
}//bsearch 함수에서 사용하는 비교 Stop 구조체의 id를 기준으로 이진 검색을 수행

double getDist(Stop* lhs, Stop* rhs) {
    double x = (rhs->x - lhs->x) * cos((rhs->y + lhs->y) / 2);
    double y = rhs->y - lhs->y;
    return sqrt(x * x + y * y) * 6371;
}//두 정류장 간의 거리를 계산 위도와 경도를 이용하여 거리를 계산

typedef struct Edge {
    double cost;
    int dst;
} Edge;

struct element;
typedef struct element element;
typedef struct list {
    element* begin;
    element* end;
} list;

struct element {
    Edge item;
    element* next;
};

void initList(list* ls) {
    ls->begin = (element*)malloc(sizeof(element));
    ls->end = ls->begin;
}//연결 리스트를 초기화하는 역할 새로운 element를 동적 할당하여 리스트의 시작과 끝을 가리키게 하는 역할

void emplace_back(list* ls, int t, double cost) {
    ls->end->next = (element*)malloc(sizeof(element));
    ls->end->item.dst = t;
    ls->end->item.cost = cost;
    ls->end = ls->end->next;
}// 연결리스트에 새로운 요소 추가, 동적할당으로, 목적지와 비용을 가지고 잇음


typedef struct pq {
    Edge heap[10010];
    int size;
} priority_queue;

priority_queue pq;
double dists[10000];
int from[10000];

void swap(Edge* lhs, Edge* rhs) {
    Edge tmp = *lhs;
    *lhs = *rhs;
    *rhs = tmp;
}

void push(Edge v) {
    pq.heap[++pq.size] = v;
    int me = pq.size;
    while (me > 1) {
        int p = me / 2;
        double pv = pq.heap[p].cost;
        if (pv > v.cost) swap(pq.heap + me, pq.heap + p);
        else break;
    }
}

int empty() {
    return pq.size == 0;
}

Edge top() {
    return pq.heap[1];
}

void pop() {
    int me = 1;
    swap(pq.heap + me, pq.heap + pq.size--);
    double v = pq.heap[me].cost;
    int l = me * 2; int r = l + 1, c;
    double lv, rv, cv;
    while (r <= pq.size) {
        lv = pq.heap[l].cost;
        rv = pq.heap[r].cost;
        c = lv < rv ? l : r;
        cv = pq.heap[c].cost;
        if (cv < v) {
            swap(pq.heap + me, pq.heap + c);
            me = c;
            l = me * 2;
            r = l + 1;
        }
        else break;
    }
    if (l <= pq.size) {
        lv = pq.heap[l].cost;
        if (lv < v) swap(pq.heap + me, pq.heap + l);
    }
}

Stop stops[10000];
list adj[10000];
bool chk[10000];

bool solve(int S, int E, int N) {
    for (int i = 0; i < N; i++) {
        dists[i] = 1e9;
        chk[i] = false;
        from[i] = i;
    }
    /*이 함수는 시작 정류장(S)에서 종점 정류장(E)까지의 최단 경로를 찾는 역할
     다익스트라 알고리즘
     모든 정류장의 거리를 무한대로 초기화한 후, 시작 정류장의 거리를 0으로 설정 그리고 우선순위 큐에 시작 정류장 추가
     우선순위 큐가 빌 때까지 다음의 과정을 반복
     우선순위 큐에서 가장 거리가 짧은 정류장을 추출
     해당 정류장에서 갈 수 있는 모든 정류장에 대해, 현재 정류장을 거쳐 가는 거리가
     기존의 거리보다 짧으면 거리를 업데이트 해당 정류장을 우선순위 큐에 추가*/
    dists[S] = 0;
    Edge edge;
    edge.cost = 0;
    edge.dst = S;
    push(edge);
    while (!empty()) {
        edge = top();
        pop();
        int s = edge.dst;
        if (chk[s]) continue;
        chk[s] = true;
        if (s == E) break;
        for (element* it = adj[s].begin; it != adj[s].end; it = it->next) {
            int e = it->item.dst;
            double c = it->item.cost;
            if (dists[s] + c < dists[e]) {
                edge.cost = dists[s] + c;
                edge.dst = e;
                push(edge);
                dists[e] = dists[s] + c;
                from[e] = s;
            }
        }
    }
    return from[E] != E;
}


int stack[10000];
void printAnswer(int S, int E) {
    if (S == E) {
        printf("%s\n", stops[S].fullname);
        return;
    }
    int t = 0;
    for (int v = E; from[v] != v; v = from[v]) stack[t++] = v;
    printf("%s\n", stops[S].fullname);
    for (int i = t - 1; i >= 0; i--) printf("%s\n", stops[stack[i]].fullname);
}

int main()
{
    pq.size = 0;
    char start_point[257];
    scanf("%s", start_point);
    char end_point[257];
    scanf("%s", end_point);
    int N;
    scanf("%d", &N); fgetc(stdin);
    for (int i = 0; i < N; i++) {
        char stop_name[257];
        scanf("%[^\n]", stop_name); fgetc(stdin);
        initStop(stops + i, stop_name);
    }
    qsort(stops, N, sizeof(Stop), cmp);
    for (int i = 0; i < N; i++) initList(adj + i);

    int M;
    scanf("%d", &M); fgetc(stdin);
    for (int i = 0; i < M; i++) {
        char src[257], dst[257];
        scanf(" %s %s", src, dst);
        Stop key;
        key.id = src;
        int lhs = (Stop*)bsearch(&key, stops, N, sizeof(Stop), bicmp) - stops;
        key.id = dst;
        int rhs = (Stop*)bsearch(&key, stops, N, sizeof(Stop), bicmp) - stops;
        emplace_back(adj + lhs, rhs, getDist(stops + lhs, stops + rhs));
    }
    int S, E;
    Stop key;
    key.id = start_point;
    S = (Stop*)bsearch(&key, stops, N, sizeof(Stop), bicmp) - stops;
    key.id = end_point;
    E = (Stop*)bsearch(&key, stops, N, sizeof(Stop), bicmp) - stops;

    if (S == E || solve(S, E, N)) printAnswer(S, E);
    else printf("IMPOSSIBLE");

    return 0;
}
