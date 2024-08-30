#include <iostream>
#include <queue>
#include <string>
#include <ctime>
#include <unordered_map>
#include <utility>
#include <chrono>
#include <iomanip>
#include <fstream>

using namespace std;

int orderCount = 0;

class Order {
public:
    int m_orderId;
    std::string client_order_id;
    std::string instrument;
    int side;
    int quantity;
    double price;

    void newOrder() const;
    void pfillOrder(int value, double price) const;
    void fillOrder(double price) const;

    Order(std::string  clientOrderId, std::string  instrumentName, int orderSide,
          int orderQuantity, double orderPrice)
            : client_order_id(std::move(clientOrderId)),
              instrument(std::move(instrumentName)),
              side(orderSide),
              quantity(orderQuantity),
              price(orderPrice)
    {
        m_orderId = ++orderCount;
    }
};


struct CompareBuy {
    bool operator()(const Order& order1, const Order& order2) const {
        if (order1.price != order2.price)
            return order1.price < order2.price;
        return order1.m_orderId > order2.m_orderId;
    }
};


struct CompareSell {
    bool operator()(const Order& order1, const Order& order2) const {
        if (order1.price != order2.price)
            return order1.price > order2.price;
        return order1.m_orderId > order2.m_orderId;
    }
};


const int numInstruments = 5;  // Number of different instruments

std::priority_queue<Order, std::vector<Order>, CompareBuy> buyOrdersQueue[numInstruments];
std::priority_queue<Order, std::vector<Order>, CompareSell> sellOrdersQueue[numInstruments];

// Flower positions
std::unordered_map<std::string, int> insNo = {
        { "Rose", 0 },
        { "Lavender", 1 },
        { "Lotus", 2 },
        { "Tulip", 3 },
        { "Orchid", 4 }
};

string getTime(){
    stringstream ss;
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm local_time = *std::localtime(&now_c);
    ss << std::put_time(&local_time, "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

void Order::newOrder() const {
    std::ofstream file("execution_rep.csv", std::ios_base::app);
    if (file.is_open()) {
        file << "ord" << m_orderId << "," << client_order_id << "," << instrument << "," << side << "," << "new" << "," << quantity << "," << price << ",," << getTime() << std::endl;
        file.close();
    }
}

void Order::fillOrder(double fillPrice) const {
    std::ofstream file("execution_rep.csv", std::ios_base::app);
    if (file.is_open()) {
        file << "ord" << m_orderId << "," << client_order_id << "," << instrument << "," << side << "," << "fill" << "," << quantity << "," << fillPrice << ",," << getTime() << std::endl;
        file.close();
    }
}

void Order::pfillOrder(int value, double fillPrice) const {
    std::ofstream file("execution_rep.csv", std::ios_base::app);
    if (file.is_open()) {
        file << "ord" << m_orderId << "," << client_order_id << "," << instrument << "," << side << "," << "pfill" << "," << value << "," << fillPrice << ",," << getTime() << std::endl;
        file.close();
    }
}

void processSell(int insNom) {

    if(sellOrdersQueue[insNom].top().price > buyOrdersQueue[insNom].top().price){
        sellOrdersQueue[insNom].top().newOrder();
    }
    else{

        while (sellOrdersQueue[insNom].top().price <= buyOrdersQueue[insNom].top().price){

            if (sellOrdersQueue[insNom].top().quantity == buyOrdersQueue[insNom].top().quantity){
                sellOrdersQueue[insNom].top().fillOrder(buyOrdersQueue[insNom].top().price);
                buyOrdersQueue[insNom].top().fillOrder(buyOrdersQueue[insNom].top().price);
                sellOrdersQueue[insNom].pop();
                buyOrdersQueue[insNom].pop();
                break;
            }

            else if(buyOrdersQueue[insNom].top().quantity > sellOrdersQueue[insNom].top().quantity){

                Order topBuyOrder = buyOrdersQueue[insNom].top();  // get a clone
                int newQuantity = topBuyOrder.quantity - sellOrdersQueue[insNom].top().quantity;

                topBuyOrder.quantity = newQuantity;

                topBuyOrder.pfillOrder(sellOrdersQueue[insNom].top().quantity, buyOrdersQueue[insNom].top().price);

                sellOrdersQueue[insNom].top().fillOrder(buyOrdersQueue[insNom].top().price);

                buyOrdersQueue[insNom].pop();
                buyOrdersQueue[insNom].push(topBuyOrder);  // Step 3

                sellOrdersQueue[insNom].pop();
                break;
            }

            else {
                Order topSellOrder = sellOrdersQueue[insNom].top();  // get a clone
                int newQuantity = topSellOrder.quantity - buyOrdersQueue[insNom].top().quantity;

                topSellOrder.quantity = newQuantity;

                topSellOrder.pfillOrder(buyOrdersQueue[insNom].top().quantity, buyOrdersQueue[insNom].top().price);

                sellOrdersQueue[insNom].pop();
                sellOrdersQueue[insNom].push(topSellOrder);  // Step 3

                buyOrdersQueue[insNom].top().fillOrder(buyOrdersQueue[insNom].top().price);
                buyOrdersQueue[insNom].pop();

                if (buyOrdersQueue[insNom].empty()){
                    break;
                }

            }
        }
    }
}


void processBuy(int insNom) {

    if(sellOrdersQueue[insNom].top().price > buyOrdersQueue[insNom].top().price){
        buyOrdersQueue[insNom].top().newOrder();
    }
    else{

        while (sellOrdersQueue[insNom].top().price <= buyOrdersQueue[insNom].top().price){

            if (sellOrdersQueue[insNom].top().quantity == buyOrdersQueue[insNom].top().quantity){
                sellOrdersQueue[insNom].top().fillOrder(sellOrdersQueue[insNom].top().price);
                buyOrdersQueue[insNom].top().fillOrder(sellOrdersQueue[insNom].top().price);
                sellOrdersQueue[insNom].pop();
                buyOrdersQueue[insNom].pop();
                break;
            }

            else if(buyOrdersQueue[insNom].top().quantity > sellOrdersQueue[insNom].top().quantity){

                Order topBuyOrder = buyOrdersQueue[insNom].top();  // get a clone
                int newQuantity = topBuyOrder.quantity - sellOrdersQueue[insNom].top().quantity;

                topBuyOrder.quantity = newQuantity;

                topBuyOrder.pfillOrder(sellOrdersQueue[insNom].top().quantity, sellOrdersQueue[insNom].top().price);

                buyOrdersQueue[insNom].pop();
                buyOrdersQueue[insNom].push(topBuyOrder);  // Step 3

                sellOrdersQueue[insNom].top().fillOrder(sellOrdersQueue[insNom].top().price);
                sellOrdersQueue[insNom].pop();

                if (buyOrdersQueue[insNom].empty()){
                    break;
                }
            }

            else {
                Order topSellOrder = sellOrdersQueue[insNom].top();  // get a clone
                int newQuantity = topSellOrder.quantity - buyOrdersQueue[insNom].top().quantity;

                //change the quantity
                topSellOrder.quantity = newQuantity;

                //pfill the buy order
                topSellOrder.pfillOrder(buyOrdersQueue[insNom].top().quantity, sellOrdersQueue[insNom].top().price);

                buyOrdersQueue[insNom].top().fillOrder(sellOrdersQueue[insNom].top().price);

                sellOrdersQueue[insNom].pop();
                sellOrdersQueue[insNom].push(topSellOrder);  // Step 3

                buyOrdersQueue[insNom].pop();

                break;

            }
        }
    }
}


void insertBuyOrder(const Order& order) {

    int position;

    if (insNo.find(order.instrument) != insNo.end()) {
        position = insNo.at(order.instrument);
        buyOrdersQueue[position].push(order); // Buy side

        if (sellOrdersQueue[position].empty()){
            order.newOrder();
        } else{
            processBuy(position); // call sell handler
        }
    }
}


void insertSellOrder(const Order& order) {
    int position;

    if (insNo.find(order.instrument) != insNo.end()) {

        position = insNo.at(order.instrument);

        sellOrdersQueue[position].push(order); 

        if (buyOrdersQueue[position].empty()){
            order.newOrder();
        } else{
            processSell(position); 
        }

    }

}

int main() {

    std::ifstream file("orders.csv");

    std::string line;

    //remove first line
    std::getline(file, line);

    std::string coID, instrument , side, quantity, price;

    std::ofstream outfile("execution_rep.csv", std::ios::trunc);
    if (outfile.is_open()) {
        // Add a header to the file
        outfile << "Cl.Ord.Id" << "," << "Instrument" << ","<< "Side" << ","<< "Status" << "," << "Quantity" << "," << "Price" << "," << "time" << "," << "reason"  << std::endl;

    } else {
        std::cerr << "Can't make a file 'execution_rep.csv'." << std::endl;
    }
    

    std::ofstream writeFile("execution_rep.csv", std::ios::app);
    if (writeFile.is_open()) {
        while (std::getline(file, line)) {
            std::stringstream ss(line);

            std::getline(ss, coID, ',');
            std::getline(ss, instrument, ',');
            std::getline(ss, side, ',');
            std::getline(ss, quantity, ',');
            std::getline(ss, price, ',');


            int m_side, m_quantity;
            double m_price;

            // Convert side, quantity, and price to integers
            m_side = stoi(side);
            m_quantity = stoi(quantity);
            m_price = stod(price);
            
            Order order = { coID, instrument, m_side, m_quantity, m_price };

            if(m_side == 1){
                insertBuyOrder(order);
            } else if (m_side == 2){
                insertSellOrder(order);
            }
        }

    } else {
        std::cerr << "order.csv not found." << std::endl;
    }

    return 0;
}
