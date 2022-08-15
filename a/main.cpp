#include <bits/stdc++.h>

using namespace std;

#define rep(i, n) for (int i = 0; i < (int)n; ++i)
#define FOR(i, a, b) for (int i = (int)a; i < (int)b; ++i)
#define rrep(i, n) for (int i = ((int)n - 1); i >= 0; --i)

using ll = long long;
using ld = long double;

__attribute__((unused)) const ll INF = 1e18;
__attribute__((unused)) const int Inf = 1e9;
__attribute__((unused)) const double EPS = 1e-9;
__attribute__((unused)) const ll MOD = 1000000007;

int dx[4] = {0, 0, 1, -1};
int dy[4] = {1, -1, 0, 0};

// =======================================================
// Disjoint Set
// =======================================================

class DisjointSet {
 public:
  vector<int> rank, p, size;

  DisjointSet() {}
  DisjointSet(int s) {
    rank.resize(s, 0);
    p.resize(s, 0);
    size.resize(s, 0);
    rep(i, s) init(i);
  }

  void init(int x) {
    p[x] = x;
    rank[x] = 0;
    size[x] = 1;
  }

  bool isSame(int x, int y) { return root(x) == root(y); }

  void makeSet(int x, int y) {
    if (isSame(x, y)) return;
    link(root(x), root(y));
  }

  void link(int x, int y) {
    if (rank[x] > rank[y]) {
      p[y] = x;
      size[x] += size[y];
    } else {
      p[x] = y;
      size[y] += size[x];
      if (rank[x] == rank[y]) {
        rank[y]++;
      }
    }
  }

  int root(int x) {
    if (x != p[x]) {
      p[x] = root(p[x]);
    }
    return p[x];
  }

  int getSize(int x) { return size[root(x)]; }
};

// =======================================================
// 各マスを管理する構造体
// =======================================================

struct grid {
  int x;
  int y;

  bool operator==(const grid& rhs) const { return x == rhs.x && y == rhs.y; }
  bool operator<(const grid& rhs) const {
    if (x == rhs.x) {
      return y < rhs.y;
    }
    return x < rhs.x;
  }
};

// =======================================================
// コンピュータのIDと位置を管理するためのマップ
// =======================================================

struct BidirectionalMap {
  map<int, grid> id_to_computer;
  map<grid, int> computer_to_id;

  void set(int id, grid g) {
    grid prev = id_to_computer[id];
    id_to_computer[id] = g;
    computer_to_id.erase(prev);
    computer_to_id[g] = id;
  }
  grid get(int id) { return id_to_computer[id]; }
  int get(grid g) { return computer_to_id[g]; }
};

// =======================================================
// 変数群
// =======================================================

const int CABLE = 9;
const int EMPTY = 8;

int max_action = 0;  // 最大行動回数
int n, k;
vector<vector<int>> g;
BidirectionalMap computers;
vector<int> computer_ids;

// =======================================================
// 初期化
// =======================================================

void init() {
  // 入力を読み取る
  cin >> n >> k;
  g = vector<vector<int>>(n, vector<int>(n));
  computer_ids = vector<int>(k * 100);
  vector<int> ids(k);
  int idx = 0;
  rep(i, n) {
    string s;
    cin >> s;
    rep(j, n) {
      int type = s[j] - '0';
      if (type == 0) {
        type = EMPTY;
      } else {
        type--;
        // 各コンピュータに id を設定
        int id = type * 1000 + ids[type];
        ids[type]++;
        computers.set(id, {i, j});
        computer_ids[idx++] = id;
      }
      g[i][j] = type;
    }
  }

  // 最大行動回数を設定
  max_action = 100 * k;
}

// =======================================================
// マス同士が接続可能かどうかを確認する
// =======================================================

bool can_connnect(grid begin, grid end, bool is_horizontal = true) {
  bool ok = true;
  if (is_horizontal) {
    int row = begin.x;
    FOR(i, begin.y + 1, end.y) {
      if (g[row][i] != EMPTY) {
        ok = false;
        break;
      }
    }
    if (ok) {
      FOR(i, begin.y + 1, end.y) { g[row][i] = CABLE; }
    }
  } else {
    int col = begin.y;
    FOR(i, begin.x + 1, end.x) {
      if (g[i][col] != EMPTY) {
        ok = false;
        break;
      }
    }
    if (ok) {
      FOR(i, begin.x + 1, end.x) { g[i][col] = CABLE; }
    }
  }

  return ok;
}

// =======================================================
// サーバが移動可能かどうかを判定する
// =======================================================

bool can_move(grid pos, int x, int y) {
  int nx = pos.x + x, ny = pos.y + y;
  if (nx < 0 || nx >= n || ny < 0 || ny >= n) {
    return false;
  }

  return g[nx][ny] == EMPTY;
}

// =======================================================
// サーバ同士を接続する
// =======================================================

vector<pair<grid, grid>> connect() {
  vector<pair<grid, grid>> res;
  // 横方向の接続
  rep(i, n) {
    int cur_type = EMPTY;
    grid pos = {i, 0};
    rep(j, n) {
      if (g[i][j] == EMPTY || g[i][j] == CABLE) {
        // コンピュータがない
        continue;
      }
      if (cur_type != g[i][j]) {
        // 違う種類のコンピュータが置いてある
        cur_type = g[i][j];
      } else {
        grid now = {i, j};
        if (can_connnect(pos, now)) {
          res.push_back({pos, now});
        }
      }
      pos = {i, j};
    }
  }

  // 縦方向の接続
  rep(j, n) {
    int cur_type = EMPTY;
    grid pos = {0, j};
    rep(i, n) {
      if (g[i][j] == EMPTY || g[i][j] == CABLE) {
        // コンピュータがない
        continue;
      }
      if (cur_type != g[i][j]) {
        // 違う種類のコンピュータが置いてある
        cur_type = g[i][j];
      } else {
        grid now = {i, j};
        if (can_connnect(pos, now, false)) {
          res.push_back({pos, now});
        }
      }
      pos = {i, j};
    }
  }

  return res;
}

// =======================================================
// スコアを計算する
// =======================================================

int calc_score(const vector<pair<grid, grid>>& connection) {
  DisjointSet dj(10000 * k);
  for (const auto& conn : connection) {
    grid source = conn.first, dest = conn.second;
    int source_id = computers.get(source), dest_id = computers.get(dest);
    dj.makeSet(source_id, dest_id);
  }

  int score = 0;
  rep(i, computer_ids.size()) {
    FOR(j, i + 1, computer_ids.size()) {
      int id1 = computer_ids[i], id2 = computer_ids[j];
      // 同一のクラスタに所属していない
      if (!dj.isSame(id1, id2)) {
        continue;
      }
      // コンピュータのタイプが一緒
      if (id1 / 1000 == id2 / 1000) {
        score++;
      } else {
        score--;
      }
    }
  }

  return max(score, 0);
}

// =======================================================
// 解答を出力
// =======================================================

void print() {
  // 移動

  // 接続
}

void solve() {
  // 移動回数
  cout << 0 << endl;
  auto connection = connect();
  int num_connect = min(max_action, (int)connection.size());
  cout << num_connect << endl;
  rep(i, num_connect) {
    grid source = connection[i].first;
    grid destination = connection[i].second;
    cout << source.x << " " << source.y << " " << destination.x << " "
         << destination.y << endl;
  }

  cout << "score: " << calc_score(connection) << endl;
  ;
}

int main() {
  cin.tie(nullptr);
  ios::sync_with_stdio(0);

  init();
  solve();

  return 0;
}
