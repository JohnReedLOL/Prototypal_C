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

#include <memory>

// Function pointer cast. 
// Produces a function that takes in an arbitrary # of args and returns a shared pointer
typedef std::shared_ptr<void> (*pcast) (...);

// type is stored as a 64 bit value at the head of an object

long long getType(void * head) {
    return (long long) head;
}
// Function is a container for a re-assignable function pointer
struct Function;
struct Function {
    // type is used to get the type of an object via the getType global function.
    static const long long type = ACTION;
    
protected:
    Function * my_parent;

private:
    std::unordered_map<std::string, std::shared_ptr<void> > Variables_Table;

    // execute is a re-assignable general purpose function pointer. 
    // Takes in an arbitrary # of args and returns a void *
public:
    pcast execute;

    Function() {
        this->execute = nullptr; //apparently you can't set a variadic funtion pointer to zero.
        this->my_parent = nullptr;
        Variables_Table = {};
    }

    void operator=(Function& other) // copy assignment
    {
        this->execute = other.execute;
        this->Variables_Table = other.Variables_Table;
    }

    pcast &operator()() {
        return this->execute;
    }

    // Add a copy of a single object property to the properties hash table.

    template <class Type> void addVariable(const std::string property_name, const Type &property_value) {
        Variables_Table[property_name] = std::shared_ptr<void>(new Type(property_value));
    }
protected:
    std::shared_ptr<void> get(const std::string &toGet) {

        //If the element exists, get it.
        try {
            return this->Variables_Table.at(toGet);
        }//Else check to see if the my_parent has it
        catch (const std::out_of_range& oor) {

            if (this->my_parent != nullptr) {
                return this->my_parent->get(toGet);
            } else {
                throw std::out_of_range("Member not found.");
                std::shared_ptr<void> ERROR_POINTER(nullptr);
                return ERROR_POINTER;
            }
        }
    }
};
//Thing is a container of local variables and actions.

class Thing {
    friend class Function; //Functions with pointers to thing can access thing.
public:
    const long long type = THING;

private:
    std::unordered_map<std::string, std::shared_ptr<void> > Properties_Table;

private:
    std::unordered_map<std::string, Function > Functions_Table;

public:
    Thing * my_parent;

    Thing() {
        Properties_Table = {};
        Functions_Table = {};
    }

    Thing(Thing &new_parent) {
        Properties_Table = {};
        Functions_Table = {};
        this->my_parent = &new_parent;
    }

    // Add a copy of a single object property to the properties hash table.

    template <class Type> void addProperty(const std::string property_name, const Type &property_value) {
        Properties_Table[property_name] = std::shared_ptr<void>(new Type(property_value));
    }

    // Add a single action to the actions hash table.

    void addFunction(std::string action_name, Function action_value) {
        Functions_Table[action_name] = action_value;
    }

    std::shared_ptr<void> get(const std::string &toGet) {

        //If the element exists, get it.
        try {
            return Properties_Table.at(toGet);
        }//Else check to see if the my_parent has it
        catch (const std::out_of_range& oor) {

            if (this->my_parent != nullptr) {
                return this->my_parent->get(toGet);
            } else {
                throw std::out_of_range("Member not found.");
                std::shared_ptr<void> ERROR_POINTER(nullptr);
                return ERROR_POINTER;
            }
        }
    }

    // Provides the pointer to call a function
    // Function_name is the name as a std::string. 

    template<class Type> pcast Do(const Type function_name) {

        pcast return_value = nullptr;

        //Calls the corresponding function in the hash table of function objects.
        try {
            return_value = (Functions_Table.at((std::string)function_name)).execute;
        }//C++ map throws an exception if function not found.
        catch (const std::out_of_range& oor) {
            //Check my_parent.
            if (this->my_parent != nullptr)
                return_value = this->my_parent->Do(function_name);
                //Give up.
            else
                throw std::out_of_range("Function cound not be found.");
        }

        return return_value;
    }

    // Assigns a function pointer to an action in the table of actions.

    template< class Type> void set(const std::string function_name, const Type newFunction) {

        (Functions_Table.at(function_name)).execute = (pcast) newFunction;
    }

};

#define a auto
#define pbool (std::static_pointer_cast<bool>)
#define pint (std::static_pointer_cast<int>)
#define pdouble (std::static_pointer_cast<double>)
#define pstring (std::static_pointer_cast<std::string>)
#define pThing (std::static_pointer_cast<Thing>)

std::shared_ptr<int> add(int aa, int bb) {
    std::shared_ptr<int> return_pointer(new int(aa + bb));
    return return_pointer;
}

int main() {
    Thing snake; //Create a snake
    Function doMath; //add is the add function
    doMath() = (pcast) add; //Make doMath execute function add
    snake.addFunction("add", doMath);
    a distance = pint((snake.Do("add"))(5, 6)); //Tell snake to call doMath
    printf("Parent: 5 + 6 = %d \n", *distance); //doMath executes add

    Thing baby_snake(snake); //Create a baby whose parent is snake
    a baby_distance = pint((baby_snake.Do("add"))(1, 2)); //Call parent function
    printf("Child: 1 + 2 = %d \n", *baby_distance);

    a rr = 444;
    snake.addProperty("age", rr); //Give snake an age
    a age_of_snake = pint(snake.get("age")); //retrieve age with a string
    printf("Age of parent is: %d \n", *age_of_snake); //don't call free on this

    a segments = 32.5;
    snake.addProperty("seg", segments);
    a number_of_segments = pdouble(snake.get("seg")); //retrieve segments
    cout << "Number of segments in snake: " << *number_of_segments << endl;

    a number_of_segments_in_baby = pdouble(baby_snake.get("seg"));
    cout << "Child inherits segment variable: " << *number_of_segments_in_baby << endl;

    static struct New_Function : public Function { //Create a new function closure

        static std::shared_ptr<int> execute(int aa, int bb, int dd) {
            cout << "Call return5 function to get: " << return5() << endl;
            cout << "do subtract" << endl;
            std::shared_ptr<int> ret(new int(aa - bb - dd));
            return ret;
        }

        New_Function() {
            cout << "Function type value is: " << this->type << endl;
            cout << "Function parent's type value is: " << Function::type << endl;
            Function::execute = (pcast)this->execute;
            this->addVariable("var1",1);
            int var1 = *pint(this->get("var1"));
            cout << var1 << endl;
        }

        static int return5() {
            return 5;
        }
    } new_action;

    snake.addFunction("measure", new_action); //"measure" will call execute
    a subtraction = pint((snake.Do("measure"))(10, 2, 1));
    printf("10 - 2 - 1 = %d", *subtraction);

    return 0;
}


