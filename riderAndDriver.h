#ifndef USER_DRIVER_HASHTABLE_H
#define USER_DRIVER_HASHTABLE_H

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

// Constants and Hash Function
const int HASH_TABLE_SIZE = 30;
int hashFunction(const std::string &key);

// User class
class User {
public:
    std::string userName;
    std::string userPassword;
    long long userPhoneNo;

    User();
    User(std::string name, std::string password, long long phoneNo);
    void displayUser() const;
    std::string toCSV() const;
    static User fromCSV(const std::string &line);
    void editUserInfo(const std::vector<std::vector<User>>& userTable);
    static bool isPhoneNumberUnique(long long phoneNo, const std::vector<std::vector<User>>& userTable);
};

// Driver class
class Driver {
public:
    std::string driverName;
    std::string driverPassword;
    long long driverPhoneNo;
    std::string cnic;
    std::string licenseNo;
    std::string criminalRecord;
    std::string vehicleNumber;
    std::string vehicleModel;
    std::string vehicleColor;
    int vehicleYear;
    std::string vehicleRegistration;

    Driver();
    Driver(std::string name, std::string password, long long phoneNo, std::string cnic, std::string license, std::string record,
           std::string vehicleNo, std::string model, std::string color, int year, std::string registration);
    void displayDriver() const;
    std::string toCSV() const;
    static Driver fromCSV(const std::string &line);
    void editDriverInfo(const std::vector<std::vector<Driver>>& driverTable);
    static bool isPhoneNumberUnique(long long phoneNo, const std::vector<std::vector<Driver>>& driverTable);
    void acceptRide();
};

// UserDriverHashTable class
class UserDriverHashTable {
private:
    std::vector<std::vector<User>> userTable;
    std::vector<std::vector<Driver>> driverTable;

public:
    UserDriverHashTable();
    bool isPhoneNumberUnique(long long phoneNo);
    void insertUser();
    void insertDriver();
    void saveToFile(const std::string &userFile, const std::string &driverFile);
    void loadFromFile(const std::string &userFile, const std::string &driverFile);
    bool logInUser(const std::string &userName, const std::string &password);
    bool logInDriver(const std::string &driverName, const std::string &password);
    void displayUsers() const;
    void displayDrivers() const;
    const std::vector<std::vector<User>>& getUserTable() const;
    const std::vector<std::vector<Driver>>& getDriverTable() const;
};

#endif // USER_DRIVER_HASHTABLE_H
