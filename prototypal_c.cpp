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

#include <iostream>
#include <unordered_map>

// out_of_range exceptions are thrown by unordered_map
#include <exception>

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
    
    template <class Type> void addFunc(Type function_pointer) {
        this->execute = (pcast)function_pointer;
    }

    //Note: Function do returns shared pointer.
    template <class Return_Type, class ...A> Return_Type Do(A... Parameters) 
    {
        return * (std::static_pointer_cast<Return_Type>)(this->execute(Parameters...));
    }

    // Add a copy of a single object property to the properties hash table.
    template <class Type> void addVar(const std::string property_name, const Type &property_value) {
        //auto thing1 = std::shared_ptr<void>(new Type(property_value));
        //auto thing2 = std::make_shared<void>(property_value); - Can't do this
        Variables_Table[property_name] = std::shared_ptr<void>(new Type(property_value));
        //Variables_Table[property_name] = std::make_shared<void>(property_value)); - This doesn't work
    }
public:

    template <class Return_Type> Return_Type get(const std::string &toGet) {

        //If the element exists, get it.
        try {
            return * ((std::static_pointer_cast<Return_Type>)(this->Variables_Table.at(toGet)));
        }//Else check to see if the my_parent has it
        catch (const std::out_of_range& oor) {

            if (this->my_parent != nullptr) {
                return this->my_parent->get<Return_Type>(toGet);
            } else {
                throw std::out_of_range("Member not found.");
                Return_Type ERROR;
                return ERROR;
            }
        }
    }
};
//Thing is a container of local variables and actions.

class Object {
    friend class Function; //Functions with pointers to thing can access thing.
public:
    const long long type = THING;

private:
    std::unordered_map<std::string, std::shared_ptr<void> > Variables_Table;

private:
    std::unordered_map<std::string, Function > Functions_Table;

public:
    Object * my_parent;

    Object() {
        Variables_Table = {};
        Functions_Table = {};
    }

    Object(Object &new_parent) {
        Variables_Table = {};
        Functions_Table = {};
        this->my_parent = &new_parent;
    }

    // Add a copy of a single object property to the properties hash table.

    template <class Type> void addVar(const std::string property_name, const Type &property_value) {
        Variables_Table[property_name] = std::shared_ptr<void>(new Type(property_value));
    }

    // Add a single action to the actions hash table.

    void addFunc(std::string action_name, Function action_value) {
        Functions_Table[action_name] = action_value;
    }

    template <class Return_Type> Return_Type get(const std::string &toGet) {

        //If the element exists, get it.
        try {
            return * ((std::static_pointer_cast<Return_Type>)(this->Variables_Table.at(toGet)));
        }//Else check to see if the my_parent has it
        catch (const std::out_of_range& oor) {

            if (this->my_parent != nullptr) {
                return this->my_parent->get<Return_Type>(toGet);
            } else {
                throw std::out_of_range("Member not found.");
                Return_Type ERROR;
                return ERROR;
            }
        }
    }

    // Provides the pointer to call a function
    // Function_name is the name as a std::string. 

    //template <class Return_Type, class ...A> Return_Type operator()(A... Parameters) 
    template<class Return_Type, class ...A> Return_Type Do(const std::string function_name, A... Parameters) {

        //pcast return_value = nullptr;

        //Calls the corresponding function in the hash table of function objects.
        try {
            // Return_Type = int, Type = const char *, parameters = 1,2 
            return (Functions_Table.at((std::string) function_name)).Do<Return_Type>(Parameters... );
        }//C++ map throws an exception if function not found.
        catch (const std::out_of_range& oor) {
            //Check my_parent.
            cout << "Value not found" << endl;
            if (this->my_parent != nullptr)
                return this->my_parent->Do<Return_Type>(function_name, Parameters...);
                //Give up.
            else
            {
                throw std::out_of_range("Function cound not be found.");
                Return_Type r;
                return r;
            }
        }
    }

    // Assigns a function pointer to an action in the table of actions.

    template< class Type> void set(const std::string function_name, const Type newFunction) {

        (Functions_Table.at(function_name)).execute = (pcast) newFunction;
    }

};

//#define a auto
#define s std::string

std::shared_ptr<int> add(int aa, int bb) {
    //std::shared_ptr<int> return_pointer(new int(aa + bb));
    auto return_pointer = std::make_shared<int>(aa + bb);
    return return_pointer;
}


int main() {

    Function adder;
    adder.addVar("var", 5);
    
    cout << adder.get<int>("var") << endl;
    
    adder.addFunc(add);
    int n = adder.Do<int>(3,4);
    cout << n << endl;
    
    Object obj;
    obj.addVar("var", 9);
    cout << obj.get<int>("var") << endl;
    
    obj.addFunc("add", adder);
    cout << obj.Do<int>("add",1,2) << endl;
    
    return 0;
}
