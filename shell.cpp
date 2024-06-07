#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
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
        string distance_bucket[16]; //Una celda por año, de 2023 a 2038.
        int cost[16];
        int yearly_range; //Constante de año a año

        //Almacenamos todos los combustibles posibles en el vehículo
        //así podemos modificar el combustible actual facilmente en la propuesta de solución
        int current_fuel; //índice del combustible actual asignado al vehículo
        vector<Fuel> fuels; //Lista con los combustibles posibles
};

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

vector<Vehicle> readVehicles(vector<Fuel> fuels){
    ifstream file("vehicles.csv");
    string line;
    vector<Vehicle> data;
    getline(file, line);
    for (int vehicle_index = 0; vehicle_index < 12; vehicle_index++){
        Vehicle aux;
        for (int line_index = 0; line_index < 16; line_index++){
            getline(file, line);
            stringstream ss(line);
            string element;
            int i = 0;
            while (getline(ss, element, ',')) {
                if (i == 0) aux.id = element;
                if (i == 1) aux.type = element;
                if (i == 2) aux.size_bucket = element;
                if (i == 3) aux.original_year = stoi(element);
                if (i == 4) aux.cost[line_index] = stoi(element);
                if (i == 5) aux.yearly_range = stoi(element);
                if (i == 6) aux.distance_bucket[line_index] = element;
                i++;
            }
        }
        aux.current_fuel = 0;
        //Agregamos fuels posibles
        for (Fuel fuel : fuels){
            if (fuel.consumption.find(aux.id) != fuel.consumption.end()){
                aux.fuels.push_back(fuel);
            }
        }
        data.push_back(aux);
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


int main(){
    vector<int> carbonTargets = readCarbonTargets();
    vector<CostProfile> costProfiles = readCostProfiles();
    vector<Fuel> fuels = readFuels();
    vector<Vehicle> vehicles = readVehicles(fuels);
    vector<Operation> operations = readOperations();
    for (Operation v : operations){
        cout << v << endl;
    }

    
}