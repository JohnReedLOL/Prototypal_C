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

using namespace std;

// 64 bit integers used for type identification
#define ____OBJECT_TYPE -1111111111111111111LL

#include <iostream>
#include <unordered_map>

// out_of_range exceptions are thrown by unordered_map
#include <exception>

#include <memory>

// Function pointer cast produces a function that takes in an arbitrary # of args and returns a void pointer.
typedef void * (*pcast) (...);

struct Object;

/*
 * Checks to see if a value is an object
 * @param value - address of value whose type is being checked
 * @return bool - true if return type is Object
 */
template <class T> bool isObject(T * value) {
    return *((long long *) value) == ____OBJECT_TYPE;
}

/*
 * Dynamic object which is capable of adding static function pointers and values to itself.
 */
struct Object {
    
    // type is stored as a 64 bit value at the head of an object
    const long long my_type = ____OBJECT_TYPE;

    // contents table stores persistent variables and other objects
    std::unordered_map<std::string, void * > my_contents;

    // re-assignable pointer to parent of this object.
    Object * my_parent;

    // re-assignable function pointer - set with Object::setFunc and called with Object::call<Return_Type>
    pcast execute_me;

    // Default constructor
    Object() {
        my_parent = nullptr;
        my_contents = {};
    }

    /*
     * Sets function pointer execute_me to the address of a static function.
     * @param function_pointer - a generic function pointer
     */
    template <class Type> void setFunc (Type function_pointer) {
        this->execute_me = (pcast) function_pointer;
    }

    // Add a copy of a single object property to the properties hash table.

    /*
     * Add a single object property to the properties hash table with key string::name and generic value
     * @param name - name that will be used to retrieve value
     * @param value - a generic value to be added
     */
    template <class Type> void set(const std::string name, const Type &value) {
        Type * toSet = new Type;
        *toSet = value;
        my_contents[name] = (void *) (toSet);
    }

    /*
     * Checks to see if this object has a variable with name value equal to std::string name
     * Throws out_of_range exception when name cannot be found
     * @param name - name of the variable that we are searching for
     * @return true if element of name name can be found in this object or an object somewhere in its parent tree.
     */
    bool has(const std::string &name) {

        //If the element exists, get it.
        try {
            ((this->my_contents.at(name)));
            return true;
        }//Else check to see if the my_parent has it
        catch (const std::out_of_range& oor) {

            if (this->my_parent != nullptr) {
                return this->my_parent->has(name);
            } else {
                return false;
            }
        }
    }

    /*
     * Retrieves an element from this object with non-void return type
     * Throws out_of_range exception when name cannot be found
     * @param name - string name of the variable that we are searching for
     * @return Return_Type - return type which must be specified in angle brackets
     */
    template <class Return_Type> Return_Type get(const std::string &name) {

        //If the element exists, get it.
        try {
            return * ((Return_Type *) (this->my_contents.at(name)));
        }//Else check to see if the my_parent has it
        catch (const std::out_of_range& oor) {

            if (this->my_parent != nullptr) {
                return this->my_parent->get<Return_Type>(name);
            } else {
                throw std::out_of_range("Member not found.");
                Return_Type ERROR;
                return ERROR;
            }
        }
    }

    /*
     * Directly calls the generic function pointer execute_me contained within this object if the return type is void
     * Throws bad function call if pointer is null
     * @param Parameters - generic list of function parameters
     */
    template <class ...A> void call(A... Parameters) {
        if(this->execute_me != nullptr) 
        {
                (this->execute_me(Parameters...)); //ignore return value of execute_me
                return;
        }
        else if(this->my_parent != nullptr)
        {
            this->my_parent->call(Parameters...);
            return;
        }
        else {
            throw std::bad_function_call("Function pointer never set.");
        }
    }
    
     /*
      * Directly calls the generic function pointer execute_me contained within this object if the return type is non-void
      * Throws bad function call if pointer is null
      * @param Parameters - generic list of generic list of comma delimited function parameters
      * @return Return_Type - generic return type - must be specified in <>
      */
    template <class Return_Type, class ...A> Return_Type call(A... Parameters) {
        if(this->execute_me != nullptr)
        {
                return * (Return_Type *) (this->execute_me(Parameters...));
        }
        else if (this->my_parent != nullptr)
        {
            return this->my_parent->call<Return_Type>(Parameters...);
        }
        else
        {
            throw std::bad_function_call("Function pointer never set.");
        }
    }

    /*
     * Executes a function with no return type by name std::string function_name 
     * @param function_name - the name of the function
     * @param Parameters - generic list of function parameters
     */
    template<class ...A> void Do(const std::string function_name, A... Parameters) {

        //pcast return_value = nullptr;

        //Calls the corresponding function in the hash table of function objects.
        try {
            ((Object *) (my_contents.at((std::string) function_name)))->call(Parameters...);
            return;
        }//C++ map throws an exception if function not found.
        catch (const std::out_of_range& oor) {

            //Check my_parent.
            if (this->my_parent != nullptr) {
                this->my_parent->Do(function_name, Parameters...);
                return;
            }                //Give up.
            else {
                throw std::out_of_range("Function cound not be found.");
                return;
            }
        }
    }

    /*
     * Executes a value returning function by name std::string function_name 
     * @param function_name - the name of the function
     * @param Parameters - generic list of function parameters
     * @return Return_Type - generic return type - must be specified in <>
     */
    template<class Return_Type, class ...A> Return_Type Do(const std::string function_name, A... Parameters) {

        //pcast return_value = nullptr;

        //Calls the corresponding function in the hash table of function objects.
        try {
            // Return_Type = int, Type = const char *, parameters = 1,2 
            return (my_contents.at((std::string) function_name)).call<Return_Type>(Parameters...);
        }//C++ map throws an exception if function not found.
        catch (const std::out_of_range& oor) {
            //Check my_parent.
            cout << "Value not found" << endl;
            if (this->my_parent != nullptr)
                return this->my_parent->Do<Return_Type>(function_name, Parameters...);
                //Give up.
            else {
                throw std::out_of_range("Function cound not be found.");
                Return_Type r;
                return r;
            }
        }
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

    return 0;
}
