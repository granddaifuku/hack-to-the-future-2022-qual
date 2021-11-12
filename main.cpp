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

int reset_days = 100;  // メンバーの評価をリセットする期間
double init_step = 3.0; // メンバーの初期値を決定する値
bool first_half = true;

// 実行可能なタスク
// 要素: tuple<タスクNo, median(d_{タスクNo}), 依存されているタスク数>
auto cmp_tasks = [](const tuple<int, double, int>& l,
					const tuple<int, double, int>& r) {
  return get<1>(l) < get<1>(r);
  // if (get<1>(l) != get<1>(r)) {
  // 	// median(d_{i}) が小さい順に取り出す
  // 	return get<1>(l) < get<1>(r);
  // } else if (get<2>(l) != get<2>(r)) {
  // 	// 依存されているタスク数が多い順に取り出す
  // 	return get<2>(l) > get<2>(r);
  // }
  // // タスク番号が小さいものから取り出す
  // return get<0>(l) < get<0>(r);
 };
priority_queue<tuple<int, double, int>, vector<tuple<int, double, int>>,
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
vector<pair<double, int>> weight;  // メンバーの重み: <重み, タスクの実行回数>
vector<vector<int>> ability;  // メンバーの能力

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
vector<double> num_dependent;
// タスクiに依存しているタスクの重みの合計値
vector<double> sum_dependent;
// タスクの重み
vector<double> task_weight;

// ---------------------------------------------------------------------------
// Utility Functions
// ---------------------------------------------------------------------------

// メンバーの評価を行う
void estimate(const int member_id, const int task_id, const int duration) {
  int w = 0;
  int index = 0;
  rep(i, K) {
    if (w < d[task_id][i] - ability[member_id][i]) {
      w = d[task_id][i] - ability[member_id][i];
      index = i;
    }
  }
  if (duration == 1) {
    ability[member_id][index] = w;
  } else {
  }
}

// 評価をリセットする
void clear_eval() {
  if (first_half) {
	rep(i, M/2) {
	  weight[i].first = 0LL;
	  weight[i].second = 0;
	}
  } else {
	FOR(i, M/2, M) {
	  weight[i].first = 0LL;
	  weight[i].second = 0;
	}
  }
  first_half ^= true;
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
      tasks.push({i, task_weight[i] * sum_dependent[i], num_dependent[i]});
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
    double w = (day - is_finished[task_no] + 1) * task_weight[task_no];
    is_finished[task_no] = -1;
    // 重みの更新
    weight[assignee].first += w;
    weight[assignee].second++;
    // 空いている人に追加
    double ave = weight[assignee].first / (double)weight[assignee].second;
    available.push({assignee, ave});
  }

  // 評価のリセット
  // if (day != 0 && day % reset_days == 0) {
  // 	clear_eval();
  // }

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
  d.resize(N, vector<int>(K));
  u.resize(R);
  v.resize(R);

  is_finished.resize(N, -2);
  is_inQueue.resize(N, false);

  working.resize(M, -1);
  weight.resize(M, {0.0, 0});
  ability.resize(M, vector<int>(K));

  relations.resize(N);
  num_dependent.resize(N);
  sum_dependent.resize(N, 0.0);
  task_weight.resize(N);

  // Read Input
  rep(i, N) {
    double sum = 0.0;
    rep(j, K) {
      cin >> d[i][j];
      sum += (double)d[i][j];
    }
    sum /= (double)K;
    task_weight[i] = sum;
  }

  rep(i, R) {
    cin >> u[i] >> v[i];
    u[i]--, v[i]--;
    relations[v[i]].push_back(u[i]);
    num_dependent[u[i]]++;
	sum_dependent[u[i]] += task_weight[v[i]];
  }
  // 初期段階で実行可能なタスク一蘭
  rep(i, N) {
    // 依存するタスクが何もない
    if ((int)relations[i].size() == 0) {
      tasks.push({i, task_weight[i] * sum_dependent[i], num_dependent[i]});
      is_inQueue[i] = true;
    }
  }

  // Init
  rep(i, M) {
	available.push({i, init_step * (double)i});
	weight[i].first = init_step * (double)i;
  }

  // Solve
  solve();

  return 0;
}
