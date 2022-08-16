#include <bits/stdc++.h>

#include <chrono>

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
// ランダムユーティリティの初期化
random_device seed_gen;
mt19937 engine(seed_gen());

const int BEAM_WIDTH = 3;

const int CABLE = 9;
const int EMPTY = 8;

int max_action = 0;  // 最大行動回数
int N, K;
vector<vector<int>> G;
BidirectionalMap COMPUTERS;

// =======================================================
// ビームサーチ用の状態
// =======================================================

struct Status {
  int num_remain_action;
  vector<vector<int>> g;
  BidirectionalMap computers;
  int score;
  vector<pair<grid, grid>> movements;
  vector<pair<grid, grid>> connections;

  int target_id;

 public:
  Status(int num_remain_action_, vector<vector<int>> g_,
         BidirectionalMap computers_) {
    this->num_remain_action = num_remain_action_;
    this->g = g_;
    this->computers = computers_;
    score = 0;
  }
  Status(const Status& old) {
    this->num_remain_action = old.num_remain_action;

    this->score = old.score;
    this->target_id = old.target_id;

    g = vector<vector<int>>(N, vector<int>(N));

    rep(i, N) {
      rep(j, N) { this->g[i][j] = old.g[i][j]; }
    }
    for (const auto& e : old.movements) {
      this->movements.push_back(e);
    }
    for (const auto& e : old.connections) {
      this->connections.push_back(e);
    }

    for (const auto& e : old.computers.computer_to_id) {
      this->computers.computer_to_id.insert(e);
    }
    for (const auto& e : old.computers.id_to_computer) {
      this->computers.id_to_computer.insert(e);
    }
  }

  void remove_cable() {
    rep(i, N) {
      rep(j, N) {
        if (this->g[i][j] == CABLE) {
          this->g[i][j] = EMPTY;
        }
      }
    }
  }
};

bool operator<(const Status& lhs, const Status& rhs) {
  return lhs.score < rhs.score;
}

// =======================================================
// 初期化
// =======================================================

void init() {
  // 入力を読み取る
  cin >> N >> K;
  G = vector<vector<int>>(N, vector<int>(N));
  int id = 0;
  rep(i, N) {
    string s;
    cin >> s;
    rep(j, N) {
      int type = s[j] - '0';
      if (type == 0) {
        type = EMPTY;
      } else {
        type--;
        // 各コンピュータに id を設定
        COMPUTERS.set(id, {i, j});
        id++;
      }
      G[i][j] = type;
    }
  }

  // 最大行動回数を設定
  max_action = 100 * K;
}

// =======================================================
// ビームサーチ用の関数群
// =======================================================

// =======================================================
// サーバが移動可能かどうかを判定する
// =======================================================

bool can_move(const Status& status, int i, int j, int x, int y) {
  int nx = i + x, ny = j + y;
  if (nx < 0 || nx >= N || ny < 0 || ny >= N) {
    return false;
  }

  return status.g[nx][ny] == EMPTY;
}

void apply_move(Status& status, int i, int j, int x, int y) {
  int nx = i + x, ny = j + y;
  grid prev = {i, j}, next = {nx, ny};

  int id = status.computers.get(prev);
  int type = status.g[i][j];
  status.g[i][j] = EMPTY;
  status.computers.set(id, next);
  status.g[nx][ny] = type;

  status.movements.push_back({prev, next});
  status.num_remain_action--;
}

// =======================================================
// サーバを移動する
// =======================================================

vector<Status> move(Status status) {
  vector<bool> moved(K * 100);
  int r = engine() % K;
  vector<Status> res;
  // 各コンピュータに対してランダムに3マス分動かしてみる
  const int NUM_MOVE = 5;
  rep(i, N) {
    rep(j, N) {
      if (status.g[i][j] == EMPTY) {
        continue;
      }
      if (status.g[i][j] != r) {
        continue;
      }
      if (engine() % 10 != 0) {
        continue;
      }
      int target_id = status.computers.get({i, j});
      if (moved[target_id]) {
        continue;
      }

      moved[target_id] = true;
      Status after = status;
      after.target_id = target_id;
      grid pos = {i, j};
      rep(k, NUM_MOVE) {
        // 移動できない場合は抜ける
        bool moveable = false;
        rep(l, 4) { moveable |= can_move(after, pos.x, pos.y, dx[l], dy[l]); }
        if (!moveable) {
          break;
        }

        // 移動できる場合
        int idx = engine() % 4;  //移動方向のインデックス
        while (!can_move(after, pos.x, pos.y, dx[idx], dy[idx])) {
          idx = engine() % 4;
        }
        apply_move(after, pos.x, pos.y, dx[idx], dy[idx]);
        pos.x += dx[idx];
        pos.y += dy[idx];
      }
      res.push_back(after);
    }
  }

  return res;
}

// =======================================================
// マス同士が接続可能かどうかを確認する
// =======================================================

bool can_connnect(vector<vector<int>>& tmp_graph, grid begin, grid end,
                  bool is_horizontal = true) {
  bool ok = true;
  if (is_horizontal) {
    int row = begin.x;
    FOR(i, begin.y + 1, end.y) {
      if (tmp_graph[row][i] != EMPTY) {
        ok = false;
        break;
      }
    }
    if (ok) {
      FOR(i, begin.y + 1, end.y) { tmp_graph[row][i] = CABLE; }
    }
  } else {
    int col = begin.y;
    FOR(i, begin.x + 1, end.x) {
      if (tmp_graph[i][col] != EMPTY) {
        ok = false;
        break;
      }
    }
    if (ok) {
      FOR(i, begin.x + 1, end.x) { tmp_graph[i][col] = CABLE; }
    }
  }

  return ok;
}

// =======================================================
// サーバを接続する
// =======================================================

vector<pair<grid, grid>> connect(Status& st) {
  DisjointSet dj(K * 100);
  vector<pair<grid, grid>> res;

  // 横方向の接続
  rep(i, N) {
    int cur_type = EMPTY;
    grid pos = {i, 0};
    rep(j, N) {
      if ((int)res.size() >= st.num_remain_action) {
        return res;
      }
      if (st.g[i][j] == EMPTY || st.g[i][j] == CABLE) {
        // コンピュータがない
        continue;
      }
      if (cur_type != st.g[i][j]) {
        // 違う種類のコンピュータが置いてある
        cur_type = st.g[i][j];
      } else {
        grid now = {i, j};
        if (can_connnect(st.g, pos, now)) {
          int id1 = st.computers.get(pos), id2 = st.computers.get(now);
          if (!dj.isSame(id1, id2)) {
            res.push_back({pos, now});
            dj.makeSet(id1, id2);
          }
        }
      }
      pos = {i, j};
    }
  }

  // 縦方向の接続
  rep(j, N) {
    int cur_type = EMPTY;
    grid pos = {0, j};
    rep(i, N) {
      if ((int)res.size() >= st.num_remain_action) {
        return res;
      }
      if (st.g[i][j] == EMPTY || st.g[i][j] == CABLE) {
        // コンピュータがない
        continue;
      }
      if (cur_type != st.g[i][j]) {
        // 違う種類のコンピュータが置いてある
        cur_type = st.g[i][j];
      } else {
        grid now = {i, j};
        if (can_connnect(st.g, pos, now, false)) {
          int id1 = st.computers.get(pos), id2 = st.computers.get(now);
          if (!dj.isSame(id1, id2)) {
            res.push_back({pos, now});
            dj.makeSet(id1, id2);
          }
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

void calc_score(Status& st) {
  DisjointSet prev(K * 100);
  // 直前の状態を再現
  for (const auto& conn : st.connections) {
    grid source = conn.first, dest = conn.second;
    int source_id = st.computers.get(source), dest_id = st.computers.get(dest);
    prev.makeSet(source_id, dest_id);
  }

  DisjointSet now = prev;
  auto connection = connect(st);
  for (const auto& conn : connection) {
    grid source = conn.first, dest = conn.second;
    int source_id = st.computers.get(source), dest_id = st.computers.get(dest);
    now.makeSet(source_id, dest_id);
  }
  st.connections = connection;

  int score_diff = 0;
  // 対象ノードに関連したスコアの計算を行う
  int id = st.target_id;
  rep(i, K * 100) {
    if (i == id) {
      continue;
    }

    if (prev.isSame(id, i)) {
      if (!now.isSame(id, i)) {
        score_diff -= prev.getSize(id) - 1;
      }
    } else {
      if (now.isSame(id, i)) {
        score_diff += now.getSize(id) - 1;
      }
    }
  }

  st.score = max(0, st.score + score_diff);
}

// =======================================================
// 出力する
// =======================================================

void print(const Status& st) {
  // 移動
  cout << st.movements.size() << endl;
  for (const auto& move : st.movements) {
    auto src = move.first, dest = move.second;
    cout << src.x << " " << src.y << " " << dest.x << " " << dest.y << endl;
  }
  // 接続
  cout << st.connections.size() << endl;
  for (const auto& conn : st.connections) {
    auto src = conn.first, dest = conn.second;
    cout << src.x << " " << src.y << " " << dest.x << " " << dest.y << endl;
  }
}

void solve() {
  chrono::system_clock::time_point start, now;
  start = chrono::system_clock::now();

  const int MOVE_THRESHOLD = K * 100 / 2;
  priority_queue<Status> beam;
  Status init = Status(max_action, G, COMPUTERS);
  beam.push(init);

  priority_queue<Status> next;

  while (!beam.empty()) {
    now = chrono::system_clock::now();
    double elapsed = static_cast<double>(
        chrono::duration_cast<chrono::microseconds>(now - start).count() /
        1000.0);
    if (elapsed >= 2850) {
      break;
    }
    Status st = beam.top();

    if (K * 100 - st.num_remain_action > MOVE_THRESHOLD) {
      break;
    }
    beam.pop();

    // 移動
    // start = chrono::system_clock::now();
    vector<Status> movements = move(st);
    // end = chrono::system_clock::now();
    // cout << "Move "
    //      << static_cast<double>(
    // 							chrono::duration_cast<chrono::microseconds>(end
    // -
    // start) 							.count() /
    // 1000.0)
    //      << endl;

    // 点数を計算する
    // start = chrono::system_clock::now();
    for (Status& status : movements) {
      calc_score(status);
      status.remove_cable();
      next.push(status);
    }
    // end = chrono::system_clock::now();
    // cout << "Calc "
    //      << static_cast<double>(
    //             chrono::duration_cast<chrono::microseconds>(end - start)
    //                 .count() /
    //             1000.0)
    //      << endl;

    // 要素を追加する
    // start = chrono::system_clock::now();
    if (beam.empty()) {
      int count = 0;
      while (!next.empty()) {
        auto top = next.top();
        next.pop();
        beam.push(top);
        count++;
        if (count > BEAM_WIDTH) {
          break;
        }
      }
      // nextをクリアする
      next = priority_queue<Status>();
    }
    // end = chrono::system_clock::now();
    // cout << "Append "
    //      << static_cast<double>(
    // 							chrono::duration_cast<chrono::microseconds>(end
    // - start) 							.count()
    // /
    //             1000.0)
    //      << endl;
  }

  print(beam.top());
}

int main() {
  cin.tie(nullptr);
  ios::sync_with_stdio(0);

  init();
  solve();

  return 0;
}
