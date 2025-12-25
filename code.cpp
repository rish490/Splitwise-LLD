#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <climits>
#include <cmath>
#include <mutex>
#include <thread>

using namespace std;

/* =========================
   User Entity
   ========================= */
class User {
public:
    int id;
    string name;
    User(int id, const string& name) : id(id), name(name) {}
};

/* =========================
   Balance Entity
   - amount: current balance
   - version: for optimistic locking
   - mtx: mutex to simulate atomic read/write for thread safety
   ========================= */
struct Balance {
    double amount;
    int version;
    mutex mtx;
    Balance(double a = 0) : amount(a), version(0) {}
};

/* =========================
   Split Entity
   - Represents a share of an expense for a user
   ========================= */
class Split {
public:
    User* user;
    double amount;
    Split(User* user, double amount) : user(user), amount(amount) {}
};

/* =========================
   Expense Entity
   - Each expense has a payer, splits, and description
   ========================= */
class Expense {
public:
    int id;
    User* paidBy;
    vector<Split> splits;
    string description;
    Expense(int id, User* paidBy, const vector<Split>& splits, const string& description)
        : id(id), paidBy(paidBy), splits(splits), description(description) {}
};

/* =========================
   Group Entity
   - Contains users, expenses, and balances
   - balances store per-user balance with version for optimistic locking
   ========================= */
class Group {
public:
    int id;
    string name;
    vector<User*> users;
    vector<Expense*> expenses;
    unordered_map<int, Balance> balances;

    Group(int id, const string& name) : id(id), name(name) {}
    void addUser(User* user) {
        users.push_back(user);
        balances[user->id] = Balance(0); // initialize balance
    }
    ~Group() {
        for (Expense* e : expenses) delete e;
    }
};

/* =========================
   Expense & Settlement Service
   - Updates balances using optimistic locking
   - Retry logic ensures safe updates in concurrent environment
   ========================= */
class ExpenseService {
public:
    static void addExpense(Group* group, User* paidBy, const vector<Split>& splits, const string& description) {
        int expenseId = group->expenses.size() + 1;
        Expense* expense = new Expense(expenseId, paidBy, splits, description);
        group->expenses.push_back(expense);

        double total = 0;
        for (const Split& s : splits) total += s.amount;

        // Update payer balance
        updateBalanceOptimistic(group, paidBy->id, total);

        // Update participant balances
        for (const Split& s : splits) updateBalanceOptimistic(group, s.user->id, -s.amount);
    }

    static void settle(Group* group, User* from, User* to, double amount) {
        updateBalanceOptimistic(group, from->id, amount);
        updateBalanceOptimistic(group, to->id, -amount);
    }

private:
    // Optimistic locking: read version, update if version unchanged, retry if conflict
    static void updateBalanceOptimistic(Group* group, int userId, double delta) {
        while (true) {
            Balance& bal = group->balances[userId];
            int currentVersion;
            double currentAmount;

            // Atomic read
            {
                lock_guard<mutex> lock(bal.mtx);
                currentVersion = bal.version;
                currentAmount = bal.amount;
            }

            double newAmount = currentAmount + delta;

            // Attempt write if version unchanged
            {
                lock_guard<mutex> lock(bal.mtx);
                if (bal.version == currentVersion) {
                    bal.amount = newAmount;
                    bal.version++;
                    break; // success
                }
                // else retry
            }
        }
    }
};

/* =========================
   Balance Service
   - Display group balances
   ========================= */
class BalanceService {
public:
    static void showGroupBalances(Group* group) {
        cout << "\nBalances for group: " << group->name << "\n";
        for (auto& entry : group->balances) {
            cout << "UserId " << entry.first << " -> Balance: " << entry.second.amount << endl;
        }
    }
};

/* =========================
   Debt Simplification
   - Uses backtracking to minimize number of transactions
   - Transactions represented as from → to → amount
   ========================= */
struct Transaction {
    User* from;
    User* to;
    double amount;
};

class SimplifyDebtService {
public:
    static vector<Transaction> simplify(Group* group) {
        vector<double> debts;
        vector<User*> userList;

        // Collect non-zero balances
        for (User* u : group->users) {
            double bal = group->balances[u->id].amount;
            if (fabs(bal) > 1e-6) {
                debts.push_back(bal);
                userList.push_back(u);
            }
        }

        vector<Transaction> currentPath, bestPath;
        int minTx = INT_MAX;

        dfs(0, debts, userList, currentPath, bestPath, minTx);
        return bestPath;
    }

private:
    static void dfs(int index, vector<double>& debts, vector<User*>& users,
                    vector<Transaction>& currentPath, vector<Transaction>& bestPath, int& minTx) {
        // Skip settled balances
        while (index < debts.size() && fabs(debts[index]) < 1e-6) index++;
        if (index == debts.size()) {
            if ((int)currentPath.size() < minTx) {
                minTx = currentPath.size();
                bestPath = currentPath;
            }
            return;
        }

        for (int i = index + 1; i < debts.size(); i++) {
            if (debts[index] * debts[i] < 0) {
                double settled = min(fabs(debts[index]), fabs(debts[i]));
                Transaction t;

                if (debts[index] < 0) {
                    t.from = users[index];
                    t.to = users[i];
                    t.amount = settled;
                    debts[i] += debts[index];
                } else {
                    t.from = users[i];
                    t.to = users[index];
                    t.amount = settled;
                    debts[i] += debts[index];
                }

                currentPath.push_back(t);
                dfs(index + 1, debts, users, currentPath, bestPath, minTx);
                currentPath.pop_back();

                if (debts[index] < 0) debts[i] -= debts[index];
                else debts[i] -= debts[index];
            }
        }
    }
};

/* =========================
   Demo Main
   ========================= */
int main() {
    // Create users
    User* alice = new User(1, "Alice");
    User* bob = new User(2, "Bob");
    User* charlie = new User(3, "Charlie");

    // Create group and add users
    Group* tripGroup = new Group(1, "Trip");
    tripGroup->addUser(alice);
    tripGroup->addUser(bob);
    tripGroup->addUser(charlie);

    // Add expense: Alice pays 300 split equally
    vector<Split> splits = { Split(alice, 100), Split(bob, 100), Split(charlie, 100) };
    ExpenseService::addExpense(tripGroup, alice, splits, "Hotel");

    BalanceService::showGroupBalances(tripGroup);

    // Bob settles 50 with Alice
    ExpenseService::settle(tripGroup, bob, alice, 50);
    BalanceService::showGroupBalances(tripGroup);

    // Compute simplified debts
    auto settlements = SimplifyDebtService::simplify(tripGroup);
    cout << "\nSimplified Transactions (minimal number of transactions):\n";
    for (auto& t : settlements) {
        cout << t.from->name << " pays " << t.to->name << " " << t.amount << endl;
    }

    // Cleanup
    delete tripGroup;
    delete alice;
    delete bob;
    delete charlie;

    return 0;
}
//Optimistic locking will be better for splitwise as conflicts are rare and app is read heavy
