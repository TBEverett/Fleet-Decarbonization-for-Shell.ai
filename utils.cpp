#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <math.h>
using namespace std;

struct CostProfile{
    int year;
    float resale_value;
    float insurance_cost;
    float maintenance_cost;
};

struct Fuel{
    string type;
    int year[16];
    float emissions_per_unit; //En todos los años es el mismo valor por combustible
    float cost_per_unit[16]; //Una celda por año, de 2023 a 2038.
    float cost_uncertainty[16];
    unordered_map<string,float> consumption; //Un hashmap que toma una ID de auto y retorna el consumption (unit_fuel/km).
};

//Redefinimos print con cout para poder printear vehiculos
std::ostream& operator<<(std::ostream& os, const Fuel& f) {
    os << "Fuel: " << f.type;
    return os;
}

class Vehicle{
    public:
        string id;
        int original_year;
        string type;
        string size_bucket;
        string distance_bucket; //Una celda por año, de 2023 a 2038.
        int cost;
        int yearly_range; //Constante de año a año
};

//Funcion de utilidad para quitarle el año a un id de vehiculo y así poder comparar dos vehiculos iguales de distintos años y que sea True
string removeYear(string id) {
    size_t pos = id.find_last_of('_');
    if (pos == std::string::npos) {
        // No underscore found, return the original string
        return id;
    }
    return id.substr(0, pos);
}

Vehicle getVehicle(vector<Vehicle> vehicles, string id){
    for (Vehicle v : vehicles){
        if (v.id == id){
            return v;
        }
    }
    Vehicle v;
    cout << "ERROR: getVehicle did not find the vehicle with id " << id << endl;
    return v;
}


//Redefinimos print con cout para poder printear vehiculos
std::ostream& operator<<(std::ostream& os, const Vehicle& v) {
    os << "Vehicle: " << v.id;
    return os;
}

struct Operation{
    int year;
    string id;
    int num_vehicles;
    string type;
    string fuel_type;
    string distance_bucket;
    float distance_per_vehicle;
};

//Redefinimos print con cout para poder printear vehiculos
std::ostream& operator<<(std::ostream& os, const Operation& o) {
    os << "Operation: " << o.year << "," << o.id << "," << o.num_vehicles << ","<< o.type << "," << o.fuel_type << "," << o.distance_bucket << "," << o.distance_per_vehicle;
    return os;
}

class Solution{
    public:
        vector<vector<Operation>> operations_per_year;

        Solution(vector<Operation> operations){
            //Recorremos operations y almacenamos las de cada año en su celda
            vector<Operation> aux_operations;
            int current_year = 2023;
            for (Operation op : operations){
                if (op.year > current_year){
                    current_year++;
                    operations_per_year.push_back(aux_operations);
                    aux_operations.clear();
                }
                aux_operations.push_back(op);
            }
            operations_per_year.push_back(aux_operations);
        }

        vector<Operation> flatten(){
            vector<Operation> all_operations;
            for(auto v : operations_per_year){
                all_operations.insert(all_operations.end(), v.begin(), v.end());
            }
            return all_operations;
        }

        void print(bool verbose){
            cout << "Solution: " << endl;
            for (vector<Operation> v : operations_per_year){
                cout << "Year: " << v[0].year << endl;
                if (!verbose){
                    for(int i = 0; i < 3; i++){
                        cout << v[i].id << " " << v[i].num_vehicles << " " << v[i].type << endl;
                    }
                    cout << "..." << endl << endl;
                }
                else{
                    for (Operation op : v){
                        cout << op.id << " " << op.num_vehicles << " " << op.type << endl;
                    }
                    cout << endl;
                }

            }
        }
};

//Funciones de lectura de archivos
vector<int> readCarbonTargets(){
    ifstream file("carbon_emissions.csv");
    string line;
    vector<int> data;
    getline(file, line);
    while (getline(file, line)) {
        stringstream ss(line);
        string element;
        int i = 0;
        while (getline(ss, element, ',')) {
            if (i++ % 2) data.push_back(stoi(element));
        }
    }
    file.close();
    return data;
}

vector<CostProfile> readCostProfiles(){
    ifstream file("cost_profiles.csv");
    string line;
    vector<CostProfile> data;
    getline(file, line);
    while (getline(file, line)) {
        stringstream ss(line);
        string element;
        int i = 0;
        CostProfile aux;
        while (getline(ss, element, ',')) {
            if (i == 0) aux.year = stoi(element);
            if (i == 1) aux.resale_value = stof(element) / 100;
            if (i == 2) aux.insurance_cost = stof(element) / 100;
            if (i == 3) aux.maintenance_cost = stof(element) / 100;
            i++;
        }
        data.push_back(aux);
    }
    file.close();
    return data;
}

vector<Fuel> readFuels(){
    ifstream file("fuels.csv");
    string line;
    vector<Fuel> data;
    getline(file, line);
    for(int fuel_index = 0; fuel_index < 5; fuel_index++){ //0 a 5 para 5 combustibles distintos
        Fuel aux;
        for(int line_index = 0; line_index < 16; line_index++){ // 0 a 16 para 16 años por combustible
            getline(file, line);
            stringstream ss(line);
            string element;
            int i = 0;
            while (getline(ss, element, ',')) {
                if (i == 0) aux.type = element;
                if (i == 1) aux.year[line_index] = stoi(element);
                if (i == 2) aux.emissions_per_unit = (float) stod(element);
                if (i == 3) aux.cost_per_unit[line_index] = (float) stod(element);
                if (i == 4) aux.cost_uncertainty[line_index] = (float) stod(element);
                i++;
            }
        }
        data.push_back(aux);
    }
    file.close();
    ifstream file2("vehicles_fuels.csv");
    getline(file2,line);
    for (int fuel_index = 0; fuel_index < 20; fuel_index++){
        for (int line_index = 0; line_index < 16; line_index++){
            getline(file2, line);
            stringstream ss(line);
            string element;
            int i = 0;
            string vehicle_id;
            string fuel_type;
            float consumption;
            while (getline(ss, element, ',')) {
                if (i == 0) vehicle_id = element;
                if (i == 1) fuel_type = element;
                if (i == 2) consumption = (float) stod(element);
                i++;
            }
            //Obtenemos fuel correspondiente
            for (Fuel &fuel : data){
                if (fuel.type == fuel_type){
                    fuel.consumption[vehicle_id] = consumption;
                }
            }
        }
    }
    file2.close();
    return data;
}

vector<Vehicle> readVehicles(){
    ifstream file("vehicles.csv");
    string line;
    vector<Vehicle> data;
    getline(file, line);
    for (int vehicle_index = 0; vehicle_index < 12; vehicle_index++){
        for (int line_index = 0; line_index < 16; line_index++){
            Vehicle aux;
            getline(file, line);
            stringstream ss(line);
            string element;
            int i = 0;
            while (getline(ss, element, ',')) {
                if (i == 0) aux.id = element;
                if (i == 1) aux.type = element;
                if (i == 2) aux.size_bucket = element;
                if (i == 3) aux.original_year = stoi(element);
                if (i == 4) aux.cost = stoi(element);
                if (i == 5) aux.yearly_range = stoi(element);
                if (i == 6) aux.distance_bucket = element;
                i++;
            }
            data.push_back(aux);
        }
    }
    file.close();
    return data;
}

vector<Operation> readOperations(){
    ifstream file("sample_submission.csv");
    string line;
    vector<Operation> data;
    getline(file, line);
    while (getline(file, line)) {
        stringstream ss(line);
        string element;
        int i = 0;
        Operation aux;
        while (getline(ss, element, ',')) {
            if (i == 0) aux.year = stoi(element);
            if (i == 1) aux.id = element;
            if (i == 2) aux.num_vehicles = stoi(element);
            if (i == 3) aux.type = element;
            if (i == 4) aux.fuel_type = element;
            if (i == 5) aux.distance_bucket = element;
            if (i == 6) aux.distance_per_vehicle = stof(element);
            i++;
        }
        data.push_back(aux);
    }
    file.close();
    return data;
}
struct Demand{
    int year;
    string size_bucket;
    string distance_bucket;
    int demand;
};

vector<Demand> readDemands(){
    ifstream file("demand.csv");
    string line;
    vector<Demand> data;
    getline(file, line);
    while (getline(file, line)) {
        stringstream ss(line);
        string element;
        int i = 0;
        Demand aux;
        while (getline(ss, element, ',')) {
            if (i == 0) aux.year = stoi(element);
            if (i == 1) aux.size_bucket = element;
            if (i == 2) aux.distance_bucket = stoi(element);
            if (i == 3) aux.demand = stoi(element);
            i++;
        }
        data.push_back(aux);
    }
    file.close();
    return data;
}

int getDemand(vector<Demand> demands, int year, string size_bucket, string distance_bucket){
    for (auto d : demands){
        if (d.year == year && d.size_bucket == size_bucket && d.distance_bucket == distance_bucket){
            return d.demand;
        }
    }
}

//Información que es útil tener globalmente
vector<int> carbonTargets = readCarbonTargets();
vector<CostProfile> costProfiles = readCostProfiles();
vector<Fuel> fuels = readFuels();
vector<Vehicle> vehicles = readVehicles();
vector<Operation> operations = readOperations();
vector<Demand> demands = readDemands();


//
// Funciones de evaluación
//

int buyCost(vector<Operation> operations){
    int total_cost = 0;
    for (Operation op : operations){
        if (op.type == "Buy"){
            int year = op.year;
            string id = op.id;
            int amount = op.num_vehicles;
            Vehicle v = getVehicle(vehicles, id);
            total_cost += v.cost * amount;
        }
    }
    return total_cost;
}

float sellCost(vector<Operation> operations){
    float total_cost = 0;
    for (Operation op : operations){
        if (op.type == "Sell"){
            int year = op.year;
            string id = op.id;
            int amount = op.num_vehicles;
            Vehicle v = getVehicle(vehicles, id);
            int length_owned = op.year - v.original_year;
            total_cost += v.cost * costProfiles[length_owned].resale_value * amount;
        }
    }
    return total_cost;
}

float fuelCost(vector<Operation> operations){
    float total_cost = 0;
    for (Operation op : operations){
        if (op.type == "Use"){
            float consumption;
            float cost_per_unit;
            for (Fuel f : fuels){
                if (f.type == op.fuel_type){
                    consumption = f.consumption[op.id];
                    cost_per_unit = f.cost_per_unit[op.year - 2023];
                    break;
                }
            }
            total_cost += op.distance_per_vehicle * op.num_vehicles * consumption * cost_per_unit;
        }
    }
    return total_cost;
}

float insuranceAndMaintenanceCost(vector<Operation> operations){
    float total_insurance_cost = 0;
    float total_maint_cost = 0;
    unordered_map<string,int> currently_owned_vehicles;
    int current_year = 2023;
    for (Operation op : operations){
        //Revisamos si pasamos al proximo año, en cuyo caso calculamos el costo del actual
        if (op.year > current_year){
            //CALCULAR COSTO
            for (auto item : currently_owned_vehicles){
                Vehicle v = getVehicle(vehicles, item.first);
                int length_owned = current_year - v.original_year;
                total_insurance_cost += v.cost * costProfiles[length_owned].insurance_cost * item.second;
                total_maint_cost += v.cost * costProfiles[length_owned].maintenance_cost * item.second;
            }
            //CALCULAR COSTO
            current_year++;
        }
        if (op.type == "Buy"){
            //Si la id no esta en el map, la crea
            if (currently_owned_vehicles.find(op.id) == currently_owned_vehicles.end()){
                currently_owned_vehicles[op.id] = 0;
            }
            currently_owned_vehicles[op.id] += op.num_vehicles;  
        }    
        if (op.type == "Sell"){
            if (currently_owned_vehicles.find(op.id) == currently_owned_vehicles.end()){
                cout << "ERROR: Sold unowned vehicle: " << op.id << endl;
            }
            else{
                currently_owned_vehicles[op.id] -= op.num_vehicles;
                if (currently_owned_vehicles[op.id] == 0){
                    currently_owned_vehicles.erase(op.id);
                }
            }
        }
    }
    for (auto item : currently_owned_vehicles){
                Vehicle v = getVehicle(vehicles, item.first);
                int length_owned = current_year - v.original_year;
                total_insurance_cost += v.cost * costProfiles[length_owned].insurance_cost * item.second;
                total_maint_cost += v.cost * costProfiles[length_owned].maintenance_cost * item.second;
            }
    float total_cost = total_insurance_cost + total_maint_cost;
    return total_cost;
}

// No estoy 100% seguro de si la evaluación esta perfecta porque en la pagina restan además los autos que sobran al final de 2038
float eval(Solution solution){
    vector<Operation> operations = solution.flatten();
    return buyCost(operations) + insuranceAndMaintenanceCost(operations) + fuelCost(operations) - sellCost(operations);
}

//
//Revisión de restricciones
//
bool belowEmissions(Solution solution){
    int current_year = 0;
    for (auto vec_ops : solution.operations_per_year){
        for (auto op : vec_ops){
            if (op.type == "Use"){
                //Obtenemos fuel correspondiente
                float emissions;
                float consumption;
                for (Fuel fuel : fuels){
                    if (fuel.type == op.fuel_type){
                        emissions = fuel.emissions_per_unit;
                        consumption = fuel.consumption[op.id];
                        break;
                    }
                }
                float yearly_emissions = op.distance_per_vehicle * op.num_vehicles * consumption * emissions;
                if (yearly_emissions > carbonTargets[current_year]){
                    return false;
                }
            }
        }
        current_year++;
    }
    return true;
}

bool coveredDistanceBuckets(Solution solution){
    for (auto d : demands){ //Para cada demand revisamos las ops de ese año
        int demand_distance = d.demand;
        for (auto op : solution.operations_per_year[d.year - 2023]){
            Vehicle v = getVehicle(vehicles, op.id);
            if (op.type == "Use" && op.distance_bucket == d.distance_bucket && v.size_bucket == d.size_bucket){
                demand_distance -= op.distance_per_vehicle * op.num_vehicles;
            }
        }
        //Si demand distance es >0 tras haber revisado todas las ops del año, no fue satisfecha la demanda del año para ese size_bucket y distance_bucket
        if (demand_distance > 0){
            return false;
        }
    }
    return true;
}

