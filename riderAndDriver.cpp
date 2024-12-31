#include "riderAndDriver.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <regex>
#include <limits>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring> // Include this header for strlen

// Hash function for calculating index
int hashFunction(const std::string &key) {
    int hashValue = 0;
    for (char ch : key) {
        hashValue += ch;
    }
    return hashValue % HASH_TABLE_SIZE;
}

// User class definitions
User::User() : userName(""), userPassword(""), userPhoneNo(0) {}

User::User(std::string name, std::string password, long long phoneNo)
    : userName(name), userPassword(password), userPhoneNo(phoneNo) {}

void User::displayUser() const {
    std::cout << "User Name: " << userName << ", Phone Number: " << userPhoneNo << std::endl;
}

std::string User::toCSV() const {
    return userName + "," + userPassword + "," + std::to_string(userPhoneNo);
}

User User::fromCSV(const std::string &line) {
    size_t pos1 = line.find(',');
    size_t pos2 = line.find(',', pos1 + 1);
    std::string name = line.substr(0, pos1);
    std::string password = line.substr(pos1 + 1, pos2 - pos1 - 1);
    long long phoneNo = std::stoll(line.substr(pos2 + 1));
    return User(name, password, phoneNo);
}

void User::editUserInfo(const std::vector<std::vector<User>>& userTable) {
    int choice;
    std::cout << "Edit User Information:\n";
    std::cout << "1. Edit Username (Current: " << userName << ")\n";
    std::cout << "2. Edit Password (Current: " << userPassword << ")\n";
    std::cout << "3. Edit Phone Number (Current: " << userPhoneNo << ")\n";
    std::cout << "Enter your choice: ";
    std::cin >> choice;

    switch (choice) {
        case 1:
            std::cout << "Enter new username: ";
            std::cin >> userName;
            break;
        case 2:
            std::cout << "Enter new password: ";
            std::cin >> userPassword;
            break;
        case 3:
            long long newPhoneNo;
            do {
                std::cout << "Enter new phone number (11 digits): ";
                std::cin >> newPhoneNo;

                // Check if the input is a number and 11 digits long
                if (std::cin.fail() || std::to_string(newPhoneNo).length() != 11) {
                    std::cin.clear();  // Clears the fail state
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');  // Ignores the invalid input
                    std::cout << "Phone number must be 11 digits long. Please try again." << std::endl;
                } else if (!User::isPhoneNumberUnique(newPhoneNo, userTable)) {
                    std::cout << "Phone number already in use. Please enter a different phone number." << std::endl;
                }
            } while (std::to_string(newPhoneNo).length() != 11 || !User::isPhoneNumberUnique(newPhoneNo, userTable));
            userPhoneNo = newPhoneNo;
            break;
        default:
            std::cout << "Invalid choice.\n";
            break;
    }
}

bool User::isPhoneNumberUnique(long long phoneNo, const std::vector<std::vector<User>>& userTable) {
    for (const auto &bucket : userTable) {
        for (const User &user : bucket) {
            if (user.userPhoneNo == phoneNo) {
                return false;
            }
        }
    }
    return true;
}

// Driver class definitions
Driver::Driver() : driverName(""), driverPassword(""), driverPhoneNo(0), vehicleYear(0) {}

Driver::Driver(std::string name, std::string password, long long phoneNo, std::string cnic, std::string license, 
               std::string record, std::string vehicleNo, std::string model, std::string color, 
               int year, std::string registration)
    : driverName(name), driverPassword(password), driverPhoneNo(phoneNo), cnic(cnic), licenseNo(license),
      criminalRecord(record), vehicleNumber(vehicleNo), vehicleModel(model), vehicleColor(color),
      vehicleYear(year), vehicleRegistration(registration) {}

void Driver::displayDriver() const {
    std::cout << "Driver Name: " << driverName << "\nPhone number: " << driverPhoneNo
              << "\nCNIC: " << cnic << "\nLicense No: " << licenseNo
              << "\nCriminal Record: " << criminalRecord
              << "\nVehicle Info: " << vehicleModel << ", " << vehicleColor << ", " << vehicleYear
              << ", " << vehicleNumber << ", " << vehicleRegistration << std::endl;
}

std::string Driver::toCSV() const {
    return driverName + "," + driverPassword + "," + std::to_string(driverPhoneNo) + "," + cnic + "," + licenseNo + "," +
           criminalRecord + "," + vehicleNumber + "," + vehicleModel + "," + vehicleColor + "," +
           std::to_string(vehicleYear) + "," + vehicleRegistration;
}

Driver Driver::fromCSV(const std::string &line) {
    size_t pos = 0;
    size_t nextPos;
    std::vector<std::string> fields;
    while ((nextPos = line.find(',', pos)) != std::string::npos) {
        fields.push_back(line.substr(pos, nextPos - pos));
        pos = nextPos + 1;
    }
    fields.push_back(line.substr(pos));

    return Driver(fields[0], fields[1], std::stoll(fields[2]), fields[3], fields[4], fields[5], fields[6],
                  fields[7], fields[8], std::stoi(fields[9]), fields[10]);
}

void Driver::editDriverInfo(const std::vector<std::vector<Driver>>& driverTable) {
    int choice;
    std::cout << "Edit Driver Information:\n";
    std::cout << "1. Edit Username (Current: " << driverName << ")\n";
    std::cout << "2. Edit Password (Current: " << driverPassword << ")\n";
    std::cout << "3. Edit Phone Number (Current: " << driverPhoneNo << ")\n";
    std::cout << "4. Edit CNIC (Current: " << cnic << ")\n";
    std::cout << "5. Edit License Number (Current: " << licenseNo << ")\n";
    std::cout << "Enter your choice: ";
    std::cin >> choice;

    switch (choice) {
        case 1:
            std::cout << "Enter new username: ";
            std::cin >> driverName;
            break;
        case 2:
            std::cout << "Enter new password: ";
            std::cin >> driverPassword;
            break;
        case 3:
            long long newPhoneNo;
            do {
                std::cout << "Enter new phone number (11 digits): ";
                std::cin >> newPhoneNo;

                // Check if the input is a number and 11 digits long
                if (std::cin.fail() || std::to_string(newPhoneNo).length() != 11) {
                    std::cin.clear();  // Clears the fail state
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');  // Ignores the invalid input
                    std::cout << "Phone number must be 11 digits long. Please try again." << std::endl;
                } else if (!Driver::isPhoneNumberUnique(newPhoneNo, driverTable)) {
                    std::cout << "Phone number already in use. Please enter a different phone number." << std::endl;
                }
            } while (std::to_string(newPhoneNo).length() != 11 || !Driver::isPhoneNumberUnique(newPhoneNo, driverTable));
            driverPhoneNo = newPhoneNo;
            break;
        case 4:
            std::cout << "Enter new CNIC: ";
            std::cin >> cnic;
            break;
        case 5:
            std::cout << "Enter new license number: ";
            std::cin >> licenseNo;
            break;
        default:
            std::cout << "Invalid choice.\n";
            break;
    }
}

bool Driver::isPhoneNumberUnique(long long phoneNo, const std::vector<std::vector<Driver>>& driverTable) {
    for (const auto &bucket : driverTable) {
        for (const Driver &driver : bucket) {
            if (driver.driverPhoneNo == phoneNo) {
                return false;
            }
        }
    }
    return true;
}

void Driver::acceptRide() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    const char *message = "Ride accepted by driver";

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cout << "\n Socket creation error \n";
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8082);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        std::cout << "\nInvalid address/ Address not supported \n";
        return;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cout << "\nConnection Failed \n";
        return;
    }
    send(sock, message, strlen(message), 0);
    std::cout << "Ride acceptance message sent\n";
    close(sock);
}

// UserDriverHashTable class definitions
UserDriverHashTable::UserDriverHashTable() : userTable(HASH_TABLE_SIZE), driverTable(HASH_TABLE_SIZE) {}

bool UserDriverHashTable::isPhoneNumberUnique(long long phoneNo) {
    for (const auto &bucket : userTable) {
        for (const User &user : bucket) {
            if (user.userPhoneNo == phoneNo) {
                return false;
            }
        }
    }
    for (const auto &bucket : driverTable) {
        for (const Driver &driver : bucket) {
            if (driver.driverPhoneNo == phoneNo) {
                return false;
            }
        }
    }
    return true;
}

void UserDriverHashTable::insertUser() {
    std::string userName, userPassword;
    long long userPhoneNo;
    std::cout << "Enter username: ";
    std::cin >> userName;
    std::cout << "Enter password: ";
    std::cin >> userPassword;

    do {
        std::cout << "Enter phone number (11 digits): ";
        std::cin >> userPhoneNo;

        // Check if the input is a number and 11 digits long
        if (std::cin.fail() || std::to_string(userPhoneNo).length() != 11) {
            std::cin.clear();  // Clears the fail state
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');  // Ignores the invalid input
            std::cout << "Phone number must be 11 digits long. Please try again." << std::endl;
        } else if (!isPhoneNumberUnique(userPhoneNo)) {
            std::cout << "Phone number already in use. Please enter a different phone number." << std::endl;
        }
    } while (std::to_string(userPhoneNo).length() != 11 || !isPhoneNumberUnique(userPhoneNo));

    int index = hashFunction(userName);
    userTable[index].push_back(User(userName, userPassword, userPhoneNo));
    std::cout << "User \"" << userName << "\" successfully registered." << std::endl;

    saveToFile("users.txt", "drivers.txt"); // Save data to file
}

void UserDriverHashTable::insertDriver() {
    std::string driverName, driverPassword, cnic, licenseNo, criminalRecord, vehicleNumber, vehicleModel, vehicleColor, vehicleRegistration;
    long long driverPhoneNo;
    int vehicleYear;

    std::cout << "Enter driver username: ";
    std::cin >> driverName;
    std::cout << "Enter driver password: ";
    std::cin >> driverPassword;

    do {
        std::cout << "Enter driver phone number (11 digits): ";
        std::cin >> driverPhoneNo;

        // Check if the input is a number
        if (std::cin.fail() || std::to_string(driverPhoneNo).length() != 11) {
            // Clear the error flag and ignore invalid input
            std::cin.clear();  // Clears the fail state
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');  // Ignores the invalid input

            std::cout << "Phone number must be 11 digits long. Please try again." << std::endl;
        } else if (!isPhoneNumberUnique(driverPhoneNo)) {
            std::cout << "Phone number already in use. Please enter a different phone number." << std::endl;
        }
    } while (std::to_string(driverPhoneNo).length() != 11 || !isPhoneNumberUnique(driverPhoneNo));

    do {
        std::cout << "Enter CNIC (13 digits without dashes): ";
        std::cin >> cnic;
        if (cnic.length() != 13 || !std::all_of(cnic.begin(), cnic.end(), ::isdigit)) {
            std::cout << "CNIC must be 13 digits long without dashes. Please try again." << std::endl;
        }
    } while (cnic.length() != 13 || !std::all_of(cnic.begin(), cnic.end(), ::isdigit));

    do {
        std::cout << "Enter Criminal Record in last 12 months (yes/no): ";
        std::cin >> criminalRecord;

        if (criminalRecord == "yes" || criminalRecord == "Yes") {
            std::cout << "\nUnable to register driver due to criminal record in last 12 months." << std::endl;
            return;
        } else if (criminalRecord == "no" || criminalRecord == "No") {
            break;  // Continue to the next step
        } else {
            std::cout << "Invalid input. Please enter 'yes' for yes or 'no' for no." << std::endl;
        }
    } while (criminalRecord != "yes" && criminalRecord != "Yes" && criminalRecord != "no" && criminalRecord != "No");

    std::cout << "Enter Vehicle Model: ";
    std::cin >> vehicleModel;

    // Vehicle Registration Number validation
    do {
        std::cout << "Enter Vehicle Registration Number (3-4 alphabets followed by 2-3 digits): ";
        std::cin >> vehicleRegistration;
        std::regex reg("^[A-Za-z]{3,4}[0-9]{2,3}$");
        if (!std::regex_match(vehicleRegistration, reg)) {
            std::cout << "Invalid vehicle registration number. Please try again." << std::endl;
        }
    } while (!std::regex_match(vehicleRegistration, std::regex("^[A-Za-z]{3,4}[0-9]{2,3}$")));

    
    // Vehicle Year validation
    do {
        std::cout << "Enter Vehicle Year (2000-2024): ";
        std::cin >> vehicleYear;
        if (vehicleYear > 2024) {
            std::cout << "Vehicle year cannot be above 2024. Please enter a valid year." << std::endl;
        } else if (vehicleYear < 2000) {
            std::cout << "\nVehicle is too old. Unable to register." << std::endl;
            return;  // Exit if the year is too old
        }
    } while (vehicleYear > 2024 || (vehicleYear < 2000 && vehicleYear > 0));  // Keep prompting until valid year

    // Vehicle Color input validation
    do {
        std::cout << "Enter Vehicle Closest Color (Red, Blue, Green, Black, White): ";
        std::cin >> vehicleColor;
        if (vehicleColor != "Red" && vehicleColor != "Blue" && vehicleColor != "Green" &&
            vehicleColor != "Black" && vehicleColor != "White") {
            std::cout << "Invalid color. Please select from Red, Blue, Green, Black, or White." << std::endl;
        }
    } while (vehicleColor != "Red" && vehicleColor != "Blue" && vehicleColor != "Green" &&
             vehicleColor != "Black" && vehicleColor != "White");

    int index = hashFunction(driverName);
    driverTable[index].push_back(Driver(driverName, driverPassword, driverPhoneNo, cnic, licenseNo, criminalRecord,
                                        vehicleNumber, vehicleModel, vehicleColor, vehicleYear, vehicleRegistration));
    std::cout << "\n\nDriver \"" << driverName << "\" successfully registered." << std::endl;

    saveToFile("users.txt", "drivers.txt"); // Save data to file
}

void UserDriverHashTable::saveToFile(const std::string &userFile, const std::string &driverFile) {
    std::ofstream outFile(userFile);
    for (const auto &bucket : userTable) {
        for (const User &user : bucket) {
            outFile << user.toCSV() << std::endl;
        }
    }
    outFile.close();

    outFile.open(driverFile);
    for (const auto &bucket : driverTable) {
        for (const Driver &driver : bucket) {
            outFile << driver.toCSV() << std::endl;
        }
    }
    outFile.close();
}

void UserDriverHashTable::loadFromFile(const std::string &userFile, const std::string &driverFile) {
    std::ifstream inFile(userFile);
    std::string line;
    while (std::getline(inFile, line)) {
        User user = User::fromCSV(line);
        int index = hashFunction(user.userName);
        userTable[index].push_back(user);
    }
    inFile.close();

    inFile.open(driverFile);
    while (std::getline(inFile, line)) {
        Driver driver = Driver::fromCSV(line);
        int index = hashFunction(driver.driverName);
        driverTable[index].push_back(driver);
    }
    inFile.close();
}

bool UserDriverHashTable::logInUser(const std::string &userName, const std::string &password) {
    int index = hashFunction(userName);
    for (const User &u : userTable[index]) {
        if (u.userName == userName) {
            if (u.userPassword == password) {
                std::cout << "Log in successful. Welcome " << userName << std::endl;
                return true; // Return true for successful login
            } else {
                std::cout << "Error: Incorrect password." << std::endl;
                return false; // Return false for incorrect password
            }
        }
    }
    std::cout << "Error: Username not found." << std::endl;
    return false; // Return false if username not found
}

bool UserDriverHashTable::logInDriver(const std::string &driverName, const std::string &password) {
    int index = hashFunction(driverName);
    for (const Driver &d : driverTable[index]) {
        if (d.driverName == driverName) {
            if (d.driverPassword == password) {
                std::cout << "Log in successful. Welcome " << driverName << std::endl;
                return true; // Return true for successful login
            } else {
                std::cout << "Error: Incorrect password." << std::endl;
                return false; // Return false for incorrect password
            }
        }
    }
    std::cout << "Error: Driver username not found." << std::endl;
    return false; // Return false if driver username not found
}

void UserDriverHashTable::displayUsers() const {
    for (const auto &bucket : userTable) {
        for (const User &user : bucket) {
            user.displayUser();
        }
    }
}

void UserDriverHashTable::displayDrivers() const {
    for (const auto &bucket : driverTable) {
        for (const Driver &driver : bucket) {
            driver.displayDriver();
        }
    }
}

const std::vector<std::vector<User>>& UserDriverHashTable::getUserTable() const {
    return userTable;
}

const std::vector<std::vector<Driver>>& UserDriverHashTable::getDriverTable() const {
    return driverTable;
}
