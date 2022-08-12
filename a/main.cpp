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

const int CABLE = 9;

struct grid {
  int x;
  int y;
};

int max_action = 0;  // 最大行動回数
int n, k;
vector<vector<int>> g;

void init() {
  // 入力を読み取る
  cin >> n >> k;
  g = vector<vector<int>>(n, vector<int>(n));
  rep(i, n) {
    string s;
    cin >> s;
    rep(j, n) { g[i][j] = s[j] - '0'; }
  }

  // 最大行動回数を設定
  max_action = 100 * k;
}

bool can_connnect(grid begin, grid end, bool is_horizontal = true) {
  bool ok = true;
  if (is_horizontal) {
    int row = begin.x;
    FOR(i, begin.y + 1, end.y) {
      if (g[row][i] != 0) {
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
      if (g[i][col] != 0) {
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

// サーバ同志を接続する
vector<pair<grid, grid>> connect() {
  vector<pair<grid, grid>> res;
  // 横方向の接続
  rep(i, n) {
    int cur_type = 0;
    grid pos = {i, 0};
    rep(j, n) {
      if (g[i][j] == 0 || g[i][j] == CABLE) {
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
    int cur_type = 0;
    grid pos = {0, j};
    rep(i, n) {
      if (g[i][j] == 0 || g[i][j] == CABLE) {
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
}

int main() {
  cin.tie(nullptr);
  ios::sync_with_stdio(0);

  init();
  solve();

  return 0;
}
