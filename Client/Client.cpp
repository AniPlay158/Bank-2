#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <windows.h>
#include <ws2tcpip.h>
#include <cstdlib> 
#include <conio.h> 
#include <mutex>
#include <vector>
#include <thread>
#include <chrono>
#include <stdexcept> 
using namespace std;

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27016"

SOCKET ConnectSocket = INVALID_SOCKET;

void HideCursor() {
    CONSOLE_CURSOR_INFO cursorInfo;
    cursorInfo.dwSize = 100;
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
}

DWORD WINAPI Sender(LPVOID param) {
    while (true) {

    }
    return 0;
}

DWORD WINAPI Receiver(LPVOID param) {
    while (true) {
        char answer[DEFAULT_BUFLEN];
        int iResult = recv(ConnectSocket, answer, DEFAULT_BUFLEN, 0);
        answer[iResult] = '\0';

        if (iResult > 0) {
            cout << answer << "\n";
        }
        else if (iResult == 0)
            cout << "соединение с сервером закрыто.\n";
        else
            cout << "recv завершился с ошибкой: " << WSAGetLastError() << "\n";
    }
    return 0;
}

class AccountHolder {
public:
    // Setters
    void setLastName(const string& lastName) { this->lastName = lastName; }
    void setFirstName(const string& firstName) { this->firstName = firstName; }
    void setBalance(double balance) { this->balance = balance; }
    void setCreditRating(int credit_ring) { this->credit_ring = credit_ring; }
    void setRegistrationDate(string date) { this->date = date; }

    // Getters
    const string& getLastName() const { return lastName; }
    const string& getFirstName() const { return firstName; }
    double getBalance() const { return balance; }
    double setCreditRating() const { return credit_ring; }
    const string setRegistrationDate() const { return date; }

private:
    string lastName;
    string firstName;
    double balance;
    string date;
    double credit_ring;
};

class Operation {
public:
    enum Type {
        INCOME,
        EXPENSE
    };

    Type operationType;
    time_t creationDate;
    bool status;  // true - виконується, false - виконана

    Operation(Type type) : operationType(type), creationDate(time(0)), status(true) {}
};

class Account {
public:
    // Setters
    void setHolder(const AccountHolder& holder) { this->holder = holder; }
    void setAccountNumber(int accountNumber) { this->accountNumber = accountNumber; }

    // Getters
    AccountHolder& getHolder() { return holder; }
    int getAccountNumber() const { return accountNumber; }

    // TransferMoney method
    void TransferMoney(Account& destinationAccount, double amount) {
        // Проверка наличия необходимой суммы на счету отправителя
        if (holder.getBalance() >= amount) {
            // Simulate a delay to emulate real-world transactions
            this_thread::sleep_for(chrono::seconds(2));

            // Lock mutex to ensure atomicity of the transaction
            mutex.lock();
            // Списание средств со счета отправителя
            holder.setBalance(holder.getBalance() - amount);

            // Увеличение баланса получателя
            double dest = destinationAccount.getHolder().getBalance();
            destinationAccount.getHolder().setBalance(dest + amount);

            // Release mutex
            mutex.unlock();

            // Вывод сообщения об успешном переводе
            cout << "Перевод выполнен успешно. Баланс счета " << getAccountNumber() << ": " << holder.getBalance()
                << ", Баланс счета " << destinationAccount.getAccountNumber() << ": "
                << destinationAccount.getHolder().getBalance() << endl;
        }
        else {
            // Вывод сообщения о недостаточности средств на счету отправителя
            cout << "Недостаточно средств для перевода." << endl;
            throw runtime_error("Недостаточно средств для перевода.");
        }
    }

private:
    AccountHolder holder;
    int accountNumber;
    static mutex mutex;

    // Generate unique account number (replace with your logic)
    int generateAccountNumber() {
        static int nextAccountNumber = 1;
        return nextAccountNumber++;
    }
};

mutex Account::mutex;

void ShowAllAccounts(vector<Account>& accounts) {
    cout << "\nСписок всех аккаунтов и их баланса:" << endl;
    for (size_t i = 0; i < accounts.size(); ++i) {
        Account& account = accounts[i];
        cout << i + 1 << ". Карта " << account.getHolder().getLastName() << ": " << account.getHolder().getBalance() << " USD\n";
    }
}



class BankAccount {
public:
    AccountHolder owner;
    double balance;

    BankAccount(const AccountHolder& owner, double initialBalance = 0.0) : owner(owner), balance(initialBalance) {}
};

void TransferMoneyInBackground(double amount) {

    cout << "Перевод " << amount << " в фоновом потоке..." << endl;

    this_thread::sleep_for(chrono::seconds(10));
    cout << "Перевод выполнен." << endl;
}
void CancelPayment() {

    cout << "Платеж отменен." << endl;
}

void TransferThread(Account& senderAccount, Account& receiverAccount, double paymentAmount) {
    try {
        senderAccount.TransferMoney(receiverAccount, paymentAmount);
        cout << "Перевод выполнен.\n";
    }
    catch (const exception& e) {
        cerr << "Ошибка: " << e.what() << endl;
        CancelPayment(); // Cancel payment in case of exception
    }
}

int main() {
    setlocale(0, "");
    system("title БАНК");

    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        cout << "WSAStartup завершился с ошибкой: " << iResult << "\n";
        return 11;
    }

    addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    const char* ip = "localhost";
    addrinfo* result = NULL;
    iResult = getaddrinfo(ip, DEFAULT_PORT, &hints, &result);

    if (iResult != 0) {
        cout << "getaddrinfo завершился с ошибкой: " << iResult << "\n";
        WSACleanup();
        return 12;
    }

    for (addrinfo* ptr = result; ptr != NULL; ptr = ptr->ai_next) {
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

        if (ConnectSocket == INVALID_SOCKET) {
            cout << "socket завершился с ошибкой: " << WSAGetLastError() << "\n";
            WSACleanup();
            return 13;
        }

        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }

        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        cout << "невозможно подключиться к серверу!\n";
        WSACleanup();
        return 14;
    }


    HANDLE senderThreadHandle = CreateThread(0, 0, Sender, 0, 0, 0);
    HANDLE receiverThreadHandle = CreateThread(0, 0, Receiver, 0, 0, 0);

    if (senderThreadHandle == NULL || receiverThreadHandle == NULL) {
        cout << "Ошибка создания потоков.\n";
        closesocket(ConnectSocket);
        WSACleanup();
        return 16;
    }

    vector<Account> accounts;
    AccountHolder ah;
    ah.setFirstName("Alex");
    ah.setLastName("Alexoff");
    ah.setBalance(10000);
    ah.setCreditRating(4.3);
    ah.setRegistrationDate("13.02.2012");
    Account a;
    a.setAccountNumber(1234);
    a.setHolder(ah);
    accounts.push_back(a);

    AccountHolder bh;
    bh.setFirstName("Boris");
    bh.setLastName("Borisoff");
    bh.setBalance(20000);
    bh.setBalance(14363);
    bh.setCreditRating(2.5);
    bh.setRegistrationDate("03.6.2017");
    Account b;
    b.setAccountNumber(1254);
    b.setHolder(bh);
    accounts.push_back(b);

    AccountHolder ch;
    ch.setFirstName("Nazar");
    ch.setLastName("Vasilyev");
    ch.setBalance(1247);
    ch.setCreditRating(4.5);
    ch.setRegistrationDate("14.05.2016");
    Account c;
    c.setAccountNumber(1247);
    c.setHolder(ch);
    accounts.push_back(c);

    AccountHolder dh;
    dh.setFirstName("Inna");
    dh.setLastName("Trofim");
    dh.setBalance(78);
    dh.setCreditRating(3.7);
    dh.setRegistrationDate("27.11.2022");
    Account d;
    d.setAccountNumber(2356);
    d.setHolder(dh);
    accounts.push_back(d);

    while (true) {
        ShowAllAccounts(accounts);

        // Выбор отправителя
        int senderChoice;
        cout << "Выберите отправителя (введите номер): ";
        cin >> senderChoice;
        if (senderChoice < 1 || senderChoice > accounts.size()) {
            cerr << "Ошибка: Неверный выбор отправителя.\n";
            continue;
        }
        Account& senderAccount = accounts[senderChoice - 1];

        // Выбор получателя
        int receiverChoice;
        cout << "Выберите получателя (введите номер): ";
        cin >> receiverChoice;
        if (receiverChoice < 1 || receiverChoice > accounts.size() || receiverChoice == senderChoice) {
            cerr << "Ошибка: Неверный выбор получателя." << endl;
            continue;
        }
        Account& receiverAccount = accounts[receiverChoice - 1];

        // Ввод суммы
        double paymentAmount;
        cout << "\nВведите сумму перевода: ";
        cin >> paymentAmount;

        thread transferThread(TransferThread, ref(senderAccount), ref(receiverAccount), paymentAmount);

        transferThread.join();
    }

    closesocket(ConnectSocket);
    WSACleanup();

    return 0;
}