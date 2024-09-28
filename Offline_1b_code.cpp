#include<bits/stdc++.h>
#include "lcgrand.c"
#include "lcgrand.h"
using namespace std;

// Declare global integer variables
int next_event, total_events, small_s, big_S, initial_inventory_level, current_inventory_level;
int total_months_required, number_of_policies, number_of_demand_values, ordered_amount;

// Declare global double variables
double mean_interdemand, min_lag, max_lag, setup_cost, holding_cost, shortage_cost, incremental_cost;
double holding_area, shortage_area, total_ordering_cost, current_simulation_time, last_event_time;
double next_event_time[5], cumulative_prob_distribution_demand[26];

// Declare input and output file streams
ifstream input_file;
ofstream output_file;

// Declare function prototypes
void initialize();
void timing_function();
double exponential_random_variable(double mean);
int generate_random_integer_for_demand(double prob_distribution_demand[]);
double uniform_random_variable(double a, double b);
void update_statistics();
void order_arrival();
void demand();
void evaluate();
void departure();
void report();


// Main function
int main(){
    // Open input and output files
    input_file.open("in.txt");
    output_file.open("out.txt");

    // Check if files are opened successfully
    if(!input_file.is_open()){
        cerr << "Error opening input file" << endl;
        return 1;
    }

    if(!output_file.is_open()){
        cerr << "Error opening output file" << endl;
        return 1;
    }

    // Specify the number of events for the simulation
    total_events = 4;

    // Read input values from input file
    input_file >> initial_inventory_level >> total_months_required >> number_of_policies;
    input_file >> number_of_demand_values >> mean_interdemand;
    input_file >> setup_cost >> incremental_cost >> holding_cost >> shortage_cost;
    input_file >> min_lag >> max_lag;

    for(int i = 1; i <= number_of_demand_values; i++){
        input_file >> cumulative_prob_distribution_demand[i];
    }

    // Write heading and input values to output file
    output_file << "------Single-Product Inventory System------\n\n";
    output_file << "Initial inventory level: " << initial_inventory_level << " items\n\n";
    output_file << "Number of demand sizes: " << number_of_demand_values << "\n\n";

    output_file << "Distribution function of demand sizes: ";
    for(int i = 1; i <= number_of_demand_values; i++){
        output_file << setprecision(2) << fixed << cumulative_prob_distribution_demand[i] << " ";
    }
    output_file << "\n\n";

    output_file << "Mean inter-demand time: " << setprecision(2) << fixed << mean_interdemand << " months\n\n";
    output_file << "Delivery lag range: " << setprecision(2) << fixed << min_lag << " to " << setprecision(2) << max_lag << " months\n\n";
    output_file << "Length of simulation: " << total_months_required << " months\n\n";

    output_file << "Costs:\n";
    output_file << "K = " << setprecision(2) << fixed << setup_cost << "\n";
    output_file << "i = " << setprecision(2) << fixed << incremental_cost << "\n";
    output_file << "h = " << setprecision(2) << fixed << holding_cost << "\n";
    output_file << "pi = " << setprecision(2) << fixed << shortage_cost << "\n\n";

    output_file << "Number of policies: " << number_of_policies << "\n\n";
    output_file << "Policies:\n";
    output_file << "--------------------------------------------------------------------------------------------------\n";
    output_file << " Policy        Avg_total_cost     Avg_ordering_cost      Avg_holding_cost     Avg_shortage_cost\n";
    output_file << "--------------------------------------------------------------------------------------------------\n\n";


    // Loop through the number of policies
    for(int i = 1; i <= number_of_policies; i++){
        // Read input values from input file
        input_file >> small_s >> big_S;

        // Initialize the simulation
        initialize();

        // Run the simulation
        do{
            // Call timing function to get the next event
            timing_function();

            // Update shortage area or holding area
            update_statistics();

            // Call the proper functions after determining the next event, terminate simulation after calling event 3
            if(next_event == 1){
                order_arrival();
            }
            else if(next_event == 2){
                demand();
            }
            else if(next_event == 3){
                report();
            }
            else if(next_event == 4){
                evaluate();
            }


        } while(next_event != 3);
    }

    output_file << "--------------------------------------------------------------------------------------------------";

    // Close input and output files
    input_file.close();
    output_file.close();
    return 0;
}


// Initializing function
void initialize(){
    // Set the simulation clock to 0.0
    current_simulation_time = 0.0;

    // Initialize the state variables
    current_inventory_level = initial_inventory_level;
    last_event_time = 0.0;

    // Initialize the statistical counters
    total_ordering_cost = 0.0;
    holding_area = 0.0;
    shortage_area = 0.0;

    // Initialize the event list
    next_event_time[1] = 1.0e+30;
    next_event_time[2] = current_simulation_time + exponential_random_variable(mean_interdemand);
    next_event_time[3] = total_months_required;
    next_event_time[4] = 0.0;
}


// Timing function to determine the next event
void timing_function(){
    double min_time_next_event = 1.0e+29;
    next_event = 0;

    // Determine the next event
    for(int i = 1; i <= total_events; i++){
        if(next_event_time[i] < min_time_next_event){
            min_time_next_event = next_event_time[i];
            next_event = i;
        }
    }

    // Event list empty if next event is 0   
    if(next_event == 0){
        cerr << "\nEvent list empty at time " << current_simulation_time << endl;
        exit(1);
    }

    // Update the simulation time
    current_simulation_time = min_time_next_event;
}


// Random variable generator between 0 to 1
double exponential_random_variable(double mean){
    return -mean * log(lcgrand(1));
}


// Random variable generator between a to b
double uniform_random_variable(double a, double b){
    return a + lcgrand(1) * (b - a);
}


// Random integer generator for demand
int generate_random_integer_for_demand(double prob_distribution_demand[]){
    // Generate a random number between 0 to 1
    double u = lcgrand(1);
    int num = 1;

    // Find the range that the random number falls into
    while(u >= prob_distribution_demand[num]){
        num++;
    }
    return num;
}


// Update the statistical counters
void update_statistics(){
    // Calculate the time passed after the last event
    double time_passed_after_last_event = current_simulation_time - last_event_time;
    last_event_time = current_simulation_time;

    // Update the holding area or shortage area based on the inventory level
    if(current_inventory_level < 0){
        shortage_area -= current_inventory_level * time_passed_after_last_event;
    }
    else if(current_inventory_level > 0){
        holding_area += current_inventory_level * time_passed_after_last_event;
    }
}


// Order Arrival Event function
void order_arrival(){
    // Update the inventory level
    current_inventory_level += ordered_amount;

    // Eliminate the order arrival event
    next_event_time[1] = 1.0e+30;
}


// Demand Event function
void demand(){
    // Update the inventory level based on the generated demand
    current_inventory_level -= generate_random_integer_for_demand(cumulative_prob_distribution_demand);

    // Schedule the next demand event
    next_event_time[2] = current_simulation_time + exponential_random_variable(mean_interdemand);
}


// Evaluate function
void evaluate(){
    // If the current inventory level is less that small_s, then place an order
    if(current_inventory_level < small_s){

        // Calculate the order amount and total ordering cost
        ordered_amount = big_S - current_inventory_level;
        total_ordering_cost += setup_cost + incremental_cost * ordered_amount;

        // Schedule the order arrival event
        next_event_time[1] = current_simulation_time + uniform_random_variable(min_lag, max_lag);
    }
    
    // Schedule the next evaluation event
    next_event_time[4] = current_simulation_time + 1.0;
}


// Report function
void report(){
    
    // Calculate the average ordering cost, average holding cost, average shortage cost
    double average_ordering_cost = total_ordering_cost / total_months_required;
    double average_holding_cost = holding_cost * holding_area / total_months_required;
    double average_shortage_cost = shortage_cost * shortage_area / total_months_required;

    // Calculate the average total cost by summing the average ordering cost, average holding cost, average shortage cost
    double average_total_cost = average_ordering_cost + average_holding_cost + average_shortage_cost;

    // Write the performance measures to the output file
    output_file << "(" << setw(2) << small_s << "," << setw(3) << big_S << ")"; 
    output_file << setw(20) << fixed << setprecision(2) << average_total_cost; 
    output_file << setw(20) << fixed << setprecision(2) << average_ordering_cost; 
    output_file << setw(20) << fixed << setprecision(2) << average_holding_cost; 
    output_file << setw(20) << fixed << setprecision(2) << average_shortage_cost << "\n\n";
}
