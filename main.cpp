#include <bits/stdc++.h>

using namespace std;

#define rep(i, n) for (int i = 0; i < (int)n; ++i)
#define FOR(i, a, b) for (int i = a; i < (int)b; ++i)
#define rrep(i, n) for (int i = ((int)n - 1); i >= 0; --i)

using ll = long long;
using ld = long double;

__attribute__((unused)) const ll INF = 1e18;
__attribute__((unused)) const int Inf = 1e9;
__attribute__((unused)) const double EPS = 1e-9;
__attribute__((unused)) const ll MOD = 1000000007;

// ---------------------------------------------------------------------------
// Inputs
// ---------------------------------------------------------------------------
int N;  // タスク数
int M;  // メンバー数
int K;  // 技能数
int R;  // 依存関係数
int D = 2000;
vector<vector<int>> d;
vector<int> u, v;

// ---------------------------------------------------------------------------
// Parameters
// ---------------------------------------------------------------------------

int reset_days = 50;  // メンバーの評価をリセットする期間

// 実行可能なタスク
// 要素: tuple<タスクNo, 依存されているタスク数, max(d_{タスクNo})>
auto cmp_tasks = [](const tuple<int, int, int>& l,
                    const tuple<int, int, int>& r) {
  if (get<1>(l) != get<1>(r)) {
    // 依存されているタスク数が多い順に取り出す
    return get<1>(l) > get<1>(r);
  } else if (get<2>(l) != get<2>(r)) {
    // max(d_{i}) が大きいものから順に取り出す
    return get<2>(l) > get<2>(r);
  }
  return get<0>(l) < get<0>(r);
};
priority_queue<tuple<int, int, int>, vector<tuple<int, int, int>>,
               decltype(cmp_tasks)>
    tasks(cmp_tasks);

// 空いているチームメンバーを管理
// 要素: pair<メンバーNo, 重み>
// 重みが大きい順に取り出す
// 各タスク終了後に重み付けをして push
auto cmp_members = [](const pair<int, double>& l, const pair<int, double>& r) {
  return l.second > r.second;
};
priority_queue<pair<int, double>, vector<pair<int, double>>,
               decltype(cmp_members)>
    available(cmp_members);

vector<int> working;  // 各メンバーが取り組んでいるタスクNo
vector<pair<ll, int>> weight;  // メンバーの重み: <重み, タスクの実行回数>

// 各タスクの管理
// 終わっている: -1
// 始まっていない: -2
// 始まった: タスクの開始日
vector<int> is_finished;

// 実行可能タスクキューにいるかどうか
vector<bool> is_inQueue;

// タスクiが依存しているタスク一覧
vector<vector<int>> relations;
// タスクiに依存しているタスクの数
vector<int> num_dependent;

// ---------------------------------------------------------------------------
// Utility Functions
// ---------------------------------------------------------------------------

// 評価をリセットする
void clear_eval() {
  rep(i, M) {
    weight[i].first = 0LL;
    weight[i].second = 0;
  }
}

// メンバーと担当タスクを選ぶ
void choose_tasks(vector<int>& a, vector<int>& b, const int day) {
  while ((int)available.size() > 0 && (int)tasks.size() > 0) {
    int member = available.top().first;
    available.pop();
    a.push_back(member);
    int task = get<0>(tasks.top());
    tasks.pop();
    b.push_back(task);
    // 取り組んでいるにチェック
    working[member] = task;
    is_finished[task] = day;
  }
}

// 依存が解消されたタスクを新たに追加
void add_tasks() {
  rep(i, N) {
    if (is_inQueue[i]) {
      // 追加されていないタスクを考える
      continue;
    }
    bool solved = true;
    rep(j, relations[i].size()) {
      // 全ての依存タスクが終了していること
      if (is_finished[relations[i][j]] != -1) {
        solved = false;
        break;
      }
    }
    // 全ての依存タスクが終了していれば新たに追加
    if (solved) {
      tasks.push({i, num_dependent[i], d[i][K]});
      is_inQueue[i] = true;
    }
  }
}

// 1日が終了した際の処理
void finish_day(const vector<int>& f, const int day) {
  // それぞれの担当者の重み付け
  rep(i, (int)f.size()) {
    int assignee = f[i] - 1;
    int task_no = working[assignee];
    ll w = (ll)(day - is_finished[task_no] + 1) * d[task_no][K];
    is_finished[task_no] = -1;
    // 重みの更新
    weight[assignee].first += w;
    weight[assignee].second++;
    // 空いている人に追加
    double ave =
        (double)weight[assignee].first / (double)weight[assignee].second;
    available.push({assignee, ave});
  }

  // 実行可能なタスクの追加
  add_tasks();
}

// ---------------------------------------------------------------------------
// Solutions
// ---------------------------------------------------------------------------

void solve() {
  rep(day, D) {
    // メンバーの技能レベルが高い順番に割り振る
    vector<int> a, b;
    // メンバーと担当タスクを選ぶ
    choose_tasks(a, b, day);
    assert(a.size() == b.size());
    int cnt = (int)a.size();
    cout << cnt << " ";
    rep(i, cnt) {
      cout << a[i] + 1 << " " << b[i] + 1;
      if (i != cnt - 1) cout << " ";
    }
    cout << endl;

    // 入力を受け付ける
    int n;
    cin >> n;
    if (n == -1) {
      // プログラムの終了
      break;
    } else {
      vector<int> f(n);
      rep(i, n) cin >> f[i];
      finish_day(f, day);
    }
  }
}

int main() {
  cin.tie(nullptr);
  ios::sync_with_stdio(0);

  // Input
  cin >> N >> M >> K >> R;

  // Resize
  d.resize(N, vector<int>(K + 1));  // K番目に max(d_i)
  u.resize(R);
  v.resize(R);
  is_finished.resize(N, -2);
  is_inQueue.resize(N, false);
  working.resize(M, -1);
  weight.resize(M, {0LL, 0});
  relations.resize(N);
  num_dependent.resize(N);

  // Read Input
  rep(i, N) {
    int maxi = -Inf;
    rep(j, K) {
      cin >> d[i][j];
      maxi = max(maxi, d[i][j]);
    }
    d[i][K] = maxi;
  }
  rep(i, R) {
    cin >> u[i] >> v[i];
    u[i]--, v[i]--;
    relations[v[i]].push_back(u[i]);
    num_dependent[u[i]]++;
  }
  // 初期段階で実行可能なタスク一蘭
  rep(i, N) {
    // 依存するタスクが何もない
    if ((int)relations[i].size() == 0) {
      tasks.push({i, num_dependent[i], d[i][K]});
      is_inQueue[i] = true;
    }
  }

  // Init
  rep(i, M) { available.push({i, 0.0}); }

  // Solve
  solve();

  return 0;
}
