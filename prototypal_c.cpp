/*
Copyright [2014] [John-Michael Reed]

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

/* 
 * File:   main.cpp
 * Author: johnmichaelreed2
 *
 * Created on November 29, 2014, 3:35 PM
 */

#include <cstdlib>

using namespace std;

// 64 bit integers used for type identification
#define THING -9223372036854775808LL
#define ACTION -9223372036854775807LL

// Functional allows me to bind functions and call them with different numbers of parameters
#include <functional>

#include <iostream>
#include <unordered_map>

// cstdarg allows for variadic function pointers
#include <cstdarg>

// out_of_range exceptions are thrown by unordered_map
#include <exception>
#include <bits/unordered_map.h>

// Re-assignable function pointer type cast. 
// Produces a function that takes in an arbitrary # of args and returns a void*
typedef void * (*pointer_cast) (...);

// type is stored as a 64 bit value at the head of an object
long long getType(void * head) {
    return (long long)head;
}

// Function is a container for a re-assignable function pointer
struct Function {

    // type is used to get the type of an object via the getType global function.
    static const long long type = ACTION;

    // execute is a re-assignable general purpose function pointer. 
    // Takes in an arbitrary # of args and returns a void *
    public: pointer_cast execute; 

    Function () {
         this->execute = nullptr; //apparently you can't set a variadic funtion pointer to zero.
    }
    
    void operator=(const Function& other) // copy assignment
{
        this->execute = other.execute;
}

};

//Thing is a container of local variables and actions.
class Thing {

public: const long long type = THING; 

private: std::unordered_map<std::string, void * > Properties_Table;

private: std::unordered_map<std::string, Function > Functions_Table;

public: Thing * my_parent;

Thing()
{
    Properties_Table = { };
    Functions_Table = { };
}

Thing(Thing &new_parent)
{
    Properties_Table = { };
    Functions_Table = { };
    this->my_parent = &new_parent;
}

// Add a single object property to the properties hash table.
template <class Type> void addProperty (std::string property_name, Type &property_value)
{
    Properties_Table[property_name] = &property_value;
} 

// Add a single action to the actions hash table.
void addFunction (std::string action_name, Function action_value)
{
        Functions_Table[action_name] = action_value;
} 

void * get (std::string toGet) {
    void * return_pointer = nullptr;

    //If the element exists, get it.
    try {
        return_pointer = Properties_Table.at(toGet);
    }
    
    //Else check to see if the my_parent has it
    catch (const std::out_of_range& oor) {
        
        if (this->my_parent != nullptr) 
            return_pointer = this->my_parent->get(toGet);
        else 
            throw std::out_of_range("Member not found.");
    }
    
    return return_pointer;
}

// Provides the pointer to call a function
// Function_name is the name as a std::string. 
template<class Type> pointer_cast Do (Type function_name) {

    pointer_cast return_value = nullptr;

    //Calls the corresponding function in the hash table of function objects.
    try {
        return_value = (Functions_Table.at( (std::string)function_name )).execute; 
    }
     
     //C++ map throws an exception if function not found.
     catch (const std::out_of_range& oor) {
         //Check my_parent.
         if (this->my_parent != nullptr)
                return_value = this->my_parent->Do(function_name);
        //Give up.
            else 
                throw std::out_of_range ("Function cound not be found.");
     }
     
     return return_value;
}

// Assigns a function pointer to an action in the table of actions.
template< class Type> void set (std::string function_name, Type newFunction) {

    (Functions_Table.at(function_name)).execute = (pointer_cast) newFunction;
}

};

#define a auto
int * add(int aa, int bb) {
    return new int (aa+bb);
}
int main() {
    Thing snake; //Create a snake
    Function doMath; //add is the add function
    doMath.execute = (pointer_cast)add; //Make doMath execute function add
    snake.addFunction("add", doMath); 
    a * distance = (int*)((snake.Do("add"))(5,6)); //Tell snake to call doMath
    printf("Parent: 5 + 6 = %d \n", *distance); //doMath executes add
    delete distance;
    
    Thing baby_snake(snake); //Create a baby whose parent is snake
    a * baby_distance = (int*)((baby_snake.Do("add"))(1,2)); //Call parent function
    printf("Child: 1 + 2 = %d \n", *baby_distance);
    delete baby_distance;
    
    a rr = 444;
    snake.addProperty("age",rr); //Give snake an age
    a age_of_snake = *(int*)(snake.get("age")); //retrieve age with a string
    printf("Age of parent is: %d \n", age_of_snake); //don't call free on this
    
    a segments = 32.5;
    snake.addProperty("seg", segments);
    a number_of_segments = *(double*)(snake.get("seg")); //retrieve segments
    cout << "Number of segments in snake: " << number_of_segments << endl;
    
    a number_of_segments_in_baby = *(double*)(baby_snake.get("seg"));
    cout << "Child inherits segment variable: " << number_of_segments_in_baby << endl;
    
    static struct New_Function : public Function { //Create a new function closure
        long long value = 123ll;
        static int * execute(int aa, int bb, int dd){
            cout << "Call return5 function to get: " << return5() << endl;
            cout << "do subtract" <<endl; 
            return new int(aa-bb-dd);
        }
        
        New_Function() {
            cout << "Function type value is: " << this->type << endl;
            cout << "Function parent's type value is: " << Function::type << endl;
            Function::execute = (pointer_cast)this->execute;
        }
        static int return5() {return 5;}
    } new_action;
    
    snake.addFunction("measure", new_action); //"measure" will call execute
    a * subtraction = (int*)(snake.Do("measure"))(10,2,1);
    printf("10 - 2 - 1 = %d", *subtraction);
    delete subtraction;
    
    return 0;
}

