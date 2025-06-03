// Updated sales_system.cpp with refined loadInventory

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <cstdlib>
#include <algorithm>
#include <limits>
#include <tuple> 

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

using namespace std;

// COLORS
const string YELLOW = "\033[0;33m";
const string RESET = "\033[0m";
const string BOLD_GREEN = "\033[1;32m";
const string BOLD_CYAN = "\033[1;36m";
const string BOLD_MAGENTA = "\033[1;35m";
const string RED = "\033[0;31m";
const string BOLD_BLUE = "\033[1;34m";
const string BOLD_YELLOW = "\033[1;33m";
const string CYAN = "\033[0;36m";
const string MAGENTA = "\033[0;35m";
const string UND_RED = "\033[4;31m";

// UTILITY FUNCTIONS
void clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void pauseScreen() {
    cout << CYAN << "\n";
    cout << "           ______________________________\n";
    cout << "          |                              |\n";
    cout << "          |   Press Enter to continue.   |\n";
    cout << "          |______________________________|\n";
    cout << RESET << "\n";
    if (cin.rdbuf()->in_avail() > 0) {
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    if (cin.peek() == '\n') { 
        cin.ignore();
    }
    cin.get();
}

string generateProductID() {
    int id = rand() % 900000 + 100000;
    return to_string(id);
}

string generateReceiptID() {
    int id = rand() % 900000 + 100000;
    return to_string(id);
}

// STRUCTURES
struct Product {
    string id;
    string name;
    int quantity;
    double price;
};

struct Sale {
    string receiptID;
    string customerName;
    vector<pair<string, int>> products;
    double totalAmount;
    double customerCash;
    double change;
    string dateTime;
};

// GLOBALS
map<string, Product> inventory;
vector<Sale> salesHistory;

Product* searchProductByID(const string& id); // Forward declaration

// --- REFINED loadInventory FUNCTION ---
void loadInventory() {
    ifstream file("inventory.txt");
    if (!file.is_open()) {
        // Optional: cerr << "Warning: Could not open inventory.txt for loading." << endl;
        return;
    }

    string line;
    while (getline(file, line)) {
        // Skip empty or whitespace-only lines
        if (line.empty() || line.find_first_not_of(" \t\n\v\f\r") == string::npos) {
            continue;
        }

        istringstream iss(line);
        Product p;
        
        // 1. Read the product ID
        if (!(iss >> p.id)) {
            // Optional: cerr << "Warning: Failed to read product ID from line: " << line << endl;
            continue; // Skip malformed line
        }

        // 2. Consume ALL leading whitespace before the product name.
        iss >> std::ws; 

        // 3. Read the product name up to (but not including) the '|' delimiter.
        if (!getline(iss, p.name, '|')) {
            // Optional: cerr << "Warning: Failed to read product name from line: " << line << " after ID: " << p.id << endl;
            continue; // Skip malformed line if name can't be read
        }
        
        // 4. Read quantity and price
        if (!(iss >> p.quantity >> p.price)) {
            // Optional: cerr << "Warning: Failed to read quantity/price from line: " << line << " for product: " << p.name << endl;
            continue; // Skip malformed line
        }
        
        inventory[p.id] = p;
    }
    file.close();
}
// --- END OF REFINED loadInventory FUNCTION ---

void loadSalesHistory() {
    ifstream file("sales_history.txt");
    if (!file.is_open()) return;

    string line;
    while (getline(file, line)) {
        if (line.find("Receipt ID:") != string::npos) {
            Sale sale;
            sale.receiptID = line.substr(line.find(":") + 2);
            
            getline(file, line); 
            sale.customerName = line.substr(line.find(":") + 2);
            
            getline(file, line); 
            sale.dateTime = line.substr(line.find(":") + 2);
            
            getline(file, line); 
            
            while (getline(file, line) && line.find("---") == string::npos && !line.empty()) {
                size_t id_sep = line.find("|");
                size_t xpos = line.find(" x", id_sep != string::npos ? id_sep + 1 : 0);
                size_t atpos = line.find(" @ $", xpos != string::npos ? xpos + 1 : 0);

                if (id_sep != string::npos && xpos != string::npos && atpos != string::npos) {
                    string productIDFromFile = line.substr(0, id_sep);
                    int quantity = stoi(line.substr(xpos + 2, atpos - (xpos + 2)));
                    if (!productIDFromFile.empty()) {
                         sale.products.push_back({productIDFromFile, quantity});
                    }
                }
            }
            if (line.find("---") == string::npos) { 
                 while (getline(file, line) && line.find("Total Amount:") == string::npos) {
                    if (line.find(string(40, '=')) != string::npos) break; 
                 }
            }
            if (line.find("Total Amount:") == string::npos && line.find(string(40, '=')) == string::npos) getline(file, line); 
            if (line.find("$") != string::npos) sale.totalAmount = stod(line.substr(line.find("$") + 1));
            
            getline(file, line); 
             if (line.find("$") != string::npos) sale.customerCash = stod(line.substr(line.find("$") + 1));
            
            getline(file, line); 
            if (line.find("$") != string::npos) sale.change = stod(line.substr(line.find("$") + 1));
            
            if (line.find(string(40, '=')) == string::npos) getline(file, line);
            
            salesHistory.push_back(sale);
        }
    }
    file.close();
}

void saveInventory() {
    ofstream file("inventory.txt");
    if (!file.is_open()) {
        cerr << "Error: Could not open inventory.txt for saving." << endl;
        return;
    }
    for (const auto& pair : inventory) {
        file << pair.second.id << " " << pair.second.name << "|" 
             << pair.second.quantity << " " << fixed << setprecision(2) << pair.second.price << endl;
    }
    file.close();
}

void saveSalesHistory() {
    ofstream file("sales_history.txt");
    if (!file.is_open()) {
        cerr << "Error: Could not open sales_history.txt for saving." << endl;
        return;
    }
    for (const auto& sale : salesHistory) {
        file << "Receipt ID: " << sale.receiptID << endl;
        file << "Customer Name: " << sale.customerName << endl;
        file << "Date and Time: " << sale.dateTime; 
        if (!sale.dateTime.empty() && sale.dateTime.back() != '\n') file << endl; 
        file << "Sales Record:\n";
        for (const auto& item : sale.products) { 
            Product* p = searchProductByID(item.first);
            if (p) {
                file << item.first << "|" << p->name << " x" << item.second << " @ $" << fixed << setprecision(2) << p->price 
                     << " = $" << fixed << setprecision(2) << (item.second * p->price) << endl;
            } else {
                file << item.first << "|Unknown Product x" << item.second << " @ $0.00 = $0.00" << endl;
            }
        }
        file << string(40, '-') << endl;
        file << "Total Amount: $" << fixed << setprecision(2) << sale.totalAmount << endl;
        file << "Customer Cash: $" << fixed << setprecision(2) << sale.customerCash << endl;
        file << "Change: $" << fixed << setprecision(2) << sale.change << endl;
        file << string(40, '=') << endl << endl;
    }
    file.close();
}

void displayInventory() {
    cout << "\n";
    cout << left << setw(10) << YELLOW << "ID" << setw(30) << "   Product Name" 
         << setw(10) << "   Quantity" << setw(10) << "   Price" << "     Status" << RESET << endl;
    cout << YELLOW << string(70, '-') << RESET << endl;
    if (inventory.empty()) {
        cout << RED << "Inventory is empty." << RESET << endl;
    } else {
        for (const auto& pair : inventory) {
            const Product& p = pair.second;
            string status;
            string quantityColor = BOLD_GREEN; 

            if (p.quantity == 0) {
                status = "Out of Stock";
                quantityColor = RED;
            } else if (p.quantity < 21) { 
                status = "Low Stock";
                quantityColor = BOLD_YELLOW;
            } else if (p.quantity >= 100) {
                status = "Full Stock";
            } else {
                status = "Medium Stock";
            }

            cout << BOLD_GREEN << left << setw(10) << p.id << setw(30) << p.name 
                 << quantityColor << setw(10) << p.quantity << BOLD_GREEN << setw(10) << fixed << setprecision(2) << p.price 
                 << (p.quantity == 0 ? RED : (p.quantity < 21 ? BOLD_YELLOW : BOLD_GREEN)) << status << RESET << endl;
        }
    }
    cout << YELLOW << string(70, '-') << RESET << endl;
}

Product* searchProductByID(const string& id) {
    auto it = inventory.find(id);
    if (it != inventory.end()) {
        return &it->second;
    }
    return nullptr;
}

void addNewProduct() {
    Product p;
    bool idExists;
    do {
        idExists = false;
        p.id = generateProductID();
        if (inventory.count(p.id)) {
            idExists = true;
        }
    } while (idExists);
    
    if (cin.peek() == '\n') cin.ignore(); 

    do {
        cout << CYAN << "Enter product name: " << BOLD_GREEN;
        getline(cin, p.name);
        if (p.name.empty()) {
            cout << RED << "Error: Product name cannot be empty. Please try again.\n" << RESET;
        }
    } while (p.name.empty());

    do {
        cout << CYAN << "Enter quantity: " << BOLD_GREEN;
        string input;
        getline(cin, input); 
        try {
            p.quantity = stoi(input);
            if (p.quantity < 0) {
                cout <<  RED <<"Error: Quantity cannot be negative. Please enter a non-negative number.\n" << RESET;
                continue;
            }
            break; 
        } catch (const std::invalid_argument& ia) {
            cout << RED << "Error: Invalid quantity. Please enter a whole number.\n" << RESET;
        } catch (const std::out_of_range& oor) {
            cout << RED << "Error: Quantity out of range. Please enter a smaller whole number.\n" << RESET;
        }
    } while (true);

    do {
        cout << CYAN << "Enter price: " << BOLD_GREEN << "$";
        string input;
        getline(cin, input); 
        try {
            p.price = stod(input);
            if (p.price <= 0) {
                cout << RED << "Error: Price must be greater than 0. Please enter a valid price.\n" << RESET;
                continue;
            }
            break; 
        } catch (const std::invalid_argument& ia) {
            cout << RED << "Error: Invalid price format. Please enter a numeric value (e.g., 12.99).\n" << RESET;
        } catch (const std::out_of_range& oor) {
            cout << RED << "Error: Price out of range. Please enter a valid price.\n" << RESET;
        }
    } while (true);

    inventory[p.id] = p;
    cout << BOLD_GREEN << "\n";
    cout << "           _________________________________\n";
    cout << "          |                                 |\n";
    cout << "          |   Product added successfully.   |\n";
    cout << "          |_________________________________|\n";
    cout << RESET << "\n";
    cout << YELLOW << "ID: "<< BOLD_GREEN << p.id << YELLOW << " | Name: " << BOLD_GREEN << p.name 
         << YELLOW << " | Qty: "<< BOLD_GREEN << p.quantity << YELLOW << " | Price: " << "$" << BOLD_GREEN << fixed << setprecision(2) << p.price << RESET << endl;
    saveInventory();
    cout << "\n";
}

void displayCurrentProducts(const Sale& currentSale, bool showSubtotal) {
    if (currentSale.products.empty()) {
        cout << UND_RED << "\nNo Products added yet.\n" << RESET;
        return;
    }
    
    cout << CYAN <<  "\nCurrent Products in Sale (Receipt ID: " << BOLD_GREEN << currentSale.receiptID << CYAN << "):\n" << RESET;
    cout << YELLOW << string(65, '-') << endl;
    cout << left << setw(10) << "ID" << setw(25) << "Product Name"<< setw(10) << "Quantity" << setw(10) << "Unit $" << setw(10) << "Total $" << RESET << endl;
    cout << YELLOW << string(65, '-') << endl;
    
    double subtotal = 0.0;
    for (const auto& item : currentSale.products) { 
        Product* p = searchProductByID(item.first);
        if (p) {
            double itemTotal = item.second * p->price;
            subtotal += itemTotal;
            cout << BOLD_GREEN << left << setw(10) << p->id << setw(25) << p->name 
                 << setw(10) << item.second 
                 << setw(10) << fixed << setprecision(2) << p->price
                 << setw(10) << fixed << setprecision(2) << itemTotal << RESET << endl;
        }
    }
    cout << YELLOW <<  string(65, '-') << RESET << endl;
    if (showSubtotal && !currentSale.products.empty()) {
        cout << BOLD_CYAN << right << setw(55) << "Current Subtotal: " 
             << BOLD_GREEN << "$" << fixed << setprecision(2) << subtotal << RESET << "\n\n";
    } else {
        cout << "\n";
    }
}

void cashierMode() {
    Sale currentSale;
    currentSale.receiptID = generateReceiptID();
    currentSale.totalAmount = 0.0;
    currentSale.customerCash = 0.0;
    currentSale.change = 0.0;

    while (true) {
        clearScreen();
        cout << BOLD_CYAN << "\nAvailable Products in Inventory:\n" << RESET;
        displayInventory();
        cout << "\n";
        
        displayCurrentProducts(currentSale, true); 

        vector<tuple<int, string, string>> menu_options_tuples;
        int opt_idx = 1;
        int choice_add = opt_idx++;
        menu_options_tuples.emplace_back(choice_add, to_string(choice_add) + ". Add Product Sale", BOLD_GREEN);
        
        int choice_delete = opt_idx++;
        menu_options_tuples.emplace_back(choice_delete, to_string(choice_delete) + ". Delete Punched Product", RED);
        
        int choice_payment = -1; 
        if (!currentSale.products.empty()) {
            choice_payment = opt_idx++;
            menu_options_tuples.emplace_back(choice_payment, to_string(choice_payment) + ". Proceed to Payment", BOLD_BLUE);
        }
        
        int choice_cancel = opt_idx++;
        menu_options_tuples.emplace_back(choice_cancel, to_string(choice_cancel) + ". Cancel Transaction", YELLOW);
        
        const int boxWidth = 32;
        cout << BOLD_CYAN;
        for (size_t i = 0; i < menu_options_tuples.size(); i += 2) {
            cout << "   +"; for(int k=0; k<boxWidth-2; ++k) cout << "-"; cout << "+   ";
            if (i + 1 < menu_options_tuples.size()) {
                cout << "+"; for(int k=0; k<boxWidth-2; ++k) cout << "-"; cout << "+";
            }
            cout << "\n";

            string opt1_text = get<1>(menu_options_tuples[i]);
            string opt1_display = get<2>(menu_options_tuples[i]) + opt1_text + RESET; // Add RESET for color
            cout << "   | " << left << setw(boxWidth - 2 + get<2>(menu_options_tuples[i]).length() + RESET.length() ) << opt1_display << BOLD_CYAN << "|   ";


            if (i + 1 < menu_options_tuples.size()) {
                 string opt2_text = get<1>(menu_options_tuples[i+1]);
                 string opt2_display = get<2>(menu_options_tuples[i+1]) + opt2_text + RESET;
                 cout << "| " << left << setw(boxWidth - 2 + get<2>(menu_options_tuples[i+1]).length() + RESET.length()) << opt2_display << BOLD_CYAN << "|";
            } else {
                 cout << string(boxWidth + 3, ' '); 
            }
            cout << "\n";
            cout << "   +"; for(int k=0; k<boxWidth-2; ++k) cout << "-"; cout << "+   ";
            if (i + 1 < menu_options_tuples.size()) {
                cout << "+"; for(int k=0; k<boxWidth-2; ++k) cout << "-"; cout << "+";
            }
            cout << "\n";
        }
        cout << RESET;

        int user_choice_input;
        int max_valid_choice = opt_idx - 1;
        cout << BOLD_YELLOW << "\nEnter choice: " << RESET;
        string raw_input_choice;
        getline(cin, raw_input_choice); 

        try {
            if(raw_input_choice.empty()) throw std::invalid_argument("empty input");
            user_choice_input = stoi(raw_input_choice);
            if (user_choice_input < 1 || user_choice_input > max_valid_choice) {
                 throw std::out_of_range("invalid choice");
            }
            // Validate specific choice against availability (e.g., payment only if items exist)
            if (user_choice_input == choice_payment && choice_payment == -1) { // Trying to pay when not an option
                 throw std::out_of_range("payment not available");
            }

        } catch (const std::exception& e) {
            cout << RED << "Invalid choice. Please enter a valid option number.\n" << RESET;
            pauseScreen();
            continue;
        }
        
        if (user_choice_input == choice_add) {
            clearScreen();
            displayInventory(); 
            cout << "\n";

            int searchChoice_val;
            cout << BOLD_YELLOW << "How would you like to find the product?\n";
            cout << "1. By Product ID\n";
            cout << "2. By Product Name\n";
            cout << "0. Cancel Adding\n";
            cout << "Enter choice: " << RESET;
            
            string search_choice_str;
            getline(cin, search_choice_str);
            try {
                if(search_choice_str.empty()) throw std::invalid_argument("empty");
                searchChoice_val = stoi(search_choice_str);
                if (searchChoice_val < 0 || searchChoice_val > 2) throw std::out_of_range("invalid");
            } catch(const std::exception& e) {
                cout << RED << "Invalid search choice. Cancelling add product.\n" << RESET;
                pauseScreen();
                continue;
            }

            if (searchChoice_val == 0) continue;

            Product* p_selected = nullptr;
            if (searchChoice_val == 1) { 
                string productID_input;
                cout << BOLD_YELLOW << "Enter Product ID (or '0' to cancel): " << RESET;
                getline(cin, productID_input);
                if (productID_input == "0" || productID_input.empty()) continue;
                p_selected = searchProductByID(productID_input);
            } else { 
                string searchTerm;
                cout << BOLD_YELLOW << "Enter Product Name (or '0' to cancel): " << RESET;
                getline(cin, searchTerm);
                if (searchTerm == "0" || searchTerm.empty()) continue;
                
                string searchTermLower = searchTerm;
                transform(searchTermLower.begin(), searchTermLower.end(), searchTermLower.begin(), 
                          [](unsigned char c){ return std::tolower(c); });
                
                vector<Product*> matchedProducts;
                for (auto& inv_pair : inventory) {
                    string currentNameLower = inv_pair.second.name;
                    transform(currentNameLower.begin(), currentNameLower.end(), currentNameLower.begin(),
                              [](unsigned char c){ return std::tolower(c); });
                    if (currentNameLower.find(searchTermLower) != string::npos) {
                        matchedProducts.push_back(&inv_pair.second);
                    }
                }

                if (matchedProducts.empty()) { /* p_selected remains nullptr */ } 
                else if (matchedProducts.size() == 1) { p_selected = matchedProducts[0]; } 
                else {
                    cout << BOLD_CYAN << "Multiple products found. Please choose one:\n" << RESET;
                    for (size_t i = 0; i < matchedProducts.size(); ++i) {
                        cout << BOLD_GREEN << i + 1 << ". " << matchedProducts[i]->name
                             << " (ID: " << matchedProducts[i]->id << ", Stock: " << matchedProducts[i]->quantity 
                             << ", Price: $" << fixed << setprecision(2) << matchedProducts[i]->price << ")" << RESET << endl;
                    }
                    cout << BOLD_YELLOW << "Enter your choice (number) or 0 to cancel: " << RESET;
                    string sub_choice_str;
                    getline(cin, sub_choice_str);
                    int subChoiceNum_val;
                    try {
                        if(sub_choice_str.empty()) throw std::invalid_argument("empty");
                        subChoiceNum_val = stoi(sub_choice_str);
                        if (subChoiceNum_val < 0 || subChoiceNum_val > static_cast<int>(matchedProducts.size())) throw std::out_of_range("invalid");
                        if (subChoiceNum_val > 0) {
                            p_selected = matchedProducts[subChoiceNum_val - 1];
                        }
                    } catch (const std::exception& e) {
                         cout << RED << "Invalid selection. Product not chosen.\n" << RESET;
                    }
                }
            }

            if (p_selected) {
                if (p_selected->quantity <= 0) {
                    cout << RED << "Product " << BOLD_GREEN << "'"<< p_selected->name << "' " << RED << "is out of stock.\n" << RESET;
                } else {
                    cout << CYAN << "\nSelected Product: " << BOLD_GREEN << p_selected->name << RESET << endl;
                    cout << CYAN << "Available Stock: " << BOLD_GREEN << p_selected->quantity << RESET << endl;
                    cout << CYAN << "Price per piece: " << BOLD_GREEN << "$" << fixed << setprecision(2) << p_selected->price << RESET << endl;
                    
                    int qty_to_add_val;
                    while (true) {
                        cout << BOLD_YELLOW << "Enter quantity to add (or '0' to cancel): " << RESET;
                        string qty_str;
                        getline(cin, qty_str);
                        try {
                             if(qty_str.empty()) throw std::invalid_argument("empty");
                             qty_to_add_val = stoi(qty_str);
                        } catch (const std::exception& e) {
                            cout << RED << "Error: Invalid input. Please enter a number.\n" << RESET;
                            continue;
                        }

                        if (qty_to_add_val == 0) break;
                        if (qty_to_add_val < 0) {
                            cout << RED << "Error: Quantity cannot be negative.\n" << RESET;
                            continue;
                        }
                        if (qty_to_add_val > p_selected->quantity) {
                            cout << RED << "Insufficient stock. Available: " << p_selected->quantity << RESET << endl;
                            char add_available_choice_char;
                            cout << BOLD_YELLOW << "Add available quantity (" << p_selected->quantity << ") instead? (y/n): " << RESET;
                            string add_avail_str;
                            getline(cin, add_avail_str);
                            add_available_choice_char = (add_avail_str.empty() ? 'n' : tolower(add_avail_str[0]));
                            
                            if (add_available_choice_char == 'y') {
                                qty_to_add_val = p_selected->quantity;
                            } else {
                                cout << RED << "Product not added.\n" << RESET;
                                qty_to_add_val = 0; 
                                break;
                            }
                        }
                        currentSale.products.push_back({p_selected->id, qty_to_add_val});
                        p_selected->quantity -= qty_to_add_val; 
                        cout << BOLD_GREEN << "\nProduct added to sale: " << qty_to_add_val << " x " << p_selected->name << RESET << endl;
                        cout << CYAN << "Cost: " << BOLD_GREEN << qty_to_add_val << CYAN << " pcs x $" << BOLD_GREEN << fixed << setprecision(2) << p_selected->price 
                             << CYAN << " = $" << BOLD_GREEN << fixed << setprecision(2) << (qty_to_add_val * p_selected->price) << RESET << endl;
                        break; 
                    }
                }
            } else {
                cout << RED << "Product not found or selection cancelled.\n" << RESET;
            }
            pauseScreen();
        }
        else if (user_choice_input == choice_delete) {
            if (currentSale.products.empty()) {
                cout << RED << "No products in the current sale to delete.\n" << RESET;
            } else {
                string adminKey;
                cout << BOLD_GREEN << "Enter Admin Key to delete product from sale: " << RESET;
                getline(cin, adminKey);
                if (adminKey == "admin123") {
                    string productID_to_remove;
                    cout << BOLD_YELLOW << "Enter Product ID of item to remove from sale: " << RESET;
                    getline(cin, productID_to_remove);
                    
                    auto it = find_if(currentSale.products.begin(), currentSale.products.end(),
                                      [&](const pair<string, int>& item){ return item.first == productID_to_remove; });
                    
                    if (it != currentSale.products.end()) {
                        Product* p_inv = searchProductByID(it->first);
                        if (p_inv) {
                            p_inv->quantity += it->second; 
                        }
                        currentSale.products.erase(it);
                        cout << BOLD_GREEN << "Product removed from sale. Stock restored.\n" << RESET;
                    } else {
                        cout << RED << "Product ID not found in current sale.\n" << RESET;
                    }
                } else {
                    cout << RED << "Invalid Admin Key. Deletion cancelled.\n" << RESET;
                }
            }
            pauseScreen();
        }
        else if (choice_payment != -1 && user_choice_input == choice_payment) {
            clearScreen();
            if (currentSale.customerName.empty()) {
                 do {
                    cout << BOLD_YELLOW << "Enter Customer Name: " << RESET;
                    getline(cin, currentSale.customerName);
                    if (currentSale.customerName.empty()) {
                        cout << RED << "Error: Customer name cannot be empty. Please try again.\n" << RESET;
                    }
                } while (currentSale.customerName.empty());
            }

            currentSale.totalAmount = 0.0;
            for (const auto& item : currentSale.products) {
                Product* p = searchProductByID(item.first);
                if (p) currentSale.totalAmount += (item.second * p->price);
            }

            cout << CYAN << "\n           RECEIPT PREVIEW\n" << RESET;
            cout << BOLD_YELLOW << "======================================\n" << RESET;       
            cout << YELLOW << "Receipt ID: " << BOLD_GREEN << currentSale.receiptID << endl;
            cout << YELLOW << "Customer Name: " << BOLD_GREEN << currentSale.customerName << endl;
            cout << YELLOW << "Items:\n";
            for (const auto& item : currentSale.products) {
                Product* p = searchProductByID(item.first);
                if (p) {
                    cout << YELLOW << "  " << p->name << " x" << item.second << " @ $" << fixed << setprecision(2) << p->price 
                         << " = " << BOLD_GREEN << "$" << fixed << setprecision(2) << (item.second * p->price) << RESET << endl;
                }
            }
            cout << BOLD_YELLOW << "--------------------------------------\n" << RESET;
            cout << YELLOW << "Total Amount: " << BOLD_GREEN << "$" << fixed << setprecision(2) << currentSale.totalAmount << endl;
            cout << BOLD_YELLOW << "======================================\n" << RESET;
            
            string cash_str;
            do {
                cout << YELLOW << "Customer Cash: " << BOLD_GREEN << "$";
                getline(cin, cash_str);
                try {
                    if(cash_str.empty()) throw std::invalid_argument("empty");
                    currentSale.customerCash = stod(cash_str);
                    if (currentSale.customerCash < currentSale.totalAmount) {
                       cout << RED << "Error: Insufficient cash. Total amount is " << BOLD_GREEN << "$" << fixed << setprecision(2) << currentSale.totalAmount << RESET << endl;
                    }
                } catch (const std::exception& e) {
                     cout << RED << "Invalid input for cash. Please enter a numeric value.\n" << RESET;
                     currentSale.customerCash = -1; 
                }
            } while (currentSale.customerCash < currentSale.totalAmount);
            
            currentSale.change = currentSale.customerCash - currentSale.totalAmount;
            time_t now_time_t = time(0);
            char time_buf[100];
            strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", localtime(&now_time_t));
            currentSale.dateTime = time_buf; 

            salesHistory.push_back(currentSale);
            saveSalesHistory();
            saveInventory();
            
            clearScreen();
            cout << CYAN << "\n           FINAL RECEIPT\n" << RESET;
            cout << BOLD_YELLOW << "======================================\n" << RESET;       
            cout << YELLOW << "Receipt ID: " << BOLD_GREEN << currentSale.receiptID << endl;
            cout << YELLOW << "Customer Name: " << BOLD_GREEN << currentSale.customerName << endl;
            cout << YELLOW << "Date and Time: " << BOLD_GREEN << currentSale.dateTime << RESET << endl;
            cout << BOLD_YELLOW << "--------------------------------------\n" << RESET;
            cout << YELLOW << "Items:\n";
            for (const auto& item : currentSale.products) {
                Product* p = searchProductByID(item.first);
                if (p) {
                    cout << YELLOW << "  " << p->name << " x" << item.second << " @ $" << fixed << setprecision(2) << p->price 
                         << " = " << BOLD_GREEN << "$" << fixed << setprecision(2) << (item.second * p->price) << RESET << endl;
                }
            }
            cout << BOLD_YELLOW << "--------------------------------------\n" << RESET;
            cout << YELLOW << "Total Amount:  $" << BOLD_GREEN << fixed << setprecision(2) << currentSale.totalAmount << endl;
            cout << YELLOW << "Customer Cash: $" << BOLD_GREEN << fixed << setprecision(2) << currentSale.customerCash << endl;
            cout << YELLOW << "Change:        $" << BOLD_GREEN << fixed << setprecision(2) << currentSale.change << endl;
            cout << BOLD_YELLOW << "======================================\n" << RESET;
            cout << BOLD_GREEN << "\nTransaction completed. Receipt saved.\n" << RESET;
            pauseScreen();
            return; 
        }
        else if (user_choice_input == choice_cancel) {
            if (!currentSale.products.empty()) {
                cout << BOLD_YELLOW << "Restoring stock for cancelled items...\n" << RESET;
                for (const auto& item : currentSale.products) {
                    Product* p = searchProductByID(item.first);
                    if (p) p->quantity += item.second;
                }
                saveInventory(); 
            }
            cout << RED << "\nTransaction cancelled.\n" << RESET;
            pauseScreen();
            return; 
        }
    }
}

void editProduct() {
    string adminKey;
    cout << BOLD_GREEN << "Enter Admin Key to edit stocks: " << RESET;
    getline(cin, adminKey);

    if (adminKey != "admin123") {
        cout << RED << "\nInvalid Admin Key. Operation cancelled.\n" << RESET;
        return;
    }

    displayInventory(); 
    string productID;
    cout << BOLD_YELLOW << "\nEnter Product ID to edit (or '0' to cancel): " << RESET;
    getline(cin, productID);

    if (productID == "0" || productID.empty()) {
        cout << CYAN << "Edit operation cancelled.\n" << RESET;
        return;
    }

    Product* p_to_edit = searchProductByID(productID);
    if (!p_to_edit) {
        cout << RED << "\nProduct with ID '" << productID << "' not found.\n" << RESET;
        return;
    }

    clearScreen();
    cout << CYAN << "\nEditing Product ID: " << BOLD_GREEN << p_to_edit->id << RESET << endl;
    cout << YELLOW << "Current Details:\n";
    cout << "  1. Name:     " << BOLD_GREEN << p_to_edit->name << RESET << "\n";
    cout << "  2. Quantity: " << BOLD_GREEN << p_to_edit->quantity << RESET << "\n";
    cout << "  3. Price:    $" << BOLD_GREEN << fixed << setprecision(2) << p_to_edit->price << RESET << "\n";
    cout << YELLOW << "-----------------------------------\n" << RESET;
    cout << BOLD_CYAN << "Enter new values. Press Enter to keep current value.\n" << RESET;

    string tempInput;

    cout << "New Name (current: " << BOLD_GREEN << p_to_edit->name << RESET << "): ";
    getline(cin, tempInput);
    if (!tempInput.empty()) {
        p_to_edit->name = tempInput;
    }

    cout << "New Quantity (current: " << BOLD_GREEN << p_to_edit->quantity << RESET << "): ";
    getline(cin, tempInput);
    if (!tempInput.empty()) {
        try {
            int new_qty = stoi(tempInput);
            if (new_qty < 0) {
                cout << RED << "Quantity cannot be negative. Value not changed.\n" << RESET;
            } else {
                p_to_edit->quantity = new_qty;
            }
        } catch (const std::exception& e) {
            cout << RED << "Invalid quantity input. Value not changed.\n" << RESET;
        }
    }

    cout << "New Price (current: $" << BOLD_GREEN << fixed << setprecision(2) << p_to_edit->price << RESET << "): $";
    getline(cin, tempInput);
    if (!tempInput.empty()) {
        try {
            double new_price = stod(tempInput);
            if (new_price <= 0) {
                cout << RED << "Price must be positive. Value not changed.\n" << RESET;
            } else {
                p_to_edit->price = new_price;
            }
        } catch (const std::exception& e) {
            cout << RED << "Invalid price input. Value not changed.\n" << RESET;
        }
    }

    saveInventory();
    cout << BOLD_GREEN << "\nProduct details updated successfully!\n" << RESET;
    cout << YELLOW << "New Details:\n";
    cout << "  ID:        " << BOLD_GREEN << p_to_edit->id << RESET << "\n";
    cout << "  Name:      " << BOLD_GREEN << p_to_edit->name << RESET << "\n";
    cout << "  Quantity:  " << BOLD_GREEN << p_to_edit->quantity << RESET << "\n";
    cout << "  Price:     $" << BOLD_GREEN << fixed << setprecision(2) << p_to_edit->price << RESET << "\n";
}

void refillStock() {
    displayInventory();
    string productID;
    cout << BOLD_YELLOW <<  "Enter product ID to refill (or '0' to cancel): " << RESET;
    getline(cin, productID);
    
    if (productID == "0" || productID.empty()) {
        cout << CYAN << "Refill operation cancelled.\n" << RESET;
        return;
    }
    
    Product* p = searchProductByID(productID);
    if (p) {
        cout << CYAN << "\nCurrent Product Details:\n";
        cout << YELLOW << "ID: " << BOLD_GREEN << p->id << YELLOW << " | Name: " << BOLD_GREEN << p->name 
             << YELLOW << " | Current Qty: " << BOLD_GREEN << p->quantity << YELLOW << " | Price: $" << BOLD_GREEN << fixed << setprecision(2) << p->price << RESET << endl;
        
        int addQuantity_val;
        string input_qty_str;
        do {
            cout << BOLD_YELLOW <<  "\nEnter quantity to add (must be positive, or '0' to cancel): " << RESET;
            getline(cin, input_qty_str);
            if (input_qty_str == "0") {
                cout << CYAN << "Refill cancelled for this item.\n" << RESET;
                return;
            }
            try {
                if(input_qty_str.empty()) throw std::invalid_argument("empty");
                addQuantity_val = stoi(input_qty_str);
                if (addQuantity_val <= 0) {
                    cout << RED << "Error: Quantity to add must be positive.\n" << RESET;
                } else {
                    break; 
                }
            } catch (const std::exception& e) {
                cout << RED << "Error: Invalid quantity. Please enter a whole number.\n" << RESET;
            }
        } while (true);

        p->quantity += addQuantity_val;
        cout << BOLD_GREEN << "\nStock updated successfully!\n" << RESET;
        cout << CYAN << "New quantity for " << BOLD_GREEN << p->name << RESET << CYAN << ": " << BOLD_GREEN << p->quantity << RESET << endl;
        saveInventory();
    } else {
        cout << RED << "\nError: Product with ID '" << productID << "' not found.\n" << RESET;
    }
}

void inventoryMode() {
    while (true) {
        clearScreen();
        cout << "\n" << BOLD_MAGENTA;
        cout << "                         =========================================================\n";
        cout << "                         ==========                                    ===========\n";
        cout << "                         ==========" << RESET << BOLD_CYAN << "             INVENTORY MENU" << RESET << BOLD_MAGENTA << "            ===========\n";
        cout << "                         ==========                                    ===========\n";
        cout << "                         =========================================================\n" << RESET;
        cout << "\n" << BOLD_CYAN;
        
        cout << "                          _________________________          _________________________\n";
        cout << "                         |                         |        |                         |\n";
        cout << "                         |" << RESET << BOLD_GREEN << "    1. Add New Product " << RESET << BOLD_CYAN << "    |";          cout << "        |" << RESET << YELLOW << "  2. Display All Stocks" << RESET << BOLD_CYAN << " |\n";
        cout << "                         |_________________________|        |_________________________|\n";
        cout << "\n";
        cout << "                          _________________________          _________________________\n";
        cout << "                         |                         |        |                         |\n";
        cout << "                         |" << RESET << MAGENTA << "   3. Search Product" << RESET << BOLD_CYAN << "     |";          cout << "        |" << RESET << BOLD_BLUE << "    4. Refill Stock" << RESET << BOLD_CYAN << "     |\n";
        cout << "                         |_________________________|        |_________________________|\n";
        cout << "\n";
        cout << "                          _________________________          _________________________\n";
        cout << "                         |                         |        |                         |\n";
        cout << "                         |" << RESET << RED << "     5. Edit Stocks" << RESET << BOLD_CYAN << "      |"; 
                 cout << "        |" << RESET << BOLD_YELLOW << "      6. Exit Menu" << RESET << BOLD_CYAN << "      |\n";
        cout << "                         |_________________________|        |_________________________|\n";
        
		cout << "\n";
        cout << BOLD_YELLOW << "Enter choice: " << RESET;
        
        string choice_str;
        getline(cin, choice_str);
        int choice_val;

        try {
            if(choice_str.empty()) throw std::invalid_argument("empty");
            choice_val = stoi(choice_str);
        } catch (const std::exception& e) {
            cout << RED << "Invalid input. Please enter a number.\n" << RESET;
            pauseScreen();
            continue;
        }

        if (choice_val == 1) {
            addNewProduct();
            pauseScreen();
        } else if (choice_val == 2) {
            displayInventory();
            pauseScreen();
        } else if (choice_val == 3) {
            string productID;
            cout << BOLD_YELLOW << "Enter Product ID to search: " << RESET;
            getline(cin, productID);
            Product* p = searchProductByID(productID);
            if (p) {
                cout << CYAN << "\nProduct ID: "<< BOLD_GREEN << p->id << endl;
                cout << CYAN << "Product Name: " << BOLD_GREEN << p->name << endl;
                cout << CYAN << "Quantity: " << BOLD_GREEN << p->quantity << endl;
                cout << CYAN << "Price: $" << BOLD_GREEN << fixed << setprecision(2) << p->price << RESET << endl;
            } else {
                cout << RED << "\nProduct not found.\n" << RESET;
            }
            pauseScreen();
        } else if (choice_val == 4) { 
            refillStock();
            pauseScreen();
        } else if (choice_val == 5) { 
            editProduct(); 
            pauseScreen();
        } else if (choice_val == 6) {
            break;
        } else {
            cout << RED << "Invalid choice. Please enter a number between 1 and 6.\n" << RESET;
            pauseScreen();
        }
    }
}

void displayAggregatedSales() {
    if (salesHistory.empty()) {
        cout << RED << "\nNo sales data available to report.\n" << RESET;
        return;
    }

    map<string, tuple<string, int, double, double>> aggregated_data;
    double grand_total_revenue = 0.0;

    for (const auto& sale : salesHistory) {
        for (const auto& sale_item : sale.products) { 
            const string& p_id = sale_item.first;
            int qty_sold = sale_item.second;
            
            Product* product_info = searchProductByID(p_id);
            if (!product_info) continue; 

            double unit_price = product_info->price; 

            if (aggregated_data.find(p_id) == aggregated_data.end()) {
                aggregated_data[p_id] = make_tuple(product_info->name, 0, unit_price, 0.0);
            }
            get<1>(aggregated_data[p_id]) += qty_sold;
            get<2>(aggregated_data[p_id]) = unit_price; 
        }
    }

    clearScreen();
    cout << BOLD_CYAN << "\n                         Aggregated Sales Report\n";
    cout << "=====================================================================================\n" << RESET;
    cout << YELLOW << left
         << setw(10) << "ID"
         << setw(30) << "Product Name"
         << setw(15) << "Qty Sold"
         << setw(15) << "Unit Price"
         << setw(15) << "Subtotal" << RESET << endl;
    cout << YELLOW << string(85, '-') << RESET << endl;

    for (auto& entry : aggregated_data) { 
        double subtotal_for_product = get<1>(entry.second) * get<2>(entry.second);
        get<3>(entry.second) = subtotal_for_product;
        grand_total_revenue += subtotal_for_product;

        cout << BOLD_GREEN << left
             << setw(10) << entry.first
             << setw(30) << get<0>(entry.second)
             << setw(15) << get<1>(entry.second)
             << "$" << fixed << setprecision(2) << setw(13) << get<2>(entry.second) 
             << "$" << fixed << setprecision(2) << setw(13) << get<3>(entry.second) 
             << RESET << endl;
    }
    cout << YELLOW << string(85, '-') << RESET << endl;
    cout << BOLD_CYAN << right << setw(70) << "Grand Total Revenue: "
         << BOLD_GREEN << "$" << fixed << setprecision(2) << grand_total_revenue << RESET << endl;
    cout << BOLD_YELLOW << "=====================================================================================\n" << RESET;
}

void adminMode() {
    while (true) {
        clearScreen();
        cout << "\n" << BOLD_MAGENTA;
        cout << "                         =========================================================\n";
        cout << "                         ==========                                    ===========\n";
        cout << "                         ==========" << RESET << BOLD_GREEN << "              ADMIN PANEL" << RESET << BOLD_MAGENTA << "             ===========\n";
        cout << "                         ==========                                    ===========\n";
        cout << "                         =========================================================\n" << RESET;
        cout << "\n" << BOLD_CYAN;
        cout << "                     __________________________          _____________________________\n";
        cout << "                    |                          |        |                             |\n";
        cout << "                    |" << RESET << BOLD_GREEN << "      1. Total Sales" << RESET << BOLD_CYAN << "      |";          
                 cout << "        |" << RESET << BOLD_GREEN << "   2. Inventory Management" << RESET << BOLD_CYAN << "   |\n";
        cout << "                    |__________________________|        |_____________________________|\n";
        cout << "\n";
        cout << "                                     __________________________\n";
        cout << "                                    |                          |\n";
        cout << "                                    |" << RESET << RED << "    3. Exit Admin Panel" << RESET << BOLD_CYAN << "    |\n";
        cout << "                                    |__________________________|\n";
        cout << "\n" << RESET;
        cout << BOLD_YELLOW << "Enter choice: " << RESET;
         
        string choice_str;
        getline(cin, choice_str);
        int choice_val;

        try {
            if(choice_str.empty()) throw std::invalid_argument("empty");
            choice_val = stoi(choice_str);
        } catch (const std::exception& e) {
            cout << RED << "Invalid input. Please enter a number.\n" << RESET;
            pauseScreen();
            continue;
        }

        if (choice_val == 1) { 
            displayAggregatedSales();
            pauseScreen();
        } else if (choice_val == 2) { 
            inventoryMode();
        } else if (choice_val == 3) { 
            break;
        } else {
            cout <<  RED << "Invalid choice. Please enter a number between 1 and 3.\n" << RESET;
            pauseScreen();
        }
    }
}

int main() {
    srand(time(0)); 
    loadInventory();
    loadSalesHistory();
    
    while (true) {
        clearScreen();
        cout << "\n" << BOLD_MAGENTA;
        cout << "                         =========================================================\n"; 
        cout << "                         =========================================================\n";
        cout << "                         ==========                                    ===========\n";
        cout << "                         ==========" << RESET << BOLD_CYAN << "      SALES INFORMATION SYSTEM" << RESET << BOLD_MAGENTA << "      ===========\n";
        cout << "                         ==========                                    ===========\n";
        cout << "                         =========================================================\n";
        cout << "                         =========================================================\n";
        
        cout << "\n" << RESET;
        cout << "\n" << BOLD_CYAN;
        cout << "      ______________________          ________________________          ____________________\n";
        cout << "     |                      |        |                        |        |                    |\n";
        cout << "     |" << RESET << YELLOW << "      1. Cashier" << RESET << BOLD_CYAN << "      |";          
             cout << "        |" << RESET << BOLD_BLUE << "       2. Inventory" << RESET << BOLD_CYAN << "     |        |";  
             cout << BOLD_GREEN << "      3. Admin" << RESET << BOLD_CYAN <<"      |\n";
        cout << "     |______________________|        |________________________|        |____________________|\n";
        cout << "\n";
        cout << "                                     __________________________\n";
        cout << "                                    |                          |\n";
        cout << "                                    |" << RESET << RED << "     4. Exit System" << RESET << BOLD_CYAN << "       |\n";
        cout << "                                    |__________________________|\n";
        cout << "\n" << RESET;
        cout << BOLD_YELLOW << "Enter choice: " << RESET;
        
        string choice_str;
        getline(cin, choice_str); 
        int choice_val;

        try {
            if(choice_str.empty()) throw std::invalid_argument("empty input");
            choice_val = stoi(choice_str); 
        } catch (const std::exception& e) {
            cout << RED << "Invalid input. Please enter a number.\n" << RESET;
            pauseScreen();
            continue; 
        }

        if (choice_val == 1) {
            cashierMode();
        } else if (choice_val == 2) {
            inventoryMode(); 
        } else if (choice_val == 3) {
            string adminKey;
            cout << BOLD_GREEN << "Enter Admin Key: " << RESET;
            getline(cin, adminKey);
            if (adminKey == "admin123") {
                adminMode();
            } else {
                cout << RED << "\nInvalid Admin Key.\n" << RESET;
                pauseScreen();
            }
        } else if (choice_val == 4) {
            cout << BOLD_GREEN << "\nExiting system. Goodbye!\n" << RESET;
            break;
        } else {
             cout << RED << "Invalid choice. Please enter a number between 1 and 4.\n" << RESET;
             pauseScreen();
        }
    }

    return 0;
}